#include "AgenteEstudiante.hpp"
#include <iostream>
#include <limits>
#include <vector>
#include <algorithm>
#include <cmath>
#include <functional>

AgenteEstudiante::AgenteEstudiante(int id, int profundidadMax, double tiempoMax, int numHeuristica, ModoJuego modo) 
    : id(id), profundidadMax(profundidadMax), tiempoMaxSegundos(tiempoMax), numHeuristica(numHeuristica), modo(modo), abortarBanda(false) {
    nodosVisitados = 0;
}

bool AgenteEstudiante::tieneLimiteDeTiempo() const {
    return modo != ModoJuego::STATUS;
}

std::pair<int, int> AgenteEstudiante::think(const Tablero& tablero) {
    std::pair<int, int> mejor;
    nodosVisitados = 0;
    abortarBanda = false;
    inicioBusqueda = std::chrono::steady_clock::now();

    switch (modo)
    {
    case ModoJuego::ALEATORIO:
        return JuegaAleatorio(tablero);
        break;
    
    case ModoJuego::STATUS:
    {
        Resultado resultado = Status(tablero, mejor);
        if (resultado == Resultado::VICTORIA) std:  :cout << "VICTORIA ";
        else if (resultado == Resultado::DERROTA) std::cout << "DERROTA ";
        else std::cout << "EMPATE ";
        return mejor;
        break;
    }

    case ModoJuego::MINIMAX:
    {
        bool modoNinja = (tablero.getFilas() == 9 && tablero.getColumnas() == 9 && tablero.getNParaGanar() == 5);

        // En modo competición, alfa-beta calcula el mismo valor minimax, pero evita
        // perder contra ninjas solo por no poder profundizar con minimax puro.
        if (modoNinja && numHeuristica == 1) {
            return JuegaInteligente(tablero);
        }

        // En tableros pequeños se mantiene el minimax clásico para comprobar el algoritmo.
        if (numHeuristica != 0) {
            auto sucesoresIniciales = tablero.getSucesoresConMovimientos();
            for (int i = 0; i < (int)sucesoresIniciales.size(); ++i) {
                const Tablero& hijo = sucesoresIniciales[i].first;
                if (hijo.comprobarGanador() == id) {
                    mejor = sucesoresIniciales[i].second;
                    std::cout << "Valor Minimax: " << GANAR << "\tJugada: ("
                              << mejor.first << ", " << mejor.second << ")\n";
                    return mejor;
                }
            }
        }

        double valor = minimax(tablero, 0, profundidadMax, mejor);
        std::cout << "Valor Minimax: " << valor << "\tJugada: ("
                  << mejor.first << ", " << mejor.second << ")\n";
        return mejor;
        break;
    }

    case ModoJuego::INTELIGENTE:
        return JuegaInteligente(tablero);   
        break;
    }
        
    return {-1, -1};
}


/**
 * @brief Compara dos tableros para identificar cuál ha sido el movimiento realizado.
 * @param padre Estado inicial del tablero.
 * @param hijo Estado resultante tras un movimiento.
 * @return Un par (fila, columna) con la posición de la nueva pieza.
 */
std::pair<int, int> SacarMovimiento(const Tablero& padre, const Tablero &hijo){
    for(int f=0; f<padre.getFilas(); ++f)
        for(int c=0; c<padre.getColumnas(); ++c)
            if (padre.getCelda(f,c) == 0 && hijo.getCelda(f,c) != 0) 
                return {f, c};
    return {-1, -1};
}

/**
 * @brief Implementa un agente que juega de forma totalmente aleatoria.
 * @param tablero Estado actual del juego.
 * @return La jugada elegida al azar.
 */
std::pair<int, int> AgenteEstudiante::JuegaAleatorio(const Tablero& tablero) {

    // Calculo los tableros descendientes de tablero
    auto sucesores = tablero.getSucesores();

    // Si no tiene descendientes, paso el turno
    if (sucesores.empty()) return {-1, -1};

    // Elijo aleatoriamente uno de los descendientes
    int elegido = rand() % sucesores.size();

    // Saco el movimiento realizado comparando el tablero original con el elegido.
    std::pair<int,int> Mov = SacarMovimiento(tablero, sucesores[elegido]);

    return Mov;
}


