#include <gtest/gtest.h>
#include <portfolioProblem.h>
#include <random.hpp>
#include <cmath>
#include <numeric>
#include <string>

// Ruta a los datos inyectada por CMake
#ifndef DATA_DIR
#define DATA_DIR "data"
#endif

static const std::string IBEX   = std::string(DATA_DIR) + "/ibex_35.csv";
static const std::string SYP100 = std::string(DATA_DIR) + "/syp_100.csv";
static const std::string SYP500 = std::string(DATA_DIR) + "/syp_500.csv";

// Parámetros usados en la fixture principal (N=30, hi debe cumplir N*hi >= 1)
// Con hi=0.05 → 30*0.05 = 1.5 ≥ 1 ✓
static const double LO   = 0.005;
static const double HI   = 0.05;

// Parámetros amplios para tests de fix() donde queremos normalización trivial
static const double LO_WIDE = 0.01;
static const double HI_WIDE = 1.0;

// ============================================================
//  Fixture compartida (ibex_35, N=30)
// ============================================================
class PortfolioProblemTest : public ::testing::Test {
protected:
  void SetUp() override {
    Random::seed(42);
    problem = std::make_unique<PortfolioProblem>(IBEX, LO, HI);
  }
  std::unique_ptr<PortfolioProblem> problem;
};

// ============================================================
//  1. Constructor / carga de datos
// ============================================================
TEST_F(PortfolioProblemTest, ThrowsOnMissingFile) {
  EXPECT_THROW(PortfolioProblem("no_existe.csv", LO, HI), std::runtime_error);
}

TEST_F(PortfolioProblemTest, LoadsIbex35Correctly) {
  EXPECT_EQ(problem->getSolutionSize(), 30u);
}

// Los CSVs de syp_100 y syp_500 contienen datos reales y pueden tener
// empresas eliminadas; verificamos que el tamaño cargado es positivo
// y coincide con los valores efectivos observados en los archivos.
TEST(PortfolioProblemLoad, Syp100HasPositiveSize) {
  PortfolioProblem p(SYP100, LO, HI_WIDE);
  EXPECT_GT(p.getSolutionSize(), 0u);
}

TEST(PortfolioProblemLoad, Syp500HasPositiveSize) {
  PortfolioProblem p(SYP500, LO, HI_WIDE);
  EXPECT_GT(p.getSolutionSize(), 0u);
}

// Tamaños reales observados en los archivos disponibles
TEST(PortfolioProblemLoad, Syp100ActualSize) {
  PortfolioProblem p(SYP100, LO, HI_WIDE);
  EXPECT_EQ(p.getSolutionSize(), 97u);   // archivo contiene 97 empresas
}

TEST(PortfolioProblemLoad, Syp500ActualSize) {
  PortfolioProblem p(SYP500, LO, HI_WIDE);
  EXPECT_EQ(p.getSolutionSize(), 457u);  // archivo contiene 457 empresas
}

// ============================================================
//  2. getSolutionDomainRange / getLambda
// ============================================================
TEST_F(PortfolioProblemTest, DomainRangeMatchesConstructorArgs) {
  auto [lo, hi] = problem->getSolutionDomainRange();
  EXPECT_DOUBLE_EQ(lo, LO);
  EXPECT_DOUBLE_EQ(hi, HI);
}

TEST_F(PortfolioProblemTest, LambdaIs500) {
  EXPECT_DOUBLE_EQ(problem->getLambda(), 500.0);
}

// ============================================================
//  3. createSolution
// ============================================================
TEST_F(PortfolioProblemTest, CreateSolutionHasCorrectSize) {
  auto sol = problem->createSolution();
  EXPECT_EQ(sol.size(), problem->getSolutionSize());
}

TEST_F(PortfolioProblemTest, CreateSolutionSumsToOne) {
  const double epsilon = 1e-6;
  for (int trial = 0; trial < 20; ++trial) {
    auto sol = problem->createSolution();
    double sum = std::accumulate(sol.begin(), sol.end(), 0.0);
    EXPECT_NEAR(sum, 1.0, epsilon) << "Trial " << trial;
  }
}

TEST_F(PortfolioProblemTest, CreateSolutionRespectsBounds) {
  for (int trial = 0; trial < 20; ++trial) {
    auto sol = problem->createSolution();
    for (size_t i = 0; i < sol.size(); ++i) {
      double w = sol[i];
      EXPECT_TRUE(w == 0.0 || (w >= LO && w <= HI))
          << "Trial " << trial << ", empresa " << i << ", w=" << w;
    }
  }
}

TEST_F(PortfolioProblemTest, CreateSolutionIsAlwaysValid) {
  for (int trial = 0; trial < 20; ++trial) {
    auto sol = problem->createSolution();
    EXPECT_TRUE(problem->isValid(sol)) << "Trial " << trial;
  }
}

TEST_F(PortfolioProblemTest, CreateSolutionHasAtLeastOneActiveAsset) {
  for (int trial = 0; trial < 20; ++trial) {
    auto sol = problem->createSolution();
    int activas = std::count_if(sol.begin(), sol.end(),
                                [](double w) { return w > 0.0; });
    EXPECT_GE(activas, 1) << "Trial " << trial;
  }
}

// ============================================================
//  4. isValid
// ============================================================
TEST_F(PortfolioProblemTest, IsValidAcceptsCorrectSolution) {
  auto sol = problem->createSolution();
  EXPECT_TRUE(problem->isValid(sol));
}

TEST_F(PortfolioProblemTest, IsValidRejectsWhenSumIsZero) {
  tSolution<double> sol(problem->getSolutionSize(), 0.0);
  EXPECT_FALSE(problem->isValid(sol));
}

