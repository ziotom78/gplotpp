cmake_minimum_required(VERSION 3.12)

project(gplotpp
  VERSION 0.3.0
  DESCRIPTION "A C++ header-only interface to Gnuplot"
  LANGUAGES CXX
  )

add_executable(example-complex
  example-complex.cpp
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

