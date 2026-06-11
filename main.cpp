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

// Valores por defecto; se pueden cambiar con argumentos de línea de comandos:
//   ./main [max_evals] [n_seeds]
int N_SEEDS = 50;
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

// Ejecuta un algoritmo N_SEEDS veces y recoge resultados
template <typename Alg>
vector<RunResult> runAlgorithm(Alg& alg, PortfolioProblem& problem,
                               const vector<long>& seeds) {
  vector<RunResult> results;
  results.reserve(N_SEEDS);

  for (long seed : seeds) {
    Random::seed(seed);
    auto t0 = chrono::steady_clock::now();
    auto res = alg.optimize(problem, MAX_EVALS);
    auto t1 = chrono::steady_clock::now();

    double elapsed = chrono::duration<double>(t1 - t0).count();

    results.push_back(
        {(double)res.fitness, (double)problem.fitnessTest(res.solution),
         problem.beneficioTest(res.solution), res.evaluations, elapsed});
  }
  return results;
}

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
  for (auto& r : v) {
    double d = field(r) - m;
    s += d * d;
  }
  return std::sqrt(s / v.size());
}

void printTable(const string& dataset_name,
                const vector<pair<string, vector<RunResult>>>& alg_results) {
  cout << "\n=== Tabla de resultados: " << dataset_name << " (medias ± desv.típ., "
       << N_SEEDS << " semillas) ===\n";
  // Cabecera: media ± σ para Fit.Train y Fit.2025; resto igual
  cout << left << setw(10) << "Algoritmo"
       << right << setw(22) << "Fit.Train (±σ)"
       << setw(22) << "Fit.2025 (±σ)"
       << setw(12) << "Ben.2025"
       << setw(14) << "Evaluaciones"
       << setw(10) << "Tiempo(s)" << "\n";
  cout << string(90, '-') << "\n";

  for (auto& entry : alg_results) {
    const string& name     = entry.first;
    const vector<RunResult>& res = entry.second;

    auto f_train = [](const RunResult& r) { return r.fitness_train; };
    auto f_test  = [](const RunResult& r) { return r.fitness_test; };
    auto f_ben   = [](const RunResult& r) { return r.beneficio_test; };
    auto f_evals = [](const RunResult& r) { return (double)r.evaluations; };
    auto f_time  = [](const RunResult& r) { return r.time_sec; };

    // "media ±σ" como string para alineación limpia
    auto fmt = [&](auto field) {
      ostringstream os;
      os << fixed << setprecision(4) << mean(res, field)
         << " ±" << setprecision(4) << stddev(res, field);
      return os.str();
    };

    cout << left  << setw(10) << name
         << right << setw(22) << fmt(f_train)
                  << setw(22) << fmt(f_test)
         << fixed << setprecision(4) << setw(12) << mean(res, f_ben)
                  << setprecision(1) << setw(14) << mean(res, f_evals)
                  << setprecision(3) << setw(10) << mean(res, f_time)
         << "\n";
  }
}

// Escribe CSV para el script boxplot.py: columnas alg,fitness
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

