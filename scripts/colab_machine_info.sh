#!/usr/bin/env bash
# Write Colab host facts. Never invent missing fields.
set -euo pipefail

OUT="${1:-}"
print_block() {
  echo "=== Colab machine info ==="
  echo "date: $(date -Iseconds 2>/dev/null || date)"
  echo "hostname: $(hostname 2>/dev/null || echo 'could not verify')"
  echo "nproc: $(nproc 2>/dev/null || echo 'could not verify')"
  echo "--- lscpu ---"
  if command -v lscpu >/dev/null 2>&1; then
    lscpu
  else
    echo "lscpu: not detected"
  fi
  echo "--- memory ---"
  if [[ -r /proc/meminfo ]]; then
    head -n 5 /proc/meminfo
  else
    echo "meminfo: could not verify"
  fi
  echo "--- compilers ---"
  if command -v g++ >/dev/null 2>&1; then
    g++ --version | head -n 1
  else
    echo "g++: not detected"
  fi
  if command -v cmake >/dev/null 2>&1; then
    cmake --version | head -n 1
  else
    echo "cmake: not detected"
  fi
  if command -v nvcc >/dev/null 2>&1; then
    nvcc --version
  else
    echo "nvcc: not detected"
  fi
  if command -v nvidia-smi >/dev/null 2>&1; then
    nvidia-smi -L || echo "nvidia-smi: present but no GPU listed"
  else
    echo "nvidia-smi: not detected"
  fi
  echo "--- python ---"
  python3 --version 2>/dev/null || echo "python3: not detected"
  echo "note: Free Colab CPU VMs often have 2 vCPUs. Benchmark only 1..nproc threads."
}

if [[ -n "${OUT}" ]]; then
  mkdir -p "$(dirname "${OUT}")"
  print_block | tee "${OUT}"
  echo "wrote ${OUT}"
else
  print_block
fi
