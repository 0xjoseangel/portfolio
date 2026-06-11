#include "sa.h"

#include <cmath>

template <typename tDomain>
SA<tDomain>::SA(double phi, double mu, double Tf, int k_vecinos, int k_exitos,
                double proportion)
    : phi(phi), mu(mu), Tf(Tf), k_vecinos(k_vecinos), k_exitos(k_exitos),
      proportion(proportion) {}

template <typename tDomain>
tSolution<tDomain> SA<tDomain>::generarVecino(const tSolution<tDomain>& sol,
                                               Problem<tDomain>& problem) {
  int N = sol.size();
  auto [lo, hi] = problem.getSolutionDomainRange();

  int origen = Random::get(0, N - 1);
  int destino;
  do { destino = Random::get(0, N - 1); } while (destino == origen);

  double cantidad = sol[origen] * proportion;
  cantidad = std::min(cantidad, sol[origen] - lo);
  cantidad = std::min(cantidad, hi - sol[destino]);

  auto vecino = sol;
  if (cantidad > 1e-15) {
    vecino[origen] -= cantidad;
    vecino[destino] += cantidad;
  }
  return vecino;
}

template <typename tDomain>
ResultMH<tDomain> SA<tDomain>::optimize(Problem<tDomain>& problem,
                                        int maxevals) {
  return optimize(problem, problem.createSolution(), maxevals);
}

template <typename tDomain>
ResultMH<tDomain> SA<tDomain>::optimize(Problem<tDomain>& problem,
                                        tSolution<tDomain> sol_inicial,
                                        int maxevals) {
  int N = problem.getSolutionSize();

  int max_vecinos = k_vecinos * N;
  int max_éxitos = k_exitos * N;

  // Solución inicial y evaluación
  auto sol_actual = sol_inicial;
  double fitness_actual = problem.fitness(sol_actual);
  auto mejor = sol_actual;
  double mejor_fitness = fitness_actual;
  int evals = 1;

  // Temperatura inicial y parámetros de enfriamiento
  double T0 = (mu * std::abs(fitness_actual)) / (-log(phi));
  int M = std::max(1, maxevals / max_vecinos);
  double beta = (T0 - Tf) / (M * T0 * Tf);
  double T = T0;

  // Bucle de enfriamientos
  for (int enfr = 0; enfr < M && evals < maxevals; enfr++) {
    int num_vecinos = 0;
    int num_éxitos = 0;

    // Bucle interno: generar vecinos a temperatura T
    while (num_vecinos < max_vecinos && num_éxitos < max_éxitos &&
           evals < maxevals) {
      auto sol_vecino = generarVecino(sol_actual, problem);
      double fitness_vecino = problem.fitness(sol_vecino);
      evals++;

      // Criterio de aceptación (Metropolis)
      bool aceptar = false;
      if (fitness_vecino > fitness_actual) {
        aceptar = true;  // Mejor solución
      } else {
        double delta = fitness_vecino - fitness_actual;
        double prob = exp(delta / T);
        if (Random::get() < prob) {
          aceptar = true;  // Acepta peor con probabilidad
        }
      }

      if (aceptar) {
        sol_actual = sol_vecino;
        fitness_actual = fitness_vecino;
        num_éxitos++;

        if (fitness_actual > mejor_fitness) {
          mejor = sol_actual;
          mejor_fitness = fitness_actual;
        }
      }

      num_vecinos++;
    }

    // Si no hubo éxitos en este enfriamiento, terminar
    if (num_éxitos == 0) break;

    // Enfriar temperatura (Cauchy modificado)
    T = T / (1.0 + beta * T);

    // Si alcanzó temperatura final, terminar
    if (T < Tf) break;
  }

  return {mejor, (tFitness)mejor_fitness, (unsigned int)evals};
}

template class SA<double>;
