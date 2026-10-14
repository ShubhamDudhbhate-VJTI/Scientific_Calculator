#include "psc/solvers/ode.h"
#include <cmath>
#include <algorithm>

namespace psc {
namespace solvers {

ODEResult euler(std::function<double(double, double)> f, double t0, double y0, double t_end, double h) {
    ODEResult result;
    double t = t0;
    double y = y0;
    
    result.t.push_back(t);
    result.y.push_back(y);
    
    while (t < t_end) {
        double h_step = std::min(h, t_end - t);
        y += h_step * f(t, y);
        t += h_step;
        
        result.t.push_back(t);
        result.y.push_back(y);
    }
    
    return result;
}

ODEResult rk4(std::function<double(double, double)> f, double t0, double y0, double t_end, double h) {
    ODEResult result;
    double t = t0;
    double y = y0;
    
    result.t.push_back(t);
    result.y.push_back(y);
    
    while (t < t_end) {
        double h_step = std::min(h, t_end - t);
        
        double k1 = f(t, y);
        double k2 = f(t + h_step/2, y + h_step*k1/2);
        double k3 = f(t + h_step/2, y + h_step*k2/2);
        double k4 = f(t + h_step, y + h_step*k3);
        
        y += h_step * (k1 + 2*k2 + 2*k3 + k4) / 6.0;
        t += h_step;
        
        result.t.push_back(t);
        result.y.push_back(y);
    }
    
    return result;
}

ODEResult rk4_adaptive(std::function<double(double, double)> f, double t0, double y0, double t_end,
                       double h_init, double tol, double h_min, double h_max) {
    ODEResult result;
    double t = t0;
    double y = y0;
    double h = h_init;
    
    result.t.push_back(t);
    result.y.push_back(y);
    
    while (t < t_end) {
        h = std::min(h, t_end - t);
        h = std::max(h, h_min);
        h = std::min(h, h_max);
        
        // Full step
        double k1 = f(t, y);
        double k2 = f(t + h/2, y + h*k1/2);
        double k3 = f(t + h/2, y + h*k2/2);
        double k4 = f(t + h, y + h*k3);
        double y_full = y + h * (k1 + 2*k2 + 2*k3 + k4) / 6.0;
        
        // Two half steps
        double t_half = t + h/2;
        double k1_half = f(t, y);
        double k2_half = f(t + h/4, y + h*k1_half/4);
        double k3_half = f(t + h/4, y + h*k2_half/4);
        double k4_half = f(t_half, y + h*k3_half/4);
        double y_half = y + h/2 * (k1_half + 2*k2_half + 2*k3_half + k4_half) / 6.0;
        
        k1_half = f(t_half, y_half);
        k2_half = f(t_half + h/4, y_half + h*k1_half/4);
        k3_half = f(t_half + h/4, y_half + h*k2_half/4);
        k4_half = f(t + h, y_half + h*k3_half/4);
        double y_two_half = y_half + h/2 * (k1_half + 2*k2_half + 2*k3_half + k4_half) / 6.0;
        
        // Error estimate
        double error = std::abs(y_two_half - y_full);
        
        if (error < tol) {
            // Accept step
            y = y_two_half;
            t += h;
            result.t.push_back(t);
            result.y.push_back(y);
            
            // Increase step size
            h *= 1.5;
        } else {
            // Reject step, decrease step size
            h *= 0.5;
        }
    }
    
    return result;
}

ODEResult euler_system(std::function<std::vector<double>(double, const std::vector<double>&)> f,
                       double t0, const std::vector<double>& y0, double t_end, double h) {
    ODEResult result;
    size_t n = y0.size();
    double t = t0;
    std::vector<double> y = y0;
    
    for (size_t i = 0; i < n; ++i) {
        result.t.push_back(t);
        result.y.push_back(y[i]);
    }
    
    while (t < t_end) {
        double h_step = std::min(h, t_end - t);
        std::vector<double> dydt = f(t, y);
        
        for (size_t i = 0; i < n; ++i) {
            y[i] += h_step * dydt[i];
        }
        t += h_step;
        
        for (size_t i = 0; i < n; ++i) {
            result.t.push_back(t);
            result.y.push_back(y[i]);
        }
    }
    
    return result;
}

ODEResult rk4_system(std::function<std::vector<double>(double, const std::vector<double>&)> f,
                     double t0, const std::vector<double>& y0, double t_end, double h) {
    ODEResult result;
    size_t n = y0.size();
    double t = t0;
    std::vector<double> y = y0;
    
    for (size_t i = 0; i < n; ++i) {
        result.t.push_back(t);
        result.y.push_back(y[i]);
    }
    
    while (t < t_end) {
        double h_step = std::min(h, t_end - t);
        
        std::vector<double> k1 = f(t, y);
        
        std::vector<double> y_temp = y;
        for (size_t i = 0; i < n; ++i) {
            y_temp[i] += h_step * k1[i] / 2;
        }
        std::vector<double> k2 = f(t + h_step/2, y_temp);
        
        for (size_t i = 0; i < n; ++i) {
            y_temp[i] = y[i] + h_step * k2[i] / 2;
        }
        std::vector<double> k3 = f(t + h_step/2, y_temp);
        
        for (size_t i = 0; i < n; ++i) {
            y_temp[i] = y[i] + h_step * k3[i];
        }
        std::vector<double> k4 = f(t + h_step, y_temp);
        
        for (size_t i = 0; i < n; ++i) {
            y[i] += h_step * (k1[i] + 2*k2[i] + 2*k3[i] + k4[i]) / 6.0;
        }
        t += h_step;
        
        for (size_t i = 0; i < n; ++i) {
            result.t.push_back(t);
            result.y.push_back(y[i]);
        }
    }
    
    return result;
}

} // namespace solvers
} // namespace psc
