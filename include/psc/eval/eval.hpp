#pragma once

#include "psc/common/exec.hpp"
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
};

// Serial recursive evaluation. Phase 3 will swap std::sin etc. for Taylor/CORDIC.
double evaluate(const Expr& ast, const EvalContext& ctx);

double evaluate_string(const std::string& input, const EvalContext& ctx);

const char* angle_mode_name(AngleMode mode);

}  // namespace psc
