#include "genetic.h"

#include "localsearch.h"

template <typename tDomain>
AlgoritmoGenetico<tDomain>::AlgoritmoGenetico(TipoEsquema e, TipoCruce c,
                                              TipoMemetico m) {
  this->esquema = e;
  this->cruce = c;
  this->tipo_memetico = m;

  if (this->tipo_memetico == TipoMemetico::GENETICO) {
    if (this->esquema == TipoEsquema::ESTACIONARIO) {
      this->pCruce = 1.0;
    } else {
      this->pCruce = 0.8;
    }
    this->pMutacion = 0.1;  // por individuo
  } else {
    this->pCruce = 0.8;
    this->pMutacion = 0.1;

    this->tamPoblacion = 100;
  }
}

// Metodos auxiliares (torneo, mutar, cruzar)
template <typename tDomain>
Individuo AlgoritmoGenetico<tDomain>::torneo(
    const std::vector<Individuo>& poblacion, int k) {
  // 1. Elegimos al primer luchador al azar y asumimos que es el ganador
  // provisional
  int mejor_idx = Random::get(0, (int)poblacion.size() - 1);
  Individuo mejor = poblacion[mejor_idx];

  // 2. Hacemos que compita contra los k-1 restantes
  for (int i = 1; i < k; ++i) {
    int rival_idx = Random::get(0, (int)poblacion.size() - 1);

    // Si el rival tiene mejor fitness, le quita el puesto de campeón
    if (poblacion[rival_idx].fitness > mejor.fitness) {
      mejor = poblacion[rival_idx];
    }
  }

  // 3. Devolvemos al campeón absoluto
  return mejor;
}

// Mutar
template <typename tDomain>
void AlgoritmoGenetico<tDomain>::mutar(Individuo& ind,
                                       Problem<tDomain>& problem,
                                       int& evaluaciones) {
  auto [lo, hi] = problem.getSolutionDomainRange();
  int N = (int)ind.cromosomas.size();
  bool mutado = false;

  if (this->tipo_memetico == TipoMemetico::GENETICO) {
    // AGs: mutación de un gen con probabilidad 0.1 por individuo
    if (Random::get(0.0, 1.0) <= this->pMutacion) {
      int gen_mutado = Random::get(0, N - 1);
      ind.cromosomas[gen_mutado] = Random::get(lo, hi);
      mutado = true;
    }
  } else {
    // AMs: mutación por gen con probabilidad 0.1/N
    double pMut_gene = 0.1 / N;
    for (int i = 0; i < N; ++i) {
      if (Random::get(0.0, 1.0) <= pMut_gene) {
        ind.cromosomas[i] = Random::get(lo, hi);
        mutado = true;
      }
    }
  }

  if (mutado) {
    problem.fix(ind.cromosomas);
    ind.fitness = problem.fitness(ind.cromosomas);
    evaluaciones++;
  }
}
template <typename tDomain>
std::pair<Individuo, Individuo> AlgoritmoGenetico<tDomain>::cruzar(
    const Individuo& p1, const Individuo& p2, Problem<tDomain>& problem,
    int& evaluaciones) {
  auto h1 = p1;
  auto h2 = p2;

  if (this->cruce == TipoCruce::ARITMETICO) {  // Cruce aritmético
    for (int i = 0; i < problem.getSolutionSize(); ++i) {
      double r = Random::get(0.0, 1.0);
      h1.cromosomas[i] = r * p1.cromosomas[i] + (1 - r) * p2.cromosomas[i];
      h2.cromosomas[i] = r * p2.cromosomas[i] + (1 - r) * p1.cromosomas[i];
    }
  } else {  // BLX
    auto [lo, hi] = problem.getSolutionDomainRange();
    double alpha = 0.3;

    for (int i = 0; i < problem.getSolutionSize(); ++i) {
      double intervalo = std::abs(p1.cromosomas[i] - p2.cromosomas[i]);

      // Calculamos los límites expandidos
      double hmin =
          std::min(p1.cromosomas[i], p2.cromosomas[i]) - intervalo * alpha;
      double hmax =
          std::max(p1.cromosomas[i], p2.cromosomas[i]) + intervalo * alpha;

      // VEMOS QUE NO SEA NEGATIVO
      hmin = std::max(0.0, hmin);
      hmax = std::min(hi, hmax);

      h1.cromosomas[i] = Random::get(hmin, hmax);
      h2.cromosomas[i] = Random::get(hmin, hmax);
    }
  }

  problem.fix(h1.cromosomas);
  problem.fix(h2.cromosomas);

  // Recalculamos el fitness
  h1.fitness = problem.fitness(h1.cromosomas);
  h2.fitness = problem.fitness(h2.cromosomas);

  evaluaciones += 2;

  return {h1, h2};
}

