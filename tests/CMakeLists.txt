cmake_minimum_required(VERSION 3.12)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(gplotpp_test
  VERSION 0.3.0
  DESCRIPTION "gplotpp unit testing"
  LANGUAGES CXX
)

enable_testing()
add_test(NAME run_tests COMMAND run_tests)
add_executable(run_tests src/run_tests.cpp)
target_include_directories(run_tests PUBLIC include)
target_link_libraries(run_tests gplotpp)
