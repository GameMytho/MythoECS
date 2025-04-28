# CMakeLists.txt for the test directory

# import gtest
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/heads/main.zip
)
FetchContent_MakeAvailable(googletest)

# check all the source files in the src directory and add them to the test executable
file(GLOB_RECURSE MYTHO_ECS_TEST_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*)

# add test executable
add_executable(MythoECSTest ${MYTHO_ECS_TEST_SOURCES})

# set include directories for the test executable
target_include_directories(MythoECSTest PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../src/include)

# link the library to the test executable
target_link_libraries(MythoECSTest PRIVATE MythoECS gtest)

# add test to CMake
add_test(NAME MythoECSTest COMMAND MythoECSTest)