#pragma once

#include <string>

namespace psc {

// Collects host facts for results/machine_info.txt. Values come from the OS
// at runtime. Missing fields are written as "could not verify".
std::string collect_machine_info();

}  // namespace psc
