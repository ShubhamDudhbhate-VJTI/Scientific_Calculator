#pragma once

#include "psc/common/exec.hpp"

#include <cstddef>
#include <vector>

namespace psc {

// Software elementary functions. This is not FPGA CORDIC: it is a sequential
// loop in C++. Parallelism is over batches of inputs (OpenMP), not the circuit.
enum class FunBackend { Std, Taylor, Cordic };

constexpr int kDefaultIters = 40;
// Term-level Taylor parallelism is only worth it for a huge iteration count.
constexpr int kTaylorTermParallelCutoff = 256;

const char* backend_name(FunBackend b);

double elem_sin(double x, FunBackend b, int iters = kDefaultIters);
double elem_cos(double x, FunBackend b, int iters = kDefaultIters);
double elem_tan(double x, FunBackend b, int iters = kDefaultIters);
double elem_sinh(double x, FunBackend b, int iters = kDefaultIters);
double elem_cosh(double x, FunBackend b, int iters = kDefaultIters);
double elem_exp(double x, FunBackend b, int iters = kDefaultIters);
double elem_ln(double x, FunBackend b, int iters = kDefaultIters);
double elem_sqrt(double x, FunBackend b, int iters = kDefaultIters);
double elem_atan(double x, FunBackend b, int iters = kDefaultIters);

struct AccuracyReport {
  double max_abs = 0.0;
  double max_rel = 0.0;  // only where |reference| >= 1e-12
  int points = 0;
};

AccuracyReport compare_to_std_sin(FunBackend b, double lo, double hi, int n, int iters = kDefaultIters);

void batch_sin(const double* in, double* out, std::size_t n, FunBackend b, const ExecOptions& opts,
               int iters = kDefaultIters);
void batch_cos(const double* in, double* out, std::size_t n, FunBackend b, const ExecOptions& opts,
               int iters = kDefaultIters);
void batch_exp(const double* in, double* out, std::size_t n, FunBackend b, const ExecOptions& opts,
               int iters = kDefaultIters);
void batch_ln(const double* in, double* out, std::size_t n, FunBackend b, const ExecOptions& opts,
              int iters = kDefaultIters);

}  // namespace psc
