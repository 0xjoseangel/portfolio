// main_p3.cpp — Ejecutable de comparación para la Práctica 3.
//
// Algoritmos incluidos (según guion P3, sección 6):
//   · Greedy, Random, BL           → referencias de la Práctica 1
//   · AM-Best                      → mejor algoritmo de la Práctica 2
//   · BMB, ILS, SA, ILS-ES         → algoritmos nuevos de la Práctica 3
//
// Justificación de AM-Best como "Mejor_P2":
//   En los tres datasets AM-Best obtiene el mejor fitness de entrenamiento
//   y la menor desviación típica de todos los algoritmos de P2, siendo
//   el único que supera consistentemente a la BL en los tres casos.

#include <bmb.h>
#include <genetic.h>
#include <greedy.h>
#include <ils.h>
#include <ils_es.h>
#include <localsearch.h>
#include <portfolioProblem.h>
#include <problem.h>
#include <randomSearch.h>
#include <sa.h>

#include <chrono>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random.hpp>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

int N_SEEDS  = 50;
int MAX_EVALS = 10000;

struct RunResult {
  double fitness_train;
  double fitness_test;
  double beneficio_test;
  unsigned int evaluations;
  double time_sec;
};

struct DatasetConfig {
  string file;
  string name;
  double lo;
  double hi;
};

// ── Ejecución ─────────────────────────────────────────────────────────────────

template <typename Alg>
vector<RunResult> runAlgorithm(const string& label, Alg& alg,
                               PortfolioProblem& problem,
                               const vector<long>& seeds) {
  cout << "Ejecutando " << label << "... ";
  cout.flush();

  vector<RunResult> results;
  results.reserve(seeds.size());

  for (long seed : seeds) {
    Random::seed(seed);
    auto t0 = chrono::steady_clock::now();
    auto res = alg.optimize(problem, MAX_EVALS);
    auto t1 = chrono::steady_clock::now();

    results.push_back({(double)res.fitness,
                       (double)problem.fitnessTest(res.solution),
                       problem.beneficioTest(res.solution),
                       res.evaluations,
                       chrono::duration<double>(t1 - t0).count()});
  }
  cout << "OK\n";
  return results;
}

// ── Estadísticos ──────────────────────────────────────────────────────────────

template <typename F>
double mean(const vector<RunResult>& v, F field) {
  double s = 0;
  for (auto& r : v) s += field(r);
  return s / v.size();
}

template <typename F>
double stddev(const vector<RunResult>& v, F field) {
  double m = mean(v, field);
  double s = 0;
  for (auto& r : v) { double d = field(r) - m; s += d * d; }
  return std::sqrt(s / v.size());
}

// ── Salida ────────────────────────────────────────────────────────────────────

void printTable(const string& dataset_name,
                const vector<pair<string, vector<RunResult>>>& alg_results) {
  cout << "\n=== Tabla P3: " << dataset_name
       << " (medias ± σ, " << N_SEEDS << " semillas) ===\n";
  cout << left  << setw(12) << "Algoritmo"
       << right << setw(22) << "Fit.Train (±σ)"
                << setw(22) << "Fit.2025 (±σ)"
                << setw(12) << "Ben.2025"
                << setw(14) << "Evaluaciones"
                << setw(10) << "Tiempo(s)" << "\n";
  cout << string(92, '-') << "\n";

  auto f_train = [](const RunResult& r) { return r.fitness_train; };
  auto f_test  = [](const RunResult& r) { return r.fitness_test; };
  auto f_ben   = [](const RunResult& r) { return r.beneficio_test; };
  auto f_evals = [](const RunResult& r) { return (double)r.evaluations; };
  auto f_time  = [](const RunResult& r) { return r.time_sec; };

  auto fmt = [&](const vector<RunResult>& res, auto field) {
    ostringstream os;
    os << fixed << setprecision(4) << mean(res, field)
       << " ±" << setprecision(4) << stddev(res, field);
    return os.str();
  };

  // Separador visual entre grupos de prácticas
  auto separator = [&](const string& label) {
    cout << "  -- " << label << " --\n";
  };

  for (size_t i = 0; i < alg_results.size(); ++i) {
    // Insertar separadores entre grupos
    if (i == 3) separator("Mejor P2");
    if (i == 4) separator("P3 nuevos");

    const auto& [name, res] = alg_results[i];
    cout << left  << setw(12) << name
         << right << setw(22) << fmt(res, f_train)
                  << setw(22) << fmt(res, f_test)
         << fixed << setprecision(4) << setw(12) << mean(res, f_ben)
                  << setprecision(1) << setw(14) << mean(res, f_evals)
                  << setprecision(3) << setw(10) << mean(res, f_time)
         << "\n";
  }
}

