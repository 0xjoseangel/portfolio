#include <chrono>
#include <fstream>
#include <greedy.h>
#include <iomanip>
#include <iostream>
#include <localsearch.h>
#include <portfolioProblem.h>
#include <problem.h>
#include <random.hpp>
#include <randomSearch.h>
#include <string>
#include <vector>

using namespace std;

// Valores por defecto; se pueden cambiar con argumentos de línea de comandos:
//   ./main [max_evals] [n_seeds]
int N_SEEDS   = 50;
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
vector<RunResult> runAlgorithm(Alg &alg, PortfolioProblem &problem,
                               const vector<long> &seeds) {
  vector<RunResult> results;
  results.reserve(N_SEEDS);

  for (long seed : seeds) {
    Random::seed(seed);
    auto t0 = chrono::steady_clock::now();
    auto res = alg.optimize(problem, MAX_EVALS);
    auto t1 = chrono::steady_clock::now();

    double elapsed =
        chrono::duration<double>(t1 - t0).count();

    results.push_back({(double)res.fitness,
                       (double)problem.fitnessTest(res.solution),
                       problem.beneficioTest(res.solution),
                       res.evaluations,
                       elapsed});
  }
  return results;
}

// Calcula media de un campo extraído con una lambda
template <typename F>
double mean(const vector<RunResult> &v, F field) {
  double s = 0;
  for (auto &r : v) s += field(r);
  return s / v.size();
}

void printTable(const string &dataset_name,
                const vector<pair<string, vector<RunResult>>> &alg_results) {
  cout << "\n=== Tabla de resultados: " << dataset_name << " (medias, "
       << N_SEEDS << " semillas) ===\n";
  cout << left << setw(10) << "Algoritmo"
       << right << setw(12) << "Fit.Train"
       << setw(12) << "Fit.2025"
       << setw(12) << "Ben.2025"
       << setw(14) << "Evaluaciones"
       << setw(10) << "Tiempo(s)" << "\n";
  cout << string(70, '-') << "\n";

  for (auto &[name, res] : alg_results) {
    cout << left  << setw(10) << name
         << right << fixed << setprecision(4)
         << setw(12) << mean(res, [](const RunResult &r){ return r.fitness_train; })
         << setw(12) << mean(res, [](const RunResult &r){ return r.fitness_test; })
         << setw(12) << mean(res, [](const RunResult &r){ return r.beneficio_test; })
         << setw(14) << fixed << setprecision(1)
         << mean(res, [](const RunResult &r){ return (double)r.evaluations; })
         << setw(10) << fixed << setprecision(3)
         << mean(res, [](const RunResult &r){ return r.time_sec; })
         << "\n";
  }
}

// Escribe CSV para el script boxplot.py: columnas alg,fitness
void writeBoxplotCSV(const string &filename,
                     const vector<pair<string, vector<RunResult>>> &alg_results) {
  ofstream f(filename);
  f << "alg,fitness\n";
  for (auto &[name, res] : alg_results)
    for (auto &r : res)
      f << name << "," << fixed << setprecision(6) << r.fitness_train << "\n";
  cout << "CSV boxplot escrito en: " << filename << "\n";
}

int main(int argc, char* argv[]) {
  if (argc > 1) MAX_EVALS = stoi(argv[1]);
  if (argc > 2) N_SEEDS   = stoi(argv[2]);

  // Semillas compartidas para todos los algoritmos
  vector<long> seeds(N_SEEDS);
  for (int i = 0; i < N_SEEDS; ++i) seeds[i] = i + 1;

  vector<DatasetConfig> datasets = {
      {"data/ibex_35.csv",  "ibex_35",  0.005, 0.08},
      {"data/syp_100.csv",  "syp_100",  0.005, 0.05},
      {"data/syp_500.csv",  "syp_500",  0.005, 0.02},
  };

  for (auto &ds : datasets) {
    cout << "\n========================================\n";
    cout << "Dataset: " << ds.name << "\n";
    cout << "========================================\n";

    PortfolioProblem problem(ds.file, ds.lo, ds.hi);
    cout << "N=" << problem.getSolutionSize()
         << "  T_train=" << problem.getT()
         << "  MAX_EVALS=" << MAX_EVALS << "\n";

    RandomSearch<double>    ralg;
    Greedy<double>          greedy;
    LocalSearch<double>     ls;

    cout << "Ejecutando Random...";   cout.flush();
    auto res_rand   = runAlgorithm(ralg,   problem, seeds);
    cout << " OK\n";

    cout << "Ejecutando Greedy...";   cout.flush();
    auto res_greedy = runAlgorithm(greedy, problem, seeds);
    cout << " OK\n";

    cout << "Ejecutando BL...";       cout.flush();
    auto res_ls     = runAlgorithm(ls,     problem, seeds);
    cout << " OK\n";

    vector<pair<string, vector<RunResult>>> alg_results = {
        {"greedy", res_greedy},
        {"random", res_rand},
        {"bl",     res_ls},
    };

    printTable(ds.name, alg_results);
    writeBoxplotCSV("results_" + ds.name + ".csv", alg_results);
  }

  return 0;
}
