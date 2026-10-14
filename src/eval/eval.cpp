#include "psc/eval/eval.hpp"

#include "psc/common/error.hpp"
#include "psc/parser/parser.hpp"

#include <cmath>

namespace psc {
namespace {

constexpr double kPi = 3.14159265358979323846;

double to_radians(double x, AngleMode mode) {
  return mode == AngleMode::Degrees ? x * kPi / 180.0 : x;
}

double from_radians(double x, AngleMode mode) {
  return mode == AngleMode::Degrees ? x * 180.0 / kPi : x;
}

bool nearly_zero(double x) { return std::abs(x) < 1e-14; }

double factorial_n(double x, std::size_t pos) {
  if (x < 0.0 || !std::isfinite(x)) {
    throw CalcError("fact is defined for non-negative integers only", pos);
  }
  const double rounded = std::round(x);
  if (std::abs(x - rounded) > 1e-10) {
    throw CalcError("fact is defined for non-negative integers only", pos);
  }
  if (rounded > 170.0) {
    throw CalcError("fact overflow in double (n > 170); large n is Phase 8 bignum", pos);
  }
  double acc = 1.0;
  const int n = static_cast<int>(rounded);
  for (int i = 2; i <= n; ++i) {
    acc *= static_cast<double>(i);
  }
  return acc;
}

double call_function(const std::string& name, double x, AngleMode angle, std::size_t pos) {
  if (name == "sin") {
    return std::sin(to_radians(x, angle));
  }
  if (name == "cos") {
    return std::cos(to_radians(x, angle));
  }
  if (name == "tan") {
    const double r = to_radians(x, angle);
    const double c = std::cos(r);
    if (nearly_zero(c)) {
      throw CalcError("tan is undefined at this angle (cosine is 0)", pos);
    }
    return std::sin(r) / c;
  }
  if (name == "asin") {
    if (x < -1.0 || x > 1.0) {
      throw CalcError("asin domain is [-1, 1]", pos);
    }
    return from_radians(std::asin(x), angle);
  }
  if (name == "acos") {
    if (x < -1.0 || x > 1.0) {
      throw CalcError("acos domain is [-1, 1]", pos);
    }
    return from_radians(std::acos(x), angle);
  }
  if (name == "atan") {
    return from_radians(std::atan(x), angle);
  }
  if (name == "sinh") {
    return std::sinh(x);
  }
  if (name == "cosh") {
    return std::cosh(x);
  }
  if (name == "tanh") {
    return std::tanh(x);
  }
  if (name == "ln") {
    if (x <= 0.0) {
      throw CalcError("ln is defined for x > 0", pos);
    }
    return std::log(x);
  }
  if (name == "log10") {
    if (x <= 0.0) {
      throw CalcError("log10 is defined for x > 0", pos);
    }
    return std::log10(x);
  }
  if (name == "exp") {
    return std::exp(x);
  }
  if (name == "sqrt") {
    if (x < 0.0) {
      throw CalcError("sqrt is defined for x >= 0", pos);
    }
    return std::sqrt(x);
  }
  if (name == "abs") {
    return std::abs(x);
  }
  if (name == "fact") {
    return factorial_n(x, pos);
  }
  throw CalcError("unknown function '" + name + "'", pos);
}

double eval_node(const Expr& n, const EvalContext& ctx) {
  switch (n.kind) {
    case Expr::Kind::Literal:
      return n.value;
    case Expr::Kind::Variable: {
      auto it = ctx.variables.find(n.name);
      if (it == ctx.variables.end()) {
        throw CalcError("unknown variable '" + n.name + "'", n.position);
      }
      return it->second;
    }
    case Expr::Kind::Unary:
      return -eval_node(*n.left, ctx);
    case Expr::Kind::Binary: {
      const double a = eval_node(*n.left, ctx);
      const double b = eval_node(*n.right, ctx);
      switch (n.op) {
        case TokenKind::Plus:
          return a + b;
        case TokenKind::Minus:
          return a - b;
        case TokenKind::Star:
          return a * b;
        case TokenKind::Slash:
          if (b == 0.0) {
            throw CalcError("division by zero", n.position);
          }
          return a / b;
        case TokenKind::Caret:
          if (a < 0.0 && std::abs(b - std::round(b)) > 1e-10) {
            throw CalcError("negative base to a non-integer power is not supported", n.position);
          }
          return std::pow(a, b);
        default:
          throw CalcError("internal: unknown binary operator", n.position);
      }
    }
    case Expr::Kind::Call: {
      if (n.args.size() != 1) {
        throw CalcError("function '" + n.name + "' takes one argument", n.position);
      }
      const double x = eval_node(*n.args[0], ctx);
      const double y = call_function(n.name, x, ctx.angle, n.position);
      if (!std::isfinite(y)) {
        throw CalcError("result is not finite", n.position);
      }
      return y;
    }
  }
  throw CalcError("internal: unknown AST node");
}

}  // namespace

double evaluate(const Expr& ast, const EvalContext& ctx) { return eval_node(ast, ctx); }

double evaluate_string(const std::string& input, const EvalContext& ctx) {
  auto ast = parse_expression(input);
  return evaluate(*ast, ctx);
}

const char* angle_mode_name(AngleMode mode) {
  return mode == AngleMode::Degrees ? "deg" : "rad";
}

}  // namespace psc
