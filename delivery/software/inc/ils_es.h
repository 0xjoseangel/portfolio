#ifndef ILS_ES_H
#define ILS_ES_H

#include <sa.h>
#include "../common/mh.h"

template <typename tDomain>
class ILSES : public MH<tDomain> {
 private:
  int iteraciones;
  SA<tDomain> sa;

  tSolution<tDomain> mutar(const tSolution<tDomain>& sol,
                           Problem<tDomain>& problem);

 public:
  ILSES(int it = 5);
  ~ILSES() override = default;

  ResultMH<tDomain> optimize(Problem<tDomain>& problem, int maxevals) override;
};

#endif
