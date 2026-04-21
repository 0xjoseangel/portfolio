#include "localsearch.h"

template <typename tDomain>
LocalSearch<tDomain>::LocalSearch(double p) {
  this->proportion = p;
}

template <typename tDomain>
std::vector<std::pair<int, int>> LocalSearch<tDomain>::generatePairs(int N) {
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

template <typename tDomain>
ResultMH<tDomain> LocalSearch<tDomain>::optimize(Problem<tDomain>& problem,
                                                 int maxevals) {
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
        break;  // Rompe el for
      }

      auto solucion_nueva = solucion_actual;

      int origen = P[k].first;
      int destino = P[k].second;

      double cantidad = solucion_actual[origen] * this->proportion;
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

  return {solucion_actual, fitness_actual, evals};
}
template <typename tDomain>
ResultMH<tDomain> LocalSearch<tDomain>::optimize(
    Problem<tDomain>& problem, tSolution<tDomain> solucion_inicial,
    int maxevals) {
  int N = problem.getSolutionSize();
  auto P = generatePairs(N);

  auto solucion_actual = solucion_inicial;
  double fitness_actual = problem.fitness(solucion_actual);

  bool mejora = true;
  int evals = 1;

  while (mejora && evals < maxevals) {
    mejora = false;
    Random::shuffle(P);

    for (size_t k = 0; k < P.size(); ++k) {
      if (evals >= maxevals) {
        break;  // Rompe el for
      }

      auto solucion_nueva = solucion_actual;

      int origen = P[k].first;
      int destino = P[k].second;

      double cantidad = solucion_actual[origen] * this->proportion;
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

  return {solucion_actual, fitness_actual, evals};
}

template class LocalSearch<double>;
