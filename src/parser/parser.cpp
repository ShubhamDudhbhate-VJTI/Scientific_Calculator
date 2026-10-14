#include "psc/parser/parser.hpp"

#include "psc/common/error.hpp"
#include "psc/parser/tokenizer.hpp"

#include <utility>
#include <vector>

namespace psc {
namespace {

int precedence(TokenKind k) {
  switch (k) {
    case TokenKind::Plus:
    case TokenKind::Minus:
      return 1;
    case TokenKind::Star:
    case TokenKind::Slash:
      return 2;
    case TokenKind::UnaryMinus:
      return 3;
    case TokenKind::Caret:
      return 4;
    case TokenKind::Identifier:  // function application
      return 5;
    default:
      return 0;
  }
}

bool right_associative(TokenKind k) { return k == TokenKind::Caret || k == TokenKind::UnaryMinus; }

bool should_pop(TokenKind on_stack, TokenKind incoming) {
  const int ps = precedence(on_stack);
  const int pi = precedence(incoming);
  if (ps == 0) {
    return false;
  }
  if (right_associative(incoming)) {
    return ps > pi;
  }
  return ps >= pi;
}

std::unique_ptr<Expr> apply_operator(const Token& op, std::vector<std::unique_ptr<Expr>>& values) {
  if (op.kind == TokenKind::UnaryMinus) {
    if (values.empty()) {
      throw CalcError("unary minus is missing an operand", op.position);
    }
    auto node = std::make_unique<Expr>();
    node->kind = Expr::Kind::Unary;
    node->op = TokenKind::UnaryMinus;
    node->position = op.position;
    node->left = std::move(values.back());
    values.pop_back();
    return node;
  }
  if (op.kind == TokenKind::Identifier) {
    if (values.empty()) {
      throw CalcError("function '" + op.text + "' is missing an argument", op.position);
    }
    auto node = std::make_unique<Expr>();
    node->kind = Expr::Kind::Call;
    node->name = op.text;
    node->position = op.position;
    node->args.push_back(std::move(values.back()));
    values.pop_back();
    return node;
  }
  if (values.size() < 2) {
    throw CalcError("operator '" + op.text + "' is missing an operand", op.position);
  }
  auto node = std::make_unique<Expr>();
  node->kind = Expr::Kind::Binary;
  node->op = op.kind;
  node->position = op.position;
  node->right = std::move(values.back());
  values.pop_back();
  node->left = std::move(values.back());
  values.pop_back();
  return node;
}

}  // namespace

std::unique_ptr<Expr> parse_expression(const std::string& input) {
  if (input.find_first_not_of(" \t\r\n") == std::string::npos) {
    throw CalcError("empty expression");
  }

  const auto tokens = tokenize(input);
  std::vector<std::unique_ptr<Expr>> values;
  std::vector<Token> ops;

  for (std::size_t i = 0; i < tokens.size(); ++i) {
    const Token& tok = tokens[i];
    if (tok.kind == TokenKind::End) {
      break;
    }
    if (tok.kind == TokenKind::Number) {
      values.push_back(make_literal(tok.number, tok.position));
      continue;
    }
    if (tok.kind == TokenKind::Identifier) {
      const bool is_call =
          (i + 1 < tokens.size() && tokens[i + 1].kind == TokenKind::LParen);
      if (is_call) {
        ops.push_back(tok);
      } else if (tok.text == "pi" || tok.text == "e") {
        const double v = (tok.text == "pi") ? 3.14159265358979323846 : 2.71828182845904523536;
        values.push_back(make_literal(v, tok.position));
      } else {
        values.push_back(make_variable(tok.text, tok.position));
      }
      continue;
    }
    if (tok.kind == TokenKind::LParen) {
      ops.push_back(tok);
      continue;
    }
    if (tok.kind == TokenKind::RParen) {
      bool found = false;
      while (!ops.empty()) {
        if (ops.back().kind == TokenKind::LParen) {
          found = true;
          ops.pop_back();
          break;
        }
        values.push_back(apply_operator(ops.back(), values));
        ops.pop_back();
      }
      if (!found) {
        throw CalcError("unmatched ')'", tok.position);
      }
      // function name sitting above the '(' becomes a Call node
      if (!ops.empty() && ops.back().kind == TokenKind::Identifier) {
        values.push_back(apply_operator(ops.back(), values));
        ops.pop_back();
      }
      continue;
    }
    if (tok.kind == TokenKind::Comma) {
      bool found = false;
      while (!ops.empty()) {
        if (ops.back().kind == TokenKind::LParen) {
          found = true;
          break;
        }
        values.push_back(apply_operator(ops.back(), values));
        ops.pop_back();
      }
      if (!found) {
        throw CalcError("comma outside a function call", tok.position);
      }
      continue;
    }
    if (is_infix_operator(tok.kind) || tok.kind == TokenKind::UnaryMinus) {
      while (!ops.empty() && should_pop(ops.back().kind, tok.kind)) {
        values.push_back(apply_operator(ops.back(), values));
        ops.pop_back();
      }
      ops.push_back(tok);
      continue;
    }
    throw CalcError("unexpected token", tok.position);
  }

  while (!ops.empty()) {
    if (ops.back().kind == TokenKind::LParen) {
      throw CalcError("unmatched '('", ops.back().position);
    }
    values.push_back(apply_operator(ops.back(), values));
    ops.pop_back();
  }

  if (values.size() != 1) {
    throw CalcError("expression is incomplete or has extra operands");
  }
  return std::move(values.front());
}

}  // namespace psc
