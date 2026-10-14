# Testing

Run in Colab with `notebooks/01_build_and_test.ipynb` or:

```bash
bash scripts/colab_build.sh
bash scripts/colab_build.sh --tsan
```

## Phase 1 (Colab, recorded)

- Compiler: GNU 13.3.0
- OpenMP 4.5
- Release `ctest`: passed (hello tests) — student Phase 1 run
- ThreadSanitizer (`-fsanitize=thread`, RelWithDebInfo): **psc_tests Passed (1.03 s)**, `100% tests passed`, `OK: ThreadSanitizer ctest finished`
- Colab `nproc`: 2 (from notebook 00)

## Phase 2 tests (not Colab-tested yet)

- `tests/unit/test_parser.cpp` — tokens, unary minus, unmatched parens, `sin(x)` AST
- `tests/unit/test_eval.cpp` — precedence, `-2^2`, `2^3^2`, `sin(30)` deg, `tan(90)` deg, `ln(0)`, `sqrt(-1)`, `fact`, variables

Re-run notebook 01 after pushing Phase 2 to GitHub.
