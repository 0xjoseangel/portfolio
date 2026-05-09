#include <genetic.h>
#include <portfolioProblem.h>
#include <problem.h>

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

int N_SEEDS = 50;
int MAX_EVALS = 10000;

struct RunResult {
  double fitness_train;
  double fitness_test;
  double beneficio_test;
  double time_sec;
};

struct DatasetConfig {
  string file;
  string name;
  double lo;
  double hi;
};

template <typename Alg>
vector<RunResult> runAlgorithm(Alg& alg, PortfolioProblem& problem,
                               const vector<long>& seeds) {
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
                       chrono::duration<double>(t1 - t0).count()});
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
  for (auto& r : v) { double d = field(r) - m; s += d * d; }
  return std::sqrt(s / v.size());
}

void printStudyTable(const string& dataset_name,
                     const vector<double>& alphas,
                     const vector<vector<RunResult>>& all_results) {
  cout << "\n=== Estudio BLX-alpha: " << dataset_name
       << " (medias ± σ, " << N_SEEDS << " semillas, AGG) ===\n";
  cout << right << setw(8)  << "alpha"
                << setw(22) << "Fit.Train (±σ)"
                << setw(22) << "Fit.2025 (±σ)"
                << setw(12) << "Ben.2025"
                << setw(10) << "Tiempo(s)" << "\n";
  cout << string(74, '-') << "\n";

  auto f_train = [](const RunResult& r) { return r.fitness_train; };
  auto f_test  = [](const RunResult& r) { return r.fitness_test; };
  auto f_ben   = [](const RunResult& r) { return r.beneficio_test; };
  auto f_time  = [](const RunResult& r) { return r.time_sec; };

  auto fmt = [&](const vector<RunResult>& res, auto field) {
    ostringstream os;
    os << fixed << setprecision(4) << mean(res, field)
       << " ±" << setprecision(4) << stddev(res, field);
    return os.str();
  };

  for (size_t i = 0; i < alphas.size(); ++i) {
    const auto& res = all_results[i];
    cout << fixed << setprecision(2) << setw(8)  << alphas[i]
         << right              << setw(22) << fmt(res, f_train)
                               << setw(22) << fmt(res, f_test)
         << setprecision(4)   << setw(12) << mean(res, f_ben)
         << setprecision(3)   << setw(10) << mean(res, f_time)
         << "\n";
  }
}

void writeCSV(const string& filename,
              const vector<double>& alphas,
              const vector<vector<RunResult>>& all_results) {
  ofstream f(filename);
  f << "alpha,fitness_train,fitness_test\n";
  for (size_t i = 0; i < alphas.size(); ++i) {
    for (auto& r : all_results[i]) {
      f << fixed << setprecision(2) << alphas[i] << ","
        << setprecision(6) << r.fitness_train << ","
        << r.fitness_test << "\n";
    }
  }
  cout << "CSV escrito en: " << filename << "\n";
}

int main(int argc, char* argv[]) {
  if (argc > 1) MAX_EVALS = stoi(argv[1]);
  if (argc > 2) N_SEEDS   = stoi(argv[2]);

  const vector<double> alphas = {0.0, 0.1, 0.2, 0.3, 0.5, 0.7, 1.0};

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

    vector<vector<RunResult>> all_results;
    all_results.reserve(alphas.size());

    for (double a : alphas) {
      cout << "  alpha=" << fixed << setprecision(2) << a << "...";
      cout.flush();
      AlgoritmoGenetico<double> ag(TipoEsquema::GENERACIONAL, TipoCruce::BLX,
                                   TipoMemetico::GENETICO, a);
      all_results.push_back(runAlgorithm(ag, problem, seeds));
      cout << " OK\n";
    }

    printStudyTable(ds.name, alphas, all_results);
    writeCSV("blx_alpha_" + ds.name + ".csv", alphas, all_results);
  }

  return 0;
}
