#pragma once

#include <stdexcept>
#include <string>

namespace psc {

// User-facing calculation or input error. The REPL catches this type.
class CalcError : public std::runtime_error {
public:
  explicit CalcError(const std::string& message);
  CalcError(const std::string& message, std::size_t position);

  bool has_position() const noexcept;
  std::size_t position() const noexcept;

private:
  bool has_position_;
  std::size_t position_;
};

}  // namespace psc
