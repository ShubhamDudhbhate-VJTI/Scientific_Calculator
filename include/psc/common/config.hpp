#pragma once

#ifndef PSC_VERSION
#define PSC_VERSION "0.1.0-dev"
#endif

namespace psc {

inline const char* project_name() { return "Parallel Scientific Calculator"; }
inline const char* project_version() { return PSC_VERSION; }

}  // namespace psc