int main(int argc, char* argv[]) {
  if (argc > 1) MAX_EVALS = stoi(argv[1]);
  if (argc > 2) N_SEEDS = stoi(argv[2]);

  // Semillas compartidas para todos los algoritmos
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
    cout << "N=" << problem.getSolutionSize() << "  T_train=" << problem.getT()
         << "  MAX_EVALS=" << MAX_EVALS << "\n";

    RandomSearch<double> ralg;
    Greedy<double> greedy;
    LocalSearch<double> ls;
    AlgoritmoGenetico<double> agg_arit(TipoEsquema::GENERACIONAL,
                                       TipoCruce::ARITMETICO);
    AlgoritmoGenetico<double> agg_blx(TipoEsquema::GENERACIONAL,
                                      TipoCruce::BLX);
    AlgoritmoGenetico<double> age_arit(TipoEsquema::ESTACIONARIO,
                                       TipoCruce::ARITMETICO);
    AlgoritmoGenetico<double> age_blx(TipoEsquema::ESTACIONARIO,
                                      TipoCruce::BLX);
    AlgoritmoGenetico<double> am_all(TipoEsquema::GENERACIONAL, TipoCruce::BLX,
                                     TipoMemetico::ALL);
    AlgoritmoGenetico<double> am_rand(TipoEsquema::GENERACIONAL, TipoCruce::BLX,
                                      TipoMemetico::RAND);
    AlgoritmoGenetico<double> am_best(TipoEsquema::GENERACIONAL, TipoCruce::BLX,
                                      TipoMemetico::BEST);
    BMB<double> bmb;
    ILS<double> ils;
    SA<double> sa;
    ILSES<double> ils_es;

    cout << "Ejecutando Random...";
    cout.flush();
    auto res_rand = runAlgorithm(ralg, problem, seeds);
    cout << " OK\n";

    cout << "Ejecutando Greedy...";
    cout.flush();
    auto res_greedy = runAlgorithm(greedy, problem, seeds);
    cout << " OK\n";

    cout << "Ejecutando BL...";
    cout.flush();
    auto res_ls = runAlgorithm(ls, problem, seeds);
    cout << " OK\n";

    cout << "Ejecutando AGG-Arit...";
    cout.flush();
    auto res_agg_arit = runAlgorithm(agg_arit, problem, seeds);
    cout << " OK\n";

    cout << "Ejecutando AGG-BLX...";
    cout.flush();
    auto res_agg_blx = runAlgorithm(agg_blx, problem, seeds);
    cout << " OK\n";

    cout << "Ejecutando AGE-Arit...";
    cout.flush();
    auto res_age_arit = runAlgorithm(age_arit, problem, seeds);
    cout << " OK\n";

    cout << "Ejecutando AGE-BLX...";
    cout.flush();
    auto res_age_blx = runAlgorithm(age_blx, problem, seeds);
    cout << " OK\n";

    cout << "Ejecutando AM-All...";
    cout.flush();
    auto res_am_all = runAlgorithm(am_all, problem, seeds);
    cout << " OK\n";

    cout << "Ejecutando AM-Rand...";
    cout.flush();
    auto res_am_rand = runAlgorithm(am_rand, problem, seeds);
    cout << " OK\n";

    cout << "Ejecutando AM-Best...";
    cout.flush();
    auto res_am_best = runAlgorithm(am_best, problem, seeds);
    cout << " OK\n";

    cout << "Ejecutando BMB...";
    cout.flush();
    auto res_bmb = runAlgorithm(bmb, problem, seeds);
    cout << " OK\n";

    cout << "Ejecutando ILS...";
    cout.flush();
    auto res_ils = runAlgorithm(ils, problem, seeds);
    cout << " OK\n";

    cout << "Ejecutando SA...";
    cout.flush();
    auto res_sa = runAlgorithm(sa, problem, seeds);
    cout << " OK\n";

    cout << "Ejecutando ILS-ES...";
    cout.flush();
    auto res_ils_es = runAlgorithm(ils_es, problem, seeds);
    cout << " OK\n";

    vector<pair<string, vector<RunResult>>> alg_results = {
        {"greedy", res_greedy},   {"random", res_rand},
        {"bl", res_ls},           {"AGG-Arit", res_agg_arit},
        {"AGG-BLX", res_agg_blx}, {"AGE-Arit", res_age_arit},
        {"AGE-BLX", res_age_blx}, {"AM-All", res_am_all},
        {"AM-Rand", res_am_rand}, {"AM-Best", res_am_best},
        {"BMB", res_bmb},         {"ILS", res_ils},
        {"SA", res_sa},           {"ILS-ES", res_ils_es},
    };

    printTable(ds.name, alg_results);
    writeBoxplotCSV("results/all_algorithms/results_" + ds.name + ".csv", alg_results);
  }

  return 0;
}
