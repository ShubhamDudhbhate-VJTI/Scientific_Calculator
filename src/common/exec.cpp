#include "psc/common/exec.hpp"

#ifdef _OPENMP
#include <omp.h>
#endif

namespace psc {

int max_threads_available() {
#ifdef _OPENMP
  return omp_get_max_threads();
#else
  return 1;
#endif
}

int apply_thread_request(const ExecOptions& opts) {
#ifdef _OPENMP
  if (opts.requested_threads > 0) {
    omp_set_num_threads(opts.requested_threads);
  }
  if (opts.mode == ExecMode::Serial) {
    return 1;
  }
  return omp_get_max_threads();
#else
  (void)opts;
  return 1;
#endif
}

const char* mode_name(ExecMode mode) {
  return mode == ExecMode::Serial ? "serial" : "parallel";
}

}  // namespace psc
