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
    this->pMutacion = 0.1;
  } else {
    this->pCruce = 0.8;
    this->pMutacion = 0.1;
    this->tamPoblacion = 100;
  }
}

template <typename tDomain>
Individuo AlgoritmoGenetico<tDomain>::torneo(
    const std::vector<Individuo>& poblacion, int k) {
  int mejor_idx = Random::get(0, (int)poblacion.size() - 1);
  Individuo mejor = poblacion[mejor_idx];

  for (int i = 1; i < k; ++i) {
    int rival_idx = Random::get(0, (int)poblacion.size() - 1);
    if (poblacion[rival_idx].fitness > mejor.fitness) {
      mejor = poblacion[rival_idx];
    }
  }

  return mejor;
}

// Operador intercambio (seminario diap. 13): transfiere el 15% de gene_idx a otro gen
template <typename tDomain>
void AlgoritmoGenetico<tDomain>::mutar(Individuo& ind,
                                       Problem<tDomain>& problem,
                                       int gene_idx, int& evaluaciones) {
  auto [lo, hi] = problem.getSolutionDomainRange();
  int N = (int)ind.cromosomas.size();

  // Elegir gene_j distinto de gene_idx
  int gene_j = Random::get(0, N - 2);
  if (gene_j >= gene_idx) gene_j++;

  double delta = 0.15 * ind.cromosomas[gene_idx];
  delta = std::min(delta, ind.cromosomas[gene_idx] - lo);
  delta = std::min(delta, hi - ind.cromosomas[gene_j]);

  if (delta > 1e-15) {
    ind.cromosomas[gene_idx] -= delta;
    ind.cromosomas[gene_j] += delta;
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

  if (this->cruce == TipoCruce::ARITMETICO) {
    // r_i independiente por posición (guion Ec.4, seminario diap.14)
    for (int i = 0; i < problem.getSolutionSize(); ++i) {
      double r = Random::get(0.0, 1.0);
      h1.cromosomas[i] = r * p1.cromosomas[i] + (1 - r) * p2.cromosomas[i];
      h2.cromosomas[i] = r * p2.cromosomas[i] + (1 - r) * p1.cromosomas[i];
    }
  } else {  // BLX-alpha=0.3
    auto [lo, hi] = problem.getSolutionDomainRange();
    double alpha = 0.3;

    for (int i = 0; i < problem.getSolutionSize(); ++i) {
      double intervalo = std::abs(p1.cromosomas[i] - p2.cromosomas[i]);

      double hmin =
          std::min(p1.cromosomas[i], p2.cromosomas[i]) - intervalo * alpha;
      double hmax =
          std::max(p1.cromosomas[i], p2.cromosomas[i]) + intervalo * alpha;

      hmin = std::max(0.0, hmin);
      hmax = std::min(hi, hmax);

      h1.cromosomas[i] = Random::get(hmin, hmax);
      h2.cromosomas[i] = Random::get(hmin, hmax);
    }
  }

  problem.fix(h1.cromosomas);
  problem.fix(h2.cromosomas);

  h1.fitness = problem.fitness(h1.cromosomas);
  h2.fitness = problem.fitness(h2.cromosomas);

  evaluaciones += 2;

  return {h1, h2};
}

template <typename tDomain>
ResultMH<tDomain> AlgoritmoGenetico<tDomain>::optimize(
    Problem<tDomain>& problem, int maxevals) {
  int evaluaciones = 0;
  int generaciones = 0;
  int N = problem.getSolutionSize();
  std::vector<Individuo> poblacion;

  for (int i = 0; i < this->tamPoblacion; i++) {
    auto adn = problem.createSolution();
    double fitness = problem.fitness(adn);
    poblacion.push_back({adn, fitness});
    evaluaciones++;
  }

  while (evaluaciones < maxevals) {
    if (this->esquema == TipoEsquema::GENERACIONAL) {
      // --- SELECCIÓN: M torneos k=3 → M padres ---
      std::vector<Individuo> padres;
      padres.reserve(this->tamPoblacion);
      for (int i = 0; i < this->tamPoblacion; ++i) {
        padres.push_back(torneo(poblacion, 3));
      }

      // --- CRUCE: esperanza matemática, siempre n_cruces parejas fijas ---
      // Emparejamiento fijo: (0,1), (2,3), ..., la aleatoriedad viene del torneo
      int n_cruces = (int)(this->pCruce * this->tamPoblacion / 2);
      std::vector<Individuo> nueva_poblacion;
      nueva_poblacion.reserve(this->tamPoblacion);

      for (int i = 0; i < this->tamPoblacion / 2 && evaluaciones < maxevals;
           ++i) {
        Individuo& p1 = padres[2 * i];
        Individuo& p2 = padres[2 * i + 1];

        if (i < n_cruces) {
          auto [h1, h2] = cruzar(p1, p2, problem, evaluaciones);
          nueva_poblacion.push_back(h1);
          nueva_poblacion.push_back(h2);
        } else {
          nueva_poblacion.push_back(p1);
          nueva_poblacion.push_back(p2);
        }
      }

      // --- MUTACIÓN: esperanza matemática, siempre n_mut mutaciones fijas ---
      // n_mut = Pm * M (por individuo para AG, equivalente a Pm_gen * n * M para AM)
      int n_mut = (int)(this->pMutacion * this->tamPoblacion);
      int pop_actual = (int)nueva_poblacion.size();
      for (int i = 0; i < n_mut && evaluaciones < maxevals; ++i) {
        int chrom_idx = Random::get(0, pop_actual - 1);
        int gene_idx = Random::get(0, N - 1);
        mutar(nueva_poblacion[chrom_idx], problem, gene_idx, evaluaciones);
      }

      // --- ELITISMO ---
      auto mejor_padre_it =
          std::max_element(poblacion.begin(), poblacion.end());
      auto mejor_hijo_it =
          std::max_element(nueva_poblacion.begin(), nueva_poblacion.end());
      auto peor_hijo_it =
          std::min_element(nueva_poblacion.begin(), nueva_poblacion.end());

      if (mejor_hijo_it->fitness < mejor_padre_it->fitness) {
        *peor_hijo_it = *mejor_padre_it;
      }

      poblacion = std::move(nueva_poblacion);
      generaciones++;

      // --- MEMETICO: BL cada 10 generaciones ---
      if (generaciones % 10 == 0 &&
          this->tipo_memetico != TipoMemetico::GENETICO) {
        LocalSearch<double> ls(0.15);
        int pop_size = (int)poblacion.size();

        if (this->tipo_memetico == TipoMemetico::ALL) {
          for (int i = 0; i < pop_size && evaluaciones < maxevals; i++) {
            int budget = std::min(100, maxevals - evaluaciones);
            auto resultado =
                ls.optimize(problem, poblacion[i].cromosomas, budget);
            poblacion[i].cromosomas = resultado.solution;
            poblacion[i].fitness = resultado.fitness;
            evaluaciones += resultado.evaluations;
          }

        } else if (this->tipo_memetico == TipoMemetico::RAND) {
          for (int i = 0; i < pop_size && evaluaciones < maxevals; i++) {
            if (Random::get(0.0, 1.0) <= 0.1) {
              int budget = std::min(100, maxevals - evaluaciones);
              auto resultado =
                  ls.optimize(problem, poblacion[i].cromosomas, budget);
              poblacion[i].cromosomas = resultado.solution;
              poblacion[i].fitness = resultado.fitness;
              evaluaciones += resultado.evaluations;
            }
          }

        } else if (this->tipo_memetico == TipoMemetico::BEST) {
          std::sort(poblacion.begin(), poblacion.end(),
                    std::greater<Individuo>());
          int n_best = std::max(1, (int)(0.1 * pop_size));
          for (int i = 0; i < n_best && evaluaciones < maxevals; i++) {
            int budget = std::min(100, maxevals - evaluaciones);
            auto resultado =
                ls.optimize(problem, poblacion[i].cromosomas, budget);
            poblacion[i].cromosomas = resultado.solution;
            poblacion[i].fitness = resultado.fitness;
            evaluaciones += resultado.evaluations;
          }
        }
      }

    } else {
      // --- ESTACIONARIO: 2 torneos k=3, cruce, mutación por individuo ---
      Individuo p1 = torneo(poblacion, 3);
      Individuo p2 = torneo(poblacion, 3);

      Individuo h1, h2;
      // pCruce=1.0 para AGE → siempre cruza
      if (Random::get(0.0, 1.0) <= this->pCruce) {
        auto cruzados = cruzar(p1, p2, problem, evaluaciones);
        h1 = cruzados.first;
        h2 = cruzados.second;
      } else {
        h1 = p1;
        h2 = p2;
      }

      // Mutación probabilística por individuo (Pm=0.1)
      if (Random::get(0.0, 1.0) <= this->pMutacion) {
        int gene_idx = Random::get(0, N - 1);
        mutar(h1, problem, gene_idx, evaluaciones);
      }
      if (Random::get(0.0, 1.0) <= this->pMutacion) {
        int gene_idx = Random::get(0, N - 1);
        mutar(h2, problem, gene_idx, evaluaciones);
      }

      // Los dos hijos compiten contra los dos peores de la población
      auto peor1 = std::min_element(poblacion.begin(), poblacion.end());
      if (peor1->fitness < h1.fitness) {
        *peor1 = h1;
      }

      auto peor2 = std::min_element(poblacion.begin(), poblacion.end());
      if (peor2->fitness < h2.fitness) {
        *peor2 = h2;
      }
    }
  }

  auto mejor = std::max_element(poblacion.begin(), poblacion.end());
  return {mejor->cromosomas, mejor->fitness, evaluaciones};
}

template class AlgoritmoGenetico<double>;
