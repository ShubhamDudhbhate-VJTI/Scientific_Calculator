#pragma once

#include <cstddef>

namespace psc {

// Serial vs OpenMP path. The serial path stays available in an OpenMP build.
enum class ExecMode { Serial, Parallel };

// Passed into modules instead of global flags so threads do not share mutable config.
struct ExecOptions {
  ExecMode mode = ExecMode::Parallel;
  // 0 means "use the OpenMP runtime default / omp_get_max_threads()".
  int requested_threads = 0;
  // Skip OpenMP for workloads smaller than this (chosen per module later).
  std::size_t parallel_cutoff = 1024;
};

// Applies requested_threads to the OpenMP runtime for the current process.
// Returns the actual thread count that will be used for Parallel mode.
int apply_thread_request(const ExecOptions& opts);

int max_threads_available();

const char* mode_name(ExecMode mode);

}  // namespace psc