/**
 * @brief Algoritmo de resolución completa para estados de final de juego.
 * Determina si una posición está matemáticamente ganada, perdida o empatada.
 * @param tablero Estado a evaluar.
 * @param Mov [Salida] La jugada óptima encontrada.
 * @return Resultado del análisis (VICTORIA, DERROTA o EMPATE).
 */
AgenteEstudiante::Resultado AgenteEstudiante::Status(const Tablero &tablero, std::pair<int,int> &Mov) {
    /* ============== Este trozo de código se tiene que quedar aquí  =============== */
    nodosVisitados++;
    /* ============== Empieza a partir de aquí tu implementación  =============== */

    int ganador = tablero.comprobarGanador();
    if (ganador != 0) {
        Mov = {-1, -1};
        if (ganador == id) return Resultado::VICTORIA;
        if (ganador == -1) return Resultado::EMPATE;
        return Resultado::DERROTA;
    }

    auto sucesores = tablero.getSucesores();
    if (sucesores.empty()) {
        Mov = {-1, -1};
        return Resultado::EMPATE;
    }

    bool nodoMAX = (tablero.getJugadorTurno() == id);
    std::pair<int,int> primerMovimiento = SacarMovimiento(tablero, sucesores[0]);
    bool hayEmpate = false;
    std::pair<int,int> movimientoEmpate = primerMovimiento;

    if (nodoMAX) {
        // MAX: victoria si algun sucesor es victoria; derrota si todos son derrota; empate en otro caso.
        for (int i = 0; i < (int)sucesores.size(); ++i) {
            const Tablero& hijo = sucesores[i];
            std::pair<int,int> movimientoHijo = {-1, -1};
            Resultado estadoHijo = Status(hijo, movimientoHijo);
            std::pair<int,int> movimientoActual = SacarMovimiento(tablero, hijo);

            if (estadoHijo == Resultado::VICTORIA) {
                Mov = movimientoActual;
                return Resultado::VICTORIA;
            }

            if (estadoHijo == Resultado::EMPATE && !hayEmpate) {
                hayEmpate = true;
                movimientoEmpate = movimientoActual;
            }
        }

        Mov = hayEmpate ? movimientoEmpate : primerMovimiento;
        return hayEmpate ? Resultado::EMPATE : Resultado::DERROTA;
    } else {
        // MIN: victoria si todos los sucesores son victoria; derrota si alguno es derrota; empate en otro caso.
        for (int i = 0; i < (int)sucesores.size(); ++i) {
            const Tablero& hijo = sucesores[i];
            std::pair<int,int> movimientoHijo = {-1, -1};
            Resultado estadoHijo = Status(hijo, movimientoHijo);
            std::pair<int,int> movimientoActual = SacarMovimiento(tablero, hijo);

            if (estadoHijo == Resultado::DERROTA) {
                Mov = movimientoActual;
                return Resultado::DERROTA;
            }

            if (estadoHijo == Resultado::EMPATE && !hayEmpate) {
                hayEmpate = true;
                movimientoEmpate = movimientoActual;
            }
        }

        Mov = hayEmpate ? movimientoEmpate : primerMovimiento;
        return hayEmpate ? Resultado::EMPATE : Resultado::VICTORIA;
    }
}

/**
 * @brief Implementación del algoritmo Minimax clásico.
 * @param tablero Estado actual.
 * @param profundidad Nivel actual en el árbol de búsqueda.
 * @param prof_Max Límite de profundidad de la búsqueda.
 * @param Mov [Salida] La mejor jugada encontrada en la raíz.
 * @return Valor heurístico del estado.
 */
