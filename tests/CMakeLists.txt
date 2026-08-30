find_package(GTest CONFIG QUIET)

if(NOT TARGET GTest::gtest_main)
    include(FetchContent)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.17.0
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(googletest)
endif()

function(add_algo_test_executable target source)
    add_executable(${target} ${source})
    target_include_directories(${target} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/support)
    target_link_libraries(${target} PRIVATE algo::algo GTest::gtest_main)
endfunction()

add_algo_test_executable(algo_test_legacy algo/algo_test_1.cpp)
add_algo_test_executable(algo_test_next algo/algo_test_next.cpp)
add_algo_test_executable(algo_test_pipeline algo/algo_test_pipeline.cpp)
add_algo_test_executable(algo_test_current_behavior baseline_current_behavior.cpp)

add_test(
    NAME algo.baseline.legacy
    COMMAND algo_test_legacy --gtest_filter=-AlgoApplySpec.MultipleOpsExecuteSequentially
)
add_test(
    NAME algo.baseline.known_failure.apply_composition
    COMMAND algo_test_legacy --gtest_filter=AlgoApplySpec.MultipleOpsExecuteSequentially
)
set_tests_properties(
    algo.baseline.known_failure.apply_composition
    PROPERTIES
        WILL_FAIL TRUE
        LABELS "baseline;known-failure"
)

add_test(NAME algo.baseline.next COMMAND algo_test_next)
add_test(NAME algo.baseline.pipeline COMMAND algo_test_pipeline)
add_test(NAME algo.baseline.current_behavior COMMAND algo_test_current_behavior)

set_tests_properties(
    algo.baseline.legacy
    algo.baseline.next
    algo.baseline.pipeline
    algo.baseline.current_behavior
    PROPERTIES LABELS baseline
)
