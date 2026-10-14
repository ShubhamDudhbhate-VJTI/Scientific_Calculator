# Benchmarking

**Not yet measured.** No CSV files with timings exist.

Protocol (Phase 10):

- Serial implementation is the baseline `T_serial` (not “OpenMP with 1 thread”, unless we also report that separately).
- Threads: `1, 2, ..., nproc` only. Colab CPU often has `nproc=2`.
- Warm-up discarded; at least 5 timed repetitions; mean and sample standard deviation.
- `speedup S(p) = T_serial / T_p`, `efficiency E(p) = S(p) / p`.
- GPU rows: `threads=0`, `device=gpu`, kernel time vs H2D/D2H time recorded separately.
- Plots read CSVs; they never hard-code numbers.

Results live on Google Drive: `/content/drive/MyDrive/parallel-sci-calc/results/`.
