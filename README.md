# Práctica 1 — Problema de Portfolio
## Metaheurísticas · Curso 2025-26 · UGR ETSIIT

**Autor:** Jose Angel Carretero Montes
**Grupo:** 3

---

## Descripción

Implementación de tres algoritmos de búsqueda para el Problema de Portfolio:
asignación óptima de capital entre activos financieros maximizando retorno
esperado y minimizando riesgo (λ=500).

- **Greedy**: construcción determinista por heurística beneficio-covarianza.
- **Random Search**: búsqueda aleatoria de referencia (10 000 soluciones).
- **Búsqueda Local (BL)**: primero-mejor sobre vecindario de pares (ratio=0.4).

---

## Estructura del proyecto

```
portfolio/
├── main.cpp                   # Punto de entrada; ejecuta los 3 algoritmos × 3 datasets × 50 semillas
├── CMakeLists.txt             # Sistema de construcción (CMake ≥ 4.0)
├── boxplot.py                 # Genera boxplots desde los CSV de resultados
├── README.md                  # Este fichero
├── common/
│   ├── mh.h                   # Interfaz genérica MH<tDomain>
│   ├── problem.h              # Interfaz genérica Problem<tDomain>
│   ├── solution.h             # Tipos tSolution<T> y tFitness
│   ├── random.hpp             # Generador pseudoaleatorio de alta calidad
│   └── util.h                 # Utilidades auxiliares
├── inc/
│   ├── portfolioProblem.h     # Declaración de PortfolioProblem
│   ├── greedy.h               # Declaración de Greedy<T>
│   ├── localsearch.h          # Declaración de LocalSearch<T>
│   └── randomSearch.h         # Declaración de RandomSearch<T>
├── src/
│   ├── portfolioProblem.cpp   # Carga CSV, precómputo de matrices, fitness, fix, createSolution
│   ├── greedy.cpp             # Implementación del greedy
│   ├── localsearch.cpp        # Implementación de la BL
│   └── randomSearch.cpp       # Implementación de la búsqueda aleatoria
├── test/
│   ├── portfolio_test.cpp     # Tests unitarios (solución, fix, fitness)
│   └── portfolioProblem_test.cpp  # Tests de integración del problema
└── data/
    ├── ibex_35.csv            # 30 empresas IBEX 35, 2015-2025
    ├── syp_100.csv            # 97 empresas S&P 100, 2015-2025
    └── syp_500.csv            # 457 empresas S&P 500, 2015-2025
```

---

## Compilación

Desde el directorio raíz del proyecto:

```bash
cmake .
make
```

Esto genera el ejecutable `main` y el ejecutable de tests `all_tests`.

Para ejecutar los tests unitarios:

```bash
./all_tests
```

---

## Ejecución

El ejecutable no requiere argumentos. Lee automáticamente los ficheros de
`data/` y ejecuta los tres algoritmos sobre los tres conjuntos de datos con
50 semillas cada uno:

```bash
./main
```

Salida de ejemplo (extracto):

```
========================================
Dataset: ibex_35
========================================
N=30  T_train=2559  MAX_EVALS=10000
Ejecutando Random... OK
Ejecutando Greedy... OK
Ejecutando BL... OK

=== Tabla de resultados: ibex_35 (medias, 50 semillas) ===
Algoritmo    Fit.Train    Fit.2025    Ben.2025  Evaluaciones Tiempo(s)
----------------------------------------------------------------------
greedy         -0.2128      0.0907      0.1152           1.0     0.000
random         -0.5833      0.0910      0.1180       10000.0     0.283
bl             -0.2128      0.0907      0.1152        1852.2     0.034
```

Al terminar se generan en el directorio actual:

- `results_ibex_35.csv`
- `results_syp_100.csv`
- `results_syp_500.csv`

---

## Boxplots

Requiere Python 3 con `pandas` y `seaborn`:

```bash
pip install pandas seaborn matplotlib   # solo la primera vez
python3 boxplot.py
```

Genera `results_ibex_35.png`, `results_syp_100.png`, `results_syp_500.png`.

---

## Parámetros

Los parámetros se configuran directamente en `main.cpp`:

| Parámetro   | Valor por defecto | Descripción                        |
|-------------|-------------------|------------------------------------|
| `N_SEEDS`   | 50                | Número de semillas (1, 2, …, 50)   |
| `MAX_EVALS` | 10 000            | Presupuesto de evaluaciones de fitness |

Las semillas son las mismas para todos los algoritmos y datasets, garantizando
comparaciones justas.
