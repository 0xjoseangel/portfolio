#ifndef GENETIC_H
#define GENETIC_H

#include <utility>
#include <vector>

#include "../common/mh.h"

enum class TipoEsquema { GENERACIONAL, ESTACIONARIO };
enum class TipoCruce { ARITMETICO, BLX };
enum class TipoMemetico { ALL, RAND, BEST, GENETICO };

struct Individuo {
  tSolution<double> cromosomas;
  double fitness;

  // Un operador para ordenar de mejor a peor (útil para el Elitismo y AGE)
  bool operator>(const Individuo& otro) const { return fitness > otro.fitness; }
  bool operator<(const Individuo& otro) const { return fitness < otro.fitness; }
};

template <typename tDomain>
class AlgoritmoGenetico : public MH<tDomain> {
 private:
  TipoEsquema esquema;
  TipoCruce cruce;
  TipoMemetico tipo_memetico;
  int tamPoblacion = 100;
  double pCruce;
  double pMutacion;
  double alpha;

  Individuo torneo(const std::vector<Individuo>& poblacion, int k = 3);
  void mutar(Individuo& ind, Problem<tDomain>& problem, int gene_idx, int& evaluaciones);
  std::pair<Individuo, Individuo> cruzar(const Individuo& p1,
                                         const Individuo& p2,
                                         Problem<tDomain>& problem,
                                         int& evaluaciones);

 public:
  AlgoritmoGenetico(TipoEsquema e, TipoCruce c,
                    TipoMemetico m = TipoMemetico::GENETICO,
                    double alpha = 0.3);
  ~AlgoritmoGenetico() override = default;

  ResultMH<tDomain> optimize(Problem<tDomain>& problem, int maxevals) override;
};

#endif
