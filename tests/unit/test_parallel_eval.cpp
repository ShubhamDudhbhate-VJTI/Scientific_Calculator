#include <gtest/gtest.h>
#include "psc/eval/parallel.h"
#include "psc/eval/eval.hpp"

using namespace psc::eval;

class ParallelEvalTest : public ::testing::Test {
protected:
    EvalContext ctx;
};

TEST_F(ParallelEvalTest, BatchEvalXLinear) {
    std::string expr = "2*x + 1";
    std::vector<double> x_values = {0, 1, 2, 3, 4};
    
    auto result = batch_eval_x(expr, x_values, ctx, 2);
    
    EXPECT_EQ(result.values.size(), 5);
    EXPECT_NEAR(result.values[0], 1.0, 1e-10);
    EXPECT_NEAR(result.values[1], 3.0, 1e-10);
    EXPECT_NEAR(result.values[2], 5.0, 1e-10);
    EXPECT_NEAR(result.values[3], 7.0, 1e-10);
    EXPECT_NEAR(result.values[4], 9.0, 1e-10);
}

TEST_F(ParallelEvalTest, BatchEvalXSquared) {
    std::string expr = "x^2";
    std::vector<double> x_values = {0, 1, 2, 3, 4};
    
    auto result = batch_eval_x(expr, x_values, ctx, 2);
    
    EXPECT_EQ(result.values.size(), 5);
    EXPECT_NEAR(result.values[0], 0.0, 1e-10);
    EXPECT_NEAR(result.values[1], 1.0, 1e-10);
    EXPECT_NEAR(result.values[2], 4.0, 1e-10);
    EXPECT_NEAR(result.values[3], 9.0, 1e-10);
    EXPECT_NEAR(result.values[4], 16.0, 1e-10);
}

TEST_F(ParallelEvalTest, BatchEvalExprs) {
    std::vector<std::string> exprs = {"1+1", "2*2", "3^2", "sqrt(16)"};
    
    auto result = batch_eval_exprs(exprs, ctx, 2);
    
    EXPECT_EQ(result.values.size(), 4);
    EXPECT_NEAR(result.values[0], 2.0, 1e-10);
    EXPECT_NEAR(result.values[1], 4.0, 1e-10);
    EXPECT_NEAR(result.values[2], 9.0, 1e-10);
    EXPECT_NEAR(result.values[3], 4.0, 1e-10);
}

TEST_F(ParallelEvalTest, ParamSweep) {
    std::string expr = "a*x";
    std::vector<double> a_values = {1, 2, 3, 4};
    double x = 5;
    
    auto result = param_sweep(expr, "a", a_values, x, ctx, 2);
    
    EXPECT_EQ(result.values.size(), 4);
    EXPECT_NEAR(result.values[0], 5.0, 1e-10);
    EXPECT_NEAR(result.values[1], 10.0, 1e-10);
    EXPECT_NEAR(result.values[2], 15.0, 1e-10);
    EXPECT_NEAR(result.values[3], 20.0, 1e-10);
}

TEST_F(ParallelEvalTest, BatchEvalWithError) {
    std::string expr = "1/0";  // Division by zero
    std::vector<double> x_values = {1, 2, 3};
    
    auto result = batch_eval_x(expr, x_values, ctx, 2);
    
    // All should have errors
    for (bool error : result.errors) {
        EXPECT_TRUE(error);
    }
}
