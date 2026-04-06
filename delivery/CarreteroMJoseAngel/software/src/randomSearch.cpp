#include "randomSearch.h"
#include <limits>

template <typename tDomain>
ResultMH<tDomain> RandomSearch<tDomain>::optimize(Problem<tDomain> &problem, int maxevals) {
  tSolution<tDomain> mejor_solucion;
  double mejor_fitness = -std::numeric_limits<double>::infinity();

  for (int i = 0; i < maxevals; i++) {
    auto solucion_actual = problem.createSolution();
    double fitness_actual = problem.fitness(solucion_actual);

    if (fitness_actual > mejor_fitness) {
      mejor_fitness = fitness_actual;
      mejor_solucion = solucion_actual;
    }
  }

  return {mejor_solucion, mejor_fitness, maxevals};
}

template class RandomSearch<double>;
