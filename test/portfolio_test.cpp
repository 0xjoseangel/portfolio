/**
 * portfolio_test.cpp
 *
 * Tests de caja blanca para PortfolioProblem centrados en las tres
 * reglas del enunciado:
 *   1. isValid  — restricciones de suma y rango
 *   2. fix      — reparación de pesos
 *   3. fitness  — fórmula Beneficio - lambda * Riesgo
 *
 * Parámetros del fixture:
 *   dataset : ibex_35.csv  (N = 30)
 *   lo = 0.01,  hi = 0.50
 *   Con estos valores  N * hi = 15 >> 1,  lo que garantiza que
 *   siempre existe margen para que fix() converja.
 */

#include <gtest/gtest.h>
#include <portfolioProblem.h>
#include <random.hpp>

#include <cmath>
#include <numeric>

#ifndef DATA_DIR
#define DATA_DIR "data"
#endif

static const std::string IBEX = std::string(DATA_DIR) + "/ibex_35.csv";
static const double LO        = 0.01;
static const double HI        = 0.50;
static const double EPS_VALID = 1e-8;   // epsilon usado en isValid

// ── Fixture ────────────────────────────────────────────────────────────────
class PortfolioTest : public ::testing::Test {
protected:
  void SetUp() override {
    Random::seed(0);
    prob = std::make_unique<PortfolioProblem>(IBEX, LO, HI);
    N    = static_cast<int>(prob->getSolutionSize());
  }

  // Devuelve un vector de N ceros.
  tSolution<double> zeros() { return tSolution<double>(N, 0.0); }

  std::unique_ptr<PortfolioProblem> prob;
  int N;
};

// ═══════════════════════════════════════════════════════════════════════════
//  1. isValid
// ═══════════════════════════════════════════════════════════════════════════

// Solución correcta: suma == 1, todos los pesos en [lo, hi] o == 0.
TEST_F(PortfolioTest, IsValid_AcceptsValidSolution) {
  auto sol  = zeros();
  sol[0]    = 0.30;
  sol[1]    = 0.30;
  sol[2]    = 0.40;
  // suma = 1.0, todos en [0.01, 0.50] ✓
  EXPECT_TRUE(prob->isValid(sol));
}

// Un peso no nulo inferior a lo → inválido aunque la suma sea 1.
TEST_F(PortfolioTest, IsValid_ReturnsFalse_WeightBelowLo) {
  auto sol  = zeros();
  sol[0]    = 1.0 - LO / 2.0;   // 0.995
  sol[1]    = LO / 2.0;          // 0.005  < lo → inválido
  // suma = 1.0, pero sol[1] < lo
  EXPECT_DOUBLE_EQ(sol[0] + sol[1], 1.0);
  EXPECT_FALSE(prob->isValid(sol));
}

// Un peso superior a hi → inválido aunque la suma sea 1.
TEST_F(PortfolioTest, IsValid_ReturnsFalse_WeightAboveHi) {
  auto sol  = zeros();
  sol[0]    = HI + 0.1;          // 0.60 > hi = 0.50
  sol[1]    = 1.0 - (HI + 0.1); // 0.40
  // suma = 1.0, pero sol[0] > hi
  EXPECT_DOUBLE_EQ(sol[0] + sol[1], 1.0);
  EXPECT_FALSE(prob->isValid(sol));
}

// Suma = 0.99: la diferencia con 1 es 0.01 >> eps → inválido.
TEST_F(PortfolioTest, IsValid_ReturnsFalse_SumIs0_99) {
  auto sol  = zeros();
  sol[0]    = 0.495;   // en [lo, hi] ✓
  sol[1]    = 0.495;   // en [lo, hi] ✓
  // suma = 0.99  ≠ 1
  EXPECT_NEAR(sol[0] + sol[1], 0.99, 1e-12);
  EXPECT_FALSE(prob->isValid(sol));
}

