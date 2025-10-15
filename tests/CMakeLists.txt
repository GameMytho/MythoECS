# CMakeLists.txt for the test directory

# import gtest
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/heads/main.zip
)
FetchContent_MakeAvailable(googletest)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

function(add_unit_test SRC_DIR LABEL)
    set(oneValueArgs TARGET_NAME)
    cmake_parse_arguments(MAT "" "TARGET_NAME" "" "${ARGN}")

    if(MAT_TARGET_NAME)
        set(TARGET ${MAT_TARGET_NAME})
    else()
        set(TARGET "MythoECS_${LABEL}Tests")
    endif()

    file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS ${SRC_DIR}/src/*.cc)

    add_executable(${TARGET} ${TEST_SOURCES})

    target_include_directories(${TARGET} PRIVATE ${SRC_DIR}/include)

    target_link_libraries(${TARGET} PRIVATE MythoECS GTest::gtest GTest::gtest_main)

    # set compile options for the test executable
    target_compile_options(${TARGET} PRIVATE
      $<$<CXX_COMPILER_ID:MSVC>:
        $<$<CONFIG:Debug>:/Od /Zi>
        $<$<CONFIG:RelWithDebInfo>:/O2 /Zi>
        $<$<CONFIG:Release>:/O2>
        $<$<CONFIG:MinSizeRel>:/O1>
      >
      $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:
        $<$<CONFIG:Debug>:-O0 -g>
        $<$<CONFIG:RelWithDebInfo>:-O2 -g>
        $<$<CONFIG:Release>:-O3>
        $<$<CONFIG:MinSizeRel>:-Os>
      >
    )

    # set output directory for the test executable
    set_target_properties(${TARGET} PROPERTIES
      RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/tests/bin/$<CONFIG>"
    )

    # add test to CMake
    add_test(NAME ${TARGET} COMMAND $<TARGET_FILE:${TARGET}>)
    set_tests_properties(${TARGET} PROPERTIES LABELS "${LABEL}")
endfunction()

add_unit_test(src/unit_case unit_case TARGET_NAME MythoECS_UnitCaseTests)
add_unit_test(src/use_case use_case TARGET_NAME MythoECS_UseCaseTests)