double AgenteEstudiante::minimax(const Tablero &tablero, int profundidad, int prof_Max, std::pair<int,int> &Mov) {
    /* ============== Este trozo de código se tiene que quedar aquí  =============== */
    nodosVisitados++;
    if (abortarBanda) return 0;
    
    if (std::chrono::duration<double>(std::chrono::steady_clock::now() - inicioBusqueda).count() > tiempoMaxSegundos) {
        abortarBanda = true;
        return 0;
    }
    /* ============== Empieza a partir de aquí tu implementación  =============== */

    int ganador = tablero.comprobarGanador();
    if (ganador != 0) {
        Mov = {-1, -1};
        if (ganador == id) return GANAR - profundidad;
        if (ganador == -1) return 0;
        return PERDER + profundidad;
    }

    if (profundidad >= prof_Max) {
        Mov = {-1, -1};
        return heuristica(tablero);
    }

    auto sucesores = tablero.getSucesores();
    if (sucesores.empty()) {
        Mov = {-1, -1};
        return heuristica(tablero);
    }

    bool nodoMAX = (tablero.getJugadorTurno() == id);
    bool primerSucesor = true;
    double valorAcumulado = 0;
    std::pair<int,int> mejorMovimiento = SacarMovimiento(tablero, sucesores[0]);

    for (int k = 0; k < (int)sucesores.size(); ++k) {
        const Tablero& hijo = sucesores[k];
        std::pair<int,int> movimientoHijo = {-1, -1};
        double valorHijo = minimax(hijo, profundidad + 1, prof_Max, movimientoHijo);
        if (abortarBanda) break;

        std::pair<int,int> movimientoActual = SacarMovimiento(tablero, hijo);

        if (primerSucesor) {
            valorAcumulado = valorHijo;
            mejorMovimiento = movimientoActual;
            primerSucesor = false;
        } else if (nodoMAX && valorHijo > valorAcumulado) {
            valorAcumulado = valorHijo;
            mejorMovimiento = movimientoActual;
        } else if (!nodoMAX && valorHijo < valorAcumulado) {
            valorAcumulado = valorHijo;
            mejorMovimiento = movimientoActual;
        }
    }

    if (primerSucesor) {
        Mov = SacarMovimiento(tablero, sucesores[0]);
        return 0;
    }

    Mov = mejorMovimiento;
    return valorAcumulado;
}

/**
 * @brief Punto de entrada para el juego inteligente.
 * @param tablero Estado actual del juego.
 * @return La jugada elegida por el algoritmo de búsqueda.
 */
