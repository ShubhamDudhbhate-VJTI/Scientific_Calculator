#include "psc/calculus/integrate.hpp"
#include "psc/common/error.hpp"

#include <cmath>
#include <gtest/gtest.h>

namespace {

psc::QuadOptions serial_simpson(int n) {
  psc::QuadOptions o;
  o.method = psc::QuadMethod::Simpson;
  o.n = n;
  o.exec.mode = psc::ExecMode::Serial;
  o.exec.parallel_cutoff = 1;
  return o;
}

}  // namespace

TEST(Integrate, AtanIntegralIsPi) {
  auto f = [](double x) { return 4.0 / (1.0 + x * x); };
  const double s = psc::integrate_1d(f, 0.0, 1.0, serial_simpson(20000));
  EXPECT_NEAR(s, std::acos(-1.0), 1e-8);
}

TEST(Integrate, SinZeroToPi) {
  auto f = [](double x) { return std::sin(x); };
  EXPECT_NEAR(psc::integrate_1d(f, 0.0, std::acos(-1.0), serial_simpson(20000)), 2.0, 1e-8);
}

TEST(Integrate, GaussianVsErf) {
  auto f = [](double x) { return std::exp(-x * x); };
  const double got = psc::integrate_1d(f, 0.0, 1.0, serial_simpson(40000));
  const double ref = 0.5 * std::sqrt(std::acos(-1.0)) * std::erf(1.0);
  EXPECT_NEAR(got, ref, 1e-8);
}

TEST(Integrate, ZeroWidth) {
  auto f = [](double x) { return x * x; };
  EXPECT_DOUBLE_EQ(psc::integrate_1d(f, 2.0, 2.0, serial_simpson(10)), 0.0);
}

TEST(Integrate, ReversedBounds) {
  auto f = [](double x) { return x; };
  const double fwd = psc::integrate_1d(f, 0.0, 2.0, serial_simpson(1000));
  const double rev = psc::integrate_1d(f, 2.0, 0.0, serial_simpson(1000));
  EXPECT_NEAR(fwd, 2.0, 1e-8);
  EXPECT_NEAR(rev, -fwd, 1e-10);
}

TEST(Integrate, TrapezoidSerialMatchesParallel) {
#if defined(__SANITIZE_THREAD__)
  GTEST_SKIP() << "OpenMP reduction not TSan-clean with libgomp";
#endif
  auto f = [](double x) { return std::sin(x); };
  psc::QuadOptions s;
  s.method = psc::QuadMethod::Trapezoid;
  s.n = 5000;
  s.exec.mode = psc::ExecMode::Serial;
  s.exec.parallel_cutoff = 1;
  psc::QuadOptions p = s;
  p.exec.mode = psc::ExecMode::Parallel;
  EXPECT_NEAR(psc::integrate_1d(f, 0.0, 1.0, s), psc::integrate_1d(f, 0.0, 1.0, p), 1e-12);
}

TEST(Integrate, SimpsonOddNRoundsUp) {
  auto f = [](double x) { return 1.0; };
  auto o = serial_simpson(5);  // becomes 6
  EXPECT_NEAR(psc::integrate_1d(f, 0.0, 1.0, o), 1.0, 1e-12);
}

TEST(Integrate, AdaptivePi) {
  auto f = [](double x) { return 4.0 / (1.0 + x * x); };
  psc::QuadOptions o;
  o.method = psc::QuadMethod::Adaptive;
  o.tol = 1e-10;
  o.exec.mode = psc::ExecMode::Serial;
  EXPECT_NEAR(psc::integrate_1d(f, 0.0, 1.0, o), std::acos(-1.0), 1e-9);
}

TEST(Integrate, Gauss5Pi) {
  auto f = [](double x) { return 4.0 / (1.0 + x * x); };
  psc::QuadOptions o;
  o.method = psc::QuadMethod::Gauss5;
  o.n = 16;
  o.exec.mode = psc::ExecMode::Serial;
  EXPECT_NEAR(psc::integrate_1d(f, 0.0, 1.0, o), std::acos(-1.0), 1e-10);
}

TEST(Integrate, InvalidTol) {
  psc::QuadOptions o;
  o.method = psc::QuadMethod::Adaptive;
  o.tol = 0.0;
  o.exec.mode = psc::ExecMode::Serial;
  EXPECT_THROW(psc::integrate_1d([](double) { return 1.0; }, 0.0, 1.0, o), psc::CalcError);
}

TEST(Integrate, TwoDUnitSquare) {
  auto f = [](double, double) { return 1.0; };
  auto o = serial_simpson(40);
  EXPECT_NEAR(psc::integrate_2d(f, 0.0, 1.0, 0.0, 1.0, o), 1.0, 1e-8);
}

TEST(Integrate, MonteCarloRoughPi) {
  auto f = [](double x) { return 4.0 / (1.0 + x * x); };
  psc::QuadOptions o;
  o.method = psc::QuadMethod::MonteCarlo;
  o.samples = 200000;
  o.seed = 42;
  o.exec.mode = psc::ExecMode::Serial;
  o.exec.parallel_cutoff = 1;
  EXPECT_NEAR(psc::integrate_1d(f, 0.0, 1.0, o), std::acos(-1.0), 0.05);
}
