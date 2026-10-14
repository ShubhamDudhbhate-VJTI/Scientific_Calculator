#pragma once

#include <string>
#include <vector>
#include <functional>

namespace psc {
namespace bench {

// Benchmark result
struct BenchResult {
    std::string name;
    double time_serial_ms;
    double time_parallel_ms;
    int threads;
    double speedup;
};

// Run CPU benchmarks and return results
std::vector<BenchResult> run_cpu_benchmarks(int threads = 1, const std::string& output_csv = "");

// Individual benchmark functions
double bench_matrix_multiply(size_t n, int threads);
double bench_integration(size_t n, int threads);
double bench_batch_sin(size_t n, int threads);
double bench_root_finding(int iterations, int threads);
double bench_ode_solver(int steps, int threads);

// Write benchmark results to CSV
void write_csv(const std::vector<BenchResult>& results, const std::string& filename);

} // namespace bench
} // namespace psc
