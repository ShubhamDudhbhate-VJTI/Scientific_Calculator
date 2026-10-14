#pragma once

#include "psc/common/exec.hpp"
#include "psc/functions/elem.hpp"
#include "psc/parser/ast.hpp"

#include <string>
#include <unordered_map>

namespace psc {

enum class AngleMode { Radians, Degrees };

using VarEnv = std::unordered_map<std::string, double>;

struct EvalContext {
  VarEnv variables;
  AngleMode angle = AngleMode::Radians;
  ExecOptions exec;
  FunBackend backend = FunBackend::Std;
  int fun_iters = kDefaultIters;
};

// Serial recursive evaluation. sin/cos/... dispatch through FunBackend.
double evaluate(const Expr& ast, const EvalContext& ctx);

double evaluate_string(const std::string& input, const EvalContext& ctx);

const char* angle_mode_name(AngleMode mode);

}  // namespace psc
