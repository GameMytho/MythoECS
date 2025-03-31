include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/release-1.12.1.zip
)

FetchContent_MakeAvailable(googletest)

add_executable(MythoECSTests main.cc)

target_link_libraries(MythoECSTests PRIVATE MythoECS gtest_main)

include(GoogleTest)
gtest_discover_tests(MythoECSTests)
