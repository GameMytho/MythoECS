cmake_minimum_required(VERSION 3.14)

project(MythoECS VERSION 0.01 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
endif()

option(BUILD_TESTS "Build the unit tests" OFF)

if(BUILD_TESTS)
    enable_testing()
endif()

add_library(MythoECS INTERFACE)
target_include_directories(
    MythoECS
    INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/include>
    $<INSTALL_INTERFACE:include>
)

if(BUILD_TESTS)
    add_subdirectory(src/test)
endif()