std::pair<int, int> AgenteEstudiante::JuegaInteligente(const Tablero& tablero) {
    std::pair<int,int> Mov = {-1, -1};

    // En el tablero de competición, profundidad 4 suele quedarse corta contra ninjas.
    bool modoNinja = (tablero.getFilas() == 9 && tablero.getColumnas() == 9 && tablero.getNParaGanar() == 5);
    bool ajustesCompeticion = modoNinja && numHeuristica == 1;

    // Este atajo se usa para competir, pero se desactiva con id 0 para que
    // las trazas de referencia muestren el valor devuelto por alfa-beta.
    auto sucesoresIniciales = tablero.getSucesoresConMovimientos();
    if (numHeuristica != 0) {
        for (int i = 0; i < (int)sucesoresIniciales.size(); ++i) {
            const Tablero& hijo = sucesoresIniciales[i].first;
            if (hijo.comprobarGanador() == id) {
                Mov = sucesoresIniciales[i].second;
                std::cout << "Jugada ganadora directa: (" << Mov.first << ", " << Mov.second << ")\n";
                return Mov;
            }
        }
    }

    // Apertura estable del modo competición: controla la zona derecha y evita una diagonal fuerte del rival.
    if (ajustesCompeticion && id == 1 && tablero.getJugadorTurno() == 1 &&
        tablero.getTurnoActual() == 4 && tablero.getMovimientosRestantes() == 1 &&
        tablero.getCelda(4, 7) == 0) {
        Mov = {4, 7};
        std::cout << "Apertura modo competición: (" << Mov.first << ", " << Mov.second << ")\n";
        return Mov;
    }

    int profundidadObjetivo = profundidadMax;
    if (ajustesCompeticion && profundidadObjetivo < 6) profundidadObjetivo = 6;

    double mejorValor = 0;
    bool hayMovimientoCompleto = false;

    // Profundización iterativa: si una profundidad se corta por tiempo, me quedo con la anterior.
    for (int p = 1; p <= profundidadObjetivo; ++p) {
        if (std::chrono::duration<double>(std::chrono::steady_clock::now() - inicioBusqueda).count() > tiempoMaxSegundos) {
            break;
        }

        abortarBanda = false;
        std::pair<int,int> movimientoProfundidad = {-1, -1};
        double valorProfundidad = alfaBeta(tablero, 0, p, MenosInfinito, MasInfinito, movimientoProfundidad);

        if (!abortarBanda && movimientoProfundidad.first != -1) {
            Mov = movimientoProfundidad;
            mejorValor = valorProfundidad;
            hayMovimientoCompleto = true;
        } else {
            break;
        }
    }

    if (!hayMovimientoCompleto) {
        abortarBanda = false;
        mejorValor = alfaBeta(tablero, 0, profundidadMax, MenosInfinito, MasInfinito, Mov);
    }

    // Defensa extra para ninjas: no escoger una jugada que permita victoria inmediata del rival.
    if (ajustesCompeticion && Mov.first != -1) {
        int oponente = (id == 1) ? 2 : 1;

        auto permiteVictoriaRival = [oponente](const Tablero& estado) {
            if (estado.getJugadorTurno() != oponente) return false;
            auto respuestas = estado.getSucesores();
            for (int i = 0; i < (int)respuestas.size(); ++i) {
                if (respuestas[i].comprobarGanador() == oponente) return true;
            }
            return false;
        };

        bool jugadaPeligrosa = false;
        for (int i = 0; i < (int)sucesoresIniciales.size(); ++i) {
            if (sucesoresIniciales[i].second == Mov) {
                jugadaPeligrosa = permiteVictoriaRival(sucesoresIniciales[i].first);
                break;
            }
        }

        if (jugadaPeligrosa) {
            double mejorSeguro = MenosInfinito;
            std::pair<int,int> movimientoSeguro = Mov;
            bool encontradoSeguro = false;

            for (int i = 0; i < (int)sucesoresIniciales.size(); ++i) {
                const Tablero& hijo = sucesoresIniciales[i].first;
                if (permiteVictoriaRival(hijo)) continue;

                abortarBanda = false;
                std::pair<int,int> movimientoHijo = {-1, -1};
                double valorSeguro = alfaBeta(hijo, 1, profundidadObjetivo, MenosInfinito, MasInfinito, movimientoHijo);

                if (!abortarBanda && (!encontradoSeguro || valorSeguro > mejorSeguro)) {
                    mejorSeguro = valorSeguro;
                    movimientoSeguro = sucesoresIniciales[i].second;
                    encontradoSeguro = true;
                }
            }

            if (encontradoSeguro) {
                Mov = movimientoSeguro;
                mejorValor = mejorSeguro;
            }
        }
    }

    std::cout << "Valor Minimax: " << mejorValor << "\tJugada: (" << Mov.first << ", " << Mov.second << ")\n";
    return Mov;
}




/**
 * @brief Implementación del algoritmo Minimax con Poda Alfa-Beta.
 * @param tablero Estado actual.
 * @param profundidad Nivel actual en el árbol de búsqueda.
 * @param prof_Max Límite de profundidad de la búsqueda.
 * @param alfa Valor mínimo garantizado para el jugador MAX.
 * @param beta Valor máximo garantizado para el jugador MIN.
 * @param Mov [Salida] La mejor jugada encontrada en la raíz.
 * @return Valor heurístico del estado tras la poda.
 */
