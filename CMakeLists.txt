cmake_minimum_required(VERSION 3.12)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(gplotpp
  VERSION 0.3.0
  DESCRIPTION "A C++ header-only interface to Gnuplot"
  LANGUAGES CXX
  )

enable_testing()

add_test(NAME run_tests
  COMMAND run_tests
  )

add_executable(run_tests
  tests/run_tests.cpp
  )

target_include_directories(run_tests PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:include>
  )

add_executable(example-3d
  example-3d.cpp
  )

add_executable(example-complex
  example-complex.cpp
  )

add_executable(example-errorbars
  example-errorbars.cpp
  )

add_executable(example-histogram
  example-histogram.cpp
  )

add_executable(example-multipleseries
  example-multipleseries.cpp
  )

add_executable(example-pdfoutput
  example-pdfoutput.cpp
  )

add_executable(example-pngoutput
  example-pngoutput.cpp
  )

add_executable(example-simple
  example-simple.cpp
  )

add_executable(example-vec
  example-vec.cpp
  )

add_executable(example-vec3d
  example-vec3d.cpp
  )

add_executable(example-dumb
  example-dumb.cpp
  )
