#include "psc/bigint/bigint.h"
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <stdexcept>

namespace psc {
namespace bigint {

namespace {
    const uint64_t LIMB_BASE = 1ULL << 32;
    const uint32_t LIMB_MASK = 0xFFFFFFFF;
}

BigInt::BigInt() : limbs_({0}), negative_(false) {}

BigInt::BigInt(int64_t value) {
    negative_ = value < 0;
    uint64_t abs_val = negative_ ? -static_cast<uint64_t>(value) : static_cast<uint64_t>(value);
    
    limbs_.push_back(static_cast<uint32_t>(abs_val & LIMB_MASK));
    if (abs_val > LIMB_MASK) {
        limbs_.push_back(static_cast<uint32_t>((abs_val >> 32) & LIMB_MASK));
    }
    normalize();
}

BigInt::BigInt(const std::string& hex_str) {
    negative_ = false;
    if (hex_str.empty()) {
        limbs_ = {0};
        return;
    }
    
    size_t start = 0;
    if (hex_str[0] == '-') {
        negative_ = true;
        start = 1;
    }
    
    // Parse hex string
    std::string hex = hex_str.substr(start);
    if (hex.substr(0, 2) == "0x" || hex.substr(0, 2) == "0X") {
        hex = hex.substr(2);
    }
    
    // Convert to limbs (8 hex chars = 32 bits = 1 limb)
    size_t hex_len = hex.length();
    size_t num_limbs = (hex_len + 7) / 8;
    limbs_.resize(num_limbs);
    
    for (size_t i = 0; i < num_limbs; ++i) {
        size_t limb_start = hex_len - 8 * (i + 1);
        size_t limb_len = std::min(size_t(8), hex_len - 8 * i);
        std::string limb_hex = hex.substr(limb_start, limb_len);
        limbs_[i] = static_cast<uint32_t>(std::stoull(limb_hex, nullptr, 16));
    }
    
    normalize();
}

BigInt::BigInt(const BigInt& other) : limbs_(other.limbs_), negative_(other.negative_) {}

BigInt::BigInt(BigInt&& other) noexcept : limbs_(std::move(other.limbs_)), negative_(other.negative_) {}

BigInt& BigInt::operator=(const BigInt& other) {
    if (this != &other) {
        limbs_ = other.limbs_;
        negative_ = other.negative_;
    }
    return *this;
}

BigInt& BigInt::operator=(BigInt&& other) noexcept {
    if (this != &other) {
        limbs_ = std::move(other.limbs_);
        negative_ = other.negative_;
    }
    return *this;
}

void BigInt::normalize() {
    while (limbs_.size() > 1 && limbs_.back() == 0) {
        limbs_.pop_back();
    }
    if (limbs_.size() == 1 && limbs_[0] == 0) {
        negative_ = false;
    }
}

bool BigInt::operator==(const BigInt& other) const {
    if (negative_ != other.negative_) return false;
    return limbs_ == other.limbs_;
}

bool BigInt::operator!=(const BigInt& other) const {
    return !(*this == other);
}

bool BigInt::operator<(const BigInt& other) const {
    if (negative_ != other.negative_) {
        return negative_ && !other.negative_;
    }
    if (negative_) {
        // Both negative: compare absolute values, reverse result
        if (limbs_.size() != other.limbs_.size()) {
            return limbs_.size() > other.limbs_.size();
        }
        for (int i = limbs_.size() - 1; i >= 0; --i) {
            if (limbs_[i] != other.limbs_[i]) {
                return limbs_[i] > other.limbs_[i];
            }
        }
        return false;
    }
    // Both positive
    if (limbs_.size() != other.limbs_.size()) {
        return limbs_.size() < other.limbs_.size();
    }
    for (int i = limbs_.size() - 1; i >= 0; --i) {
        if (limbs_[i] != other.limbs_[i]) {
            return limbs_[i] < other.limbs_[i];
        }
    }
    return false;
}

bool BigInt::operator<=(const BigInt& other) const {
    return *this < other || *this == other;
}

bool BigInt::operator>(const BigInt& other) const {
    return !(*this <= other);
}

bool BigInt::operator>=(const BigInt& other) const {
    return !(*this < other);
}

BigInt BigInt::operator+(const BigInt& other) const {
    if (negative_ == other.negative_) {
        // Same sign: add absolute values
        BigInt result;
        result.negative_ = negative_;
        result.limbs_.resize(std::max(limbs_.size(), other.limbs_.size()) + 1);
        
        uint64_t carry = 0;
        for (size_t i = 0; i < result.limbs_.size(); ++i) {
            uint64_t sum = carry;
            if (i < limbs_.size()) sum += limbs_[i];
            if (i < other.limbs_.size()) sum += other.limbs_[i];
            result.limbs_[i] = static_cast<uint32_t>(sum & LIMB_MASK);
            carry = sum >> 32;
        }
        result.normalize();
        return result;
    } else {
        // Different signs: subtract
        if (is_negative()) {
            return -((-other) - (-*this));
        } else {
            return *this - (-other);
        }
    }
}

BigInt BigInt::operator-(const BigInt& other) const {
    if (negative_ != other.negative_) {
        // Different signs: add absolute values
        if (negative_) {
            return -((-other) + (-*this));
        } else {
            return *this + (-other);
        }
    }
    
    // Same sign: subtract absolute values
    const BigInt* a = this;
    const BigInt* b = &other;
    bool result_negative = false;
    
    if (*this < other) {
        a = &other;
        b = this;
        result_negative = negative_;
    }
    
    BigInt result;
    result.negative_ = result_negative;
    result.limbs_.resize(a->limbs_.size());
    
    uint64_t borrow = 0;
    for (size_t i = 0; i < result.limbs_.size(); ++i) {
        uint64_t diff = static_cast<uint64_t>(a->limbs_[i]) - borrow;
        if (i < b->limbs_.size()) diff -= b->limbs_[i];
        result.limbs_[i] = static_cast<uint32_t>(diff & LIMB_MASK);
        borrow = (diff >> 32) & 1;
    }
    result.normalize();
    return result;
}

BigInt BigInt::operator*(const BigInt& other) const {
    BigInt result;
    result.negative_ = negative_ != other.negative_;
    result.limbs_.resize(limbs_.size() + other.limbs_.size(), 0);
    
    for (size_t i = 0; i < limbs_.size(); ++i) {
        uint64_t carry = 0;
        for (size_t j = 0; j < other.limbs_.size(); ++j) {
            uint64_t product = static_cast<uint64_t>(limbs_[i]) * other.limbs_[j];
            product += result.limbs_[i + j] + carry;
            result.limbs_[i + j] = static_cast<uint32_t>(product & LIMB_MASK);
            carry = product >> 32;
        }
        if (carry) {
            result.limbs_[i + other.limbs_.size()] += static_cast<uint32_t>(carry);
        }
    }
    result.normalize();
    return result;
}

BigInt BigInt::operator/(const BigInt& other) const {
    if (other.is_zero()) {
        throw std::runtime_error("Division by zero");
    }
    
    BigInt abs_this = negative_ ? -*this : *this;
    BigInt abs_other = other.negative_ ? -other : other;
    
    if (abs_this < abs_other) {
        return BigInt(0);
    }
    
    // Simple long division
    BigInt quotient;
    BigInt remainder;
    
    for (int i = abs_this.bit_length() - 1; i >= 0; --i) {
        remainder = remainder << 1;
        if (abs_this.limbs_[i / 32] & (1U << (i % 32))) {
            remainder.limbs_[0] |= 1;
        }
        
        if (remainder >= abs_other) {
            remainder = remainder - abs_other;
            quotient.limbs_[i / 32] |= (1U << (i % 32));
        }
    }
    
    quotient.negative_ = negative_ != other.negative_;
    quotient.normalize();
    return quotient;
}

BigInt BigInt::operator%(const BigInt& other) const {
    BigInt quotient = *this / other;
    return *this - quotient * other;
}

BigInt BigInt::operator-() const {
    BigInt result(*this);
    result.negative_ = !negative_;
    if (result.is_zero()) result.negative_ = false;
    return result;
}

BigInt BigInt::operator&(const BigInt& other) const {
    BigInt result;
    result.negative_ = negative_ && other.negative_;
    size_t min_size = std::min(limbs_.size(), other.limbs_.size());
    result.limbs_.resize(min_size);
    
    for (size_t i = 0; i < min_size; ++i) {
        result.limbs_[i] = limbs_[i] & other.limbs_[i];
    }
    result.normalize();
    return result;
}

BigInt BigInt::operator|(const BigInt& other) const {
    BigInt result;
    result.negative_ = negative_ || other.negative_;
    size_t max_size = std::max(limbs_.size(), other.limbs_.size());
    result.limbs_.resize(max_size);
    
    for (size_t i = 0; i < max_size; ++i) {
        uint32_t a = (i < limbs_.size()) ? limbs_[i] : 0;
        uint32_t b = (i < other.limbs_.size()) ? other.limbs_[i] : 0;
        result.limbs_[i] = a | b;
    }
    result.normalize();
    return result;
}

BigInt BigInt::operator^(const BigInt& other) const {
    BigInt result;
    result.negative_ = negative_ != other.negative_;
    size_t max_size = std::max(limbs_.size(), other.limbs_.size());
    result.limbs_.resize(max_size);
    
    for (size_t i = 0; i < max_size; ++i) {
        uint32_t a = (i < limbs_.size()) ? limbs_[i] : 0;
        uint32_t b = (i < other.limbs_.size()) ? other.limbs_[i] : 0;
        result.limbs_[i] = a ^ b;
    }
    result.normalize();
    return result;
}

BigInt BigInt::operator~() const {
    BigInt result(*this);
    for (size_t i = 0; i < result.limbs_.size(); ++i) {
        result.limbs_[i] = ~result.limbs_[i];
    }
    result.normalize();
    return result;
}

BigInt BigInt::operator<<(size_t n) const {
    BigInt result(*this);
    size_t limb_shift = n / 32;
    size_t bit_shift = n % 32;
    
    result.limbs_.insert(result.limbs_.begin(), limb_shift, 0);
    
    if (bit_shift) {
        uint32_t carry = 0;
        for (size_t i = limb_shift; i < result.limbs_.size(); ++i) {
            uint32_t new_carry = result.limbs_[i] >> (32 - bit_shift);
            result.limbs_[i] = (result.limbs_[i] << bit_shift) | carry;
            carry = new_carry;
        }
        if (carry) {
            result.limbs_.push_back(carry);
        }
    }
    result.normalize();
    return result;
}

BigInt BigInt::operator>>(size_t n) const {
    BigInt result(*this);
    size_t limb_shift = n / 32;
    size_t bit_shift = n % 32;
    
    if (limb_shift >= result.limbs_.size()) {
        return BigInt(0);
    }
    
    result.limbs_.erase(result.limbs_.begin(), result.limbs_.begin() + limb_shift);
    
    if (bit_shift) {
        uint32_t carry = 0;
        for (int i = result.limbs_.size() - 1; i >= 0; --i) {
            uint32_t new_carry = result.limbs_[i] << (32 - bit_shift);
            result.limbs_[i] = (result.limbs_[i] >> bit_shift) | carry;
            carry = new_carry;
        }
    }
    result.normalize();
    return result;
}

std::string BigInt::to_hex() const {
    if (is_zero()) return "0";
    
    std::stringstream ss;
    if (negative_) ss << "-";
    
    bool started = false;
    for (int i = limbs_.size() - 1; i >= 0; --i) {
        if (!started && limbs_[i] == 0) continue;
        if (started) {
            ss << std::hex << std::setw(8) << std::setfill('0') << limbs_[i];
        } else {
            ss << std::hex << limbs_[i];
            started = true;
        }
    }
    return ss.str();
}

std::string BigInt::to_decimal() const {
    // Simple conversion: repeatedly divide by 10
    if (is_zero()) return "0";
    
    BigInt temp(*this);
    temp.negative_ = false;
    std::string result;
    
    while (!temp.is_zero()) {
        BigInt remainder = temp % BigInt(10);
        result.push_back('0' + remainder.limbs_[0]);
        temp = temp / BigInt(10);
    }
    
    if (negative_) result.push_back('-');
    std::reverse(result.begin(), result.end());
    return result;
}

size_t BigInt::bit_length() const {
    if (is_zero()) return 0;
    
    size_t last_limb = limbs_.size() - 1;
    uint32_t limb = limbs_[last_limb];
    size_t bits = (last_limb + 1) * 32;
    
    while (limb && !(limb & (1U << 31))) {
        limb <<= 1;
        bits--;
    }
    
    return bits;
}

bool BigInt::is_zero() const {
    return limbs_.size() == 1 && limbs_[0] == 0;
}

bool BigInt::is_negative() const {
    return negative_;
}

size_t BigInt::limb_count() {
    return sizeof(uint32_t) * 8;  // 32 bits per limb
}

size_t BigInt::max_limb_value() {
    return LIMB_MASK;
}

} // namespace bigint
} // namespace psc
