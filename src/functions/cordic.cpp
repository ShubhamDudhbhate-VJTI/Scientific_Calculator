#include "backends.hpp"

#include "psc/common/error.hpp"

#include <cmath>
#include <vector>

namespace psc {
namespace {

constexpr double kPi = 3.14159265358979323846;
constexpr double kLn2 = 0.693147180559945309417;

struct CordicTables {
  std::vector<double> atan;
  std::vector<double> atanh;
  double k_circ = 1.0;
  double k_hyp = 1.0;
};

const CordicTables& tables(int iters) {
  // Built once per process; read-only after init (C++11 magic static).
  static const CordicTables t = [] {
    CordicTables out;
    const int n = 64;
    out.atan.resize(static_cast<std::size_t>(n));
    out.atanh.resize(static_cast<std::size_t>(n));
    out.k_circ = 1.0;
    out.k_hyp = 1.0;
    for (int i = 0; i < n; ++i) {
      out.atan[static_cast<std::size_t>(i)] = std::atan(std::ldexp(1.0, -i));
      out.k_circ *= 1.0 / std::sqrt(1.0 + std::ldexp(1.0, -2 * i));
      if (i >= 1) {
        const double th = std::atanh(std::ldexp(1.0, -i));
        out.atanh[static_cast<std::size_t>(i)] = th;
        out.k_hyp *= 1.0 / std::sqrt(1.0 - std::ldexp(1.0, -2 * i));
      }
    }
    return out;
  }();
  (void)iters;
  return t;
}

int clamp_iters(int iters) {
  if (iters < 8) {
    return 8;
  }
  if (iters > 60) {
    return 60;
  }
  return iters;
}

void sincos_rotation(double angle, int iters, double& s, double& c) {
  const auto& tab = tables(iters);
  iters = clamp_iters(iters);

  double x = tab.k_circ;
  double y = 0.0;
  double z = angle;
  for (int i = 0; i < iters; ++i) {
    const double d = (z >= 0.0) ? 1.0 : -1.0;
    const double x2 = x - d * y * std::ldexp(1.0, -i);
    const double y2 = y + d * x * std::ldexp(1.0, -i);
    z -= d * tab.atan[static_cast<std::size_t>(i)];
    x = x2;
    y = y2;
  }
  c = x;
  s = y;
}

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

// Hyperbolic rotation: x,y,z with z -> 0, produces K_h*cosh and K_h*sinh if started at (1/Kh, 0, t).
void hyp_rotate(double t, int iters, double& sh, double& ch) {
  const auto& tab = tables(iters);
  iters = clamp_iters(iters);
  double x = tab.k_hyp;
  double y = 0.0;
  double z = t;
  for (int i = 1; i < iters; ++i) {
    const int repeats = (i == 4 || i == 13 || i == 40) ? 2 : 1;
    for (int r = 0; r < repeats; ++r) {
      const double d = (z >= 0.0) ? 1.0 : -1.0;
      const double x2 = x + d * y * std::ldexp(1.0, -i);
      const double y2 = y + d * x * std::ldexp(1.0, -i);
      z -= d * tab.atanh[static_cast<std::size_t>(i)];
      x = x2;
      y = y2;
    }
  }
  ch = x;
  sh = y;
}

}  // namespace

double cordic_sin(double x, int iters) {
  if (!std::isfinite(x)) {
    throw CalcError("sin: non-finite argument");
  }
  x = reduce_minus_pi_pi(x);
  double sign_s = 1.0;
  double sign_c = 1.0;
  if (x > kPi / 2.0) {
    x = kPi - x;
    sign_c = -1.0;
  } else if (x < -kPi / 2.0) {
    x = -kPi - x;
    sign_c = -1.0;
  }
  if (x < 0.0) {
    sign_s = -1.0;
    x = -x;
  }
  double s = 0.0;
  double c = 0.0;
  sincos_rotation(x, iters, s, c);
  (void)c;
  (void)sign_c;
  return sign_s * s;
}

double cordic_cos(double x, int iters) {
  if (!std::isfinite(x)) {
    throw CalcError("cos: non-finite argument");
  }
  x = reduce_minus_pi_pi(x);
  double sign_c = 1.0;
  if (x > kPi / 2.0) {
    x = kPi - x;
    sign_c = -1.0;
  } else if (x < -kPi / 2.0) {
    x = -kPi - x;
    sign_c = -1.0;
  }
  x = std::abs(x);
  double s = 0.0;
  double c = 0.0;
  sincos_rotation(x, iters, s, c);
  return sign_c * c;
}

double cordic_tan(double x, int iters) {
  const double c = cordic_cos(x, iters);
  if (std::abs(c) < 1e-14) {
    throw CalcError("tan is undefined (cosine ~ 0)");
  }
  return cordic_sin(x, iters) / c;
}

double cordic_atan(double x, int iters) {
  // Vectoring mode: drive y to 0. Start with (1, x).
  if (!std::isfinite(x)) {
    throw CalcError("atan: non-finite argument");
  }
  iters = clamp_iters(iters);
  const auto& tab = tables(iters);
  double vx = 1.0;
  double vy = x;
  double z = 0.0;
  for (int i = 0; i < iters; ++i) {
    const double d = (vy >= 0.0) ? -1.0 : 1.0;
    const double x2 = vx - d * vy * std::ldexp(1.0, -i);
    const double y2 = vy + d * vx * std::ldexp(1.0, -i);
    z -= d * tab.atan[static_cast<std::size_t>(i)];
    vx = x2;
    vy = y2;
  }
  return z;
}

double cordic_exp(double x, int iters) {
  if (!std::isfinite(x)) {
    throw CalcError("exp: non-finite argument");
  }
  const double n = std::floor(x / kLn2 + 0.5);
  const double r = x - n * kLn2;
  double sh = 0.0;
  double ch = 0.0;
  hyp_rotate(r, iters, sh, ch);
  return std::ldexp(ch + sh, static_cast<int>(n));
}

double cordic_ln(double x, int iters) {
  if (!(x > 0.0) || !std::isfinite(x)) {
    throw CalcError("ln is defined for x > 0");
  }
  int exp2 = 0;
  double m = std::frexp(x, &exp2);
  if (m < 1.0) {
    m *= 2.0;
    --exp2;
  }
  // Vectoring hyperbolic: start x=(m+1), y=(m-1), z=0, drive y->0 => z -> 0.5 ln(m)
  iters = clamp_iters(iters);
  const auto& tab = tables(iters);
  double vx = m + 1.0;
  double vy = m - 1.0;
  double z = 0.0;
  for (int i = 1; i < iters; ++i) {
    const int repeats = (i == 4 || i == 13 || i == 40) ? 2 : 1;
    for (int r = 0; r < repeats; ++r) {
      const double d = (vy >= 0.0) ? -1.0 : 1.0;
      const double x2 = vx + d * vy * std::ldexp(1.0, -i);
      const double y2 = vy + d * vx * std::ldexp(1.0, -i);
      z -= d * tab.atanh[static_cast<std::size_t>(i)];
      vx = x2;
      vy = y2;
    }
  }
  return static_cast<double>(exp2) * kLn2 + 2.0 * z;
}

double cordic_sinh(double x, int iters) {
  const double e = cordic_exp(x, iters);
  const double em = cordic_exp(-x, iters);
  return 0.5 * (e - em);
}

double cordic_cosh(double x, int iters) {
  const double e = cordic_exp(x, iters);
  const double em = cordic_exp(-x, iters);
  return 0.5 * (e + em);
}

}  // namespace psc
