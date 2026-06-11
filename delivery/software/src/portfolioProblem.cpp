#include "../inc/portfolioProblem.h"

#include <fstream>
#include <iostream>
#include <numeric>
#include <sstream>

#include "solution.h"
// --- CONSTRUCTOR ---
PortfolioProblem::PortfolioProblem(const std::string& filename, double min_val,
                                   double max_val) {
  this->lo = min_val;
  this->hi = max_val;
  this->lambda = 500.0;  // El guion exige lambda = 500

  loadData(filename);

  precomputeMatrices();
}

void PortfolioProblem::loadData(const std::string& filename) {
  std::ifstream file(filename);

  // Comprobamos si el archivo se ha abierto correctamente
  if (!file.is_open()) {
    throw std::runtime_error("Error: No se pudo abrir el archivo " + filename);
  }

  std::string line;
  std::string token;

  // Saltamos la primera linea
  if (std::getline(file, line)) {
    // No hacemos nada con ella
  }

  // Leemos línea a línea el resto del archivo
  while (std::getline(file, line)) {
    std::vector<double> daily_returns;
    std::stringstream ss(line);

    // Leemos la fecha para separar train (2015-2024) de test (2025)
    std::string fecha;
    std::getline(ss, fecha, ',');
    bool is_test = (fecha.size() >= 4 && fecha.substr(0, 4) == "2025");

    // Leemos el resto de columnas de esa fila (los valores de las empresas)
    while (std::getline(ss, token, ',')) {
      try {
        daily_returns.push_back(std::stod(token));
      } catch (const std::invalid_argument& e) {
        daily_returns.push_back(0.0);
      }
    }

    if (is_test)
      returns_test.push_back(daily_returns);
    else
      returns.push_back(daily_returns);
  }

  file.close();

  if (!returns.empty()) {
    T = returns.size();
    N = returns[0].size();
    T_test = returns_test.size();
  } else {
    T = 0;
    N = 0;
    T_test = 0;
    std::cerr << "Advertencia: El archivo " << filename << " estaba vacío."
              << std::endl;
  }
}

void PortfolioProblem::precomputeMatrices() {
  // --- TRAIN (2015-2024) ---
  means.assign(N, 0.0);
  benefit_factors.assign(N, 0.0);
  covariance.assign(N, std::vector<double>(N, 0.0));

  for (int i = 0; i < N; ++i) {
    double sum_mu = 0.0, sum_log = 0.0;
    for (int t = 0; t < T; ++t) {
      sum_mu += returns[t][i];
      sum_log += std::log(1.0 + returns[t][i]);
    }
    means[i] = sum_mu / T;

    // El nuevo guion lo calcula sin dividirlo entre T
    benefit_factors[i] = sum_log;
  }

  for (int i = 0; i < N; ++i)
    for (int j = 0; j < N; ++j) {
      double cov_sum = 0.0;
      for (int t = 0; t < T; ++t)
        cov_sum += (returns[t][i] - means[i]) * (returns[t][j] - means[j]);
      covariance[i][j] = cov_sum / T;
    }

  // --- TEST (2025) ---
  means_test.assign(N, 0.0);
  benefit_factors_test.assign(N, 0.0);
  covariance_test.assign(N, std::vector<double>(N, 0.0));

  if (T_test == 0) return;

  for (int i = 0; i < N; ++i) {
    double sum_mu = 0.0, sum_log = 0.0;
    for (int t = 0; t < T_test; ++t) {
      sum_mu += returns_test[t][i];
      sum_log += std::log(1.0 + returns_test[t][i]);
    }
    means_test[i] = sum_mu / T_test;
    // El nuevo guion no lo divide entre T
    benefit_factors_test[i] = sum_log;
  }

  for (int i = 0; i < N; ++i)
    for (int j = 0; j < N; ++j) {
      double cov_sum = 0.0;
      for (int t = 0; t < T_test; ++t)
        cov_sum += (returns_test[t][i] - means_test[i]) *
                   (returns_test[t][j] - means_test[j]);
      covariance_test[i][j] = cov_sum / T_test;
    }
}

tSolution<double> PortfolioProblem::createSolution() {
  tSolution<double> sol(N, 0.0);

  int num_ceros = Random::get(1, N - 1);
  int num_empresas_inversion = N - num_ceros;

  // Para elegir a qué empresas invertimos de forma aleatoria,
  // creamos un vector con los índices [0, 1, 2, ..., N-1] y lo barajamos.
  std::vector<int> indices(N);
  std::iota(indices.begin(), indices.end(), 0);  // Lo llena de 0 a N-1
  Random::shuffle(indices);

  // Para el resto de valores asignar un valor aleatorio entre lo y hi
  double suma = 0.0;
  for (int i = 0; i < num_empresas_inversion; ++i) {
    int empresa_idx = indices[i];
    double peso_aleatorio = Random::get(lo, hi);
    sol[empresa_idx] = peso_aleatorio;
    suma += peso_aleatorio;
  }

  // Normalizamos
  for (int i = 0; i < N; ++i) {
    if (sol[i] > 0.0) {
      sol[i] = sol[i] / suma;
    }
  }

  // Reparar la solución por si al normalizar se ha salido de los límites
  fix(sol);

  return sol;
}

