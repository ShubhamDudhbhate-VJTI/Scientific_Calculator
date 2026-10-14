#include "psc/common/error.hpp"
#include "psc/parser/parser.hpp"
#include "psc/parser/tokenizer.hpp"

#include <gtest/gtest.h>

TEST(Tokenizer, PositionsAndUnaryMinus) {
  const auto t = psc::tokenize("-2+3");
  ASSERT_GE(t.size(), 4u);
  EXPECT_EQ(t[0].kind, psc::TokenKind::UnaryMinus);
  EXPECT_EQ(t[0].position, 0u);
  EXPECT_EQ(t[1].kind, psc::TokenKind::Number);
  EXPECT_EQ(t[2].kind, psc::TokenKind::Plus);
  EXPECT_EQ(t[3].kind, psc::TokenKind::Number);
}

TEST(Tokenizer, RejectsBadCharacter) {
  EXPECT_THROW(psc::tokenize("2 $ 3"), psc::CalcError);
}

TEST(Parser, RejectsEmpty) {
  EXPECT_THROW(psc::parse_expression("   "), psc::CalcError);
}

TEST(Parser, UnmatchedParen) {
  EXPECT_THROW(psc::parse_expression("(1+2"), psc::CalcError);
  EXPECT_THROW(psc::parse_expression("1+2)"), psc::CalcError);
}

TEST(Parser, BuildsCallNode) {
  auto ast = psc::parse_expression("sin(x)");
  ASSERT_EQ(ast->kind, psc::Expr::Kind::Call);
  EXPECT_EQ(ast->name, "sin");
  ASSERT_EQ(ast->args.size(), 1u);
  EXPECT_EQ(ast->args[0]->kind, psc::Expr::Kind::Variable);
}
