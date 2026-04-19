#ifndef LOCALSEARCH_H
#define LOCALSEARCH_H

#include "../common/mh.h"
#include <utility>
#include <vector>

template <typename tDomain>
class LocalSearch : public MH<tDomain> {
private:
  std::vector<std::pair<int, int>> generatePairs(int N);

public:
  LocalSearch() = default;
  ~LocalSearch() override = default;

  ResultMH<tDomain> optimize(Problem<tDomain> &problem, int maxevals) override;
};

#endif
