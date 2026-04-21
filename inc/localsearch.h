#ifndef LOCALSEARCH_H
#define LOCALSEARCH_H

#include <utility>
#include <vector>

#include "../common/mh.h"

template <typename tDomain>
class LocalSearch : public MH<tDomain> {
 private:
  double proportion;
  std::vector<std::pair<int, int>> generatePairs(int N);

 public:
  LocalSearch(double p = 0.4);
  ~LocalSearch() override = default;

  ResultMH<tDomain> optimize(Problem<tDomain>& problem, int maxevals) override;
  // Lo usamos en el memetico para que no empiece de una solucion aleatoria
  ResultMH<tDomain> optimize(Problem<tDomain>& problem,
                             tSolution<tDomain> solucion_inicial, int maxevals);
};

#endif
