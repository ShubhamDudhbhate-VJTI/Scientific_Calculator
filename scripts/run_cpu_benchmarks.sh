#!/usr/bin/env bash
# Phase 10a entry point. Phase 1: refuse to invent timings.
set -euo pipefail
echo "CPU OpenMP benchmarks are not implemented until Phase 10a."
echo "When they exist, this script will loop threads 1..$(nproc 2>/dev/null || echo N) only."
exit 1
