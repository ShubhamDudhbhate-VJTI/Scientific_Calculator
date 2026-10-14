#include "psc/common/config.hpp"
#include "psc/common/error.hpp"
#include "psc/common/exec.hpp"
#include "psc/common/machine_info.hpp"
#include "psc/calculus/integrate.hpp"
#include "psc/eval/eval.hpp"

#include <chrono>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace {

void print_help() {
  std::cout
      << "Parallel Scientific Calculator " << psc::project_version() << "\n"
      << "Phase 3: serial eval plus Taylor/CORDIC elementary functions.\n\n"
      << "Commands:\n"
      << "  help                 Show this help\n"
      << "  version              Print version\n"
      << "  machine              Print host facts\n"
      << "  threads N            OpenMP thread request (0 = runtime default)\n"
      << "  mode serial|parallel Batch OpenMP (single expressions stay serial)\n"
      << "  mode deg|rad         Angle mode for trig functions\n"
      << "  backend std|taylor|cordic   Elementary-function implementation\n"
      << "  timing on|off        Print elapsed time after each expression\n"
      << "  integrate <expr> <a> <b> [--method simpson|trap|gauss5|adaptive|mc] [--n N] [--tol T]\n"
      << "  quit                 Exit\n\n"
      << "Any other line is an expression, e.g. 3 + 4 * (2 - 1)^2  or  sin(30) after mode deg.\n"
      << "Default backend is std (C++ library). Taylor/CORDIC are software loops, not FPGA.\n"
      << "CORDIC sqrt is not implemented.\n";
}

std::vector<std::string> split_ws(const std::string& line) {
  std::istringstream in(line);
  std::vector<std::string> parts;
  std::string w;
  while (in >> w) {
    parts.push_back(w);
  }
  return parts;
}

void print_result(double v, bool timing, double seconds, const psc::EvalContext& ctx, int actual_threads) {
  std::cout << std::setprecision(15) << v << "\n";
  if (timing) {
    std::cout << "time_s=" << std::setprecision(6) << seconds
              << " mode=" << psc::mode_name(ctx.exec.mode)
              << " angle=" << psc::angle_mode_name(ctx.angle)
              << " threads=" << actual_threads << "\n";
  }
}

}  // namespace

