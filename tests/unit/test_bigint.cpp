#include <gtest/gtest.h>
#include "psc/bigint/bigint.h"

using namespace psc::bigint;

class BigIntTest : public ::testing::Test {};

TEST_F(BigIntTest, DefaultConstructor) {
    BigInt b;
    EXPECT_TRUE(b.is_zero());
    EXPECT_FALSE(b.is_negative());
}

TEST_F(BigIntTest, IntConstructor) {
    BigInt b1(42);
    EXPECT_FALSE(b1.is_zero());
    EXPECT_FALSE(b1.is_negative());
    EXPECT_EQ(b1.to_decimal(), "42");
    
    BigInt b2(-42);
    EXPECT_TRUE(b2.is_negative());
    EXPECT_EQ(b2.to_decimal(), "-42");
}

TEST_F(BigIntTest, HexConstructor) {
    BigInt b1("0xFF");
    EXPECT_EQ(b1.to_hex(), "ff");
    EXPECT_EQ(b1.to_decimal(), "255");
    
    BigInt b2("-0x100");
    EXPECT_TRUE(b2.is_negative());
    EXPECT_EQ(b2.to_hex(), "-100");
}

TEST_F(BigIntTest, Comparison) {
    BigInt a(100);
    BigInt b(200);
    BigInt c(100);
    
    EXPECT_TRUE(a < b);
    EXPECT_TRUE(b > a);
    EXPECT_TRUE(a == c);
    EXPECT_TRUE(a <= b);
    EXPECT_TRUE(b >= a);
    
    BigInt d(-50);
    EXPECT_TRUE(d < a);
}

TEST_F(BigIntTest, Addition) {
    BigInt a(100);
    BigInt b(200);
    BigInt c = a + b;
    EXPECT_EQ(c.to_decimal(), "300");
    
    BigInt d(-50);
    BigInt e = a + d;
    EXPECT_EQ(e.to_decimal(), "50");
}

TEST_F(BigIntTest, Subtraction) {
    BigInt a(200);
    BigInt b(100);
    BigInt c = a - b;
    EXPECT_EQ(c.to_decimal(), "100");
    
    BigInt d = b - a;
    EXPECT_EQ(d.to_decimal(), "-100");
}

TEST_F(BigIntTest, Multiplication) {
    BigInt a(12);
    BigInt b(10);
    BigInt c = a * b;
    EXPECT_EQ(c.to_decimal(), "120");
    
    BigInt d(-5);
    BigInt e = a * d;
    EXPECT_EQ(e.to_decimal(), "-60");
}

TEST_F(BigIntTest, Division) {
    BigInt a(100);
    BigInt b(5);
    BigInt c = a / b;
    EXPECT_EQ(c.to_decimal(), "20");
    
    BigInt d(-100);
    BigInt e = d / b;
    EXPECT_EQ(e.to_decimal(), "-20");
}

TEST_F(BigIntTest, Modulo) {
    BigInt a(17);
    BigInt b(5);
    BigInt c = a % b;
    EXPECT_EQ(c.to_decimal(), "2");
}

TEST_F(BigIntTest, BitwiseAnd) {
    BigInt a(0xFF);  // 255
    BigInt b(0x0F);  // 15
    BigInt c = a & b;
    EXPECT_EQ(c.to_hex(), "f");
}

TEST_F(BigIntTest, BitwiseOr) {
    BigInt a(0xF0);  // 240
    BigInt b(0x0F);  // 15
    BigInt c = a | b;
    EXPECT_EQ(c.to_hex(), "ff");
}

TEST_F(BigIntTest, BitwiseXor) {
    BigInt a(0xFF);  // 255
    BigInt b(0xFF);  // 255
    BigInt c = a ^ b;
    EXPECT_EQ(c.to_hex(), "0");
}

TEST_F(BigIntTest, LeftShift) {
    BigInt a(1);
    BigInt b = a << 8;
    EXPECT_EQ(b.to_decimal(), "256");
}

TEST_F(BigIntTest, RightShift) {
    BigInt a(256);
    BigInt b = a >> 8;
    EXPECT_EQ(b.to_decimal(), "1");
}

TEST_F(BigIntTest, BitLength) {
    BigInt a(255);
    EXPECT_EQ(a.bit_length(), 8);
    
    BigInt b(256);
    EXPECT_EQ(b.bit_length(), 9);
}

TEST_F(BigIntTest, LargeNumber) {
    BigInt a("0xFFFFFFFFFFFFFFFF");  // 2^64 - 1
    BigInt b(1);
    BigInt c = a + b;
    EXPECT_EQ(c.to_hex(), "10000000000000000");
}

TEST_F(BigIntTest, Statistics) {
    EXPECT_EQ(BigInt::limb_count(), 32);
    EXPECT_EQ(BigInt::max_limb_value(), 0xFFFFFFFF);
}
