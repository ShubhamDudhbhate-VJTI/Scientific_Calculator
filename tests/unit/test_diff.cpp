#include "psc/calculus/diff.hpp"
#include "psc/common/error.hpp"

#include <cmath>
#include <gtest/gtest.h>

TEST(Diff, CentralMatchesAnalytic) {
  auto f = [](double x) { return x * x * x + std::sin(x); };
  auto df = [](double x) { return 3.0 * x * x + std::cos(x); };
  const double x = 2.0;
  EXPECT_NEAR(psc::finite_diff(f, x, 1e-6, psc::DiffScheme::Central), df(x), 1e-6);
}

TEST(Diff, ForwardWorseThanCentral) {
  auto f = [](double x) { return x * x; };
  const double fwd = std::abs(psc::finite_diff(f, 1.0, 1e-3, psc::DiffScheme::Forward) - 2.0);
  const double cen = std::abs(psc::finite_diff(f, 1.0, 1e-3, psc::DiffScheme::Central) - 2.0);
  EXPECT_GT(fwd, cen);
}

TEST(Diff, DualPolyAndSin) {
  psc::DualEnv env;
  env["x"] = {2.0, 1.0};
  const psc::Dual y = psc::evaluate_dual_string("x^3 + sin(x)", env, psc::AngleMode::Radians);
  EXPECT_NEAR(y.v, 8.0 + std::sin(2.0), 1e-12);
  EXPECT_NEAR(y.d, 12.0 + std::cos(2.0), 1e-12);
}

TEST(Diff, GradientTwoVars) {
  psc::ExecOptions ex;
  ex.mode = psc::ExecMode::Serial;
  auto g = psc::gradient_ad("x^2*y + y^3", {"x", "y"}, {1.0, 2.0}, psc::AngleMode::Radians, ex);
  ASSERT_EQ(g.size(), 2u);
  EXPECT_NEAR(g[0], 4.0, 1e-12);   // 2 x y
  EXPECT_NEAR(g[1], 13.0, 1e-12);  // x^2 + 3 y^2
}

TEST(Diff, GradientSerialMatchesParallel) {
#if defined(__SANITIZE_THREAD__)
  GTEST_SKIP() << "OpenMP + TSan/libgomp";
#endif
  psc::ExecOptions s;
  s.mode = psc::ExecMode::Serial;
  s.parallel_cutoff = 1;
  psc::ExecOptions p = s;
  p.mode = psc::ExecMode::Parallel;
  const std::vector<std::string> names = {"x", "y", "z"};
  const std::vector<double> vals = {1.0, 2.0, 3.0};
  auto a = psc::gradient_ad("x*y + z^2", names, vals, psc::AngleMode::Radians, s);
  auto b = psc::gradient_ad("x*y + z^2", names, vals, psc::AngleMode::Radians, p);
  ASSERT_EQ(a.size(), b.size());
  for (std::size_t i = 0; i < a.size(); ++i) {
    EXPECT_NEAR(a[i], b[i], 1e-12);
  }
}

TEST(Diff, Jacobian) {
  psc::ExecOptions ex;
  ex.mode = psc::ExecMode::Serial;
  auto J = psc::jacobian_ad({"x+y", "x*y"}, {"x", "y"}, {2.0, 3.0}, psc::AngleMode::Radians, ex);
  ASSERT_EQ(J.size(), 4u);
  EXPECT_NEAR(J[0], 1.0, 1e-12);
  EXPECT_NEAR(J[1], 1.0, 1e-12);
  EXPECT_NEAR(J[2], 3.0, 1e-12);
  EXPECT_NEAR(J[3], 2.0, 1e-12);
}

TEST(Diff, StepStudyHasUShape) {
  auto f = [](double x) { return std::sin(x); };
  auto df = [](double x) { return std::cos(x); };
  auto rows = psc::step_size_study(f, 0.5, df, 10);
  ASSERT_EQ(rows.size(), 10u);
  EXPECT_GT(rows.front().abs_error, rows[3].abs_error);
}

TEST(Diff, ZeroStepThrows) {
  EXPECT_THROW(psc::finite_diff([](double x) { return x; }, 0.0, 0.0, psc::DiffScheme::Forward), psc::CalcError);
}