int main(int argc, char** argv) {
  psc::EvalContext ctx;
  bool timing = false;
  const char* expr_arg = nullptr;

  for (int i = 1; i < argc; ++i) {
    const std::string a = argv[i];
    if (a == "--help") {
      print_help();
      return 0;
    }
    if (a == "--threads") {
      if (i + 1 >= argc) {
        std::cerr << "error: --threads requires N\n";
        return 1;
      }
      ctx.exec.requested_threads = std::stoi(argv[++i]);
      if (ctx.exec.requested_threads < 0) {
        std::cerr << "error: thread count must be >= 0\n";
        return 1;
      }
    } else if (a == "--expr") {
      if (i + 1 >= argc) {
        std::cerr << "error: --expr requires a string\n";
        return 1;
      }
      expr_arg = argv[++i];
    } else {
      std::cerr << "error: unknown argument '" << a << "'. Try --help.\n";
      return 1;
    }
  }

  const int actual_threads = psc::apply_thread_request(ctx.exec);

  if (expr_arg != nullptr) {
    try {
      const double v = psc::evaluate_string(expr_arg, ctx);
      std::cout << std::setprecision(15) << v << "\n";
      return 0;
    } catch (const std::exception& e) {
      std::cerr << "error: " << e.what() << "\n";
      return 1;
    }
  }

  std::cout << "psc " << psc::project_version() << " (Phase 3)\n"
            << "Type 'help'. Angle=" << psc::angle_mode_name(ctx.angle)
            << " exec=" << psc::mode_name(ctx.exec.mode)
            << " backend=" << psc::backend_name(ctx.backend) << "\n";

  std::string line;
  while (true) {
    std::cout << "psc> " << std::flush;
    if (!std::getline(std::cin, line)) {
      std::cout << "\n";
      break;
    }
    if (line.find_first_not_of(" \t") == std::string::npos) {
      continue;
    }
    try {
      const auto parts = split_ws(line);
      if (parts.empty()) {
        continue;
      }
      if (parts[0] == "help" || parts[0] == "?") {
        print_help();
      } else if (parts[0] == "quit" || parts[0] == "exit") {
        break;
      } else if (parts[0] == "version") {
        std::cout << psc::project_version() << "\n";
      } else if (parts[0] == "machine") {
        std::cout << psc::collect_machine_info();
      } else if (parts[0] == "threads") {
        if (parts.size() != 2) {
          throw psc::CalcError("usage: threads N");
        }
        const int n = std::stoi(parts[1]);
        if (n < 0) {
          throw psc::CalcError("thread count must be >= 0");
        }
        ctx.exec.requested_threads = n;
        const int actual = psc::apply_thread_request(ctx.exec);
        std::cout << "requested_threads=" << n << " actual_threads=" << actual << "\n";
      } else if (parts[0] == "timing") {
        if (parts.size() != 2 || (parts[1] != "on" && parts[1] != "off")) {
          throw psc::CalcError("usage: timing on|off");
        }
        timing = parts[1] == "on";
        std::cout << "timing=" << (timing ? "on" : "off") << "\n";
      } else if (parts[0] == "backend") {
        if (parts.size() != 2) {
          throw psc::CalcError("usage: backend std|taylor|cordic");
        }
        if (parts[1] == "std") {
          ctx.backend = psc::FunBackend::Std;
        } else if (parts[1] == "taylor") {
          ctx.backend = psc::FunBackend::Taylor;
        } else if (parts[1] == "cordic") {
          ctx.backend = psc::FunBackend::Cordic;
        } else {
          throw psc::CalcError("usage: backend std|taylor|cordic");
        }
        std::cout << "backend=" << psc::backend_name(ctx.backend) << "\n";
      } else if (parts[0] == "integrate") {
        psc::QuadOptions q;
        q.exec = ctx.exec;
        q.exec.parallel_cutoff = 256;
        std::vector<std::string> pos;
        for (std::size_t i = 1; i < parts.size(); ++i) {
          if (parts[i] == "--method" && i + 1 < parts.size()) {
            q.method = psc::parse_quad_method(parts[++i]);
          } else if (parts[i] == "--n" && i + 1 < parts.size()) {
            q.n = std::stoi(parts[++i]);
          } else if (parts[i] == "--tol" && i + 1 < parts.size()) {
            q.tol = std::stod(parts[++i]);
          } else if (parts[i] == "--samples" && i + 1 < parts.size()) {
            q.samples = static_cast<std::size_t>(std::stoull(parts[++i]));
          } else {
            pos.push_back(parts[i]);
          }
        }
        if (pos.size() < 3) {
          throw psc::CalcError("usage: integrate <expr> <a> <b> [--method ...] [--n N]");
        }
        const double bnd_b = std::stod(pos.back());
        pos.pop_back();
        const double bnd_a = std::stod(pos.back());
        pos.pop_back();
        std::string expr;
        for (std::size_t i = 0; i < pos.size(); ++i) {
          if (i) {
            expr += " ";
          }
          expr += pos[i];
        }
        psc::EvalContext local = ctx;
        auto f = [&](double x) {
          local.variables["x"] = x;
          return psc::evaluate_string(expr, local);
        };
        const auto t0 = std::chrono::steady_clock::now();
        const double v = psc::integrate_1d(f, bnd_a, bnd_b, q);
        const auto t1 = std::chrono::steady_clock::now();
        const double seconds = std::chrono::duration<double>(t1 - t0).count();
        std::cout << std::setprecision(15) << v << "  [" << psc::quad_method_name(q.method) << "]\n";
        if (timing) {
          std::cout << "time_s=" << std::setprecision(6) << seconds
                    << " mode=" << psc::mode_name(ctx.exec.mode)
                    << " threads=" << actual_threads << "\n";
        }
      } else if (parts[0] == "mode") {
        if (parts.size() != 2) {
          throw psc::CalcError("usage: mode serial|parallel|deg|rad");
        }
        if (parts[1] == "serial") {
          ctx.exec.mode = psc::ExecMode::Serial;
          std::cout << "exec=" << psc::mode_name(ctx.exec.mode) << "\n";
        } else if (parts[1] == "parallel") {
          ctx.exec.mode = psc::ExecMode::Parallel;
          std::cout << "exec=" << psc::mode_name(ctx.exec.mode)
                    << " (expression eval is still serial until Phase 9)\n";
        } else if (parts[1] == "deg") {
          ctx.angle = psc::AngleMode::Degrees;
          std::cout << "angle=deg\n";
        } else if (parts[1] == "rad") {
          ctx.angle = psc::AngleMode::Radians;
          std::cout << "angle=rad\n";
        } else {
          throw psc::CalcError("usage: mode serial|parallel|deg|rad");
        }
      } else {
        const auto t0 = std::chrono::steady_clock::now();
        const double v = psc::evaluate_string(line, ctx);
        const auto t1 = std::chrono::steady_clock::now();
        const double seconds = std::chrono::duration<double>(t1 - t0).count();
        print_result(v, timing, seconds, ctx, actual_threads);
      }
    } catch (const psc::CalcError& e) {
      std::cerr << "error: " << e.what() << "\n";
    } catch (const std::exception& e) {
      std::cerr << "error: " << e.what() << "\n";
    }
  }
  return 0;
}
