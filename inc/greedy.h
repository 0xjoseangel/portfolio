#ifndef GREEDY_H
#define GREEDY_H

#include "../common/mh.h"
#include "./portfolioProblem.h"
#include <limits>

template <typename tDomain> class Greedy : public MH<tDomain> {
private:
  std::vector<double> companies;

  std::vector<std::vector<double>> covariance;
  double lambda;
  std::vector<double> benefit_factors;

  double calculate_heur(Problem<tDomain> &problem, int companie) {
    // 1. Desenmascaramos al padre y lo convertimos en puntero al hijo
    PortfolioProblem *p_prob = dynamic_cast<PortfolioProblem *>(&problem);

    // 2. Ahora usamos p_prob-> en lugar de problem.
    covariance = p_prob->getCovariance();
    lambda = p_prob->getLambda();
    benefit_factors = p_prob->getBenefitFactors();

    double heuristica_companie = benefit_factors[companie];
    double suma_covarianzas = 0.0;

    // Aquí el guion dividía entre N, así que usamos p_prob->getSolutionSize()
    // que es la N
    int N = p_prob->getSolutionSize();
    for (int j = 0; j < N; ++j) {
      suma_covarianzas += covariance[companie][j];
    }

    heuristica_companie -= lambda * (suma_covarianzas / N);

    return heuristica_companie;
  }

public:
  Greedy() = default;
  ~Greedy() override = default;

  // Pongo el codigo con el .h porque con los templates me daba fallo
  // con los linkers...
  ResultMH<tDomain> optimize(Problem<tDomain> &problem, int maxevals) override {
    int N = problem.getSolutionSize();
    auto [lo, hi] = problem.getSolutionDomainRange();

    std::vector<std::pair<double, int>> empresas_heur;

    for (int i = 0; i < N; ++i) {
      double heur = calculate_heur(problem, i);
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

    double fitness = problem.fitness(solucion);

    return {solucion, fitness, 1};
  }
};

#endif
