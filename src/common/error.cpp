#include "psc/common/error.hpp"

namespace psc {

CalcError::CalcError(const std::string& message)
    : std::runtime_error(message), has_position_(false), position_(0) {}

CalcError::CalcError(const std::string& message, std::size_t position)
    : std::runtime_error(message + " (at position " + std::to_string(position) + ")"),
      has_position_(true),
      position_(position) {}

bool CalcError::has_position() const noexcept { return has_position_; }

std::size_t CalcError::position() const noexcept { return position_; }

}  // namespace psc
