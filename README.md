# Parallel Scientific Calculator (PSC)

College project: a scientific calculator whose heavy operations are implemented with **OpenMP on CPU** and **CUDA on GPU**, with honest serial/parallel comparisons.

**Execution rule:** code is written locally; **all builds, tests, and benchmarks run in Google Colab**. Do not treat a laptop without a GPU as a measurement machine. Free Colab CPU runtimes often have **2 vCPUs**; speedup on CPU is limited by that. Meaningful speedup is expected from the **GPU** modules (Phase 10b), not from inventing 4- or 8-thread CPU results.

## Current status (Phase 1)

| Area | Status |
|---|---|
| CMake + OpenMP library | Implemented (not Colab-tested yet) |
| Hello unit tests | Implemented (not Colab-tested yet) |
| Machine-info helper | Implemented (not Colab-tested yet) |
| REPL skeleton (`help`, `quit`, `threads`, `mode`) | Implemented (not Colab-tested yet) |
| Parser / evaluator | Not Started |
| Numerical modules | Not Started |
| CUDA kernels | Not Started (CMake rejects `-DUSE_CUDA=ON`) |
| Benchmarks / plots | Not Started (no CSV numbers exist) |
| GUI | Not Started |

## Colab pipeline

1. Push this repository to GitHub (public clone is simplest).
2. Open `notebooks/00_setup.ipynb` in Colab (CPU runtime).
3. Set `REPO_URL` in the first code cell.
4. Run all cells. Results go to `/content/drive/MyDrive/parallel-sci-calc/`.
5. Then `01_build_and_test.ipynb`.

## Layout

See `docs/architecture.md`. Public headers live under `include/psc/`. CUDA sources will live under `cuda/` after Phase 10b.

## Local laptop

Do **not** benchmark here. The Colab notebooks are the supported path:

```text
apt-get install -y cmake g++ libomp-dev
bash scripts/colab_build.sh
```

Optional later: `-DUSE_CUDA=ON` (Phase 10b only).

## What is not done

Expression parsing, Taylor/CORDIC, integration, AD, matrices, solvers, big integers, statistics, and CUDA kernels. The REPL will reject unknown commands instead of faking answers.

Paper citations in `docs/papers.md` are **verification pending** until a Colab/web check is recorded.
