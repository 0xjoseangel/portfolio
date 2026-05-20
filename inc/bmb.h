#ifndef BMB_H
#define BMB_H

#include <localsearch.h>

#include <utility>
#include <vector>

#include "../common/mh.h"

template <typename tDomain>
class BMB : public MH<tDomain> {
 private:
  int iteraciones;
  LocalSearch<double> ls;

 public:
  BMB(int it = 5);
  ~BMB() override = default;

  ResultMH<tDomain> optimize(Problem<tDomain>& problem, int maxevals) override;
};

#endif
