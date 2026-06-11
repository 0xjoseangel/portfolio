#include <portfolioProblem.h>
#include <problem.h>
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
  double evals;
  double time_sec;
};

struct DatasetConfig {
  string file;
  string name;
  double lo;
  double hi;
};

// ── Helpers estadísticos ─────────────────────────────────────────────────────

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

// ── Ejecución ─────────────────────────────────────────────────────────────────

vector<RunResult> runSA(SA<double>& sa, PortfolioProblem& problem,
                        const vector<long>& seeds) {
  vector<RunResult> results;
  results.reserve(seeds.size());
  for (long seed : seeds) {
    Random::seed(seed);
    auto t0 = chrono::steady_clock::now();
    auto res = sa.optimize(problem, MAX_EVALS);
    auto t1 = chrono::steady_clock::now();
    results.push_back({(double)res.fitness,
                       (double)problem.fitnessTest(res.solution),
                       problem.beneficioTest(res.solution),
                       (double)res.evaluations,
                       chrono::duration<double>(t1 - t0).count()});
  }
  return results;
}

// ── Impresión ─────────────────────────────────────────────────────────────────

template <typename Label>
void printStudyTable(const string& dataset_name, const string& param_name,
                     const vector<Label>& labels,
                     const vector<vector<RunResult>>& all_results) {
  cout << "\n=== Estudio SA [" << param_name << "]: " << dataset_name
       << " (medias ± σ, " << N_SEEDS << " semillas) ===\n";
  cout << right
       << setw(10) << param_name
       << setw(22) << "Fit.Train (±σ)"
       << setw(22) << "Fit.2025 (±σ)"
       << setw(12) << "Ben.2025"
       << setw(12) << "Evals"
       << setw(10) << "Tiempo(s)" << "\n";
  cout << string(88, '-') << "\n";

  auto f_train = [](const RunResult& r) { return r.fitness_train; };
  auto f_test  = [](const RunResult& r) { return r.fitness_test; };
  auto f_ben   = [](const RunResult& r) { return r.beneficio_test; };
  auto f_evals = [](const RunResult& r) { return r.evals; };
  auto f_time  = [](const RunResult& r) { return r.time_sec; };

  auto fmt = [&](const vector<RunResult>& res, auto field) {
    ostringstream os;
    os << fixed << setprecision(4) << mean(res, field)
       << " ±" << setprecision(4) << stddev(res, field);
    return os.str();
  };

  for (size_t i = 0; i < labels.size(); ++i) {
    const auto& res = all_results[i];
    cout << fixed << setprecision(2) << setw(10) << labels[i]
         << right
         << setw(22) << fmt(res, f_train)
         << setw(22) << fmt(res, f_test)
         << setprecision(4) << setw(12) << mean(res, f_ben)
         << setprecision(1) << setw(12) << mean(res, f_evals)
         << setprecision(3) << setw(10) << mean(res, f_time)
         << "\n";
  }
}

template <typename Label>
void writeCSV(const string& filename, const string& param_name,
              const vector<Label>& labels,
              const vector<vector<RunResult>>& all_results) {
  ofstream f(filename);
  f << param_name << ",fitness_train,fitness_test,beneficio_test,evals\n";
  for (size_t i = 0; i < labels.size(); ++i) {
    for (auto& r : all_results[i]) {
      f << fixed << setprecision(4) << labels[i] << ","
        << setprecision(6) << r.fitness_train << ","
        << r.fitness_test << ","
        << r.beneficio_test << ","
        << r.evals << "\n";
    }
  }
  cout << "CSV escrito en: " << filename << "\n";
}

// ── Main ──────────────────────────────────────────────────────────────────────

int main(int argc, char* argv[]) {
  if (argc > 1) MAX_EVALS = stoi(argv[1]);
  if (argc > 2) N_SEEDS   = stoi(argv[2]);

  // Parámetros a estudiar
  // phi: probabilidad de aceptar peor solución al inicio → controla T0
  const vector<double> phis      = {0.05, 0.1, 0.2, 0.3, 0.5, 0.7, 0.9};
  // k_vecinos: max_vecinos = k * N → granularidad del enfriamiento
  const vector<int>    k_vecinos = {2, 5, 10, 15, 20, 30};

  // Valores por defecto para parámetros no estudiados
  const double MU_DEF  = 0.2;
  const double TF_DEF  = 1e-4;
  const int    KE_DEF  = 1;
  const double PHI_DEF = 0.3;
  const int    KV_DEF  = 10;

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

    // ── Estudio 1: phi ───────────────────────────────────────────────────────
    {
      cout << "\n[Estudio phi] mu=" << MU_DEF << " Tf=" << TF_DEF
           << " k_vec=" << KV_DEF << " k_exit=" << KE_DEF << "\n";

      vector<vector<RunResult>> results;
      results.reserve(phis.size());
      for (double phi : phis) {
        cout << "  phi=" << fixed << setprecision(2) << phi << "... ";
        cout.flush();
        SA<double> sa(phi, MU_DEF, TF_DEF, KV_DEF, KE_DEF);
        results.push_back(runSA(sa, problem, seeds));
        cout << "OK\n";
      }

      printStudyTable(ds.name, "phi", phis, results);
      writeCSV("results/studies/sa_phi_" + ds.name + ".csv", "phi", phis, results);
    }

    // ── Estudio 2: k_vecinos ─────────────────────────────────────────────────
    {
      cout << "\n[Estudio k_vecinos] phi=" << PHI_DEF << " mu=" << MU_DEF
           << " Tf=" << TF_DEF << " k_exit=" << KE_DEF << "\n";

      vector<vector<RunResult>> results;
      results.reserve(k_vecinos.size());
      for (int kv : k_vecinos) {
        cout << "  k_vecinos=" << kv << "... ";
        cout.flush();
        SA<double> sa(PHI_DEF, MU_DEF, TF_DEF, kv, KE_DEF);
        results.push_back(runSA(sa, problem, seeds));
        cout << "OK\n";
      }

      printStudyTable(ds.name, "k_vecinos", k_vecinos, results);
      writeCSV("results/studies/sa_kvecinos_" + ds.name + ".csv", "k_vecinos", k_vecinos, results);
    }
  }

  return 0;
}
