#pragma once

#include <vector>
#include <string>
#include <cstdint>

namespace psc {
namespace bigint {

// Limb-based big integer (base 2^32)
class BigInt {
public:
    // Constructors
    BigInt();
    BigInt(int64_t value);
    BigInt(const std::string& hex_str);  // Hex string input
    BigInt(const BigInt& other);
    BigInt(BigInt&& other) noexcept;
    ~BigInt() = default;

    // Assignment
    BigInt& operator=(const BigInt& other);
    BigInt& operator=(BigInt&& other) noexcept;

    // Comparison
    bool operator==(const BigInt& other) const;
    bool operator!=(const BigInt& other) const;
    bool operator<(const BigInt& other) const;
    bool operator<=(const BigInt& other) const;
    bool operator>(const BigInt& other) const;
    bool operator>=(const BigInt& other) const;

    // Arithmetic
    BigInt operator+(const BigInt& other) const;
    BigInt operator-(const BigInt& other) const;
    BigInt operator*(const BigInt& other) const;
    BigInt operator/(const BigInt& other) const;
    BigInt operator%(const BigInt& other) const;
    BigInt operator-() const;

    // Bitwise
    BigInt operator&(const BigInt& other) const;
    BigInt operator|(const BigInt& other) const;
    BigInt operator^(const BigInt& other) const;
    BigInt operator~() const;
    BigInt operator<<(size_t n) const;
    BigInt operator>>(size_t n) const;

    // Utility
    std::string to_hex() const;
    std::string to_decimal() const;
    size_t bit_length() const;
    bool is_zero() const;
    bool is_negative() const;

    // Statistics
    static size_t limb_count();
    static size_t max_limb_value();

private:
    std::vector<uint32_t> limbs_;  // Little-endian limb order
    bool negative_;

    void normalize();
    void add_limb(uint32_t value, size_t index);
};

} // namespace bigint
} // namespace psc
