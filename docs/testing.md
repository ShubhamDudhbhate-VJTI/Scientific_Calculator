# Testing

Run in Colab with `notebooks/01_build_and_test.ipynb` or:

```bash
bash scripts/colab_build.sh
bash scripts/colab_build.sh --tsan
```

## Phase 1 (Colab)

- Compiler: GNU 13.3.0, OpenMP 4.5, `nproc=2`
- Release `ctest`: passed
- TSan on hello-only suite: passed (that run did not include parser tests)

## Phase 2 (Colab, recorded)

Functional: **23 tests PASSED** (Hello, Tokenizer, Parser, Eval) under the TSan binary.

TSan process still **failed** because of:

```
WARNING: ThreadSanitizer: data race
SUMMARY: ThreadSanitizer: data race tests/unit/test_hello.cpp:45
Hello_ParallelForReduction_Test::TestBody
```

Cause: `#pragma omp parallel for reduction(+:sum)` on a stack `int`. ThreadSanitizer does not understand libgomp’s reduction. The test itself printed `[ OK ]` and `sum` was correct; CTest failed because TSan exits non-zero on warnings.

Fix applied: skip `Hello.ParallelForReduction` when `__SANITIZE_THREAD__` is set. Release builds still run that OpenMP check.

Parser/eval tests had **no** TSan reports.

Re-run after skip (Colab, GCC 13.3.0): TSan `psc_tests` **Passed** (0.02 s), `100% tests passed`, `OK: ThreadSanitizer ctest finished`. The OpenMP reduction hello-test is skipped under TSan; parser/eval had no TSan reports.
