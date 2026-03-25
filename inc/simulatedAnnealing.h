#ifndef SIMULATED_ANNEALING_H
#define SIMULATED_ANNEALING_H

#include "../common/mh.h"
#include <algorithm>
#include <cmath>
#include <vector>

template <typename tDomain>
class SimulatedAnnealing : public MH<tDomain> {
private:
  double T0;    // Temperatura inicial
  double alpha; // Factor de enfriamiento (T *= alpha en cada paso)

public:
  // t0 = 0 activa auto-calibración
  SimulatedAnnealing(double t0 = 0.0, double alpha = 0.999)
      : T0(t0), alpha(alpha) {}

  ~SimulatedAnnealing() override = default;

  // Calibra T0 para que el 80% de los movimientos malos se acepten al inicio
  double calibrateT0(Problem<tDomain> &problem, int muestras = 200) {
    int N = problem.getSolutionSize();
    auto sol = problem.createSolution();
    double fit = problem.fitness(sol);

    double suma_deltas = 0.0;
    int n_malos = 0;

    for (int i = 0; i < muestras; ++i) {
      std::vector<int> activas;
      for (int k = 0; k < N; ++k)
        if (sol[k] > 0.0)
          activas.push_back(k);

      int origen = activas[Random::get(0, (int)activas.size() - 1)];
      int destino;
      do {
        destino = Random::get(0, N - 1);
      } while (destino == origen);

      auto vecino = sol;
      double cantidad = sol[origen] * Random::get(0.1, 0.9);
      vecino[origen] -= cantidad;
      vecino[destino] += cantidad;
      problem.fix(vecino);

      double fit_vecino = problem.fitness(vecino);
      double delta = fit_vecino - fit;

      if (delta < 0.0) {
        suma_deltas += std::abs(delta);
        n_malos++;
      }

      // Avanzamos para explorar distintas zonas
      sol = vecino;
      fit = fit_vecino;
    }

    if (n_malos == 0)
      return 0.001;

    double avg_delta = suma_deltas / n_malos;
    // T0 tal que exp(-avg_delta / T0) = 0.8  =>  T0 = -avg_delta / ln(0.8)
    return -avg_delta / std::log(0.8);
  }

  ResultMH<tDomain> optimize(Problem<tDomain> &problem, int maxevals) override {
    int N = problem.getSolutionSize();

    auto sol = problem.createSolution();
    double fit = problem.fitness(sol);
    auto best_sol = sol;
    double best_fit = fit;

    // Si el usuario dejó T0 en 0, auto-calibramos
    double T = (T0 > 0.0) ? T0 : calibrateT0(problem);

    // Construimos la lista de activas una vez y la actualizamos incrementalmente
    std::vector<int> activas;
    for (int k = 0; k < N; ++k)
      if (sol[k] > 0.0)
        activas.push_back(k);

    for (int evals = 1; evals < maxevals; ++evals) {
      // Elegimos origen (empresa con peso) y destino (cualquier otra)
      int origen = activas[Random::get(0, (int)activas.size() - 1)];
      int destino;
      do {
        destino = Random::get(0, N - 1);
      } while (destino == origen);

      // Transferimos una fracción aleatoria (10-90%) del peso del origen
      auto vecino = sol;
      double cantidad = sol[origen] * Random::get(0.1, 0.9);
      vecino[origen] -= cantidad;
      vecino[destino] += cantidad;
      problem.fix(vecino);

      double fit_vecino = problem.fitness(vecino);
      double delta = fit_vecino - fit;

      // Aceptamos siempre si mejora; si empeora, con probabilidad exp(delta/T)
      if (delta > 0.0 || Random::get(0.0, 1.0) < std::exp(delta / T)) {
        // Actualizar activas si destino pasa de 0 a activo, u origen a 0
        bool destino_era_cero = (sol[destino] == 0.0);
        bool origen_queda_cero = (vecino[origen] == 0.0);

        sol = vecino;
        fit = fit_vecino;

        if (destino_era_cero && sol[destino] > 0.0)
          activas.push_back(destino);
        if (origen_queda_cero)
          activas.erase(std::find(activas.begin(), activas.end(), origen));

        if (fit > best_fit) {
          best_fit = fit;
          best_sol = sol;
        }
      }

      T *= alpha;
    }

    return {best_sol, (tFitness)best_fit, (unsigned int)maxevals};
  }
};

#endif
