# Benchmarks

Phase 1 includes only a stub `psc_bench` so the CMake target exists.

Real CPU OpenMP measurements are Phase 10a (`notebooks/02_benchmark_cpu.ipynb`).
Real GPU measurements are Phase 10b (`notebooks/03_gpu_cuda.ipynb`).

Do not commit invented CSV rows. Thread counts must be `1 .. nproc` on the Colab VM.
