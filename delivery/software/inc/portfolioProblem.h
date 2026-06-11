#ifndef PORTFOLIOPROBLEM_H
#define PORTFOLIOPROBLEM_H

#include "../common/problem.h"
#include <cmath> // Para los logaritmos
#include <string>
#include <vector>

class PortfolioProblem : public Problem<double> {
private:
  // --- DATOS DEL PROBLEMA ---
  std::vector<std::vector<double>> returns;      // mu_ti train (2015-2024)
  std::vector<std::vector<double>> returns_test; // mu_ti test  (2025)
  std::vector<double> means;                     // mu_i (train)
  std::vector<std::vector<double>> covariance;   // Sigma_ij (train)
  std::vector<double> benefit_factors;           // (train)

  // Matrices para evaluación sobre datos 2025
  std::vector<double> means_test;
  std::vector<std::vector<double>> covariance_test;
  std::vector<double> benefit_factors_test;

  double lambda;
  double lo;
  double hi;
  int N;      // Número de empresas
  int T;      // Días de entrenamiento (2015-2024)
  int T_test; // Días de test (2025)

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

  // --- EVALUACIÓN SOBRE DATOS 2025 ---
  tFitness fitnessTest(const tSolution<double> &solution);
  double beneficioTest(const tSolution<double> &solution);

  // --- GETTERS PARA EL GREEDY ---
  const std::vector<double> &getBenefitFactors() const {
    return benefit_factors;
  }
  const std::vector<std::vector<double>> &getCovariance() const {
    return covariance;
  }
  double getLambda() const { return lambda; }
  int getT() const { return T; }
};

#endif
