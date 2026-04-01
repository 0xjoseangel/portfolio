#include "greedy.h"
#include <algorithm>
#include <limits>

template <typename tDomain>
double Greedy<tDomain>::calculate_heur(int companie, int N) {
  double heuristica_companie = benefit_factors[companie];
  double suma_covarianzas = 0.0;

  for (int j = 0; j < N; ++j) {
    suma_covarianzas += covariance[companie][j];
  }

  heuristica_companie -= lambda * (suma_covarianzas / N);

  return heuristica_companie;
}

template <typename tDomain>
ResultMH<tDomain> Greedy<tDomain>::optimize(Problem<tDomain> &problem, int maxevals) {
  int N = problem.getSolutionSize();
  auto [lo, hi] = problem.getSolutionDomainRange();

  // Cargamos los datos una sola vez antes del bucle
  PortfolioProblem *p_prob = dynamic_cast<PortfolioProblem *>(&problem);
  covariance = p_prob->getCovariance();
  lambda = p_prob->getLambda();
  benefit_factors = p_prob->getBenefitFactors();

  std::vector<std::pair<double, int>> empresas_heur;

  for (int i = 0; i < N; ++i) {
    double heur = calculate_heur(i, N);
    empresas_heur.push_back({heur, i});
  }

  std::sort(empresas_heur.rbegin(), empresas_heur.rend());

  tSolution<double> solucion(N, 0.0);
  double suma_pesos = 0.0;

  int idx_lista = 0;
  while (suma_pesos < 1.0 - 1e-8 && idx_lista < N) {
    int c = empresas_heur[idx_lista].second;

    double dinero_restante = 1.0 - suma_pesos;
    double a_invertir = std::min(hi, dinero_restante);

    solucion[c] = a_invertir;
    suma_pesos += a_invertir;

    idx_lista++;
  }

  problem.fix(solucion);
  double fitness = problem.fitness(solucion);

  return {solucion, fitness, 1};
}

template class Greedy<double>;
