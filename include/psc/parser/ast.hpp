#pragma once

#include "psc/parser/token.hpp"

#include <memory>
#include <string>
#include <vector>

namespace psc {

struct Expr {
  enum class Kind { Literal, Variable, Unary, Binary, Call };

  Kind kind = Kind::Literal;
  double value = 0.0;
  std::string name;
  TokenKind op = TokenKind::End;
  std::size_t position = 0;
  std::unique_ptr<Expr> left;
  std::unique_ptr<Expr> right;
  std::vector<std::unique_ptr<Expr>> args;
};

inline std::unique_ptr<Expr> make_literal(double v, std::size_t pos) {
  auto n = std::make_unique<Expr>();
  n->kind = Expr::Kind::Literal;
  n->value = v;
  n->position = pos;
  return n;
}

inline std::unique_ptr<Expr> make_variable(std::string name, std::size_t pos) {
  auto n = std::make_unique<Expr>();
  n->kind = Expr::Kind::Variable;
  n->name = std::move(name);
  n->position = pos;
  return n;
}

}  // namespace psc
