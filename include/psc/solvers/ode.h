#pragma once

#include <functional>
#include <vector>

namespace psc {
namespace solvers {

// ODE solver types
// ODE: dy/dt = f(t, y), with initial condition y(t0) = y0

// Result of ODE integration
struct ODEResult {
    std::vector<double> t;  // Time points
    std::vector<double> y;  // Solution values
};

// Euler method - first-order, simple but inaccurate
ODEResult euler(std::function<double(double, double)> f, double t0, double y0, double t_end, double h);

// Runge-Kutta 4 (RK4) - fourth-order, much more accurate
ODEResult rk4(std::function<double(double, double)> f, double t0, double y0, double t_end, double h);

// Adaptive RK4 - adjusts step size based on error estimate
ODEResult rk4_adaptive(std::function<double(double, double)> f, double t0, double y0, double t_end, 
                       double h_init, double tol = 1e-6, double h_min = 1e-10, double h_max = 0.1);

// System of ODEs (n equations)
// dy/dt = f(t, y) where y is a vector
ODEResult euler_system(std::function<std::vector<double>(double, const std::vector<double>&)> f,
                       double t0, const std::vector<double>& y0, double t_end, double h);

ODEResult rk4_system(std::function<std::vector<double>(double, const std::vector<double>&)> f,
                     double t0, const std::vector<double>& y0, double t_end, double h);

} // namespace solvers
} // namespace psc
