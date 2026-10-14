#include "psc/common/config.hpp"
#include "psc/common/error.hpp"
#include "psc/common/exec.hpp"
#include "psc/common/machine_info.hpp"

#include <iostream>
#include <string>

namespace {

void print_help() {
  std::cout
      << "Parallel Scientific Calculator " << psc::project_version() << "\n"
      << "Phase 1: skeleton only. Parser and numerical modules are not implemented yet.\n\n"
      << "Commands:\n"
      << "  help       Show this help\n"
      << "  version    Print version\n"
      << "  machine    Print host facts collected at runtime\n"
      << "  threads N  Request OpenMP thread count (0 = runtime default)\n"
      << "  mode serial|parallel\n"
      << "  quit       Exit\n\n"
      << "Expression evaluation and scientific commands arrive in later phases.\n";
}

}  // namespace

int main(int argc, char** argv) {
  for (int i = 1; i < argc; ++i) {
    const std::string a = argv[i];
    if (a == "--help") {
      print_help();
      return 0;
    }
    if (a == "--threads") {
      std::cerr << "error: --threads is accepted after Phase 2; use the REPL command "
                   "'threads N' in this build, or pass nothing.\n";
      return 1;
    }
  }

  psc::ExecOptions opts;
  std::cout << "psc " << psc::project_version() << " (Phase 1 skeleton)\n"
            << "Type 'help'. Expressions are not parsed yet.\n";

  std::string line;
  while (true) {
    std::cout << "psc> " << std::flush;
    if (!std::getline(std::cin, line)) {
      std::cout << "\n";
      break;
    }
    if (line.empty()) {
      continue;
    }
    try {
      if (line == "help" || line == "?") {
        print_help();
      } else if (line == "quit" || line == "exit") {
        break;
      } else if (line == "version") {
        std::cout << psc::project_version() << "\n";
      } else if (line == "machine") {
        std::cout << psc::collect_machine_info();
      } else if (line.rfind("threads", 0) == 0) {
        const auto rest = line.size() > 7 ? line.substr(7) : std::string();
        int n = 0;
        if (rest.find_first_not_of(" \t") == std::string::npos) {
          throw psc::CalcError("usage: threads N");
        }
        n = std::stoi(rest);
        if (n < 0) {
          throw psc::CalcError("thread count must be >= 0");
        }
        opts.requested_threads = n;
        const int actual = psc::apply_thread_request(opts);
        std::cout << "requested_threads=" << n << " actual_threads=" << actual << "\n";
      } else if (line.rfind("mode", 0) == 0) {
        const auto rest = line.substr(4);
        auto pos = rest.find_first_not_of(" \t");
        if (pos == std::string::npos) {
          throw psc::CalcError("usage: mode serial|parallel");
        }
        const auto m = rest.substr(pos);
        if (m == "serial") {
          opts.mode = psc::ExecMode::Serial;
        } else if (m == "parallel") {
          opts.mode = psc::ExecMode::Parallel;
        } else {
          throw psc::CalcError("usage: mode serial|parallel");
        }
        std::cout << "mode=" << psc::mode_name(opts.mode) << "\n";
      } else {
        throw psc::CalcError(
            "unknown command in Phase 1. Type 'help'. Expression evaluation is not implemented.");
      }
    } catch (const psc::CalcError& e) {
      std::cerr << "error: " << e.what() << "\n";
    } catch (const std::exception& e) {
      std::cerr << "error: " << e.what() << "\n";
    }
  }
  return 0;
}