// Implementación del método optimize
template <typename tDomain>
ResultMH<tDomain> AlgoritmoGenetico<tDomain>::optimize(
    Problem<tDomain>& problem, int maxevals) {
  int evaluaciones = 0;
  int generaciones = 0;
  std::vector<Individuo> poblacion;

  // Crear la población inicial ---
  for (int i = 0; i < this->tamPoblacion; i++) {
    auto adn = problem.createSolution();
    double fitness = problem.fitness(adn);

    poblacion.push_back({adn, fitness});

    evaluaciones++;
  }
  // BUCLE PRINCIPAL DE EVOLUCIÓN ---
  while (evaluaciones < maxevals) {
    if (this->esquema == TipoEsquema::GENERACIONAL) {
      std::vector<Individuo> nueva_poblacion;

      for (int i = 0; i < 50 && evaluaciones < maxevals; ++i) {
        Individuo p1 = torneo(poblacion, 2);
        Individuo p2 = torneo(poblacion, 2);

        Individuo h1, h2;

        if (Random::get(0.0, 1.0) <= this->pCruce) {
          auto cruzados = cruzar(p1, p2, problem, evaluaciones);
          h1 = cruzados.first;
          h2 = cruzados.second;
        } else {
          h1 = p1;
          h2 = p2;
        }

        mutar(h1, problem, evaluaciones);
        mutar(h2, problem, evaluaciones);

        nueva_poblacion.push_back(h1);
        nueva_poblacion.push_back(h2);
      }

      // --- ELITISMO ---
      auto mejor_padre_it =
          std::max_element(poblacion.begin(), poblacion.end());

      auto mejor_hijo_it =
          std::max_element(nueva_poblacion.begin(), nueva_poblacion.end());
      auto peor_hijo_it =
          std::min_element(nueva_poblacion.begin(), nueva_poblacion.end());

      if (mejor_hijo_it->fitness < mejor_padre_it->fitness) {
        // Machacamos al peor hijo y metemos al mejor padre
        *peor_hijo_it = *mejor_padre_it;
      }

      // --- EL RELEVO ---
      poblacion = nueva_poblacion;

      generaciones++;
      // --- MEMETICO ---
      if (generaciones % 10 == 0 &&
          this->tipo_memetico != TipoMemetico::GENETICO) {
        LocalSearch<double> ls(0.15);

        if (this->tipo_memetico == TipoMemetico::ALL) {
          for (int i = 0; i < this->tamPoblacion; i++) {
            auto resultado = ls.optimize(problem, poblacion[i].cromosomas, 100);

            poblacion[i].cromosomas = resultado.solution;
            poblacion[i].fitness = resultado.fitness;

            evaluaciones += resultado.evaluations;
          }

        } else if (this->tipo_memetico == TipoMemetico::RAND) {
          for (int i = 0; i < this->tamPoblacion; i++) {
            if (Random::get(0.0, 1.0) <= 0.1) {
              auto resultado =
                  ls.optimize(problem, poblacion[i].cromosomas, 100);

              poblacion[i].cromosomas = resultado.solution;
              poblacion[i].fitness = resultado.fitness;

              evaluaciones += resultado.evaluations;
            }
          }

        } else if (this->tipo_memetico == TipoMemetico::BEST) {
          std::sort(poblacion.begin(), poblacion.end(),
                    std::greater<Individuo>());

          for (int i = 0; i < 10; i++) {
            auto resultado = ls.optimize(problem, poblacion[i].cromosomas, 100);

            poblacion[i].cromosomas = resultado.solution;
            poblacion[i].fitness = resultado.fitness;

            evaluaciones += resultado.evaluations;
          }
        }
      }
    } else {
      Individuo p1 = torneo(poblacion, 2);
      Individuo p2 = torneo(poblacion, 2);

      // Creamos a los hijos vacíos FUERA del if
      Individuo h1, h2;

      if (Random::get(0.0, 1.0) <=
          this->pCruce) {  // En este caso, siempre lo hara, lo dejo
        auto cruzados = cruzar(p1, p2, problem,
                               evaluaciones);  // por si se quiere cambiar
        h1 = cruzados.first;
        h2 = cruzados.second;
      } else {
        h1 = p1;
        h2 = p2;
      }

      mutar(h1, problem, evaluaciones);
      mutar(h2, problem, evaluaciones);

      // Duelos

      // Duelo del Hijo 1
      auto peor1 = std::min_element(poblacion.begin(), poblacion.end());

      if (peor1->fitness < h1.fitness) {
        // El hijo 1 es mejor que el peor de la manada: lo asesina y ocupa su
        // lugar
        *peor1 = h1;
      }

      // Duelo del Hijo 2
      auto peor2 = std::min_element(poblacion.begin(), poblacion.end());

      if (peor2->fitness < h2.fitness) {
        *peor2 = h2;
      }
    }
  }

  // --- 3. EL RETORNO ---
  // Al terminar, buscaremos al mejor de la población y lo devolveremos

  auto mejor = std::max_element(poblacion.begin(), poblacion.end());

  return {mejor->cromosomas, mejor->fitness, evaluaciones};
}

// Instanciación explícita para double
template class AlgoritmoGenetico<double>;