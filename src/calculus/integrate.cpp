#include "psc/calculus/integrate.hpp"

#include "psc/common/error.hpp"

#include <cmath>
#include <random>
#include <utility>
#include <vector>

#ifdef _OPENMP
#include <omp.h>
#endif

namespace psc {
namespace {

int even_panels(int n) {
  if (n < 2) {
    throw CalcError("integration n must be >= 2");
  }
  if (n % 2 != 0) {
    ++n;
  }
  return n;
}

double trap_serial(const Integrand1D& f, double a, double b, int n) {
  const double h = (b - a) / static_cast<double>(n);
  double sum = 0.5 * (f(a) + f(b));
  for (int i = 1; i < n; ++i) {
    sum += f(a + static_cast<double>(i) * h);
  }
  return sum * h;
}

double trap_parallel(const Integrand1D& f, double a, double b, int n, int threads) {
  const double h = (b - a) / static_cast<double>(n);
  double sum = 0.5 * (f(a) + f(b));
#ifdef _OPENMP
#pragma omp parallel for reduction(+ : sum) num_threads(threads) schedule(static)
  for (int i = 1; i < n; ++i) {
    sum += f(a + static_cast<double>(i) * h);
  }
#else
  (void)threads;
  for (int i = 1; i < n; ++i) {
    sum += f(a + static_cast<double>(i) * h);
  }
#endif
  return sum * h;
}

double simpson_serial(const Integrand1D& f, double a, double b, int n) {
  n = even_panels(n);
  const double h = (b - a) / static_cast<double>(n);
  double sum = f(a) + f(b);
  for (int i = 1; i < n; ++i) {
    const double x = a + static_cast<double>(i) * h;
    sum += (i % 2 == 0) ? 2.0 * f(x) : 4.0 * f(x);
  }
  return sum * h / 3.0;
}

double simpson_parallel(const Integrand1D& f, double a, double b, int n, int threads) {
  n = even_panels(n);
  const double h = (b - a) / static_cast<double>(n);
  double sum = f(a) + f(b);
#ifdef _OPENMP
#pragma omp parallel for reduction(+ : sum) num_threads(threads) schedule(static)
  for (int i = 1; i < n; ++i) {
    const double x = a + static_cast<double>(i) * h;
    sum += (i % 2 == 0) ? 2.0 * f(x) : 4.0 * f(x);
  }
#else
  (void)threads;
  for (int i = 1; i < n; ++i) {
    const double x = a + static_cast<double>(i) * h;
    sum += (i % 2 == 0) ? 2.0 * f(x) : 4.0 * f(x);
  }
#endif
  return sum * h / 3.0;
}

void gauss_nodes(int order, std::vector<double>& x, std::vector<double>& w) {
  x.clear();
  w.clear();
  if (order == 2) {
    const double r = 1.0 / std::sqrt(3.0);
    x = {-r, r};
    w = {1.0, 1.0};
  } else if (order == 3) {
    const double r = std::sqrt(3.0 / 5.0);
    x = {-r, 0.0, r};
    w = {5.0 / 9.0, 8.0 / 9.0, 5.0 / 9.0};
  } else if (order == 4) {
    const double a = std::sqrt((3.0 - 2.0 * std::sqrt(6.0 / 5.0)) / 7.0);
    const double b = std::sqrt((3.0 + 2.0 * std::sqrt(6.0 / 5.0)) / 7.0);
    const double wa = (18.0 + std::sqrt(30.0)) / 36.0;
    const double wb = (18.0 - std::sqrt(30.0)) / 36.0;
    x = {-b, -a, a, b};
    w = {wb, wa, wa, wb};
  } else if (order == 5) {
    const double a = (1.0 / 3.0) * std::sqrt(5.0 - 2.0 * std::sqrt(10.0 / 7.0));
    const double b = (1.0 / 3.0) * std::sqrt(5.0 + 2.0 * std::sqrt(10.0 / 7.0));
    const double wa = (322.0 + 13.0 * std::sqrt(70.0)) / 900.0;
    const double wb = (322.0 - 13.0 * std::sqrt(70.0)) / 900.0;
    x = {-b, -a, 0.0, a, b};
    w = {wb, wa, 128.0 / 225.0, wa, wb};
  } else {
    throw CalcError("Gauss–Legendre order must be 2, 3, 4, or 5");
  }
}

double gauss_composite(const Integrand1D& f, double a, double b, int n, int order, bool parallel, int threads) {
  if (n < 1) {
    throw CalcError("Gauss n (subintervals) must be >= 1");
  }
  std::vector<double> xi, wi;
  gauss_nodes(order, xi, wi);
  const double h = (b - a) / static_cast<double>(n);
  double sum = 0.0;
  if (parallel) {
#ifdef _OPENMP
#pragma omp parallel for reduction(+ : sum) num_threads(threads) schedule(static)
    for (int k = 0; k < n; ++k) {
      const double lo = a + static_cast<double>(k) * h;
      const double hi = lo + h;
      const double mid = 0.5 * (lo + hi);
      const double half = 0.5 * (hi - lo);
      double local = 0.0;
      for (std::size_t j = 0; j < xi.size(); ++j) {
        local += wi[j] * f(mid + half * xi[j]);
      }
      sum += half * local;
    }
#else
    (void)threads;
    for (int k = 0; k < n; ++k) {
      const double lo = a + static_cast<double>(k) * h;
      const double hi = lo + h;
      const double mid = 0.5 * (lo + hi);
      const double half = 0.5 * (hi - lo);
      double local = 0.0;
      for (std::size_t j = 0; j < xi.size(); ++j) {
        local += wi[j] * f(mid + half * xi[j]);
      }
      sum += half * local;
    }
#endif
  } else {
    for (int k = 0; k < n; ++k) {
      const double lo = a + static_cast<double>(k) * h;
      const double hi = lo + h;
      const double mid = 0.5 * (lo + hi);
      const double half = 0.5 * (hi - lo);
      double local = 0.0;
      for (std::size_t j = 0; j < xi.size(); ++j) {
        local += wi[j] * f(mid + half * xi[j]);
      }
      sum += half * local;
    }
  }
  return sum;
}

double simpson_three(const Integrand1D& f, double a, double b, double fa, double fm, double fb) {
  return (b - a) * (fa + 4.0 * fm + fb) / 6.0;
}

double adaptive_rec(const Integrand1D& f, double a, double b, double fa, double fm, double fb, double sab, double tol,
                    int depth, int max_depth, bool use_tasks) {
  if (depth > max_depth) {
    throw CalcError("adaptive Simpson exceeded max_depth");
  }
  const double m = 0.5 * (a + b);
  const double lm = 0.5 * (a + m);
  const double rm = 0.5 * (m + b);
  const double flm = f(lm);
  const double frm = f(rm);
  const double left = simpson_three(f, a, m, fa, flm, fm);
  const double right = simpson_three(f, m, b, fm, frm, fb);
  const double delta = left + right - sab;
  if (std::abs(delta) <= 15.0 * tol) {
    return left + right + delta / 15.0;
  }
  const double half = 0.5 * tol;
  if (use_tasks && (b - a) > 1e-3) {
    double L = 0.0;
    double R = 0.0;
#ifdef _OPENMP
#pragma omp task shared(L) default(none) firstprivate(f, a, m, fa, flm, fm, left, half, depth, max_depth)
    L = adaptive_rec(f, a, m, fa, flm, fm, left, half, depth + 1, max_depth, true);
#pragma omp task shared(R) default(none) firstprivate(f, m, b, fm, frm, fb, right, half, depth, max_depth)
    R = adaptive_rec(f, m, b, fm, frm, fb, right, half, depth + 1, max_depth, true);
#pragma omp taskwait
    return L + R;
#else
    (void)L;
    (void)R;
#endif
  }
  return adaptive_rec(f, a, m, fa, flm, fm, left, half, depth + 1, max_depth, false) +
         adaptive_rec(f, m, b, fm, frm, fb, right, half, depth + 1, max_depth, false);
}

double adaptive_simpson(const Integrand1D& f, double a, double b, const QuadOptions& opt) {
  if (!(opt.tol > 0.0)) {
    throw CalcError("adaptive tolerance must be > 0");
  }
  const double fa = f(a);
  const double fb = f(b);
  const double m = 0.5 * (a + b);
  const double fm = f(m);
  const double sab = simpson_three(f, a, b, fa, fm, fb);
  const bool par = opt.exec.mode == ExecMode::Parallel;
  if (par) {
#ifdef _OPENMP
    apply_thread_request(opt.exec);
    double out = 0.0;
#pragma omp parallel
    {
#pragma omp single
      out = adaptive_rec(f, a, b, fa, fm, fb, sab, opt.tol, 0, opt.max_depth, true);
    }
    return out;
#endif
  }
  return adaptive_rec(f, a, b, fa, fm, fb, sab, opt.tol, 0, opt.max_depth, false);
}

double monte_carlo_1d(const Integrand1D& f, double a, double b, const QuadOptions& opt) {
  if (opt.samples < 1) {
    throw CalcError("Monte Carlo samples must be >= 1");
  }
  const double width = b - a;
  const bool par = opt.exec.mode == ExecMode::Parallel && opt.samples >= opt.exec.parallel_cutoff;
  double sum = 0.0;
  if (par) {
#ifdef _OPENMP
    const int threads = apply_thread_request(opt.exec);
#pragma omp parallel num_threads(threads) reduction(+ : sum)
    {
      const int tid = omp_get_thread_num();
      const int nt = omp_get_num_threads();
      std::mt19937 rng(opt.seed + static_cast<unsigned>(tid) * 997U);
      std::uniform_real_distribution<double> dist(a, b);
      const std::size_t mine = opt.samples / static_cast<std::size_t>(nt) +
                               (static_cast<std::size_t>(tid) < opt.samples % static_cast<std::size_t>(nt) ? 1 : 0);
      for (std::size_t i = 0; i < mine; ++i) {
        sum += f(dist(rng));
      }
    }
#else
    std::mt19937 rng(opt.seed);
    std::uniform_real_distribution<double> dist(a, b);
    for (std::size_t i = 0; i < opt.samples; ++i) {
      sum += f(dist(rng));
    }
#endif
  } else {
    std::mt19937 rng(opt.seed);
    std::uniform_real_distribution<double> dist(a, b);
    for (std::size_t i = 0; i < opt.samples; ++i) {
      sum += f(dist(rng));
    }
  }
  return width * sum / static_cast<double>(opt.samples);
}

bool use_parallel(const QuadOptions& opt, int n) {
  return opt.exec.mode == ExecMode::Parallel && static_cast<std::size_t>(n) >= opt.exec.parallel_cutoff;
}

}  // namespace

QuadMethod parse_quad_method(const std::string& name) {
  if (name == "trap" || name == "trapezoid") {
    return QuadMethod::Trapezoid;
  }
  if (name == "simpson") {
    return QuadMethod::Simpson;
  }
  if (name == "gauss2") {
    return QuadMethod::Gauss2;
  }
  if (name == "gauss3") {
    return QuadMethod::Gauss3;
  }
  if (name == "gauss4") {
    return QuadMethod::Gauss4;
  }
  if (name == "gauss5") {
    return QuadMethod::Gauss5;
  }
  if (name == "adaptive") {
    return QuadMethod::Adaptive;
  }
  if (name == "mc" || name == "montecarlo") {
    return QuadMethod::MonteCarlo;
  }
  throw CalcError("unknown integrate method '" + name +
                  "' (trap|simpson|gauss2|gauss3|gauss4|gauss5|adaptive|mc)");
}

const char* quad_method_name(QuadMethod m) {
  switch (m) {
    case QuadMethod::Trapezoid:
      return "trap";
    case QuadMethod::Simpson:
      return "simpson";
    case QuadMethod::Gauss2:
      return "gauss2";
    case QuadMethod::Gauss3:
      return "gauss3";
    case QuadMethod::Gauss4:
      return "gauss4";
    case QuadMethod::Gauss5:
      return "gauss5";
    case QuadMethod::Adaptive:
      return "adaptive";
    case QuadMethod::MonteCarlo:
      return "mc";
  }
  return "unknown";
}

double integrate_1d(const Integrand1D& f, double a, double b, const QuadOptions& opt) {
  if (!std::isfinite(a) || !std::isfinite(b)) {
    throw CalcError("integration bounds must be finite");
  }
  if (a == b) {
    return 0.0;
  }
  const double sign = (b < a) ? -1.0 : 1.0;
  if (b < a) {
    std::swap(a, b);
  }
  const int threads = apply_thread_request(opt.exec);
  const bool par = use_parallel(opt, opt.n);
  double val = 0.0;
  switch (opt.method) {
    case QuadMethod::Trapezoid:
      val = par ? trap_parallel(f, a, b, opt.n, threads) : trap_serial(f, a, b, opt.n);
      break;
    case QuadMethod::Simpson:
      val = par ? simpson_parallel(f, a, b, opt.n, threads) : simpson_serial(f, a, b, opt.n);
      break;
    case QuadMethod::Gauss2:
      val = gauss_composite(f, a, b, opt.n, 2, par, threads);
      break;
    case QuadMethod::Gauss3:
      val = gauss_composite(f, a, b, opt.n, 3, par, threads);
      break;
    case QuadMethod::Gauss4:
      val = gauss_composite(f, a, b, opt.n, 4, par, threads);
      break;
    case QuadMethod::Gauss5:
      val = gauss_composite(f, a, b, opt.n, 5, par, threads);
      break;
    case QuadMethod::Adaptive:
      val = adaptive_simpson(f, a, b, opt);
      break;
    case QuadMethod::MonteCarlo:
      val = monte_carlo_1d(f, a, b, opt);
      break;
  }
  return sign * val;
}

double integrate_2d(const Integrand2D& f, double ax, double bx, double ay, double by, const QuadOptions& opt) {
  QuadOptions inner_opt = opt;
  inner_opt.exec.mode = ExecMode::Serial;  // avoid nested OpenMP
  auto inner = [&](double x) {
    return integrate_1d([&](double y) { return f(x, y); }, ay, by, inner_opt);
  };
  QuadOptions outer = opt;
  if (opt.method == QuadMethod::Adaptive || opt.method == QuadMethod::MonteCarlo) {
    outer.method = QuadMethod::Simpson;
  }
  return integrate_1d(inner, ax, bx, outer);
}

double integrate_3d(const Integrand3D& f, double ax, double bx, double ay, double by, double az, double bz,
                    const QuadOptions& opt) {
  QuadOptions inner_opt = opt;
  inner_opt.exec.mode = ExecMode::Serial;
  auto inner = [&](double x) {
    return integrate_2d([&](double y, double z) { return f(x, y, z); }, ay, by, az, bz, inner_opt);
  };
  QuadOptions outer = opt;
  if (opt.method == QuadMethod::Adaptive || opt.method == QuadMethod::MonteCarlo) {
    outer.method = QuadMethod::Simpson;
  }
  return integrate_1d(inner, ax, bx, outer);
}

}  // namespace psc
