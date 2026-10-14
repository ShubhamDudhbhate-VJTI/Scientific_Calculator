#pragma once

#include <cstddef>
#include <string>

namespace psc {

enum class TokenKind {
  Number,
  Identifier,
  Plus,
  Minus,
  UnaryMinus,
  Star,
  Slash,
  Caret,
  LParen,
  RParen,
  Comma,
  End
};

struct Token {
  TokenKind kind = TokenKind::End;
  std::string text;
  double number = 0.0;
  std::size_t position = 0;
};

inline bool is_infix_operator(TokenKind k) {
  return k == TokenKind::Plus || k == TokenKind::Minus || k == TokenKind::Star ||
         k == TokenKind::Slash || k == TokenKind::Caret;
}

}  // namespace psc
