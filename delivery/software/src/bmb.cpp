#include "bmb.h"

#include <limits>

template <typename tDomain>
BMB<tDomain>::BMB(int it) : iteraciones(it) {}

template <typename tDomain>
ResultMH<tDomain> BMB<tDomain>::optimize(Problem<tDomain>& problem,
                                         int maxevals) {
  tSolution<tDomain> mejor_sol_interna;
  double mejor_fitness = -std::numeric_limits<double>::max();
  int evals_totales = 0;

  for (int i = 0; i < this->iteraciones; i++) {
    if (evals_totales >= maxevals) break;

    int evals_disponibles = std::min(2000, maxevals - evals_totales);

    auto resultado_ls = ls.optimize(problem, evals_disponibles);
    evals_totales += resultado_ls.evaluations;

    if (i == 0 || resultado_ls.fitness > mejor_fitness) {
      mejor_sol_interna = resultado_ls.solution;
      mejor_fitness = resultado_ls.fitness;
    }
  }

  return {mejor_sol_interna, (tFitness)mejor_fitness, (unsigned int)evals_totales};
}

template class BMB<double>;
