#include "psc/functions/elem.hpp"

#include "backends.hpp"

#include "psc/common/error.hpp"

#include <cmath>

namespace psc {

const char* backend_name(FunBackend b) {
  switch (b) {
    case FunBackend::Std:
      return "std";
    case FunBackend::Taylor:
      return "taylor";
    case FunBackend::Cordic:
      return "cordic";
  }
  return "unknown";
}

double elem_sin(double x, FunBackend b, int iters) {
  switch (b) {
    case FunBackend::Std:
      return std::sin(x);
    case FunBackend::Taylor:
      return taylor_sin(x, iters);
    case FunBackend::Cordic:
      return cordic_sin(x, iters);
  }
  throw CalcError("unknown function backend");
}

double elem_cos(double x, FunBackend b, int iters) {
  switch (b) {
    case FunBackend::Std:
      return std::cos(x);
    case FunBackend::Taylor:
      return taylor_cos(x, iters);
    case FunBackend::Cordic:
      return cordic_cos(x, iters);
  }
  throw CalcError("unknown function backend");
}

double elem_tan(double x, FunBackend b, int iters) {
  switch (b) {
    case FunBackend::Std:
      return std::tan(x);
    case FunBackend::Taylor:
      return taylor_tan(x, iters);
    case FunBackend::Cordic:
      return cordic_tan(x, iters);
  }
  throw CalcError("unknown function backend");
}

double elem_sinh(double x, FunBackend b, int iters) {
  switch (b) {
    case FunBackend::Std:
      return std::sinh(x);
    case FunBackend::Taylor:
      return taylor_sinh(x, iters);
    case FunBackend::Cordic:
      return cordic_sinh(x, iters);
  }
  throw CalcError("unknown function backend");
}

double elem_cosh(double x, FunBackend b, int iters) {
  switch (b) {
    case FunBackend::Std:
      return std::cosh(x);
    case FunBackend::Taylor:
      return taylor_cosh(x, iters);
    case FunBackend::Cordic:
      return cordic_cosh(x, iters);
  }
  throw CalcError("unknown function backend");
}

double elem_exp(double x, FunBackend b, int iters) {
  switch (b) {
    case FunBackend::Std:
      return std::exp(x);
    case FunBackend::Taylor:
      return taylor_exp(x, iters);
    case FunBackend::Cordic:
      return cordic_exp(x, iters);
  }
  throw CalcError("unknown function backend");
}

double elem_ln(double x, FunBackend b, int iters) {
  switch (b) {
    case FunBackend::Std:
      if (!(x > 0.0)) {
        throw CalcError("ln is defined for x > 0");
      }
      return std::log(x);
    case FunBackend::Taylor:
      return taylor_ln(x, iters);
    case FunBackend::Cordic:
      return cordic_ln(x, iters);
  }
  throw CalcError("unknown function backend");
}

double elem_sqrt(double x, FunBackend b, int iters) {
  if (b == FunBackend::Cordic) {
    throw CalcError("CORDIC backend does not implement sqrt (no linear CORDIC in this project)");
  }
  switch (b) {
    case FunBackend::Std:
      if (x < 0.0) {
        throw CalcError("sqrt is defined for x >= 0");
      }
      return std::sqrt(x);
    case FunBackend::Taylor:
      return taylor_sqrt(x, iters);
    case FunBackend::Cordic:
      break;
  }
  throw CalcError("unknown function backend");
}

double elem_atan(double x, FunBackend b, int iters) {
  switch (b) {
    case FunBackend::Std:
      return std::atan(x);
    case FunBackend::Taylor:
      return taylor_atan(x, iters);
    case FunBackend::Cordic:
      return cordic_atan(x, iters);
  }
  throw CalcError("unknown function backend");
}

AccuracyReport compare_to_std_sin(FunBackend b, double lo, double hi, int n, int iters) {
  AccuracyReport r;
  if (n < 2) {
    return r;
  }
  for (int i = 0; i < n; ++i) {
    const double x = lo + (hi - lo) * static_cast<double>(i) / static_cast<double>(n - 1);
    const double ref = std::sin(x);
    const double y = elem_sin(x, b, iters);
    const double aerr = std::abs(y - ref);
    if (aerr > r.max_abs) {
      r.max_abs = aerr;
    }
    if (std::abs(ref) >= 1e-12) {
      const double rerr = aerr / std::abs(ref);
      if (rerr > r.max_rel) {
        r.max_rel = rerr;
      }
    }
    ++r.points;
  }
  return r;
}

}  // namespace psc
