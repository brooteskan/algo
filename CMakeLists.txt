cmake_minimum_required(VERSION 3.24)

project(
    algo
    VERSION 0.0.1
    DESCRIPTION "Deterministic non-owning functional algorithms extracted from Wozzits"
    LANGUAGES CXX
)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set(_algo_tests_default OFF)
if(PROJECT_IS_TOP_LEVEL)
    set(_algo_tests_default ON)
endif()
option(ALGO_BUILD_TESTS "Build the Wozzits baseline and characterization tests" ${_algo_tests_default})

add_library(algo INTERFACE)
add_library(algo::algo ALIAS algo)
set_target_properties(algo PROPERTIES EXPORT_NAME algo)
target_compile_features(algo INTERFACE cxx_std_20)
target_include_directories(
    algo
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

if(ALGO_BUILD_TESTS)
    include(CTest)
    add_subdirectory(tests)
endif()

install(TARGETS algo EXPORT algo-targets)
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(
    EXPORT algo-targets
    FILE algo-targets.cmake
    NAMESPACE algo::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/algo
)

configure_package_config_file(
    cmake/algo-config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/algo-config.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/algo
)
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/algo-config-version.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)
install(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/algo-config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/algo-config-version.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/algo
)
