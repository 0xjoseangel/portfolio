#include "ils.h"

#include <algorithm>
#include <numeric>

template <typename tDomain>
ILS<tDomain>::ILS(int it) : iteraciones(it) {}

template <typename tDomain>
tSolution<tDomain> ILS<tDomain>::mutar(const tSolution<tDomain>& sol,
                                       Problem<tDomain>& problem) {
  int N = sol.size();
  int n_mut = std::max(1, (int)std::round(0.2 * N));

  auto mutada = sol;

  std::vector<int> indices(N);
  std::iota(indices.begin(), indices.end(), 0);
  Random::shuffle(indices);

  std::vector<tDomain> valores_mut;
  for (int i = 0; i < n_mut; i++) {
    valores_mut.push_back(mutada[indices[i]]);
  }

  Random::shuffle(valores_mut);

  for (int i = 0; i < n_mut; i++) {
    mutada[indices[i]] = valores_mut[i];
  }

  problem.fix(mutada);
  return mutada;
}

template <typename tDomain>
ResultMH<tDomain> ILS<tDomain>::optimize(Problem<tDomain>& problem,
                                         int maxevals) {
  auto mejor = ls.optimize(problem, 2000);
  int evals = mejor.evaluations;

  for (int i = 0; i < iteraciones - 1; i++) {
    auto sol_mutada = mutar(mejor.solution, problem);
    auto sol_nueva = ls.optimize(problem, sol_mutada, 2000);
    evals += sol_nueva.evaluations;

    if (sol_nueva.fitness > mejor.fitness) mejor = sol_nueva;
  }

  return {mejor.solution, mejor.fitness, (unsigned int)evals};
}

template class ILS<double>;
