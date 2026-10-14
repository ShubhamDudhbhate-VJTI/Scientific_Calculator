#pragma once

#include "psc/parser/ast.hpp"

#include <memory>
#include <string>

namespace psc {

// Shunting-yard parser: infix expression -> AST.
// Semantics (documented for tests and the viva):
//   - ^ is right-associative: 2^3^2 = 2^(3^2) = 512
//   - unary minus binds looser than ^: -2^2 = -(2^2) = -4
//   - * / bind tighter than + -
std::unique_ptr<Expr> parse_expression(const std::string& input);

}  // namespace psc
