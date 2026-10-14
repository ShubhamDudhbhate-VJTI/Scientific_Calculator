#pragma once

#include <vector>
#include <string>
#include <functional>
#include <memory>

namespace psc {
namespace eval {

// Forward declarations
class EvalContext;
class ASTNode;

// Parallel batch evaluation
// Evaluates multiple expressions with different variable values in parallel

struct BatchEvalResult {
    std::vector<double> values;
    std::vector<bool> errors;
};

// Evaluate a single expression with different x values
BatchEvalResult batch_eval_x(const std::string& expr, const std::vector<double>& x_values, 
                            EvalContext& ctx, int threads = 1);

// Evaluate multiple expressions with the same context
BatchEvalResult batch_eval_exprs(const std::vector<std::string>& exprs, EvalContext& ctx, int threads = 1);

// Parameter sweep: evaluate expression f(x, param) for different param values
BatchEvalResult param_sweep(const std::string& expr, const std::string& param_name,
                           const std::vector<double>& param_values, double x, EvalContext& ctx, int threads = 1);

} // namespace eval
} // namespace psc
