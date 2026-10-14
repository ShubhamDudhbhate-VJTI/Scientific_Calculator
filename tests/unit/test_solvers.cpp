#include <gtest/gtest.h>
#include "psc/solvers/root.h"
#include "psc/solvers/ode.h"
#include <cmath>
#include <functional>

using namespace psc::solvers;

// ============================================================================
// Root Finding Tests
// ============================================================================

class RootTest : public ::testing::Test {
protected:
    // Test function: f(x) = x^2 - 4 (roots at x = -2, 2)
    static double f_quadratic(double x) {
        return x * x - 4;
    }
    
    // Derivative: f'(x) = 2x
    static double df_quadratic(double x) {
        return 2 * x;
    }
    
    // Test function: f(x) = x^3 - x (roots at x = -1, 0, 1)
    static double f_cubic(double x) {
        return x * x * x - x;
    }
    
    // Derivative: f'(x) = 3x^2 - 1
    static double df_cubic(double x) {
        return 3 * x * x - 1;
    }
    
    // Test function: f(x) = cos(x) - x (root near 0.739)
    static double f_cos_minus_x(double x) {
        return std::cos(x) - x;
    }
    
    // Derivative: f'(x) = -sin(x) - 1
    static double df_cos_minus_x(double x) {
        return -std::sin(x) - 1;
    }
};

TEST_F(RootTest, BisectionQuadraticPositiveRoot) {
    double root = bisection(f_quadratic, 0, 5, 1e-10);
    EXPECT_NEAR(root, 2.0, 1e-8);
    EXPECT_NEAR(f_quadratic(root), 0.0, 1e-8);
}

TEST_F(RootTest, BisectionQuadraticNegativeRoot) {
    double root = bisection(f_quadratic, -5, 0, 1e-10);
    EXPECT_NEAR(root, -2.0, 1e-8);
    EXPECT_NEAR(f_quadratic(root), 0.0, 1e-8);
}

TEST_F(RootTest, BisectionSameSignThrows) {
    EXPECT_THROW(bisection(f_quadratic, 3, 5, 1e-10), std::invalid_argument);
}

TEST_F(RootTest, NewtonQuadraticPositiveRoot) {
    double root = newton(f_quadratic, df_quadratic, 3.0, 1e-10);
    EXPECT_NEAR(root, 2.0, 1e-8);
    EXPECT_NEAR(f_quadratic(root), 0.0, 1e-8);
}

TEST_F(RootTest, NewtonQuadraticNegativeRoot) {
    double root = newton(f_quadratic, df_quadratic, -3.0, 1e-10);
    EXPECT_NEAR(root, -2.0, 1e-8);
    EXPECT_NEAR(f_quadratic(root), 0.0, 1e-8);
}

TEST_F(RootTest, NewtonCosMinusX) {
    double root = newton(f_cos_minus_x, df_cos_minus_x, 0.5, 1e-10);
    EXPECT_NEAR(root, 0.7390851332151607, 1e-8);
    EXPECT_NEAR(f_cos_minus_x(root), 0.0, 1e-8);
}

TEST_F(RootTest, SecantQuadratic) {
    double root = secant(f_quadratic, 0.0, 5.0, 1e-10);
    EXPECT_NEAR(root, 2.0, 1e-8);
    EXPECT_NEAR(f_quadratic(root), 0.0, 1e-8);
}

TEST_F(RootTest, SecantCosMinusX) {
    double root = secant(f_cos_minus_x, 0.0, 1.0, 1e-10);
    EXPECT_NEAR(root, 0.7390851332151607, 1e-8);
    EXPECT_NEAR(f_cos_minus_x(root), 0.0, 1e-8);
}

TEST_F(RootTest, FalsePositionQuadratic) {
    double root = false_position(f_quadratic, 0, 5, 1e-10);
    EXPECT_NEAR(root, 2.0, 1e-8);
    EXPECT_NEAR(f_quadratic(root), 0.0, 1e-8);
}

TEST_F(RootTest, FalsePositionSameSignThrows) {
    EXPECT_THROW(false_position(f_quadratic, 3, 5, 1e-10), std::invalid_argument);
}

// ============================================================================
// ODE Solver Tests
// ============================================================================

class ODETest : public ::testing::Test {
protected:
    // Test ODE: dy/dt = -2y, solution: y(t) = y0 * exp(-2t)
    static double f_decay(double t, double y) {
        return -2.0 * y;
    }
    
    // Analytical solution
    static double y_decay_analytical(double t, double y0) {
        return y0 * std::exp(-2.0 * t);
    }
    
