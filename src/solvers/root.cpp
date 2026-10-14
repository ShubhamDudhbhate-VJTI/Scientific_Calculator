#include "psc/solvers/root.h"
#include <cmath>
#include <iostream>

namespace psc {
namespace solvers {

double bisection(std::function<double(double)> f, double a, double b, double tol, int max_iter) {
    double fa = f(a);
    double fb = f(b);
    
    if (fa * fb > 0) {
        throw std::invalid_argument("Bisection requires f(a) and f(b) to have opposite signs");
    }
    
    if (std::abs(fa) < tol) return a;
    if (std::abs(fb) < tol) return b;
    
    double c;
    for (int i = 0; i < max_iter; ++i) {
        c = (a + b) / 2.0;
        double fc = f(c);
        
        if (std::abs(fc) < tol || (b - a) / 2.0 < tol) {
            return c;
        }
        
        if (fa * fc < 0) {
            b = c;
            fb = fc;
        } else {
            a = c;
            fa = fc;
        }
    }
    
    return c;  // Return best estimate after max iterations
}

double newton(std::function<double(double)> f, std::function<double(double)> df, double x0, double tol, int max_iter) {
    double x = x0;
    
    for (int i = 0; i < max_iter; ++i) {
        double fx = f(x);
        double dfx = df(x);
        
        if (std::abs(dfx) < 1e-15) {
            throw std::runtime_error("Newton method: derivative is zero");
        }
        
        double x_new = x - fx / dfx;
        
        if (std::abs(x_new - x) < tol) {
            return x_new;
        }
        
        x = x_new;
    }
    
    return x;  // Return best estimate after max iterations
}

double secant(std::function<double(double)> f, double x0, double x1, double tol, int max_iter) {
    double x_prev = x0;
    double x_curr = x1;
    double f_prev = f(x_prev);
    double f_curr = f(x_curr);
    
    if (std::abs(f_curr) < tol) return x_curr;
    
    for (int i = 0; i < max_iter; ++i) {
        if (std::abs(f_curr - f_prev) < 1e-15) {
            throw std::runtime_error("Secant method: denominator is zero");
        }
        
        double x_new = x_curr - f_curr * (x_curr - x_prev) / (f_curr - f_prev);
        
        if (std::abs(x_new - x_curr) < tol) {
            return x_new;
        }
        
        x_prev = x_curr;
        f_prev = f_curr;
        x_curr = x_new;
        f_curr = f(x_curr);
    }
    
    return x_curr;
}

double false_position(std::function<double(double)> f, double a, double b, double tol, int max_iter) {
    double fa = f(a);
    double fb = f(b);
    
    if (fa * fb > 0) {
        throw std::invalid_argument("False position requires f(a) and f(b) to have opposite signs");
    }
    
    if (std::abs(fa) < tol) return a;
    if (std::abs(fb) < tol) return b;
    
    double c;
    for (int i = 0; i < max_iter; ++i) {
        // Secant-like interpolation
        c = a - fa * (b - a) / (fb - fa);
        double fc = f(c);
        
        if (std::abs(fc) < tol) {
            return c;
        }
        
        if (fa * fc < 0) {
            b = c;
            fb = fc;
        } else {
            a = c;
            fa = fc;
        }
    }
    
    return c;
}

} // namespace solvers
} // namespace psc
