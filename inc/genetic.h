#ifndef GENETIC_H
#define GENETIC_H

#include <utility>
#include <vector>

#include "../common/mh.h"

enum class TipoEsquema { GENERACIONAL, ESTACIONARIO };
enum class TipoCruce { ARITMETICO, BLX };

struct Individuo {
  tSolution<double> cromosomas;
  double fitness;

  // Un operador para ordenar de mejor a peor (útil para el Elitismo y AGE)
  bool operator>(const Individuo& otro) const { return fitness > otro.fitness; }
};

template <typename tDomain>
class AlgoritmoGenetico : public MH<tDomain> {
 private:
  TipoEsquema esquema;
  TipoCruce cruce;
  int tamPoblacion = 100;
  double pCruce;
  double pMutacion;

  Individuo torneo(const std::vector<Individuo>& poblacion, int k = 2);
  void mutar(Individuo& ind, Problem<tDomain>& problem);
  std::pair<Individuo, Individuo> cruzar(const Individuo& p1,
                                         const Individuo& p2,
                                         Problem<tDomain>& problem);

 public:
  AlgoritmoGenetico(TipoEsquema e, TipoCruce c);
  ResultMH<tDomain> optimize(Problem<tDomain>& problem, int maxevals) override;
};

#endif
