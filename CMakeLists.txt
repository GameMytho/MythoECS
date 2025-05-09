cmake_minimum_required(VERSION 3.27)

# project name and version
project(MythoECS VERSION 0.1.0 LANGUAGES CXX)

# test option
# This option is used to enable or disable the building of tests
# for the MythoECS library. By default, it is set to ON, meaning that tests will be built.
option(BUILD_TESTS "Build tests for MythoECS" ON)

# c++ standard 20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# check all the source files in the src directory
file(GLOB_RECURSE MYTHO_ECS_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*)

# add interface library
# The INTERFACE library is a special type of library in CMake that does not produce an actual binary file.
add_library(MythoECS INTERFACE ${MYTHO_ECS_SOURCES})

# set include directories for the library
target_include_directories(MythoECS INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src/include)

# add subdirectory for tests if BUILD_TESTS is ON
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()