#include <iostream>
#include <portfolioProblem.h>
#include <problem.h>
#include <random.hpp>
#include <string>
#include <util.h>

using namespace std;

int main(int argc, char *argv[]) {
  long int seed = (argc > 1) ? atoi(argv[1]) : 42;
  string datafile = (argc > 2) ? argv[2] : "data/syp_500.csv";

  Random::seed(seed);
  cout << "Seed: " << seed << "\n";
  cout << "Archivo: " << datafile << "\n\n";

  // Parámetros: lo=0.01, hi=1.0
  PortfolioProblem problem(datafile, 0.005, 0.02);

  cout << "=== Info del problema ===\n";
  cout << "N (empresas): " << problem.getSolutionSize() << "\n";
  auto [lo, hi] = problem.getSolutionDomainRange();
  cout << "Rango: [" << lo << ", " << hi << "]\n";
  cout << "Lambda: " << problem.getLambda() << "\n\n";

  // --- Test createSolution ---
  cout << "=== Test createSolution() ===\n";
  tSolution<double> sol = problem.createSolution();

  double suma = 0.0;
  int num_activas = 0;
  for (size_t i = 0; i < sol.size(); ++i) {
    if (sol[i] > 0.0) {
      cout << "  w[" << i << "] = " << sol[i] << "\n";
      suma += sol[i];
      num_activas++;
    }
  }
  cout << "  Empresas con peso > 0: " << num_activas << "\n";
  cout << "  Suma de pesos: " << suma << "\n";
  cout << "  Valida: " << (problem.isValid(sol) ? "SI" : "NO") << "\n\n";

  // --- Test fitness ---
  cout << "=== Test fitness() ===\n";
  tFitness f = problem.fitness(sol);
  cout << "  fitness(sol) = " << f << "\n\n";

  // --- Test fix() con solución inválida ---
  cout << "=== Test fix() con solucion sin normalizar ===\n";
  tSolution<double> sol_mala(problem.getSolutionSize(), 0.0);
  // Invertimos cantidades arbitrarias en las primeras 3 empresas
  sol_mala[0] = 0.5;
  sol_mala[1] = 0.3;
  sol_mala[2] = 0.8;
  cout << "  Antes de fix -> valida: "
       << (problem.isValid(sol_mala) ? "SI" : "NO") << "\n";
  problem.fix(sol_mala);
  cout << "  Despues de fix -> valida: "
       << (problem.isValid(sol_mala) ? "SI" : "NO") << "\n";
  cout << "  fitness(sol_mala) = " << problem.fitness(sol_mala) << "\n\n";

  // --- Varias soluciones aleatorias ---
  cout << "=== 5 soluciones aleatorias ===\n";
  for (int k = 0; k < 5; ++k) {
    tSolution<double> s = problem.createSolution();
    cout << "  Sol " << k + 1 << ": fitness=" << problem.fitness(s)
         << "  valida=" << (problem.isValid(s) ? "SI" : "NO") << "\n";
  }

  return 0;
}
