# Colab notebooks

| File | Runtime | When to run |
|---|---|---|
| `00_setup.ipynb` | CPU | Every new Colab session |
| `01_build_and_test.ipynb` | CPU | Phase 1 and later after each code push |
| `02_benchmark_cpu.ipynb` | CPU | Phase 10a only |
| `03_gpu_cuda.ipynb` | T4 GPU | Phase 10b only |
| `04_plots_and_report.ipynb` | CPU | Phase 10c only |

Phase 1: run **00 then 01**. Paste the full output (especially FAIL lines) back into Cursor.
