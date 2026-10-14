#pragma once

#include "psc/common/exec.hpp"

#include <cstddef>
#include <functional>
#include <string>

namespace psc {

using Integrand1D = std::function<double(double)>;
using Integrand2D = std::function<double(double, double)>;
using Integrand3D = std::function<double(double, double, double)>;

enum class QuadMethod { Trapezoid, Simpson, Gauss2, Gauss3, Gauss4, Gauss5, Adaptive, MonteCarlo };

QuadMethod parse_quad_method(const std::string& name);
const char* quad_method_name(QuadMethod m);

struct QuadOptions {
  QuadMethod method = QuadMethod::Simpson;
  // Composite panel count. Simpson requires even n (we round up if odd).
  int n = 1000;
  double tol = 1e-10;
  int max_depth = 40;
  // Monte Carlo sample count.
  std::size_t samples = 100000;
  unsigned seed = 1;
  ExecOptions exec;
};

// Composite trapezoid / Simpson / Gauss–Legendre, adaptive Simpson, or Monte Carlo.
double integrate_1d(const Integrand1D& f, double a, double b, const QuadOptions& opt);

// Rectangular 2D/3D using the 1D composite rule on each axis (outer loop OpenMP).
double integrate_2d(const Integrand2D& f, double ax, double bx, double ay, double by, const QuadOptions& opt);
double integrate_3d(const Integrand3D& f, double ax, double bx, double ay, double by, double az, double bz,
                    const QuadOptions& opt);

}  // namespace psc
