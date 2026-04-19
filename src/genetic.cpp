#include "genetic.h"

// Implementación del constructor template
template <typename tDomain>
AlgoritmoGenetico<tDomain>::AlgoritmoGenetico(TipoEsquema e, TipoCruce c) {
  this->esquema = e;
  this->cruce = c;

  if (this->esquema == TipoEsquema::ESTACIONARIO) {
    this->pCruce = 1.0;
  } else {
    this->pCruce = 0.8;
  }

  this->pMutacion = 0.1;
  this->tamPoblacion = 100;
}

// Implementación del método optimize
template <typename tDomain>
ResultMH<tDomain> AlgoritmoGenetico<tDomain>::optimize(
    Problem<tDomain>& problem, int maxevals) {
  // TODO: Implementar el algoritmo genético
  tSolution<tDomain> solucion = problem.createSolution();
  tFitness fitness = problem.fitness(solucion);
  return {solucion, fitness, 1};
}

// Instanciación explícita para double
template class AlgoritmoGenetico<double>;