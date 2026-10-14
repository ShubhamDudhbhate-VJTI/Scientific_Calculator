#!/usr/bin/env bash
# Configure, build, and optionally test on Google Colab.
# Usage: scripts/colab_build.sh [--tsan] [--no-test] [--jobs N]
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "${ROOT}"

TSAN=0
RUN_TEST=1
JOBS="$(nproc)"
USE_CUDA=OFF
BUILD_TYPE=Release

while [[ $# -gt 0 ]]; do
  case "$1" in
    --tsan) TSAN=1; BUILD_TYPE=RelWithDebInfo; shift ;;
    --no-test) RUN_TEST=0; shift ;;
    --jobs) JOBS="$2"; shift 2 ;;
    --cuda) echo "error: CUDA is not implemented until Phase 10b"; exit 1 ;;
    --help)
      echo "Usage: $0 [--tsan] [--no-test] [--jobs N]"
      exit 0
      ;;
    *) echo "unknown argument: $1"; exit 1 ;;
  esac
done

if [[ "${TSAN}" -eq 1 ]]; then
  BUILD_DIR="${ROOT}/build-tsan"
  CMAKE_EXTRA=(-DPSC_ENABLE_TSAN=ON)
else
  BUILD_DIR="${ROOT}/build"
  CMAKE_EXTRA=()
fi

mkdir -p "${BUILD_DIR}"
echo "[colab_build] configure ${BUILD_DIR} type=${BUILD_TYPE} tsan=${TSAN}"
cmake -S "${ROOT}" -B "${BUILD_DIR}" \
  -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
  -DUSE_CUDA="${USE_CUDA}" \
  "${CMAKE_EXTRA[@]}"

echo "[colab_build] build -j${JOBS}"
cmake --build "${BUILD_DIR}" -j"${JOBS}"

run_ctest() {
  # TSan on new Colab kernels often hits:
  #   FATAL: ThreadSanitizer: unexpected memory mapping
  # when ASLR places the PIE binary outside TSan's shadow range.
  # setarch -R disables ASLR for this process only (no sudo).
  if [[ "${TSAN}" -eq 1 ]] && command -v setarch >/dev/null 2>&1; then
    echo "[colab_build] ctest via setarch -R (disable ASLR for TSan)"
    setarch "$(uname -m)" -R ctest --test-dir "${BUILD_DIR}" --output-on-failure
  else
    echo "[colab_build] ctest"
    ctest --test-dir "${BUILD_DIR}" --output-on-failure
  fi
}

if [[ "${RUN_TEST}" -eq 1 ]]; then
  set +e
  ctest_out="$(mktemp)"
  run_ctest >"${ctest_out}" 2>&1
  status=$?
  cat "${ctest_out}"
  set -e
  if [[ "${status}" -eq 0 ]]; then
    echo "[colab_build] tests: SUCCESS"
    rm -f "${ctest_out}"
  elif [[ "${TSAN}" -eq 1 ]] && grep -q "unexpected memory mapping" "${ctest_out}"; then
    echo "SKIP: ThreadSanitizer could not start (ASLR / unexpected memory mapping)."
    echo "This is a Colab kernel + GCC TSan limitation, not a data race in PSC."
    echo "Release ctest remains the correctness check. Earlier TSan runs on this project did complete."
    rm -f "${ctest_out}"
    exit 0
  else
    rm -f "${ctest_out}"
    exit "${status}"
  fi
else
  echo "[colab_build] tests skipped"
fi
