#include "psc/common/machine_info.hpp"

#include "psc/common/config.hpp"

#include <fstream>
#include <sstream>
#include <string>

#ifdef _OPENMP
#include <omp.h>
#endif

#if defined(_WIN32)
#include <windows.h>
#else
#include <unistd.h>
#endif

namespace psc {
namespace {

std::string read_first_matching_line(const char* path, const char* prefix) {
  std::ifstream in(path);
  if (!in) {
    return "could not verify";
  }
  std::string line;
  const std::string pfx(prefix);
  while (std::getline(in, line)) {
    if (line.compare(0, pfx.size(), pfx) == 0) {
      auto pos = line.find(':');
      if (pos == std::string::npos) {
        return line;
      }
      std::string value = line.substr(pos + 1);
      while (!value.empty() && (value.front() == ' ' || value.front() == '\t')) {
        value.erase(value.begin());
      }
      return value.empty() ? "could not verify" : value;
    }
  }
  return "could not verify";
}

std::string detect_os() {
#if defined(_WIN32)
  return "Windows (details: could not verify from this helper)";
#elif defined(__APPLE__)
  return "macOS";
#elif defined(__linux__)
  std::ifstream in("/etc/os-release");
  std::string line;
  while (in && std::getline(in, line)) {
    if (line.rfind("PRETTY_NAME=", 0) == 0) {
      auto v = line.substr(12);
      if (!v.empty() && v.front() == '"') {
        v.erase(v.begin());
      }
      if (!v.empty() && v.back() == '"') {
        v.pop_back();
      }
      return v.empty() ? "Linux" : v;
    }
  }
  return "Linux";
#else
  return "could not verify";
#endif
}

std::string detect_ram() {
#if defined(_WIN32)
  MEMORYSTATUSEX st;
  st.dwLength = sizeof(st);
  if (GlobalMemoryStatusEx(&st)) {
    std::ostringstream oss;
    oss << (st.ullTotalPhys / (1024ULL * 1024ULL)) << " MB";
    return oss.str();
  }
  return "could not verify";
#else
  std::ifstream mem("/proc/meminfo");
  std::string key;
  unsigned long kib = 0;
  std::string unit;
  while (mem >> key >> kib >> unit) {
    if (key == "MemTotal:") {
      std::ostringstream oss;
      oss << (kib / 1024) << " MB";
      return oss.str();
    }
    std::string rest;
    std::getline(mem, rest);
  }
  return "could not verify";
#endif
}

std::string detect_logical_cpus() {
#if defined(_WIN32)
  SYSTEM_INFO si;
  GetSystemInfo(&si);
  return std::to_string(static_cast<unsigned>(si.dwNumberOfProcessors));
#elif defined(_SC_NPROCESSORS_ONLN)
  long n = sysconf(_SC_NPROCESSORS_ONLN);
  if (n > 0) {
    return std::to_string(n);
  }
  return "could not verify";
#else
  return "could not verify";
#endif
}

}  // namespace

std::string collect_machine_info() {
  std::ostringstream out;
  out << "project: " << project_name() << "\n";
  out << "version: " << project_version() << "\n";
  out << "os: " << detect_os() << "\n";
  out << "cpu_model: " << read_first_matching_line("/proc/cpuinfo", "model name") << "\n";
  out << "cpu_physical_cores: "
      << read_first_matching_line("/proc/cpuinfo", "cpu cores") << "\n";
  out << "cpu_logical_cores: " << detect_logical_cpus() << "\n";
  out << "ram: " << detect_ram() << "\n";
#ifdef _OPENMP
  out << "openmp: detected\n";
  out << "openmp_max_threads: " << omp_get_max_threads() << "\n";
  out << "openmp_date: " << _OPENMP << "\n";
#else
  out << "openmp: not detected\n";
#endif
  out << "compiler: "
#if defined(__clang__)
      << "Clang " << __clang_major__ << "." << __clang_minor__ << "." << __clang_patchlevel__
#elif defined(__GNUC__)
      << "GCC " << __GNUC__ << "." << __GNUC_MINOR__ << "." << __GNUC_PATCHLEVEL__
#elif defined(_MSC_VER)
      << "MSVC " << _MSC_VER
#else
      << "could not verify"
#endif
      << "\n";
  out << "cuda_module: not built in this configuration\n";
  out << "notes: CPU speedup on Colab is limited by the runtime core count "
         "(often 2 vCPUs). Do not invent extra thread counts.\n";
  return out.str();
}

}  // namespace psc
