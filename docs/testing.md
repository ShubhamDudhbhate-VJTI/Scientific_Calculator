# Testing

Phase 1: GoogleTest via CMake FetchContent. Run in Colab with `notebooks/01_build_and_test.ipynb` or:

```bash
bash scripts/colab_build.sh
bash scripts/colab_build.sh --tsan
```

Current tests (`tests/unit/test_hello.cpp`): version string, OpenMP compiled in, serial mode forces 1 thread, `CalcError` position, machine-info text, a tiny OpenMP reduction.

Sanitizer results: **not yet run**. Do not claim ThreadSanitizer is clean until notebook 01 finishes on Colab.

ThreadSanitizer + OpenMP can be noisy or unsupported on some GCC/libgomp combinations. If Colab fails, paste the log; we will record the exact limitation rather than fake a pass.
