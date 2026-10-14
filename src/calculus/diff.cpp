#include "psc/calculus/diff.hpp"

#include "psc/common/error.hpp"
#include "psc/parser/parser.hpp"

#include <cmath>

#ifdef _OPENMP
#include <omp.h>
#endif

namespace psc {

double finite_diff(const ScalarFn& f, double x, double h, DiffScheme scheme) {
  if (h == 0.0 || !std::isfinite(h)) {
    throw CalcError("finite difference step h must be nonzero and finite");
  }
  switch (scheme) {
    case DiffScheme::Forward:
      return (f(x + h) - f(x)) / h;
    case DiffScheme::Backward:
      return (f(x) - f(x - h)) / h;
    case DiffScheme::Central:
      return (f(x + h) - f(x - h)) / (2.0 * h);
  }
  throw CalcError("unknown difference scheme");
}

std::vector<StepStudyRow> step_size_study(const ScalarFn& f, double x, const ScalarFn& analytic, int n_steps) {
  if (n_steps < 2) {
    throw CalcError("step-size study needs at least 2 steps");
  }
  const double exact = analytic(x);
  std::vector<StepStudyRow> rows;
  double h = 1.0;
  for (int i = 0; i < n_steps; ++i) {
    StepStudyRow r;
    r.h = h;
    r.approx = finite_diff(f, x, h, DiffScheme::Central);
    r.abs_error = std::abs(r.approx - exact);
    rows.push_back(r);
    h *= 0.1;
  }
  return rows;
}

std::vector<double> gradient_ad(const std::string& expr, const std::vector<std::string>& names,
                                const std::vector<double>& values, AngleMode angle, const ExecOptions& exec) {
  if (names.size() != values.size() || names.empty()) {
    throw CalcError("gradient: names and values must have the same nonzero length");
  }
  auto ast = parse_expression(expr);
  const int n = static_cast<int>(names.size());
  std::vector<double> g(static_cast<std::size_t>(n), 0.0);
  bool par = exec.mode == ExecMode::Parallel && static_cast<std::size_t>(n) >= exec.parallel_cutoff;
#ifndef _OPENMP
  par = false;
#endif
  const int threads = apply_thread_request(exec);
  if (par) {
#ifdef _OPENMP
#pragma omp parallel for num_threads(threads) schedule(static)
    for (int i = 0; i < n; ++i) {
      DualEnv env;
      for (int j = 0; j < n; ++j) {
        env[names[static_cast<std::size_t>(j)]] = Dual{values[static_cast<std::size_t>(j)], j == i ? 1.0 : 0.0};
      }
      g[static_cast<std::size_t>(i)] = evaluate_dual(*ast, env, angle).d;
    }
#endif
  }
  if (!par) {
    for (int i = 0; i < n; ++i) {
      DualEnv env;
      for (int j = 0; j < n; ++j) {
        env[names[static_cast<std::size_t>(j)]] = Dual{values[static_cast<std::size_t>(j)], j == i ? 1.0 : 0.0};
      }
      g[static_cast<std::size_t>(i)] = evaluate_dual(*ast, env, angle).d;
    }
  }
  return g;
}

std::vector<double> jacobian_ad(const std::vector<std::string>& exprs, const std::vector<std::string>& names,
                                const std::vector<double>& values, AngleMode angle, const ExecOptions& exec) {
  const int m = static_cast<int>(exprs.size());
  const int n = static_cast<int>(names.size());
  if (m == 0 || names.size() != values.size()) {
    throw CalcError("jacobian: need at least one expression and matching names/values");
  }
  std::vector<double> J(static_cast<std::size_t>(m * n), 0.0);
  ExecOptions row_exec = exec;
  row_exec.mode = ExecMode::Serial;  // each row already independent
  bool par = exec.mode == ExecMode::Parallel && static_cast<std::size_t>(m) >= exec.parallel_cutoff;
#ifndef _OPENMP
  par = false;
#endif
  const int threads = apply_thread_request(exec);
  (void)threads;
  if (par) {
#ifdef _OPENMP
#pragma omp parallel for num_threads(threads) schedule(static)
    for (int r = 0; r < m; ++r) {
      auto g = gradient_ad(exprs[static_cast<std::size_t>(r)], names, values, angle, row_exec);
      for (int c = 0; c < n; ++c) {
        J[static_cast<std::size_t>(r * n + c)] = g[static_cast<std::size_t>(c)];
      }
    }
#endif
  }
  if (!par) {
    for (int r = 0; r < m; ++r) {
      auto g = gradient_ad(exprs[static_cast<std::size_t>(r)], names, values, angle, row_exec);
      for (int c = 0; c < n; ++c) {
        J[static_cast<std::size_t>(r * n + c)] = g[static_cast<std::size_t>(c)];
      }
    }
  }
  return J;
}

}  // namespace psc
