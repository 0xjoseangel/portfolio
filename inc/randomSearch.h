#ifndef RANDOMSEARCH_H
#define RANDOMSEARCH_H

#include "../common/mh.h"
#include <limits>

template <typename tDomain> class RandomSearch : public MH<tDomain> {
public:
  RandomSearch() = default;
  ~RandomSearch() override = default;

  // Pongo el codigo con el .h porque con los templates me daba fallo
  // con los linkers...
  ResultMH<tDomain> optimize(Problem<tDomain> &problem, int maxevals) override {
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
};

#endif
