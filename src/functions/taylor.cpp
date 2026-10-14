#include "backends.hpp"

#include "psc/common/error.hpp"
#include "psc/functions/elem.hpp"

#include <cmath>

namespace psc {
namespace {

constexpr double kPi = 3.14159265358979323846;
constexpr double kLn2 = 0.693147180559945309417;

double reduce_minus_pi_pi(double x) {
  x = std::fmod(x, 2.0 * kPi);
  if (x > kPi) {
    x -= 2.0 * kPi;
  }
  if (x < -kPi) {
    x += 2.0 * kPi;
  }
  return x;
}

// Maclaurin: sin x = sum (-1)^k x^{2k+1} / (2k+1)!
// Term-level OpenMP is not used: 40 terms is far below kTaylorTermParallelCutoff.
double sin_series(double x, int iters) {
  double term = x;
  double sum = x;
  const double x2 = x * x;
  for (int k = 1; k < iters; ++k) {
    term *= -x2 / (static_cast<double>(2 * k) * static_cast<double>(2 * k + 1));
    sum += term;
  }
  return sum;
}

// Maclaurin: cos x = sum (-1)^k x^{2k} / (2k)!
double cos_series(double x, int iters) {
  double term = 1.0;
  double sum = 1.0;
  const double x2 = x * x;
  for (int k = 1; k < iters; ++k) {
    term *= -x2 / (static_cast<double>(2 * k - 1) * static_cast<double>(2 * k));
    sum += term;
  }
  return sum;
}

}  // namespace

double taylor_sin(double x, int iters) {
  if (!std::isfinite(x)) {
    throw CalcError("sin: non-finite argument");
  }
  x = reduce_minus_pi_pi(x);
  double sign = 1.0;
  if (x > kPi / 2.0) {
    x = kPi - x;
  } else if (x < -kPi / 2.0) {
    x = -kPi - x;
  }
  if (x < 0.0) {
    sign = -1.0;
    x = -x;
  }
  return sign * sin_series(x, iters);
}

double taylor_cos(double x, int iters) {
  if (!std::isfinite(x)) {
    throw CalcError("cos: non-finite argument");
  }
  x = reduce_minus_pi_pi(x);
  double sign = 1.0;
  if (x > kPi / 2.0) {
    x = kPi - x;
    sign = -1.0;
  } else if (x < -kPi / 2.0) {
    x = -kPi - x;
    sign = -1.0;
  }
  return sign * cos_series(std::abs(x), iters);
}

double taylor_tan(double x, int iters) {
  const double c = taylor_cos(x, iters);
  if (std::abs(c) < 1e-14) {
    throw CalcError("tan is undefined (cosine ~ 0)");
  }
  return taylor_sin(x, iters) / c;
}

double taylor_exp(double x, int iters) {
  if (!std::isfinite(x)) {
    throw CalcError("exp: non-finite argument");
  }
  // exp(x) = 2^n * exp(r), r = x - n ln 2, |r| small.
  const double n = std::floor(x / kLn2 + 0.5);
  const double r = x - n * kLn2;
  double term = 1.0;
  double sum = 1.0;
  for (int k = 1; k < iters; ++k) {
    term *= r / static_cast<double>(k);
    sum += term;
  }
  return std::ldexp(sum, static_cast<int>(n));
}

double taylor_ln(double x, int iters) {
  if (!(x > 0.0) || !std::isfinite(x)) {
    throw CalcError("ln is defined for x > 0");
  }
  int exp2 = 0;
  const double m = std::frexp(x, &exp2);  // x = m * 2^{exp2}, m in [0.5, 1)
  // Map m to [1, 2): if m < 1, 2m and exp2-1.
  double mantissa = m;
  int e = exp2;
  if (mantissa < 1.0) {
    mantissa *= 2.0;
    --e;
  }
  // ln(x) = e ln 2 + ln(mantissa). atanh series: ln z = 2(u + u^3/3 + ...)
  // u = (z-1)/(z+1), z in [1, 2] => u in [0, 1/3].
  const double u = (mantissa - 1.0) / (mantissa + 1.0);
  const double u2 = u * u;
  double uk = u;
  double sum = 0.0;
  for (int k = 0; k < iters; ++k) {
    sum += uk / static_cast<double>(2 * k + 1);
    uk *= u2;
  }
  return static_cast<double>(e) * kLn2 + 2.0 * sum;
}

double taylor_sqrt(double x, int iters) {
  if (x < 0.0 || !std::isfinite(x)) {
    throw CalcError("sqrt is defined for x >= 0");
  }
  if (x == 0.0) {
    return 0.0;
  }
  int exp2 = 0;
  double m = std::frexp(x, &exp2);  // [0.5, 1)
  if ((exp2 & 1) != 0) {
    m *= 2.0;
    --exp2;
  }
  // sqrt(m * 2^{even}) = sqrt(m) * 2^{exp2/2}. Binomial on (1+u), u = m-1 in [-0.5, 1).
  // Newton on m is more stable; we still run `iters` steps (same knob as series).
  double y = 1.0;
  for (int k = 0; k < iters; ++k) {
    y = 0.5 * (y + m / y);
  }
  return std::ldexp(y, exp2 / 2);
}

double taylor_sinh(double x, int iters) {
  const double e = taylor_exp(x, iters);
  const double em = taylor_exp(-x, iters);
  return 0.5 * (e - em);
}

double taylor_cosh(double x, int iters) {
  const double e = taylor_exp(x, iters);
  const double em = taylor_exp(-x, iters);
  return 0.5 * (e + em);
}

double taylor_atan(double x, int iters) {
  if (!std::isfinite(x)) {
    throw CalcError("atan: non-finite argument");
  }
  // Reduce |x|>1: atan(x) = pi/2 * sgn - atan(1/x)
  double sign = 1.0;
  if (x < 0.0) {
    sign = -1.0;
    x = -x;
  }
  bool complement = false;
  if (x > 1.0) {
    x = 1.0 / x;
    complement = true;
  }
  // atan x = x - x^3/3 + x^5/5 - ...  |x|<=1, slow near 1; use identity
  // atan x = 2 atan(x/(1+sqrt(1+x^2))) once for faster convergence.
  const double s = taylor_sqrt(1.0 + x * x, iters);
  const double u = x / (1.0 + s);
  double term = u;
  double sum = u;
  const double u2 = u * u;
  for (int k = 1; k < iters; ++k) {
    term *= -u2;
    sum += term / static_cast<double>(2 * k + 1);
  }
  double a = 2.0 * sum;
  if (complement) {
    a = kPi / 2.0 - a;
  }
  return sign * a;
}

}  // namespace psc
