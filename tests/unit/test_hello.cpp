#include "psc/common/config.hpp"
#include "psc/common/error.hpp"
#include "psc/common/exec.hpp"
#include "psc/common/machine_info.hpp"

#include <gtest/gtest.h>

#ifdef _OPENMP
#include <omp.h>
#endif

TEST(Hello, VersionIsNonEmpty) {
  ASSERT_NE(psc::project_version()[0], '\0');
}

TEST(Hello, OpenMPIsCompiledIn) {
#ifndef _OPENMP
  FAIL() << "OpenMP was not enabled. Colab needs libomp-dev and find_package(OpenMP).";
#else
  EXPECT_GE(omp_get_max_threads(), 1);
#endif
}

TEST(Hello, ApplyThreadsHonorsSerialMode) {
  psc::ExecOptions opts;
  opts.mode = psc::ExecMode::Serial;
  opts.requested_threads = 2;
  EXPECT_EQ(psc::apply_thread_request(opts), 1);
}

TEST(Hello, CalcErrorCarriesPosition) {
  psc::CalcError e("bad token", 4);
  EXPECT_TRUE(e.has_position());
  EXPECT_EQ(e.position(), 4u);
}

TEST(Hello, MachineInfoMentionsProject) {
  const std::string info = psc::collect_machine_info();
  EXPECT_NE(info.find("Parallel Scientific Calculator"), std::string::npos);
}

TEST(Hello, ParallelForReduction) {
#if defined(__SANITIZE_THREAD__)
  // TSan does not instrument libgomp reductions. Colab (GCC 13.3 + libgomp)
  // reported a stack race on this loop even though the test result was 5050.
  // Skip under TSan so a real race in project code is not hidden in the noise.
  GTEST_SKIP() << "OpenMP reduction is not TSan-clean with libgomp; skipped under ThreadSanitizer.";
#elif defined(_OPENMP)
  int sum = 0;
#pragma omp parallel for reduction(+ : sum)
  for (int i = 1; i <= 100; ++i) {
    sum += i;
  }
  EXPECT_EQ(sum, 5050);
#else
  GTEST_SKIP() << "OpenMP not compiled in";
#endif
}
