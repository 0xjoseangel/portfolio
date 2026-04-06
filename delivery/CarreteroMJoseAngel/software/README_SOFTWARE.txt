=== PRÁCTICA 1: PORTFOLIO ===
Técnicas de Búsqueda Local y Algoritmos Greedy

COMPILACIÓN:
  cmake .
  make

Genera el ejecutable ./main en este directorio

EJECUCIÓN:
  ./main [max_evals] [n_seeds]

Ejemplos:
  ./main                    # Valores por defecto (10000 evals, 50 semillas)
  ./main 5000               # 5000 evaluaciones 
  ./main 10000 30           # 10000 evaluaciones, 30 semillas

Genera archivos CSV:
  results_ibex_35.csv
  results_syp_100.csv
  results_syp_500.csv

Boxplots (requiere Python):
  python3 boxplot.py
