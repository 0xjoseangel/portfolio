#ifndef PORTFOLIOPROBLEM_H
#define PORTFOLIOPROBLEM_H

#include "../common/problem.h"
#include <cmath> // Para los logaritmos
#include <string>
#include <vector>

class PortfolioProblem : public Problem<double> {
private:
  // --- DATOS DEL PROBLEMA ---
  std::vector<std::vector<double>> returns;    // mu_ti: [día t][empresa i]
  std::vector<double> means;                   // mu_i
  std::vector<std::vector<double>> covariance; // Sigma_ij

  // TRUCO DE OPTIMIZACIÓN: Precalculamos la parte del sumatorio del beneficio
  std::vector<double> benefit_factors;

  double lambda;
  double lo;
  double hi;
  int N; // Número de empresas
  int T; // Número de días

  // --- MÉTODOS PRIVADOS AUXILIARES ---
  void loadData(const std::string &filename);
  void precomputeMatrices();

public:
  // Constructor
  PortfolioProblem(const std::string &filename, double min_val, double max_val);
  ~PortfolioProblem() override = default;

  // --- MÉTODOS OBLIGATORIOS DE LA INTERFAZ ---
  tFitness fitness(const tSolution<double> &solution) override;
  tSolution<double> createSolution() override;
  size_t getSolutionSize() override;
  std::pair<double, double> getSolutionDomainRange() override;
  bool isValid(const tSolution<double> &solution) override;
  void fix(tSolution<double> &solution) override;

  // --- GETTERS PARA EL GREEDY ---
  const std::vector<double> &getBenefitFactors() const {
    return benefit_factors;
  }
  const std::vector<std::vector<double>> &getCovariance() const {
    return covariance;
  }
  double getLambda() const { return lambda; }
};

#endif
