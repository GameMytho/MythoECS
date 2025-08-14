cmake_minimum_required(VERSION 3.27)

# project name and version
project(MythoECS VERSION 0.1.0 LANGUAGES CXX)

# test option
# This option is used to enable or disable the building of tests
# for the MythoECS library. By default, it is set to ON, meaning that tests will be built.
option(BUILD_TESTS "Build tests for MythoECS" ON)
option(MYTHO_ASSERTS_ENABLED "Enable asserts for MythoECS" ON)

# add subdirectory for src
add_subdirectory(src)

# install and export MythoECS
install(TARGETS MythoECS
    EXPORT MythoECSTargets
    FILE_SET HEADERS
)
install(EXPORT MythoECSTargets
    NAMESPACE MythoECS::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/MythoECS
)

include(CMakePackageConfigHelpers)
configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/MythoECSConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/MythoECSConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/MythoECS
)
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/MythoECSConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/MythoECSConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/MythoECSConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/MythoECS
)

# add subdirectory for tests if BUILD_TESTS is ON
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()