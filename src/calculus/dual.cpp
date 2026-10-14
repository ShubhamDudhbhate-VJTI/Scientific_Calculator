#include "psc/calculus/dual.hpp"

#include "psc/common/error.hpp"
#include "psc/parser/parser.hpp"

#include <cmath>

namespace psc {
namespace {

constexpr double kPi = 3.14159265358979323846;
constexpr double kLn10 = 2.30258509299404568402;

double to_rad(double x, AngleMode m) { return m == AngleMode::Degrees ? x * kPi / 180.0 : x; }
double from_rad(double x, AngleMode m) { return m == AngleMode::Degrees ? x * 180.0 / kPi : x; }
double d_to_rad(AngleMode m) { return m == AngleMode::Degrees ? kPi / 180.0 : 1.0; }
double d_from_rad(AngleMode m) { return m == AngleMode::Degrees ? 180.0 / kPi : 1.0; }

}  // namespace

Dual operator/(Dual a, Dual b) {
  if (b.v == 0.0) {
    throw CalcError("division by zero");
  }
  return {a.v / b.v, (a.d * b.v - a.v * b.d) / (b.v * b.v)};
}

Dual pow_dual(Dual a, Dual b) {
  if (a.v < 0.0 && std::abs(b.v - std::round(b.v)) > 1e-10) {
    throw CalcError("negative base to a non-integer power is not supported");
  }
  if (a.v == 0.0 && b.v <= 0.0) {
    throw CalcError("0^p is undefined for p <= 0");
  }
  const double y = std::pow(a.v, b.v);
  // d(a^b) = a^b * (b' ln a + b a'/a) for a>0
  if (a.v > 0.0) {
    return {y, y * (b.d * std::log(a.v) + b.v * a.d / a.v)};
  }
  if (a.d != 0.0 || b.d != 0.0) {
    throw CalcError("forward AD of a negative base power is not supported");
  }
  return {y, 0.0};
}

Dual dsin(Dual x, AngleMode angle) {
  const double r = to_rad(x.v, angle);
  const double s = std::sin(r);
  const double c = std::cos(r);
  return {s, c * x.d * d_to_rad(angle)};
}

Dual dcos(Dual x, AngleMode angle) {
  const double r = to_rad(x.v, angle);
  return {std::cos(r), -std::sin(r) * x.d * d_to_rad(angle)};
}

Dual dtan(Dual x, AngleMode angle) {
  const double r = to_rad(x.v, angle);
  const double c = std::cos(r);
  if (std::abs(c) < 1e-14) {
    throw CalcError("tan is undefined at this angle");
  }
  const double t = std::sin(r) / c;
  const double sec2 = 1.0 / (c * c);
  return {t, sec2 * x.d * d_to_rad(angle)};
}

Dual dasin(Dual x, AngleMode angle) {
  if (x.v < -1.0 || x.v > 1.0) {
    throw CalcError("asin domain is [-1, 1]");
  }
  const double y = std::asin(x.v);
  return {from_rad(y, angle), d_from_rad(angle) * x.d / std::sqrt(1.0 - x.v * x.v)};
}

Dual dacos(Dual x, AngleMode angle) {
  if (x.v < -1.0 || x.v > 1.0) {
    throw CalcError("acos domain is [-1, 1]");
  }
  const double y = std::acos(x.v);
  return {from_rad(y, angle), -d_from_rad(angle) * x.d / std::sqrt(1.0 - x.v * x.v)};
}

Dual datan(Dual x, AngleMode angle) {
  return {from_rad(std::atan(x.v), angle), d_from_rad(angle) * x.d / (1.0 + x.v * x.v)};
}

Dual dsinh(Dual x) { return {std::sinh(x.v), std::cosh(x.v) * x.d}; }
Dual dcosh(Dual x) { return {std::cosh(x.v), std::sinh(x.v) * x.d}; }
Dual dtanh(Dual x) {
  const double t = std::tanh(x.v);
  return {t, (1.0 - t * t) * x.d};
}

Dual dln(Dual x) {
  if (!(x.v > 0.0)) {
    throw CalcError("ln is defined for x > 0");
  }
  return {std::log(x.v), x.d / x.v};
}

Dual dlog10(Dual x) {
  const Dual y = dln(x);
  return {y.v / kLn10, y.d / kLn10};
}

Dual dexp(Dual x) {
  const double e = std::exp(x.v);
  return {e, e * x.d};
}

Dual dsqrt(Dual x) {
  if (x.v < 0.0) {
    throw CalcError("sqrt is defined for x >= 0");
  }
  if (x.v == 0.0) {
    if (x.d != 0.0) {
      throw CalcError("sqrt'(0) is undefined");
    }
    return {0.0, 0.0};
  }
  const double s = std::sqrt(x.v);
  return {s, x.d / (2.0 * s)};
}

Dual dabs(Dual x) {
  if (x.v > 0.0) {
    return {x.v, x.d};
  }
  if (x.v < 0.0) {
    return {-x.v, -x.d};
  }
  return {0.0, 0.0};
}

Dual dfact(Dual x) {
  if (x.d != 0.0) {
    throw CalcError("fact is not differentiated in forward AD (integer factorial)");
  }
  if (x.v < 0.0 || std::abs(x.v - std::round(x.v)) > 1e-10) {
    throw CalcError("fact is defined for non-negative integers only");
  }
  double acc = 1.0;
  const int n = static_cast<int>(std::round(x.v));
  for (int i = 2; i <= n; ++i) {
    acc *= static_cast<double>(i);
  }
  return {acc, 0.0};
}

Dual evaluate_dual(const Expr& ast, const DualEnv& vars, AngleMode angle) {
  switch (ast.kind) {
    case Expr::Kind::Literal:
      return {ast.value, 0.0};
    case Expr::Kind::Variable: {
      auto it = vars.find(ast.name);
      if (it == vars.end()) {
        throw CalcError("unknown variable '" + ast.name + "'", ast.position);
      }
      return it->second;
    }
    case Expr::Kind::Unary:
      return -evaluate_dual(*ast.left, vars, angle);
    case Expr::Kind::Binary: {
      const Dual a = evaluate_dual(*ast.left, vars, angle);
      const Dual b = evaluate_dual(*ast.right, vars, angle);
      switch (ast.op) {
        case TokenKind::Plus:
          return a + b;
        case TokenKind::Minus:
          return a - b;
        case TokenKind::Star:
          return a * b;
        case TokenKind::Slash:
          return a / b;
        case TokenKind::Caret:
          return pow_dual(a, b);
        default:
          throw CalcError("internal: unknown binary operator", ast.position);
      }
    }
    case Expr::Kind::Call: {
      if (ast.args.size() != 1) {
        throw CalcError("function '" + ast.name + "' takes one argument", ast.position);
      }
      const Dual x = evaluate_dual(*ast.args[0], vars, angle);
      if (ast.name == "sin") {
        return dsin(x, angle);
      }
      if (ast.name == "cos") {
        return dcos(x, angle);
      }
      if (ast.name == "tan") {
        return dtan(x, angle);
      }
      if (ast.name == "asin") {
        return dasin(x, angle);
      }
      if (ast.name == "acos") {
        return dacos(x, angle);
      }
      if (ast.name == "atan") {
        return datan(x, angle);
      }
      if (ast.name == "sinh") {
        return dsinh(x);
      }
      if (ast.name == "cosh") {
        return dcosh(x);
      }
      if (ast.name == "tanh") {
        return dtanh(x);
      }
      if (ast.name == "ln") {
        return dln(x);
      }
      if (ast.name == "log10") {
        return dlog10(x);
      }
      if (ast.name == "exp") {
        return dexp(x);
      }
      if (ast.name == "sqrt") {
        return dsqrt(x);
      }
      if (ast.name == "abs") {
        return dabs(x);
      }
      if (ast.name == "fact") {
        return dfact(x);
      }
      throw CalcError("unknown function '" + ast.name + "'", ast.position);
    }
  }
  throw CalcError("internal: unknown AST node");
}

Dual evaluate_dual_string(const std::string& input, const DualEnv& vars, AngleMode angle) {
  auto ast = parse_expression(input);
  return evaluate_dual(*ast, vars, angle);
}

}  // namespace psc
