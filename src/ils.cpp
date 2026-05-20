#include "ils.h"

#include <algorithm>

template <typename tDomain>
ILS<tDomain>::ILS(int it) : iteraciones(it) {}

// Muta el 20% de los valores transfiriendo su peso a empresas inactivas
// (representan un cluster distinto al de las empresas seleccionadas)
template <typename tDomain>
tSolution<tDomain> ILS<tDomain>::mutar(const tSolution<tDomain>& sol,
                                       Problem<tDomain>& problem) {
  int N = sol.size();
  auto [lo, hi] = problem.getSolutionDomainRange();

  std::vector<int> activas, inactivas;
  for (int i = 0; i < N; i++) {
    if (sol[i] > 0.0)
      activas.push_back(i);
    else
      inactivas.push_back(i);
  }

  int n_mut = std::max(1, (int)std::round(0.2 * N));

  auto mutada = sol;

  Random::shuffle(activas);
  int n_zerear = std::min(n_mut, (int)activas.size());
  double peso_liberado = 0.0;
  for (int i = 0; i < n_zerear; i++) {
    peso_liberado += mutada[activas[i]];
    mutada[activas[i]] = 0.0;
  }

  // Redistribuir el peso entre empresas inactivas (cluster distinto)
  if (!inactivas.empty() && peso_liberado > 0.0) {
    Random::shuffle(inactivas);
    int n_destinos = std::min(n_zerear, (int)inactivas.size());
    double por_destino = peso_liberado / n_destinos;
    for (int i = 0; i < n_destinos; i++)
      mutada[inactivas[i]] = std::min(por_destino, hi);
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
