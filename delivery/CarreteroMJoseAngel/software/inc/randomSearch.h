#ifndef RANDOMSEARCH_H
#define RANDOMSEARCH_H

#include "../common/mh.h"

template <typename tDomain>
class RandomSearch : public MH<tDomain> {
public:
  RandomSearch() = default;
  ~RandomSearch() override = default;

  ResultMH<tDomain> optimize(Problem<tDomain> &problem, int maxevals) override;
};

#endif
