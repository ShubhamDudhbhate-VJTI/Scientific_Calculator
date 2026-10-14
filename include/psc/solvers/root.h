#pragma once

#include <functional>
#include <stdexcept>

namespace psc {
namespace solvers {

// Root finding methods
// Function signature: double f(double x)

// Bisection method - guaranteed convergence for continuous functions on [a,b] with f(a)*f(b) < 0
double bisection(std::function<double(double)> f, double a, double b, double tol = 1e-10, int max_iter = 1000);

// Newton-Raphson method - fast convergence if initial guess is close to root
// Requires derivative function
double newton(std::function<double(double)> f, std::function<double(double)> df, double x0, double tol = 1e-10, int max_iter = 1000);

// Secant method - derivative-free alternative to Newton
double secant(std::function<double(double)> f, double x0, double x1, double tol = 1e-10, int max_iter = 1000);

// False position (regula falsi) - combines bisection and secant
double false_position(std::function<double(double)> f, double a, double b, double tol = 1e-10, int max_iter = 1000);

} // namespace solvers
} // namespace psc
