#include "psc/eval/parallel.h"
#include "psc/eval/eval.hpp"
#include "psc/parser/parser.hpp"
#include <omp.h>

namespace psc {
namespace eval {

BatchEvalResult batch_eval_x(const std::string& expr, const std::vector<double>& x_values, 
                            EvalContext& ctx, int threads) {
    BatchEvalResult result;
    result.values.resize(x_values.size());
    result.errors.resize(x_values.size(), false);
    
    // Parse expression once
    auto ast = parse_expression(expr);
    if (!ast) {
        for (size_t i = 0; i < x_values.size(); ++i) {
            result.errors[i] = true;
        }
        return result;
    }
    
    #pragma omp parallel for num_threads(threads) schedule(dynamic)
    for (size_t i = 0; i < x_values.size(); ++i) {
        try {
            EvalContext local_ctx = ctx;
            local_ctx.variables["x"] = x_values[i];
            result.values[i] = evaluate(*ast, local_ctx);
        } catch (...) {
            result.errors[i] = true;
        }
    }
    
    return result;
}

BatchEvalResult batch_eval_exprs(const std::vector<std::string>& exprs, EvalContext& ctx, int threads) {
    BatchEvalResult result;
    result.values.resize(exprs.size());
    result.errors.resize(exprs.size(), false);
    
    // Parse all expressions first
    std::vector<std::unique_ptr<Expr>> asts;
    for (const auto& expr : exprs) {
        auto ast = parse_expression(expr);
        if (!ast) {
            for (size_t i = 0; i < exprs.size(); ++i) {
                result.errors[i] = true;
            }
            return result;
        }
        asts.push_back(std::move(ast));
    }
    
    #pragma omp parallel for num_threads(threads) schedule(dynamic)
    for (size_t i = 0; i < exprs.size(); ++i) {
        try {
            result.values[i] = evaluate(*asts[i], ctx);
        } catch (...) {
            result.errors[i] = true;
        }
    }
    
    return result;
}

BatchEvalResult param_sweep(const std::string& expr, const std::string& param_name,
                           const std::vector<double>& param_values, double x, EvalContext& ctx, int threads) {
    BatchEvalResult result;
    result.values.resize(param_values.size());
    result.errors.resize(param_values.size(), false);
    
    // Parse expression once
    auto ast = parse_expression(expr);
    if (!ast) {
        for (size_t i = 0; i < param_values.size(); ++i) {
            result.errors[i] = true;
        }
        return result;
    }
    
    #pragma omp parallel for num_threads(threads) schedule(dynamic)
    for (size_t i = 0; i < param_values.size(); ++i) {
        try {
            EvalContext local_ctx = ctx;
            local_ctx.variables["x"] = x;
            local_ctx.variables[param_name] = param_values[i];
            result.values[i] = evaluate(*ast, local_ctx);
        } catch (...) {
            result.errors[i] = true;
        }
    }
    
    return result;
}

} // namespace eval
} // namespace psc
