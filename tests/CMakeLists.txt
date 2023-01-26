set (CMAKE_CXX_STANDARD ${WITH_CXX_STANDARD})
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (TARGET "${PROJECT_NAME}-test" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wmissing-include-dirs -Wfloat-equal -Wshadow")
set (TARGET "${PROJECT_NAME}-test" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wdouble-promotion -Weffc++ -Woverloaded-virtual -Wsign-promo")
set (TARGET "${PROJECT_NAME}-test" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wvla -Winvalid-pch -Winline -Wredundant-decls -Wcast-align")
set (TARGET "${PROJECT_NAME}-test" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wcast-qual -Wpointer-arith -Wold-style-cast")

add_executable (run_tests
  run_tests.cpp
  )

target_include_directories (run_tests PUBLIC
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:include>
  )

enable_testing ()

add_test (NAME run_tests
  COMMAND run_tests
  )
