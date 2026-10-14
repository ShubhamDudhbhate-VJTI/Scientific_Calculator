#include "psc/parser/tokenizer.hpp"

#include "psc/common/error.hpp"

#include <cctype>
#include <cstdlib>

namespace psc {
namespace {

bool is_ident_start(char c) {
  return std::isalpha(static_cast<unsigned char>(c)) != 0 || c == '_';
}

bool is_ident_char(char c) {
  return std::isalnum(static_cast<unsigned char>(c)) != 0 || c == '_';
}

}  // namespace

std::vector<Token> tokenize(const std::string& input) {
  std::vector<Token> tokens;
  const std::size_t n = input.size();
  std::size_t i = 0;
  bool expect_unary = true;

  auto push_op = [&](TokenKind kind, std::size_t pos, const char* text) {
    Token t;
    t.kind = kind;
    t.text = text;
    t.position = pos;
    tokens.push_back(t);
  };

  while (i < n) {
    const char c = input[i];
    if (std::isspace(static_cast<unsigned char>(c)) != 0) {
      ++i;
      continue;
    }

    if (std::isdigit(static_cast<unsigned char>(c)) != 0 ||
        (c == '.' && i + 1 < n && std::isdigit(static_cast<unsigned char>(input[i + 1])) != 0)) {
      const std::size_t start = i;
      char* end = nullptr;
      const double value = std::strtod(input.c_str() + start, &end);
      const auto consumed = static_cast<std::size_t>(end - (input.c_str() + start));
      if (consumed == 0) {
        throw CalcError("invalid number", start);
      }
      Token t;
      t.kind = TokenKind::Number;
      t.text = input.substr(start, consumed);
      t.number = value;
      t.position = start;
      tokens.push_back(t);
      i = start + consumed;
      expect_unary = false;
      continue;
    }

    if (is_ident_start(c)) {
      const std::size_t start = i;
      ++i;
      while (i < n && is_ident_char(input[i])) {
        ++i;
      }
      Token t;
      t.kind = TokenKind::Identifier;
      t.text = input.substr(start, i - start);
      t.position = start;
      tokens.push_back(t);
      expect_unary = false;
      continue;
    }

    switch (c) {
      case '+':
        push_op(TokenKind::Plus, i, "+");
        expect_unary = true;
        break;
      case '-':
        push_op(expect_unary ? TokenKind::UnaryMinus : TokenKind::Minus, i, "-");
        expect_unary = true;
        break;
      case '*':
        push_op(TokenKind::Star, i, "*");
        expect_unary = true;
        break;
      case '/':
        push_op(TokenKind::Slash, i, "/");
        expect_unary = true;
        break;
      case '^':
        push_op(TokenKind::Caret, i, "^");
        expect_unary = true;
        break;
      case '(':
        push_op(TokenKind::LParen, i, "(");
        expect_unary = true;
        break;
      case ')':
        push_op(TokenKind::RParen, i, ")");
        expect_unary = false;
        break;
      case ',':
        push_op(TokenKind::Comma, i, ",");
        expect_unary = true;
        break;
      default:
        throw CalcError(std::string("unexpected character '") + c + "'", i);
    }
    ++i;
  }

  Token end;
  end.kind = TokenKind::End;
  end.position = n;
  tokens.push_back(end);
  return tokens;
}

}  // namespace psc