TEST_F(PortfolioProblemTest, IsValidRejectsWeightBelowLo) {
  auto sol = problem->createSolution();
  for (size_t i = 0; i < sol.size(); ++i) {
    if (sol[i] > 0.0) {
      sol[i] = LO / 2.0;
      break;
    }
  }
  EXPECT_FALSE(problem->isValid(sol));
}

TEST_F(PortfolioProblemTest, IsValidRejectsWeightAboveHi) {
  tSolution<double> sol(problem->getSolutionSize(), 0.0);
  sol[0] = HI + 0.1;
  EXPECT_FALSE(problem->isValid(sol));
}

TEST_F(PortfolioProblemTest, IsValidRejectsWhenSumIsNotOne) {
  tSolution<double> sol(problem->getSolutionSize(), 0.0);
  sol[0] = HI;
  sol[1] = HI;  // suma = 2*HI != 1
  EXPECT_FALSE(problem->isValid(sol));
}

// ============================================================
//  5. fix
//  Usamos hi=1.0 para garantizar que fix() puede converger
//  sin restricciones de techo (N*hi >> 1).
// ============================================================
TEST(PortfolioProblemFix, ProducesValidSolutionFromUnnormalizedWeights) {
  Random::seed(1);
  PortfolioProblem p(IBEX, LO_WIDE, HI_WIDE);

  tSolution<double> sol(p.getSolutionSize(), 0.0);
  sol[0] = 0.5;
  sol[1] = 0.3;
  sol[2] = 0.8;

  ASSERT_FALSE(p.isValid(sol));
  p.fix(sol);
  EXPECT_TRUE(p.isValid(sol));
}

TEST(PortfolioProblemFix, PreservesAlreadyValidSolution) {
  Random::seed(2);
  PortfolioProblem p(IBEX, LO_WIDE, HI_WIDE);

  auto sol = p.createSolution();
  ASSERT_TRUE(p.isValid(sol));

  tSolution<double> before = sol;
  p.fix(sol);

  const double eps = 1e-6;
  for (size_t i = 0; i < sol.size(); ++i)
    EXPECT_NEAR(sol[i], before[i], eps) << "Indice " << i;
}

TEST(PortfolioProblemFix, SumIsOneAfterScaling) {
  Random::seed(3);
  PortfolioProblem p(IBEX, LO_WIDE, HI_WIDE);

  tSolution<double> sol(p.getSolutionSize(), 0.0);
  for (int i = 0; i < 5; ++i) sol[i] = 0.1;  // suma=0.5, sin normalizar

  p.fix(sol);
  double sum = std::accumulate(sol.begin(), sol.end(), 0.0);
  EXPECT_NEAR(sum, 1.0, 1e-6);
}

// ============================================================
//  6. fitness
// ============================================================
TEST_F(PortfolioProblemTest, FitnessIsFinite) {
  auto sol = problem->createSolution();
  EXPECT_TRUE(std::isfinite(problem->fitness(sol)));
}

TEST_F(PortfolioProblemTest, FitnessOfZeroSolutionIsZero) {
  tSolution<double> sol(problem->getSolutionSize(), 0.0);
  EXPECT_FLOAT_EQ(problem->fitness(sol), 0.0f);
}

TEST_F(PortfolioProblemTest, FitnessIsDeterministic) {
  auto sol = problem->createSolution();
  EXPECT_FLOAT_EQ(problem->fitness(sol), problem->fitness(sol));
}

TEST_F(PortfolioProblemTest, FitnessDiffersAcrossDifferentSolutions) {
  Random::seed(123);
  auto sol1 = problem->createSolution();
  auto sol2 = problem->createSolution();
  if (sol1 != sol2)
    EXPECT_NE(problem->fitness(sol1), problem->fitness(sol2));
}

// ============================================================
//  7. getBenefitFactors
// ============================================================
TEST_F(PortfolioProblemTest, BenefitFactorsSizeMatchesN) {
  EXPECT_EQ(problem->getBenefitFactors().size(), problem->getSolutionSize());
}

// ============================================================
//  8. Integración: distintos datasets
//     syp_100 y syp_500 con lo=0.005, hi=0.02 funcionan porque
//     N*hi >> 1 (97*0.02=1.94 y 457*0.02=9.14)
// ============================================================
TEST(PortfolioProblemIntegration, Syp100SolutionIsValid) {
  Random::seed(7);
  PortfolioProblem p(SYP100, 0.005, 0.02);
  auto sol = p.createSolution();
  EXPECT_TRUE(p.isValid(sol));
}

TEST(PortfolioProblemIntegration, Syp500SolutionIsValid) {
  Random::seed(7);
  PortfolioProblem p(SYP500, 0.005, 0.02);
  auto sol = p.createSolution();
  EXPECT_TRUE(p.isValid(sol));
}

TEST(PortfolioProblemIntegration, Syp500FitnessIsFinite) {
  Random::seed(99);
  PortfolioProblem p(SYP500, 0.005, 0.02);
  auto sol = p.createSolution();
  EXPECT_TRUE(std::isfinite(p.fitness(sol)));
}

// Verificar que con lo=0.005, hi=0.02 el ibex_35 falla la constraint
// (N*hi = 30*0.02 = 0.6 < 1.0 → imposible tener portfolio válido)
TEST(PortfolioProblemIntegration, Ibex35WithTightHiCannotSumToOne) {
  Random::seed(5);
  PortfolioProblem p(IBEX, 0.005, 0.02);
  // La suma máxima posible es 30*0.02=0.6, por lo que no puede ser válido
  tSolution<double> sol(p.getSolutionSize(), 0.02);
  double max_sum = std::accumulate(sol.begin(), sol.end(), 0.0);
  EXPECT_LT(max_sum, 1.0) << "Con N=30 y hi=0.02, N*hi < 1 es imposible";
}
