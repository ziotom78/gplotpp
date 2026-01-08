cmake_minimum_required(VERSION 3.12)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(gplotpp_examples
  VERSION 0.3.0
  DESCRIPTION "gplotpp example binaries"
  LANGUAGES CXX
)

add_executable(example-3d src/example-3d.cpp)
target_link_libraries(example-3d gplotpp)
add_executable(example-addpoint src/example-addpoint.cpp)
target_link_libraries(example-addpoint gplotpp)
add_executable(example-animated-gif src/example-animated-gif.cpp)
target_link_libraries(example-animated-gif gplotpp)
add_executable(example-complex src/example-complex.cpp)
target_link_libraries(example-complex gplotpp)
add_executable(example-errorbars src/example-errorbars.cpp)
target_link_libraries(example-errorbars gplotpp)
add_executable(example-histogram src/example-histogram.cpp)
target_link_libraries(example-histogram gplotpp)
add_executable(example-multipleseries src/example-multipleseries.cpp)
target_link_libraries(example-multipleseries gplotpp)
add_executable(example-pdfoutput src/example-pdfoutput.cpp)
target_link_libraries(example-pdfoutput gplotpp)
add_executable(example-pngoutput src/example-pngoutput.cpp)
target_link_libraries(example-pngoutput gplotpp)
add_executable(example-simple src/example-simple.cpp)
target_link_libraries(example-simple gplotpp)
add_executable(example-vec src/example-vec.cpp)
target_link_libraries(example-vec gplotpp)
add_executable(example-vec3d src/example-vec3d.cpp)
target_link_libraries(example-vec3d gplotpp)
add_executable(example-dumb src/example-dumb.cpp)
target_link_libraries(example-dumb gplotpp)