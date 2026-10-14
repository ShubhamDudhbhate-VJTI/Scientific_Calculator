# Parallel Scientific Calculator (PSC)

College project: a scientific calculator whose heavy operations are implemented with **OpenMP on CPU** and **CUDA on GPU**, with honest serial/parallel comparisons.

**Execution:** write code locally; **build/test/benchmark in Google Colab**. Free Colab CPU often has **2 vCPUs**. Do not invent 4- or 8-thread CPU results.

## Current status

| Area | Status |
|---|---|
| CMake + OpenMP | **Tested** on Colab (GCC 13.3.0, OpenMP 4.5, TSan passed) |
| Parser + serial evaluator + REPL | **Tested** on Colab (23 tests + TSan after skip) |
| Taylor / CORDIC + batch OpenMP | Implemented (push and re-run notebook 01) |
| Integration, AD, matrices, solvers, bignum | Not Started |
| CUDA | Not Started (`-DUSE_CUDA=ON` rejected until Phase 10b) |
| Benchmarks | Not Started (no CSV timings) |

## Colab (Phase 2)

1. `git add -A && git commit && git push` this repo.
2. Colab **CPU**: run `notebooks/00_setup.ipynb` (clones/pulls).
3. Run `notebooks/01_build_and_test.ipynb`.
4. Do **not** run notebooks 02–04 yet.

Smoke after build:

```bash
./build/psc --expr "3 + 4 * (2 - 1)^2"
./build/psc --expr "-2^2"
```

REPL: `mode deg` then `sin(30)` should print `0.5`.

## Semantics

- `-2^2` = `-4`
- `2^3^2` = `512`
- `mode deg` / `mode rad` for trig
- `mode serial` / `mode parallel` stored for later; eval is still serial
