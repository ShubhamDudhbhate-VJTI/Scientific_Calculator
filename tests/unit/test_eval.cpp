#include "psc/common/error.hpp"
#include "psc/eval/eval.hpp"

#include <cmath>
#include <gtest/gtest.h>

namespace {

double eval(const char* s, psc::AngleMode angle = psc::AngleMode::Radians) {
  psc::EvalContext ctx;
  ctx.angle = angle;
  return psc::evaluate_string(s, ctx);
}

}  // namespace

TEST(Eval, ArithmeticAndPrecedence) {
  EXPECT_DOUBLE_EQ(eval("3 + 4 * (2 - 1)^2"), 7.0);
  EXPECT_DOUBLE_EQ(eval("2+3*4"), 14.0);
  EXPECT_DOUBLE_EQ(eval("(2+3)*4"), 20.0);
}

TEST(Eval, UnaryMinusVsPower) {
  EXPECT_DOUBLE_EQ(eval("-2^2"), -4.0);
  EXPECT_DOUBLE_EQ(eval("(-2)^2"), 4.0);
}

TEST(Eval, PowerRightAssociative) {
  EXPECT_DOUBLE_EQ(eval("2^3^2"), 512.0);
}

TEST(Eval, Constants) {
  EXPECT_NEAR(eval("pi"), 3.141592653589793, 1e-15);
  EXPECT_NEAR(eval("e"), 2.718281828459045, 1e-15);
}

TEST(Eval, DivisionByZero) {
  EXPECT_THROW(eval("1/0"), psc::CalcError);
}

TEST(Eval, DegreesSin) {
  EXPECT_NEAR(eval("sin(30)", psc::AngleMode::Degrees), 0.5, 1e-12);
  EXPECT_NEAR(eval("cos(60)", psc::AngleMode::Degrees), 0.5, 1e-12);
}

TEST(Eval, Tan90Degrees) {
  EXPECT_THROW(eval("tan(90)", psc::AngleMode::Degrees), psc::CalcError);
}

TEST(Eval, LnAndSqrtDomain) {
  EXPECT_THROW(eval("ln(0)"), psc::CalcError);
  EXPECT_THROW(eval("ln(-1)"), psc::CalcError);
  EXPECT_THROW(eval("sqrt(-1)"), psc::CalcError);
}

TEST(Eval, Fact) {
  EXPECT_DOUBLE_EQ(eval("fact(5)"), 120.0);
  EXPECT_THROW(eval("fact(-1)"), psc::CalcError);
  EXPECT_THROW(eval("fact(1.5)"), psc::CalcError);
}

TEST(Eval, VariableX) {
  psc::EvalContext ctx;
  ctx.variables["x"] = 2.0;
  EXPECT_DOUBLE_EQ(psc::evaluate_string("x^3 + sin(0)", ctx), 8.0);
}

TEST(Eval, UnknownVariable) {
  EXPECT_THROW(eval("y+1"), psc::CalcError);
}

TEST(Eval, StdSinMatchesRadians) {
  EXPECT_NEAR(eval("sin(1)"), std::sin(1.0), 1e-14);
}