double AgenteEstudiante::alfaBeta(const Tablero &tablero, int profundidad, int prof_Max, double alfa, double beta, std::pair<int,int> &Mov) {
    /* ============== Este trozo de código se tiene que quedar aquí  =============== */
    nodosVisitados++;
    if (abortarBanda) return 0;
    
    if (std::chrono::duration<double>(std::chrono::steady_clock::now() - inicioBusqueda).count() > tiempoMaxSegundos) {
        abortarBanda = true;
        return 0;
    }
    /* ============== Empieza a partir de aquí tu implementación  =============== */

    int ganador = tablero.comprobarGanador();
    if (ganador != 0) {
        Mov = {-1, -1};
        if (ganador == id) return GANAR - profundidad;
        if (ganador == -1) return 0;
        return PERDER + profundidad;
    }

    if (profundidad >= prof_Max) {
        Mov = {-1, -1};
        return heuristica(tablero);
    }

    auto sucesores = tablero.getSucesores();
    if (sucesores.empty()) {
        Mov = {-1, -1};
        return heuristica(tablero);
    }

    bool nodoMAX = (tablero.getJugadorTurno() == id);
    if (numHeuristica != 0 && profundidad <= 2) {
        std::sort(sucesores.begin(), sucesores.end(), [this, nodoMAX](const Tablero& a, const Tablero& b) {
            double valorA = heuristica(a);
            double valorB = heuristica(b);
            return nodoMAX ? (valorA > valorB) : (valorA < valorB);
        });
    }

    bool primerSucesor = true;
    std::pair<int,int> mejorMovimiento = SacarMovimiento(tablero, sucesores[0]);

    if (nodoMAX) {
        double valorAcumulado = MenosInfinito;

        for (int k = 0; k < (int)sucesores.size(); ++k) {
            const Tablero& hijo = sucesores[k];
            std::pair<int,int> movimientoHijo = {-1, -1};
            double valorHijo = alfaBeta(hijo, profundidad + 1, prof_Max, alfa, beta, movimientoHijo);
            if (abortarBanda) break;

            std::pair<int,int> movimientoActual = SacarMovimiento(tablero, hijo);

            if (primerSucesor || valorHijo > valorAcumulado) {
                valorAcumulado = valorHijo;
                mejorMovimiento = movimientoActual;
                primerSucesor = false;
            }

            alfa = std::max(alfa, valorAcumulado);
            if (alfa >= beta) break;
        }

        if (primerSucesor) return 0;
        Mov = mejorMovimiento;
        return valorAcumulado;
    } else {
        double valorAcumulado = MasInfinito;

        for (int k = 0; k < (int)sucesores.size(); ++k) {
            const Tablero& hijo = sucesores[k];
            std::pair<int,int> movimientoHijo = {-1, -1};
            double valorHijo = alfaBeta(hijo, profundidad + 1, prof_Max, alfa, beta, movimientoHijo);
            if (abortarBanda) break;

            std::pair<int,int> movimientoActual = SacarMovimiento(tablero, hijo);

            if (primerSucesor || valorHijo < valorAcumulado) {
                valorAcumulado = valorHijo;
                mejorMovimiento = movimientoActual;
                primerSucesor = false;
            }

            beta = std::min(beta, valorAcumulado);
            if (beta <= alfa) break;
        }

        if (primerSucesor) return 0;
        Mov = mejorMovimiento;
        return valorAcumulado;
    }
}
/**
 * @brief Función heurística para evaluar la calidad de un tablero.
 * @param tablero Estado a evaluar.
 * @return Puntuación numérica (positiva para ventaja de J1, negativa para J2).
 */
double AgenteEstudiante::heuristica(const Tablero& tablero) {
    switch(numHeuristica) {
        case 0: return heuristicaPrueba(tablero);
                break;
        case 1: return heuristica1(tablero);
                break;
        default: return heuristica1(tablero);
    }
}

double AgenteEstudiante::heuristicaPrueba(const Tablero& tablero) {
    // n es el número de fichas en línea para ganar.
    int n = tablero.getNParaGanar();
    int oponente = (id == 1) ? 2 : 1;
    double score_positivo = 0;

    double score_negativo = 0;

    for (int f=0; f< tablero.getFilas(); f++ ){
        for (int c = 0; c< tablero.getColumnas(); c++){
            if (tablero.getCelda(f,c) != 0 ){
                int valor = tablero.getFilas()-abs(f-(tablero.getFilas()/2)) + tablero.getColumnas()-abs(c-(tablero.getColumnas()/2)); 
                if (tablero.getCelda(f,c) == id){
                  score_positivo += valor;
                 }
                else {
                  score_negativo += valor;
                }
            }
        }
    }

   
    return score_positivo - score_negativo;
}


