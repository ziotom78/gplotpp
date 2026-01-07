cmake_minimum_required(VERSION 3.12)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(gplotpp
  VERSION 0.3.0
  DESCRIPTION "A C++ header-only interface to Gnuplot"
  LANGUAGES CXX
  )

############
## Config ##
############

option(GPLOTPP_BUILD_EXAMPLES "Build examples" ON)
option(GPLOTPP_ENABLE_TESTS "Enable testing" ON)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# set the path where CMake package configuration files (*-config.cmake) will be installed
# when finding a package, CMake searches by default lib/<package name>/cmake (among others)
set(ConfigPackageLocation ${CMAKE_INSTALL_LIBDIR}/${CMAKE_PROJECT_NAME}/cmake)

# Enable as many warnings as possible and make them blocking
if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    add_compile_options(/W4 /WX)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    add_compile_options(-Wall -Wextra -Werror --pedantic)
endif()

#################
## Main Target ##
#################

# main target
add_library(${PROJECT_NAME} INTERFACE)

# include directories
target_include_directories(${PROJECT_NAME} INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/.>
                                                     $<INSTALL_INTERFACE:.>)

###########
## Tests ##
###########

if (GPLOTPP_ENABLE_TESTS)
    enable_testing()
    add_test(NAME run_tests COMMAND run_tests)
    add_executable(run_tests tests/run_tests.cpp)
    target_link_libraries(run_tests ${PROJECT_NAME})
endif()

##############
## Examples ##
##############

if (GPLOTPP_BUILD_EXAMPLES)
    add_executable(example-3d src/example-3d.cpp)
    target_link_libraries(example-3d ${PROJECT_NAME})
    add_executable(example-addpoint src/example-addpoint.cpp)
    target_link_libraries(example-addpoint ${PROJECT_NAME})
    add_executable(example-animated-gif src/example-animated-gif.cpp)
    target_link_libraries(example-animated-gif ${PROJECT_NAME})
    add_executable(example-complex src/example-complex.cpp)
    target_link_libraries(example-complex ${PROJECT_NAME})
    add_executable(example-errorbars src/example-errorbars.cpp)
    target_link_libraries(example-errorbars ${PROJECT_NAME})
    add_executable(example-histogram src/example-histogram.cpp)
    target_link_libraries(example-histogram ${PROJECT_NAME})
    add_executable(example-multipleseries src/example-multipleseries.cpp)
    target_link_libraries(example-multipleseries ${PROJECT_NAME})
    add_executable(example-pdfoutput src/example-pdfoutput.cpp)
    target_link_libraries(example-pdfoutput ${PROJECT_NAME})
    add_executable(example-pngoutput src/example-pngoutput.cpp)
    target_link_libraries(example-pngoutput ${PROJECT_NAME})
    add_executable(example-simple src/example-simple.cpp)
    target_link_libraries(example-simple ${PROJECT_NAME})
    add_executable(example-vec src/example-vec.cpp)
    target_link_libraries(example-vec ${PROJECT_NAME})
    add_executable(example-vec3d src/example-vec3d.cpp)
    target_link_libraries(example-vec3d ${PROJECT_NAME})
    add_executable(example-dumb src/example-dumb.cpp)
    target_link_libraries(example-dumb ${PROJECT_NAME})
endif()

#############
## Install ##
#############

# headers
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/gplot++.h
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# export targets
install(TARGETS ${PROJECT_NAME}
        EXPORT ${PROJECT_NAME}Targets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

############
## Export ##
############

# generate .cmake target configuration file
export(EXPORT ${PROJECT_NAME}Targets
       FILE ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake
       NAMESPACE ${PROJECT_NAME}::)

# @note: the export command makes the targets available in the build tree only.
#        to make the targets available for projects which are not part of the build tree, we also have to install them
install(EXPORT ${PROJECT_NAME}Targets
        FILE ${PROJECT_NAME}Targets.cmake
        NAMESPACE ${PROJECT_NAME}::
        DESTINATION ${ConfigPackageLocation})

###################
## CMake Package ##
###################

# to write *ConfigVersion.cmake
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
    VERSION ${PLAIN_CMAKE_VERSION}
    COMPATIBILITY SameMajorVersion)

# to write *Config.cmake
set(INCLUDE_INSTALL_DIR ${CMAKE_INSTALL_INCLUDEDIR})
configure_package_config_file(
    ${PROJECT_SOURCE_DIR}/cmake/${PROJECT_NAME}Config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
    INSTALL_DESTINATION ${ConfigPackageLocation}
    PATH_VARS INCLUDE_INSTALL_DIR)

# move both to target directory
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
    DESTINATION ${ConfigPackageLocation})