#ifndef GREEDY_H
#define GREEDY_H

#include "../common/mh.h"
#include "./portfolioProblem.h"
#include <vector>

template <typename tDomain>
class Greedy : public MH<tDomain> {
private:
  std::vector<double> companies;
  std::vector<std::vector<double>> covariance;
  double lambda;
  std::vector<double> benefit_factors;

  double calculate_heur(int companie, int N);

public:
  Greedy() = default;
  ~Greedy() override = default;

  ResultMH<tDomain> optimize(Problem<tDomain> &problem, int maxevals) override;
};

#endif
