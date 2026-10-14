#!/usr/bin/env bash
# Phase 1 stub. Full suite is scripts/run_cpu_benchmarks.sh (Phase 10a).
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
echo "Phase 1: no benchmark workloads. Use notebooks/02_benchmark_cpu.ipynb after Phase 10a."
if [[ -x "${ROOT}/build/psc_bench" ]]; then
  "${ROOT}/build/psc_bench" --help
elif [[ -x "${ROOT}/build/psc_bench.exe" ]]; then
  "${ROOT}/build/psc_bench.exe" --help
else
  echo "psc_bench is not built yet. Run scripts/colab_build.sh first."
fi
