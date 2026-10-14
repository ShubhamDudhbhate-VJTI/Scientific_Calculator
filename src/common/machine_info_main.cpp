#include "psc/common/machine_info.hpp"

#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>

int main(int argc, char** argv) {
  const char* out_path = nullptr;
  for (int i = 1; i < argc; ++i) {
    if (std::strcmp(argv[i], "--help") == 0) {
      std::cout << "Usage: psc_machine_info [--out FILE]\n"
                << "Print host facts. Does not invent missing fields.\n";
      return 0;
    }
    if (std::strcmp(argv[i], "--out") == 0) {
      if (i + 1 >= argc) {
        std::cerr << "error: --out requires a path\n";
        return 1;
      }
      out_path = argv[++i];
    }
  }

  const std::string text = psc::collect_machine_info();
  std::cout << text;
  if (out_path != nullptr) {
    std::ofstream out(out_path);
    if (!out) {
      std::cerr << "error: cannot write " << out_path << "\n";
      return 1;
    }
    out << text;
  }
  return 0;
}
