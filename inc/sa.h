#ifndef SA_H
#define SA_H

#include "../common/mh.h"

template <typename tDomain>
class SA : public MH<tDomain> {
 public:
  SA(double phi = 0.3, double mu = 0.2, double Tf = 1e-4,
     int k_vecinos = 10, int k_exitos = 1);
  ~SA() override = default;

  ResultMH<tDomain> optimize(Problem<tDomain>& problem, int maxevals) override;
  ResultMH<tDomain> optimize(Problem<tDomain>& problem,
                             tSolution<tDomain> sol_inicial, int maxevals);

 private:
  double phi;
  double mu;
  double Tf;
  int k_vecinos;
  int k_exitos;
};

#endif