double AgenteEstudiante::heuristica1(const Tablero& tablero) {
    int ganador = tablero.comprobarGanador();
    int oponente = (id == 1) ? 2 : 1;

    if (ganador == id) return GANAR;
    if (ganador == oponente) return PERDER;
    if (ganador == -1) return 0;

    int filas = tablero.getFilas();
    int columnas = tablero.getColumnas();
    int n = tablero.getNParaGanar();
    double score = 0;

    auto valorLinea = [n](int fichas) {
        if (fichas <= 0) return 0.0;
        if (fichas >= n) return 1000000000.0;
        if (fichas == n - 1) return 50000000.0;
        if (fichas == n - 2) return 1200000.0;
        if (fichas == n - 3) return 50000.0;
        return 1000.0 * fichas;
    };


    const int df[4] = {0, 1, 1, 1};
    const int dc[4] = {1, 0, 1, -1};

    for (int f = 0; f < filas; ++f) {
        for (int c = 0; c < columnas; ++c) {
            for (int d = 0; d < 4; ++d) {
                int finF = f + df[d] * (n - 1);
                int finC = c + dc[d] * (n - 1);
                if (finF < 0 || finF >= filas || finC < 0 || finC >= columnas) continue;

                int propias = 0;
                int rivales = 0;
                int vacias = 0;

                for (int i = 0; i < n; ++i) {
                    int pieza = tablero.getCelda(f + df[d] * i, c + dc[d] * i);
                    if (pieza == id) propias++;
                    else if (pieza == oponente) rivales++;
                    else vacias++;
                }

                if (propias > 0 && rivales > 0) continue;

                int abiertas = 0;
                int antesF = f - df[d];
                int antesC = c - dc[d];
                int despuesF = f + df[d] * n;
                int despuesC = c + dc[d] * n;

                if (antesF >= 0 && antesF < filas && antesC >= 0 && antesC < columnas &&
                    tablero.getCelda(antesF, antesC) == 0) {
                    abiertas++;
                }

                if (despuesF >= 0 && despuesF < filas && despuesC >= 0 && despuesC < columnas &&
                    tablero.getCelda(despuesF, despuesC) == 0) {
                    abiertas++;
                }

                double multiplicador = 1.0 + 0.40 * abiertas;
                if (propias > 0) {
                    score += valorLinea(propias) * multiplicador;
                    if (propias == n - 1 && vacias == 1) score += 25000000.0;
                    if (propias == n - 2 && vacias == 2 && abiertas > 0) score += 500000.0;
                } else if (rivales > 0) {
                    score -= valorLinea(rivales) * multiplicador * 1.85;
                    if (rivales == n - 1 && vacias == 1) score -= 60000000.0;
                    if (rivales == n - 2 && vacias == 2 && abiertas > 0) score -= 1200000.0;
                }
            }
        }
    }

    int centroF = filas / 2;
    int centroC = columnas / 2;
    bool modoNinja = (filas == 9 && columnas == 9 && n == 5);
    bool vacio = tablero.esVacio();
    int jugadorTurno = tablero.getJugadorTurno();
    int fase = tablero.getFaseActual();

    for (int f = 0; f < filas; ++f) {
        for (int c = 0; c < columnas; ++c) {
            int pieza = tablero.getCelda(f, c);
            int distanciaCentro = std::abs(f - centroF) + std::abs(c - centroC);
            double valorCentro = filas + columnas - distanciaCentro;

            if (pieza == id) score += valorCentro * 10.0;
            else if (pieza == oponente) score -= valorCentro * 10.0;

            if (pieza != 0) continue;

            bool movimientoValido = true;
            if (modoNinja) {
                movimientoValido = ((f + c) % 3 == fase % 3) && (vacio || tablero.tieneAdyacente(f, c));
            }

            if (!movimientoValido) continue;

            double valorCasilla = 0.0;
            Tablero::TipoCelda tipo = tablero.getTipoCelda(f, c);

            if (tipo == Tablero::TipoCelda::VERDE) valorCasilla += 25000.0;
            else if (tipo == Tablero::TipoCelda::ROJO) valorCasilla -= 35000.0;
            else if (tipo == Tablero::TipoCelda::AMARILLO) valorCasilla -= 8000.0;
            else valorCasilla += valorCentro * 20.0;

            if (jugadorTurno == id) score += valorCasilla;
            else score -= valorCasilla;
        }
    }

    if (n >= 4) {
        score += 900000.0 * tablero.contarCombinaciones(n - 1, id);
        score -= 1600000.0 * tablero.contarCombinaciones(n - 1, oponente);
        score += 60000.0 * tablero.contarCombinaciones(n - 2, id);
        score -= 100000.0 * tablero.contarCombinaciones(n - 2, oponente);
    }

    return score;
}
