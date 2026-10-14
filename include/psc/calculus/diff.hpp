#pragma once

#include "psc/calculus/dual.hpp"
#include "psc/common/exec.hpp"

#include <functional>
#include <string>
#include <vector>

namespace psc {

using ScalarFn = std::function<double(double)>;
using ScalarFnN = std::function<double(const std::vector<double>&)>;
using VectorFnN = std::function<std::vector<double>(const std::vector<double>&)>;

enum class DiffScheme { Forward, Backward, Central };

double finite_diff(const ScalarFn& f, double x, double h, DiffScheme scheme);

// h, approx derivative, abs error vs analytic (if given).
struct StepStudyRow {
  double h = 0.0;
  double approx = 0.0;
  double abs_error = 0.0;
};

std::vector<StepStudyRow> step_size_study(const ScalarFn& f, double x, const ScalarFn& analytic, int n_steps = 12);

// Forward-mode gradient: one Dual sweep per variable. Independent sweeps may run in OpenMP.
std::vector<double> gradient_ad(const std::string& expr, const std::vector<std::string>& names,
                                const std::vector<double>& values, AngleMode angle, const ExecOptions& exec);

// Jacobian of m expressions in n variables: m rows, n columns, row-major.
std::vector<double> jacobian_ad(const std::vector<std::string>& exprs, const std::vector<std::string>& names,
                                const std::vector<double>& values, AngleMode angle, const ExecOptions& exec);

}  // namespace psc
