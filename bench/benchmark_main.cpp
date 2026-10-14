#include "psc/common/config.hpp"
#include "psc/bench/cpu_bench.h"

#include <cstring>
#include <iostream>
#include <omp.h>

int main(int argc, char** argv) {
  int threads = 1;
  std::string output_csv = "benchmarks.csv";
  
  for (int i = 1; i < argc; ++i) {
    if (std::strcmp(argv[i], "--help") == 0) {
      std::cout
          << "Usage: psc_bench [--help] [--threads N] [--output FILE]\n"
          << "  --threads N: Number of OpenMP threads (default: 1)\n"
          << "  --output FILE: CSV output file (default: benchmarks.csv)\n";
      return 0;
    }
    if (std::strcmp(argv[i], "--threads") == 0) {
      if (i + 1 >= argc) {
        std::cerr << "error: --threads requires N\n";
        return 1;
      }
      threads = std::atoi(argv[++i]);
    }
    if (std::strcmp(argv[i], "--output") == 0) {
      if (i + 1 >= argc) {
        std::cerr << "error: --output requires FILE\n";
        return 1;
      }
      output_csv = argv[++i];
    }
  }

  std::cout << "psc_bench " << psc::project_version() << std::endl;
  std::cout << "Running CPU benchmarks with " << threads << " thread(s)" << std::endl;
  std::cout << "OpenMP max threads: " << omp_get_max_threads() << std::endl;
  
  auto results = psc::bench::run_cpu_benchmarks(threads, output_csv);
  
  std::cout << "\nBenchmark Results:" << std::endl;
  std::cout << std::left << std::setw(30) << "Name" 
            << std::setw(15) << "Serial (ms)" 
            << std::setw(15) << "Parallel (ms)" 
            << std::setw(10) << "Threads" 
            << std::setw(10) << "Speedup" << std::endl;
  std::cout << std::string(80, '-') << std::endl;
  
  for (const auto& r : results) {
    std::cout << std::left << std::setw(30) << r.name
              << std::setw(15) << r.time_serial_ms
              << std::setw(15) << r.time_parallel_ms
              << std::setw(10) << r.threads
              << std::setw(10) << r.speedup << std::endl;
  }
  
  return 0;
}
