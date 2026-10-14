#include "psc/functions/elem.hpp"

#include <cstddef>

#ifdef _OPENMP
#include <omp.h>
#endif

namespace psc {
namespace {

void run_batch(const double* in, double* out, std::size_t n, const ExecOptions& opts,
               double (*fn)(double, FunBackend, int), FunBackend b, int iters) {
  const int threads = apply_thread_request(opts);
  const bool parallel = opts.mode == ExecMode::Parallel && n >= opts.parallel_cutoff;
  if (!parallel) {
    for (std::size_t i = 0; i < n; ++i) {
      out[i] = fn(in[i], b, iters);
    }
    return;
  }
#ifdef _OPENMP
#pragma omp parallel for num_threads(threads) schedule(static)
  for (std::ptrdiff_t i = 0; i < static_cast<std::ptrdiff_t>(n); ++i) {
    out[static_cast<std::size_t>(i)] = fn(in[static_cast<std::size_t>(i)], b, iters);
  }
#else
  (void)threads;
  for (std::size_t i = 0; i < n; ++i) {
    out[i] = fn(in[i], b, iters);
  }
#endif
}

}  // namespace

void batch_sin(const double* in, double* out, std::size_t n, FunBackend b, const ExecOptions& opts, int iters) {
  run_batch(in, out, n, opts, elem_sin, b, iters);
}

void batch_cos(const double* in, double* out, std::size_t n, FunBackend b, const ExecOptions& opts, int iters) {
  run_batch(in, out, n, opts, elem_cos, b, iters);
}

void batch_exp(const double* in, double* out, std::size_t n, FunBackend b, const ExecOptions& opts, int iters) {
  run_batch(in, out, n, opts, elem_exp, b, iters);
}

void batch_ln(const double* in, double* out, std::size_t n, FunBackend b, const ExecOptions& opts, int iters) {
  run_batch(in, out, n, opts, elem_ln, b, iters);
}

}  // namespace psc
