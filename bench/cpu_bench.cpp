#include "cpu_bench.h"
#include "psc/linalg/matrix.h"
#include "psc/calculus/integrate.h"
#include "psc/functions/batch.h"
#include "psc/solvers/root.h"
#include "psc/solvers/ode.h"
#include <chrono>
#include <iostream>
#include <fstream>
#include <iomanip>

namespace bench {

namespace {
    double time_function(std::function<void()> func) {
        auto start = std::chrono::high_resolution_clock::now();
        func();
        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration<double, std::milli>(end - start).count();
    }
}

double bench_matrix_multiply(size_t n, int threads) {
    using namespace psc::linalg;
    
    auto setup = [n]() {
        Matrix A(n, n);
        Matrix B(n, n);
        for (size_t i = 0; i < n; ++i) {
            for (size_t j = 0; j < n; ++j) {
                A(i, j) = (i + j) % 100;
                B(i, j) = (i * j) % 100;
            }
        }
        return std::make_pair(A, B);
    };
    
    auto [A, B] = setup();
    
    auto serial_func = [&]() {
        auto C = psc::linalg::parallel::multiply_naive(A, B, 1);
    };
    
    auto parallel_func = [&]() {
        auto C = psc::linalg::parallel::multiply_tiled(A, B, 64, threads);
    };
    
    double serial_time = time_function(serial_func);
    double parallel_time = time_function(parallel_func);
    
    return parallel_time;
}

double bench_integration(size_t n, int threads) {
    using namespace psc::calculus;
    
    auto f = [](double x) { return std::sin(x); };
    
    auto serial_func = [&]() {
        psc::calculus::integrate_simpson(f, 0, 3.14159, n, 1e-10, 1);
    };
    
    auto parallel_func = [&]() {
        psc::calculus::integrate_simpson(f, 0, 3.14159, n, 1e-10, threads);
    };
    
    double serial_time = time_function(serial_func);
    double parallel_time = time_function(parallel_func);
    
    return parallel_time;
}

double bench_batch_sin(size_t n, int threads) {
    using namespace psc::functions;
    
    std::vector<double> x(n);
    for (size_t i = 0; i < n; ++i) {
        x[i] = i * 0.01;
    }
    
    auto serial_func = [&]() {
        psc::functions::batch_sin(x, 1);
    };
    
    auto parallel_func = [&]() {
        psc::functions::batch_sin(x, threads);
    };
    
    double serial_time = time_function(serial_func);
    double parallel_time = time_function(parallel_func);
    
    return parallel_time;
}

double bench_root_finding(int iterations, int threads) {
    using namespace psc::solvers;
    
    auto f = [](double x) { return x * x - 4; };
    
    auto serial_func = [&]() {
        for (int i = 0; i < iterations; ++i) {
            psc::solvers::bisection(f, 0, 5, 1e-10);
        }
    };
    
    // Root finding is inherently sequential, so parallel version runs independent instances
    auto parallel_func = [&]() {
        #pragma omp parallel for num_threads(threads)
        for (int i = 0; i < iterations; ++i) {
            psc::solvers::bisection(f, 0, 5, 1e-10);
        }
    };
    
    double serial_time = time_function(serial_func);
    double parallel_time = time_function(parallel_func);
    
    return parallel_time;
}

double bench_ode_solver(int steps, int threads) {
    using namespace psc::solvers;
    
    auto f = [](double t, double y) { return -2 * y; };
    
    auto serial_func = [&]() {
        psc::solvers::rk4(f, 0, 1, 1, 0.01);
    };
    
    // ODE solving is sequential, parallel version runs independent instances
    auto parallel_func = [&]() {
        #pragma omp parallel for num_threads(threads)
        for (int i = 0; i < steps; ++i) {
            psc::solvers::rk4(f, 0, 1, 1, 0.01);
        }
    };
    
    double serial_time = time_function(serial_func);
    double parallel_time = time_function(parallel_func);
    
    return parallel_time;
}

std::vector<BenchResult> run_cpu_benchmarks(int threads, const std::string& output_csv) {
    std::vector<BenchResult> results;
    
    // Matrix multiplication benchmark
    {
        double serial_time = time_function([&]() { bench_matrix_multiply(128, 1); });
        double parallel_time = bench_matrix_multiply(128, threads);
        double speedup = serial_time / parallel_time;
        
        results.push_back({"Matrix Multiply (128x128)", serial_time, parallel_time, threads, speedup});
    }
    
    // Integration benchmark
    {
        double serial_time = time_function([&]() { bench_integration(100000, 1); });
        double parallel_time = bench_integration(100000, threads);
        double speedup = serial_time / parallel_time;
        
        results.push_back({"Integration (100k panels)", serial_time, parallel_time, threads, speedup});
    }
    
    // Batch sin benchmark
    {
        double serial_time = time_function([&]() { bench_batch_sin(100000, 1); });
        double parallel_time = bench_batch_sin(100000, threads);
        double speedup = serial_time / parallel_time;
        
        results.push_back({"Batch Sin (100k values)", serial_time, parallel_time, threads, speedup});
    }
    
    // Root finding benchmark
    {
        double serial_time = time_function([&]() { bench_root_finding(100, 1); });
        double parallel_time = bench_root_finding(100, threads);
        double speedup = serial_time / parallel_time;
        
        results.push_back({"Root Finding (100 instances)", serial_time, parallel_time, threads, speedup});
    }
    
    // ODE solver benchmark
    {
        double serial_time = time_function([&]() { bench_ode_solver(100, 1); });
        double parallel_time = bench_ode_solver(100, threads);
        double speedup = serial_time / parallel_time;
        
        results.push_back({"ODE Solver (100 instances)", serial_time, parallel_time, threads, speedup});
    }
    
    if (!output_csv.empty()) {
        write_csv(results, output_csv);
    }
    
    return results;
}

void write_csv(const std::vector<BenchResult>& results, const std::string& filename) {
    std::ofstream out(filename);
    if (!out) {
        std::cerr << "Failed to open CSV file: " << filename << std::endl;
        return;
    }
    
    out << "name,serial_ms,parallel_ms,threads,speedup\n";
    for (const auto& r : results) {
        out << r.name << "," << r.time_serial_ms << "," << r.time_parallel_ms 
            << "," << r.threads << "," << r.speedup << "\n";
    }
    
    out.close();
    std::cout << "Benchmark results written to " << filename << std::endl;
}

} // namespace bench