// Suma = 1.01: la diferencia con 1 es 0.01 >> eps → inválido.
TEST_F(PortfolioTest, IsValid_ReturnsFalse_SumIs1_01) {
  auto sol  = zeros();
  sol[0]    = 0.40;
  sol[1]    = 0.40;
  sol[2]    = 0.21;
  // suma = 1.01, todos en [lo, hi] ✓
  EXPECT_NEAR(sol[0] + sol[1] + sol[2], 1.01, 1e-12);
  EXPECT_FALSE(prob->isValid(sol));
}

// Epsilon de frontera: diferencia exactamente eps → todavía válido.
TEST_F(PortfolioTest, IsValid_AcceptsSumWithinEpsilon) {
  auto sol  = zeros();
  // Construimos una suma = 1 + eps/2, que está dentro del margen
  double w  = 1.0 / 3.0;
  sol[0]    = w;
  sol[1]    = w;
  sol[2]    = 1.0 - 2.0 * w;     // 1/3 exacto en double → suma puede diferir
  // Lo que importa: isValid debe ser coherente con su propio epsilon
  double sum = sol[0] + sol[1] + sol[2];
  bool expected = (std::abs(sum - 1.0) <= EPS_VALID) &&
                  sol[0] >= LO && sol[0] <= HI &&
                  sol[1] >= LO && sol[1] <= HI &&
                  sol[2] >= LO && sol[2] <= HI;
  EXPECT_EQ(prob->isValid(sol), expected);
}

// ═══════════════════════════════════════════════════════════════════════════
//  2. fix
// ═══════════════════════════════════════════════════════════════════════════

// Comprobaciones comunes tras llamar a fix()
static void assertFixed(PortfolioProblem& p,
                         tSolution<double>& sol,
                         double lo, double hi) {
  double sum = std::accumulate(sol.begin(), sol.end(), 0.0);
  EXPECT_NEAR(sum, 1.0, 1e-8) << "La suma debe ser 1 tras fix()";

  for (size_t i = 0; i < sol.size(); ++i) {
    double w = sol[i];
    EXPECT_TRUE(w == 0.0 || (w >= lo && w <= hi))
        << "Peso [" << i << "] = " << w
        << " fuera de rango [" << lo << ", " << hi << "]";
  }
  EXPECT_TRUE(p.isValid(sol)) << "isValid debe devolver true tras fix()";
}

// Vector que suma menos de 1: fix() debe completar hasta 1.
TEST_F(PortfolioTest, Fix_SumLessThanOne) {
  auto sol = zeros();
  sol[0]   = 0.20;   // suma = 0.60 < 1
  sol[1]   = 0.20;
  sol[2]   = 0.20;

  ASSERT_LT(sol[0] + sol[1] + sol[2], 1.0);
  prob->fix(sol);
  assertFixed(*prob, sol, LO, HI);
}

// Vector que suma más de 1: fix() debe reducir hasta 1.
TEST_F(PortfolioTest, Fix_SumGreaterThanOne) {
  auto sol = zeros();
  sol[0]   = 0.40;   // suma = 1.20 > 1
  sol[1]   = 0.40;
  sol[2]   = 0.40;

  ASSERT_GT(sol[0] + sol[1] + sol[2], 1.0);
  prob->fix(sol);
  assertFixed(*prob, sol, LO, HI);
}

// Vector con valores fuera de rango: fix() debe recortarlos y reajustar.
// sol[0] < lo → debe zeroearse tras normalización.
// sol[1] > hi → debe caparse a hi.
TEST_F(PortfolioTest, Fix_ValuesOutOfRange) {
  auto sol = zeros();
  sol[0]   = LO / 5.0;   // muy pequeño, < lo
  sol[1]   = 2.0;        // muy grande, > hi

  ASSERT_FALSE(prob->isValid(sol));
  prob->fix(sol);
  assertFixed(*prob, sol, LO, HI);
}

