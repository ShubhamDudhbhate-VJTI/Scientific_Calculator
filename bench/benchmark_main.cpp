#include "psc/common/config.hpp"

#include <cstring>
#include <iostream>

int main(int argc, char** argv) {
  for (int i = 1; i < argc; ++i) {
    if (std::strcmp(argv[i], "--help") == 0) {
      std::cout
          << "Usage: psc_bench [--help] [--threads N]\n"
          << "Phase 1: no workloads yet. CPU/GPU benchmarks are added in Phase 10.\n";
      return 0;
    }
    if (std::strcmp(argv[i], "--threads") == 0) {
      if (i + 1 >= argc) {
        std::cerr << "error: --threads requires N\n";
        return 1;
      }
      ++i;
    }
  }

  std::cout << "psc_bench " << psc::project_version()
            << " — no benchmark workloads in Phase 1.\n"
            << "This executable will write CSV files in Phase 10. "
            << "It does not emit sample timings.\n";
  return 0;
}
