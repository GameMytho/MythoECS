# Interface library for MythoECS
add_library(MythoECS INTERFACE)
add_library(MythoECS::MythoECS ALIAS MythoECS)

# pass c++ standard 20
target_compile_features(MythoECS INTERFACE cxx_std_20)

# set include directories
target_include_directories(MythoECS INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# set compile definitions for the library
target_compile_definitions(MythoECS INTERFACE
    $<$<CONFIG:Debug>:MYTHO_ASSERTS_ENABLED=1>
    $<$<NOT:$<CONFIG:Debug>>:MYTHO_ASSERTS_ENABLED=0>
)

# add headers to the library
file(GLOB_RECURSE MYTHOECS_HEADERS CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp)

target_sources(MythoECS INTERFACE
    FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
    FILES ${MYTHOECS_HEADERS}
)