// Un vector que ya es válido no debe modificarse.
TEST_F(PortfolioTest, Fix_DoesNotModifyValidSolution) {
  auto sol  = zeros();
  sol[0]    = 0.30;
  sol[1]    = 0.30;
  sol[2]    = 0.40;

  ASSERT_TRUE(prob->isValid(sol));
  tSolution<double> before = sol;
  prob->fix(sol);

  for (int i = 0; i < N; ++i)
    EXPECT_NEAR(sol[i], before[i], 1e-9) << "Indice " << i;
}

// fix() sobre createSolution() siempre produce soluciones válidas.
TEST_F(PortfolioTest, Fix_CreateSolutionAlwaysValidAfterFix) {
  for (int trial = 0; trial < 10; ++trial) {
    auto sol = prob->createSolution();
    prob->fix(sol);
    EXPECT_TRUE(prob->isValid(sol)) << "Trial " << trial;
  }
}

// ═══════════════════════════════════════════════════════════════════════════
//  3. fitness
// ═══════════════════════════════════════════════════════════════════════════

// Lambda fijado en 500 según el enunciado.
TEST_F(PortfolioTest, Fitness_LambdaIs500) {
  EXPECT_DOUBLE_EQ(prob->getLambda(), 500.0);
}

// Solución todo-ceros → Beneficio = 0, Riesgo = 0 → fitness = 0.
TEST_F(PortfolioTest, Fitness_ZeroSolutionIsZero) {
  auto sol = zeros();
  EXPECT_FLOAT_EQ(prob->fitness(sol), 0.0f);
}

// La función es determinista: mismo input → mismo output.
TEST_F(PortfolioTest, Fitness_IsDeterministic) {
  auto sol = prob->createSolution();
  EXPECT_FLOAT_EQ(prob->fitness(sol), prob->fitness(sol));
}

// Verificación estructural de la fórmula usando la descomposición
// fitness = Beneficio - 500 * Riesgo.
// Si escalamos una solución válida s a 2s (inválida pero útil para
// comprobar la cuadratura del riesgo), el riesgo cuadruplica y el
// beneficio se duplica, así que el fitness debe seguir la misma relación.
TEST_F(PortfolioTest, Fitness_RiesgoEsTerminoCuadratico) {
  auto sol = prob->createSolution();

  // Construimos 2*sol (inválida, pero sirve para la comprobación)
  auto sol2 = sol;
  for (auto& w : sol2) w *= 2.0;

  double f1 = prob->fitness(sol);
  double f2 = prob->fitness(sol2);

  // fitness(s)  = B - 500*R
  // fitness(2s) = 2B - 500*4R = 2B - 2000R
  // => 2*fitness(s) - fitness(2s) = 2B - 1000R - 2B + 2000R = 1000R >= 0
  // (el riesgo R siempre es no negativo por ser forma cuadrática semidefinida)
  double lhs = 2.0 * static_cast<double>(f1) - static_cast<double>(f2);
  EXPECT_GE(lhs, 0.0)
      << "El término de riesgo debe ser no negativo (forma cuadrática)";
}

// Una solución con pesos concentrados en pocas empresas debe tener mayor
// riesgo (y por tanto menor fitness) que la solución uniforme sobre las
// mismas empresas, dado que la diversificación reduce el riesgo.
// Nota: esto sólo se cumple cuando la covarianza entre empresas es positiva,
// lo que es habitual en datos de bolsa del mismo índice.
TEST_F(PortfolioTest, Fitness_DiversificacionMejoraFitness) {
  // Solución concentrada: todo el peso en empresa 0
  auto sol_conc = zeros();
  sol_conc[0]   = 1.0;

  // Solución diversificada: mismo peso en 10 empresas
  auto sol_div  = zeros();
  for (int i = 0; i < 10; ++i) sol_div[i] = 0.1;

  double f_conc = prob->fitness(sol_conc);
  double f_div  = prob->fitness(sol_div);

  // La diversificada debe tener fitness ≥ concentrada (menor riesgo)
  EXPECT_GE(f_div, f_conc)
      << "Diversificar deberia reducir el riesgo y mejorar el fitness";
}