void writeBoxplotCSV(
    const string& filename,
    const vector<pair<string, vector<RunResult>>>& alg_results) {
  ofstream f(filename);
  f << "alg,fitness\n";
  for (auto& [name, res] : alg_results)
    for (auto& r : res)
      f << name << "," << fixed << setprecision(6) << r.fitness_train << "\n";
  cout << "CSV boxplot escrito en: " << filename << "\n";
}

// Escribe CSV de estadísticas para la tabla de la memoria:
//   alg, mean_train, std_train, mean_test, std_test, mean_ben, mean_evals, mean_time
void writeStatsCSV(
    const string& filename,
    const vector<pair<string, vector<RunResult>>>& alg_results) {
  auto f_train = [](const RunResult& r) { return r.fitness_train; };
  auto f_test  = [](const RunResult& r) { return r.fitness_test; };
  auto f_ben   = [](const RunResult& r) { return r.beneficio_test; };
  auto f_evals = [](const RunResult& r) { return (double)r.evaluations; };
  auto f_time  = [](const RunResult& r) { return r.time_sec; };

  ofstream f(filename);
  f << "alg,mean_train,std_train,mean_test,std_test,mean_ben,mean_evals,mean_time\n";
  for (auto& [name, res] : alg_results) {
    f << name << ","
      << fixed << setprecision(6)
      << mean(res, f_train) << ","
      << stddev(res, f_train) << ","
      << mean(res, f_test)  << ","
      << stddev(res, f_test) << ","
      << mean(res, f_ben)   << ","
      << setprecision(1) << mean(res, f_evals) << ","
      << setprecision(4) << mean(res, f_time)  << "\n";
  }
  cout << "CSV estadísticas escrito en: " << filename << "\n";
}

// ── Main ──────────────────────────────────────────────────────────────────────

int main(int argc, char* argv[]) {
  if (argc > 1) MAX_EVALS = stoi(argv[1]);
  if (argc > 2) N_SEEDS   = stoi(argv[2]);

  vector<long> seeds(N_SEEDS);
  for (int i = 0; i < N_SEEDS; ++i) seeds[i] = i + 1;

  vector<DatasetConfig> datasets = {
      {"data/ibex_35.csv", "ibex_35", 0.005, 0.08},
      {"data/syp_100.csv", "syp_100", 0.005, 0.05},
      {"data/syp_500.csv", "syp_500", 0.005, 0.02},
  };

  for (auto& ds : datasets) {
    cout << "\n========================================\n";
    cout << "Dataset: " << ds.name << "\n";
    cout << "========================================\n";

    PortfolioProblem problem(ds.file, ds.lo, ds.hi);
    cout << "N=" << problem.getSolutionSize()
         << "  T_train=" << problem.getT()
         << "  MAX_EVALS=" << MAX_EVALS << "\n\n";

    // ── Práctica 1: referencias ───────────────────────────────────────────────
    Greedy<double>       greedy;
    RandomSearch<double> random_alg;
    LocalSearch<double>  bl;

    // ── Práctica 2: mejor algoritmo (AM-Best, AGG-BLX base) ──────────────────
    AlgoritmoGenetico<double> am_best(TipoEsquema::GENERACIONAL,
                                      TipoCruce::BLX,
                                      TipoMemetico::BEST);

    // ── Práctica 3: algoritmos nuevos ─────────────────────────────────────────
    BMB<double>   bmb;
    ILS<double>   ils;
    SA<double>    sa;
    ILSES<double> ils_es;

    auto res_greedy  = runAlgorithm("Greedy",   greedy,    problem, seeds);
    auto res_random  = runAlgorithm("Random",   random_alg, problem, seeds);
    auto res_bl      = runAlgorithm("BL",       bl,        problem, seeds);
    auto res_am_best = runAlgorithm("AM-Best",  am_best,   problem, seeds);
    auto res_bmb     = runAlgorithm("BMB",      bmb,       problem, seeds);
    auto res_ils     = runAlgorithm("ILS",      ils,       problem, seeds);
    auto res_sa      = runAlgorithm("SA",       sa,        problem, seeds);
    auto res_ils_es  = runAlgorithm("ILS-ES",   ils_es,    problem, seeds);

    // Orden idéntico al de la tabla del guion P3 (sección 6)
    vector<pair<string, vector<RunResult>>> alg_results = {
        {"Greedy",  res_greedy},
        {"Random",  res_random},
        {"BL",      res_bl},
        {"AM-Best", res_am_best},   // Mejor_P2
        {"BMB",     res_bmb},
        {"ILS",     res_ils},
        {"SA",      res_sa},
        {"ILS-ES",  res_ils_es},
    };

    printTable(ds.name, alg_results);
    writeBoxplotCSV("p3_results_" + ds.name + ".csv", alg_results);
    writeStatsCSV("p3_stats_" + ds.name + ".csv", alg_results);
  }

  return 0;
}
