#include "psc/common/error.hpp"
#include "psc/functions/elem.hpp"

#include <cmath>
#include <gtest/gtest.h>
#include <vector>

TEST(Functions, TaylorSinCloseToStd) {
  const auto r = psc::compare_to_std_sin(psc::FunBackend::Taylor, -6.0, 6.0, 200, 40);
  EXPECT_LT(r.max_abs, 1e-9);
}

TEST(Functions, CordicSinCloseToStd) {
  const auto r = psc::compare_to_std_sin(psc::FunBackend::Cordic, -6.0, 6.0, 200, 40);
  EXPECT_LT(r.max_abs, 1e-8);
}

TEST(Functions, TaylorExpLnRoundTrip) {
  for (double x = -2.0; x <= 2.0; x += 0.25) {
    const double y = psc::elem_exp(x, psc::FunBackend::Taylor, 40);
    const double z = psc::elem_ln(y, psc::FunBackend::Taylor, 40);
    EXPECT_NEAR(z, x, 1e-9) << "x=" << x;
  }
}

TEST(Functions, TaylorSqrt) {
  EXPECT_NEAR(psc::elem_sqrt(4.0, psc::FunBackend::Taylor, 40), 2.0, 1e-12);
  EXPECT_NEAR(psc::elem_sqrt(2.0, psc::FunBackend::Taylor, 40), std::sqrt(2.0), 1e-10);
  EXPECT_THROW(psc::elem_sqrt(-1.0, psc::FunBackend::Taylor, 40), psc::CalcError);
}

TEST(Functions, CordicSqrtUnsupported) {
  EXPECT_THROW(psc::elem_sqrt(4.0, psc::FunBackend::Cordic, 40), psc::CalcError);
}

TEST(Functions, TaylorSinhCosh) {
  const double x = 0.75;
  EXPECT_NEAR(psc::elem_sinh(x, psc::FunBackend::Taylor, 40), std::sinh(x), 1e-9);
  EXPECT_NEAR(psc::elem_cosh(x, psc::FunBackend::Taylor, 40), std::cosh(x), 1e-9);
}

TEST(Functions, CordicAtan) {
  EXPECT_NEAR(psc::elem_atan(1.0, psc::FunBackend::Cordic, 40), std::atan(1.0), 1e-8);
}

TEST(Functions, BatchSerialMatchesParallel) {
  const std::size_t n = 4096;
  std::vector<double> in(n);
  for (std::size_t i = 0; i < n; ++i) {
    in[i] = -1.0 + 2.0 * static_cast<double>(i) / static_cast<double>(n);
  }
  std::vector<double> a(n), b(n);
  psc::ExecOptions serial;
  serial.mode = psc::ExecMode::Serial;
  serial.parallel_cutoff = 1;
  psc::ExecOptions parallel;
  parallel.mode = psc::ExecMode::Parallel;
  parallel.parallel_cutoff = 1;
  psc::batch_sin(in.data(), a.data(), n, psc::FunBackend::Taylor, serial, 40);
  psc::batch_sin(in.data(), b.data(), n, psc::FunBackend::Taylor, parallel, 40);
  for (std::size_t i = 0; i < n; ++i) {
    EXPECT_NEAR(a[i], b[i], 1e-12);
  }
}

TEST(Functions, LnDomain) {
  EXPECT_THROW(psc::elem_ln(0.0, psc::FunBackend::Taylor, 40), psc::CalcError);
  EXPECT_THROW(psc::elem_ln(-2.0, psc::FunBackend::Std, 40), psc::CalcError);
}
