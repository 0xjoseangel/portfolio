#ifndef ILS_H
#define ILS_H

#include <localsearch.h>
#include "../common/mh.h"

template <typename tDomain>
class ILS : public MH<tDomain> {
 private:
  int iteraciones;
  LocalSearch<double> ls;

  tSolution<tDomain> mutar(const tSolution<tDomain>& sol,
                           Problem<tDomain>& problem);

 public:
  ILS(int it = 5);
  ~ILS() override = default;

  ResultMH<tDomain> optimize(Problem<tDomain>& problem, int maxevals) override;
};

#endif
