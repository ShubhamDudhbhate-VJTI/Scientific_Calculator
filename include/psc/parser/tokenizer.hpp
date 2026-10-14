#pragma once

#include "psc/parser/token.hpp"

#include <string>
#include <vector>

namespace psc {

// Splits an expression into tokens and records the 0-based source index of each.
std::vector<Token> tokenize(const std::string& input);

}  // namespace psc