    // Test ODE: dy/dt = t, solution: y(t) = t^2/2 + y0
    static double f_linear(double t, double y) {
        return t;
    }
    
    // Analytical solution
    static double y_linear_analytical(double t, double y0) {
        return t * t / 2.0 + y0;
    }
};

TEST_F(ODETest, EulerDecay) {
    double t0 = 0.0, y0 = 1.0, t_end = 1.0, h = 0.01;
    ODEResult result = euler(f_decay, t0, y0, t_end, h);
    
    // Check final value against analytical solution
    double y_analytical = y_decay_analytical(t_end, y0);
    EXPECT_NEAR(result.y.back(), y_analytical, 0.02);  // Euler is first-order, expect ~2% error
}

TEST_F(ODETest, RK4Decay) {
    double t0 = 0.0, y0 = 1.0, t_end = 1.0, h = 0.1;
    ODEResult result = rk4(f_decay, t0, y0, t_end, h);
    
    // Check final value against analytical solution
    double y_analytical = y_decay_analytical(t_end, y0);
    EXPECT_NEAR(result.y.back(), y_analytical, 1e-6);  // RK4 is fourth-order, very accurate
}

TEST_F(ODETest, RK4Linear) {
    double t0 = 0.0, y0 = 0.0, t_end = 2.0, h = 0.1;
    ODEResult result = rk4(f_linear, t0, y0, t_end, h);
    
    // Check final value against analytical solution
    double y_analytical = y_linear_analytical(t_end, y0);
    EXPECT_NEAR(result.y.back(), y_analytical, 1e-10);
}

TEST_F(ODETest, RK4AdaptiveDecay) {
    double t0 = 0.0, y0 = 1.0, t_end = 1.0, h_init = 0.1;
    ODEResult result = rk4_adaptive(f_decay, t0, y0, t_end, h_init, 1e-6);
    
    // Check final value against analytical solution
    double y_analytical = y_decay_analytical(t_end, y0);
    EXPECT_NEAR(result.y.back(), y_analytical, 1e-5);
    
    // Adaptive should take fewer steps than fixed step
    size_t steps_adaptive = result.t.size();
    ODEResult result_fixed = rk4(f_decay, t0, y0, t_end, 0.01);
    size_t steps_fixed = result_fixed.t.size();
    EXPECT_LT(steps_adaptive, steps_fixed);
}

TEST_F(ODETest, EulerSystem) {
    // System: dy1/dt = -y1, dy2/dt = -2*y2
    auto f_system = [](double t, const std::vector<double>& y) -> std::vector<double> {
        return {-y[0], -2.0 * y[1]};
    };
    
    double t0 = 0.0, t_end = 1.0, h = 0.01;
    std::vector<double> y0 = {1.0, 1.0};
    
    ODEResult result = euler_system(f_system, t0, y0, t_end, h);
    
    // Result stores interleaved values: t[0], y1[0], y2[0], t[1], y1[1], y2[1], ...
    // For 2 variables, result.y.size() = 2 * result.t.size()
    EXPECT_EQ(result.y.size(), 2 * result.t.size());
    
    // Check final values against analytical solutions
    size_t n = result.t.size();
    double y1_final = result.y[2 * (n - 1)];
    double y2_final = result.y[2 * (n - 1) + 1];
    
    double y1_analytical = std::exp(-t_end);
    double y2_analytical = std::exp(-2 * t_end);
    
    EXPECT_NEAR(y1_final, y1_analytical, 0.02);
    EXPECT_NEAR(y2_final, y2_analytical, 0.02);
}

TEST_F(ODETest, RK4System) {
    // System: dy1/dt = -y1, dy2/dt = -2*y2
    auto f_system = [](double t, const std::vector<double>& y) -> std::vector<double> {
        return {-y[0], -2.0 * y[1]};
    };
    
    double t0 = 0.0, t_end = 1.0, h = 0.1;
    std::vector<double> y0 = {1.0, 1.0};
    
    ODEResult result = rk4_system(f_system, t0, y0, t_end, h);
    
    // Check final values against analytical solutions
    size_t n = result.t.size();
    double y1_final = result.y[2 * (n - 1)];
    double y2_final = result.y[2 * (n - 1) + 1];
    
    double y1_analytical = std::exp(-t_end);
    double y2_analytical = std::exp(-2 * t_end);
    
    EXPECT_NEAR(y1_final, y1_analytical, 1e-6);
    EXPECT_NEAR(y2_final, y2_analytical, 1e-6);
}