// 3.4 del guion
void PortfolioProblem::fix(tSolution<double>& solution) {
  const double epsilon = 1e-8;  // Error

  // Normalizar la solución inicial. No pasa nada
  // si normalizamos dos veces
  double sum = 0.0;
  for (int i = 0; i < N; ++i) {
    sum += solution[i];
  }

  if (sum > 0.0) {
    for (int i = 0; i < N; ++i) {
      solution[i] /= sum;
    }
  }

  // Asegurarse de que esté en el rango
  // w_i = 0 si w_i < lo, y w_i = hi si w_i > hi
  for (int i = 0; i < N; ++i) {
    if (solution[i] > 0.0 && solution[i] < lo) {
      solution[i] = 0.0;
    } else if (solution[i] > hi) {
      solution[i] = hi;
    }
  }

  // Recalculamos la suma tras el primer recorte
  sum = 0.0;
  for (int i = 0; i < N; ++i) sum += solution[i];

  int max_iters = 1000;  // para evitar bucles infinitos
  int iter = 0;

  // Mientras la suma no sea 1 (teniendo en cuenta el margen de error epsilon)
  while (std::abs(1.0 - sum) > epsilon && iter < max_iters) {
    double diff = 1.0 - sum;
    iter++;

    if (diff > 0) {
      // falta por invertir dinero
      std::vector<int> candidatos;

      // Buscamos empresas que ya tienen dinero (> 0) pero aún no han llegado al
      // tope (< hi)
      for (int i = 0; i < N; ++i) {
        if (solution[i] > 0.0 && solution[i] < hi) {
          candidatos.push_back(i);
        }
      }

      if (!candidatos.empty()) {
        // Dividimos la diferencia entre esos pesos sumándole un valor
        double a_repartir = diff / candidatos.size();
        for (int idx : candidatos) {
          solution[idx] += a_repartir;
          // Mantenemos el rango: si al sumarle se pasa del máximo, lo capamos a
          // 'hi'
          if (solution[idx] > hi) solution[idx] = hi;
        }
      } else {
        // Si no hay candidatos suficientes, incrementamos pesos iguales a 0 de
        // forma aleatoria
        std::vector<int> empresas_a_cero;
        for (int i = 0; i < N; ++i) {
          if (solution[i] == 0.0) empresas_a_cero.push_back(i);
        }

        if (!empresas_a_cero.empty()) {
          // Barajamos y cogemos una al azar para darle el empujón inicial
          Random::shuffle(empresas_a_cero);
          solution[empresas_a_cero[0]] =
              lo;  // La encendemos poniéndola al mínimo 'lo'
        } else {
          break;  // No se puede arreglar más (estamos al límite matemático)
        }
      }

    } else {
      // La suma es mayor que 1 (Nos hemos pasado invirtiendo)
      std::vector<int> candidatos;

      // Buscamos empresas que tienen dinero invertido mayor que el mínimo (>
      // lo)
      for (int i = 0; i < N; ++i) {
        if (solution[i] > lo) {
          candidatos.push_back(i);
        }
      }

      if (!candidatos.empty()) {
        // Dividimos la diferencia (que aquí es negativa) restándole un valor
        double a_quitar = (-diff) / candidatos.size();
        for (int idx : candidatos) {
          solution[idx] -= a_quitar;
          // Mantenemos el rango: si al restarle baja del mínimo, lo capamos a
          // 'lo'
          if (solution[idx] < lo) solution[idx] = lo;
        }
      } else {
        break;  // No se puede quitar más sin saltarse los límites
      }
    }

    // Recalculamos la suma para la siguiente comprobación del bucle while
    sum = 0.0;
    for (int i = 0; i < N; ++i) sum += solution[i];
  }
}

bool PortfolioProblem::isValid(const tSolution<double>& solution) {
  const double epsilon = 1e-8;
  double sum = 0.0;

  // Comprobamos los rangos
  for (int i = 0; i < N; ++i) {
    double peso = solution[i];
    sum += peso;

    if (peso != 0.0) {
      if (peso < lo || peso > hi) {
        return false;
      }
    }
  }

  if (std::abs(1.0 - sum) > epsilon) {
    return false;  // Falla por la suma
  }

  return true;
}

size_t PortfolioProblem::getSolutionSize() { return N; }

std::pair<double, double> PortfolioProblem::getSolutionDomainRange() {
  return {lo, hi};
}

tFitness PortfolioProblem::fitness(const tSolution<double>& solution) {
  double beneficio = 0.0, riesgo = 0.0;
  for (int i = 0; i < N; ++i) beneficio += solution[i] * benefit_factors[i];
  for (int i = 0; i < N; ++i)
    for (int j = 0; j < N; ++j)
      riesgo += solution[i] * covariance[i][j] * solution[j];
  return beneficio - (lambda * sqrt(riesgo));
}

double PortfolioProblem::beneficioTest(const tSolution<double>& solution) {
  double beneficio = 0.0;
  for (int i = 0; i < N; ++i)
    beneficio += solution[i] * benefit_factors_test[i];
  return beneficio;
}

tFitness PortfolioProblem::fitnessTest(const tSolution<double>& solution) {
  double riesgo = 0.0;
  for (int i = 0; i < N; ++i)
    for (int j = 0; j < N; ++j)
      riesgo += solution[i] * covariance_test[i][j] * solution[j];
  return beneficioTest(solution) - (lambda * sqrt(riesgo));
}
