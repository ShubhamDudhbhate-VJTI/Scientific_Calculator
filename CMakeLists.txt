cmake_minimum_required(VERSION 3.16)
project(parallel_sci_calc VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(USE_CUDA "Build CUDA kernels (Phase 10b). Must stay OFF until those sources exist." OFF)
option(PSC_ENABLE_TSAN "Build with ThreadSanitizer (GCC/Clang, Linux)." OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

if(POLICY CMP0135)
  cmake_policy(SET CMP0135 NEW)
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

if(PSC_ENABLE_TSAN)
  if(NOT CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    message(FATAL_ERROR "PSC_ENABLE_TSAN requires GCC or Clang.")
  endif()
  add_compile_options(-fsanitize=thread -g -O1 -fno-omit-frame-pointer)
  add_link_options(-fsanitize=thread)
endif()

find_package(OpenMP)
if(NOT OpenMP_CXX_FOUND)
  message(FATAL_ERROR
    "OpenMP was not found. On Colab run: apt-get install -y g++ libomp-dev\n"
    "This project requires OpenMP for CPU parallelism. A serial-only build is not used.")
endif()

if(USE_CUDA)
  message(FATAL_ERROR
    "USE_CUDA=ON is reserved for Phase 10b. CUDA kernel sources are not in the tree yet. "
    "Configure with -DUSE_CUDA=OFF.")
endif()

add_library(psc STATIC
  src/common/error.cpp
  src/common/exec.cpp
  src/common/machine_info.cpp
  src/parser/tokenizer.cpp
  src/parser/parser.cpp
  src/eval/eval.cpp
  src/eval/parallel.cpp
  src/functions/taylor.cpp
  src/functions/cordic.cpp
  src/functions/elem.cpp
  src/functions/batch.cpp
  src/calculus/integrate.cpp
  src/calculus/dual.cpp
  src/calculus/diff.cpp
  src/linalg/matrix.cpp
  src/solvers/root.cpp
  src/solvers/ode.cpp
  src/bigint/bigint.cpp
)
target_include_directories(psc PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(psc PUBLIC OpenMP::OpenMP_CXX)
target_compile_definitions(psc PUBLIC PSC_VERSION="${PROJECT_VERSION}")

add_executable(psc_repl src/repl/main.cpp)
target_link_libraries(psc_repl PRIVATE psc)
set_target_properties(psc_repl PROPERTIES OUTPUT_NAME psc)

add_executable(psc_machine_info src/common/machine_info_main.cpp)
target_link_libraries(psc_machine_info PRIVATE psc)

add_executable(psc_bench bench/benchmark_main.cpp bench/cpu_bench.cpp)
target_link_libraries(psc_bench PRIVATE psc)

include(FetchContent)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
FetchContent_MakeAvailable(googletest)

enable_testing()
add_executable(psc_tests
  tests/unit/test_hello.cpp
  tests/unit/test_parser.cpp
  tests/unit/test_eval.cpp
  tests/unit/test_functions.cpp
  tests/unit/test_integrate.cpp
  tests/unit/test_diff.cpp
  tests/unit/test_linalg.cpp
  tests/unit/test_solvers.cpp
  tests/unit/test_bigint.cpp
  tests/unit/test_parallel_eval.cpp
)
target_link_libraries(psc_tests PRIVATE psc GTest::gtest_main)
add_test(NAME psc_tests COMMAND psc_tests)

message(STATUS "PSC version: ${PROJECT_VERSION}")
message(STATUS "C++ compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "OpenMP: ${OpenMP_CXX_VERSION}")
message(STATUS "USE_CUDA: ${USE_CUDA}")
