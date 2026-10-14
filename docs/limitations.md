# Limitations (Phase 1)

- Nothing is Colab-tested yet.
- No parser, numerical methods, or CUDA kernels.
- CPU OpenMP speedup on free Colab is limited by `nproc` (often 2).
- GPU availability on Colab is not guaranteed; notebook 03 must stop cleanly.
- GMP is not used; a limb big-integer class comes in Phase 8.
- Reverse-mode AD, Parareal, and MPI are out of scope unless added later with tests.
- ThreadSanitizer with OpenMP may be incompatible on Colab; that will be recorded from a real run.
