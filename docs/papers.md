# Research papers (candidates)

These items are **design inspiration**. None of them has been implemented as-is. Bibliographic details below are **verification pending** until a Colab session or browser check records the real title/venue/year from the source page. Do not treat the table as verified.

| # | Topic | Candidate title | Candidate URL | Status | How we intend to use it |
|---|---|---|---|---|---|
| 1 | Reverse-mode AD | ParDiff | https://ppopp26.sigplan.org/details/PPoPP-2026-papers/8/ParDiff-Efficiently-Parallelizing-Reverse-Mode-Automatic-Differentiation-with-Direct | verification pending | Contrast with our forward-mode AD; we are not reproducing reverse-mode GPU AD in early phases. |
| 2 | Hardware CORDIC | RISC-V CORDIC accelerator | https://pure.bit.edu.cn/en/publications/a-risc-v-processor-with-optimized-cordic-based-trigonometric-func/ | verification pending | Reminder that FPGA/ASIC CORDIC is not our software loop. |
| 3 | CORDIC | Modified CORDIC for Givens rotator | https://par.nsf.gov//servlets/purl/10629123 | verification pending | Background for circular CORDIC; we will implement textbook rotation/vectoring, not a claimed recoding variant, unless we later match the paper. |
| 4 | CORDIC FPGA | Low latency recoding CORDIC | https://www.isi.edu/results/publications/25481/low-latency-recoding-cordic-algorithm-for-fpga-implementation | verification pending | Same as above. |
| 5 | GPU quadrature | Adaptive multidimensional quadrature on multi-GPU | https://arxiv.org/pdf/2511.01573 | verification pending | Inspiration for GPU composite rules; we will not claim a multi-GPU adaptive method. |
| 6 | Matmul CPU vs GPU | Accelerating matrix multiplication | https://arxiv.org/pdf/2507.19723 | verification pending | Motivation for naive vs tiled CPU and CUDA matmul comparison. |
| 7 | Parallel-in-time ODE | Newton-based parallel-in-time ODE | https://arxiv.org/pdf/2511.01465 | verification pending | We use **ensemble** RK4 parallelism, not parallel-in-time, unless a later experiment is added and documented as such. |
| 8 | GPU AD | Locality-aware AD on GPU | https://arxiv.org/abs/2509.00406 | verification pending | Out of scope for CPU forward-mode Dual numbers. |
| 9 | MPI vs OpenMP vs CUDA | Parallel paradigms in modern HPC | https://arxiv.org/abs/2506.15454 | verification pending | Explains why this project uses OpenMP + CUDA, not MPI. |
| 10 | MPI+X GEMV | Dense matrix-vector MPI+X | https://dl.acm.org/doi/10.1145/3784828.3786264 | unverified venue/year | We are not implementing MPI. |
| 11 | DaCe AD | Unifying high-performance AD | https://arxiv.org/pdf/2509.02197 | verification pending | Background only. |
| 12 | CUDA bigfloat | Arbitrary-precision on CUDA | https://arxiv.org/pdf/2608.00085 | unverified date | We use CPU limb big-integers; not CUDA floating-point AP. |

Background (classic, still to confirm page numbers in a library copy): R. P. Brent, “The Parallel Evaluation of General Arithmetic Expressions,” *Journal of the ACM*, 21 (1974). Used in Phase 9 as the work/span intuition for AST evaluation, not as a line-by-line reproduction.
