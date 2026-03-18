#include <greedy.h>
#include <iostream>
#include <portfolioProblem.h>
#include <problem.h>
#include <random.hpp>
#include <randomSearch.h>
#include <string>
#include <util.h>

using namespace std;

int main(int argc, char *argv[]) {
  long int seed = (argc > 1) ? atoi(argv[1]) : 42;
  string datafile = (argc > 2) ? argv[2] : "data/syp_500.csv";

  Random::seed(seed);
  cout << "Seed: " << seed << "\n";
  cout << "Archivo: " << datafile << "\n\n";

  // Ejecuciones maximas del algoritmo aleatorio
  const int MAX_EVALS = 10000;

  double min = 0;
  double max = 0;

  // Elegimos los parametros en funcion del problema
  if (datafile == "data/syp_500.csv") {
    min = 0.005;
    max = 0.02;
  } else if (datafile == "data/syp_100.csv") {
    min = 0.005;
    max = 0.05;
  } else {
    min = 0.005;
    max = 0.08;
  }

  PortfolioProblem problem(datafile, min, max);

  cout << "=== Info del problema ===\n";
  cout << "N (empresas): " << problem.getSolutionSize() << "\n";
  auto [lo, hi] = problem.getSolutionDomainRange();
  cout << "Rango: [" << lo << ", " << hi << "]\n";
  cout << "Lambda: " << problem.getLambda() << "\n\n";

  // === Algoritmo aleatorio ===
  RandomSearch<double> ralg;
  auto solution = ralg.optimize(problem, MAX_EVALS);

  cout << "=== Búsqueda Aleatoria ===\n";
  cout << "Mejor fitness:  " << solution.fitness << "\n";
  cout << "Evaluaciones:   " << solution.evaluations << "\n";
  cout << "Primeros 10 pesos: [";
  int a_imprimir = std::min((int)solution.solution.size(), 10);
  for (int i = 0; i < a_imprimir; i++)
    cout << solution.solution[i] << (i + 1 < a_imprimir ? ", " : "");
  cout << "...]\n\n";

  // === Algoritmo greedy ===
  Greedy<double> greedy;
  solution = greedy.optimize(problem, MAX_EVALS);

  cout << "=== Greedy ===\n";
  cout << "Mejor fitness:  " << solution.fitness << "\n";
  cout << "Evaluaciones:   " << solution.evaluations << "\n";
  cout << "Primeros 10 pesos: [";
  a_imprimir = std::min((int)solution.solution.size(), 10);
  for (int i = 0; i < a_imprimir; i++)
    cout << solution.solution[i] << (i + 1 < a_imprimir ? ", " : "");
  cout << "...]\n";

  return 0;
}
