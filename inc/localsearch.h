
#ifndef LOCALSEARCH_H
#define LOCALSEARCH_H

#include "../common/mh.h"

template <typename tDomain> class LocalSearch : public MH<tDomain> {
private:
  std::vector<std::pair<int, int>> generatePairs(int N) {
    std::vector<std::pair<int, int>> P;
    P.reserve(N * (N - 1));

    for (int i = 0; i < N; ++i) {
      for (int j = 0; j < N; ++j) {
        // Solo guardamos si son empresas distintas
        if (i != j) {
          P.push_back({i, j});
        }
      }
    }

    return P;
  }

public:
  LocalSearch() = default;
  ~LocalSearch() override = default;

  // Pongo el codigo con el .h porque con los templates me daba fallo
  // con los linkers...
  ResultMH<tDomain> optimize(Problem<tDomain> &problem, int maxevals) override {
    int N = problem.getSolutionSize();
    auto P = generatePairs(N);

    auto solucion_actual = problem.createSolution();
    double fitness_actual = problem.fitness(solucion_actual);

    bool mejora = true;
    int evals = 1;

    while (mejora && evals < maxevals) {
      mejora = false;
      Random::shuffle(P);

      for (size_t k = 0; k < P.size(); ++k) {
        if (evals >= maxevals) {
          break; // Rompe el for
        }

        auto solucion_nueva = solucion_actual;

        int origen = P[k].first;
        int destino = P[k].second;

        double cantidad = solucion_actual[origen] * 0.4;
        solucion_nueva[origen] -= cantidad;
        solucion_nueva[destino] += cantidad;

        // Aunque sume de un lado y lo reste de otro y la suma siga siendo 1.0
        // se pueden romper los limites
        problem.fix(solucion_nueva);
        double fitness_nueva = problem.fitness(solucion_nueva);
        evals++;

        if (fitness_nueva > fitness_actual) {
          fitness_actual = fitness_nueva;
          solucion_actual = solucion_nueva;

          mejora = true;
          break;
        }
      }
    }

    return {solucion_actual, fitness_actual, maxevals};
  }
};

#endif
