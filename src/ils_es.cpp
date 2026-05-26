#include "ils_es.h"

#include <algorithm>
#include <numeric>

template <typename tDomain>
ILSES<tDomain>::ILSES(int it) : iteraciones(it) {}

// Mismo operador de mutación que ILS: desordena el 20% de los valores
template <typename tDomain>
tSolution<tDomain> ILSES<tDomain>::mutar(const tSolution<tDomain>& sol,
                                         Problem<tDomain>& problem) {
  int N = sol.size();
  int n_mut = std::max(1, (int)std::round(0.2 * N));

  auto mutada = sol;

  std::vector<int> indices(N);
  std::iota(indices.begin(), indices.end(), 0);
  Random::shuffle(indices);

  std::vector<tDomain> valores_mut;
  for (int i = 0; i < n_mut; i++)
    valores_mut.push_back(mutada[indices[i]]);

  Random::shuffle(valores_mut);

  for (int i = 0; i < n_mut; i++)
    mutada[indices[i]] = valores_mut[i];

  problem.fix(mutada);
  return mutada;
}

template <typename tDomain>
ResultMH<tDomain> ILSES<tDomain>::optimize(Problem<tDomain>& problem,
                                           int maxevals) {
  int evals_por_iter = maxevals / iteraciones;

  auto mejor = sa.optimize(problem, evals_por_iter);
  int evals = mejor.evaluations;

  for (int i = 0; i < iteraciones - 1; i++) {
    auto sol_mutada = mutar(mejor.solution, problem);
    auto res = sa.optimize(problem, sol_mutada, evals_por_iter);
    evals += res.evaluations;

    if (res.fitness > mejor.fitness) mejor = res;
  }

  return {mejor.solution, mejor.fitness, (unsigned int)evals};
}

template class ILSES<double>;
