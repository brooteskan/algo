include_guard(GLOBAL)

# Gitlinks are the source of revision identity. Configuration never clones,
# searches installed packages, or accepts an unrelated checkout override.
function(wz_add_git_submodule repository relative target)
    find_package(Git REQUIRED QUIET)
    set(source "${repository}/${relative}")
    if(NOT EXISTS "${source}/CMakeLists.txt")
        message(FATAL_ERROR "Missing submodule ${relative}. Run git submodule update --init --recursive in ${repository}")
    endif()
    execute_process(COMMAND "${GIT_EXECUTABLE}" -C "${repository}" ls-files --stage -- "${relative}"
        OUTPUT_VARIABLE entry OUTPUT_STRIP_TRAILING_WHITESPACE RESULT_VARIABLE status)
    if(NOT status EQUAL 0 OR NOT entry MATCHES "^160000 ([0-9a-f]+) 0")
        message(FATAL_ERROR "${relative} must be a pinned Git submodule of ${repository}")
    endif()
    set(revision "${CMAKE_MATCH_1}")
    execute_process(COMMAND "${GIT_EXECUTABLE}" -C "${source}" rev-parse HEAD
        OUTPUT_VARIABLE actual OUTPUT_STRIP_TRAILING_WHITESPACE RESULT_VARIABLE status)
    if(NOT status EQUAL 0 OR NOT actual STREQUAL revision)
        message(FATAL_ERROR "Submodule revision mismatch for ${relative}; run git submodule update --init --recursive")
    endif()
    if(TARGET ${target})
        get_target_property(existing_source ${target} SOURCE_DIR)
        execute_process(COMMAND "${GIT_EXECUTABLE}" -C "${existing_source}" rev-parse HEAD
            OUTPUT_VARIABLE existing_revision OUTPUT_STRIP_TRAILING_WHITESPACE RESULT_VARIABLE status)
        execute_process(COMMAND "${GIT_EXECUTABLE}" -C "${existing_source}" rev-parse --show-superproject-working-tree
            OUTPUT_VARIABLE superproject OUTPUT_STRIP_TRAILING_WHITESPACE RESULT_VARIABLE parent_status)
        if(NOT status EQUAL 0 OR NOT parent_status EQUAL 0 OR NOT superproject OR NOT existing_revision STREQUAL revision)
            message(FATAL_ERROR "Conflicting ${target}: all consumers must use the same pinned submodule revision ${revision}")
        endif()
        return()
    endif()
    get_filename_component(name "${relative}" NAME)
    add_subdirectory("${source}" "${CMAKE_CURRENT_BINARY_DIR}/submodules/${name}" EXCLUDE_FROM_ALL)
    if(NOT TARGET ${target})
        message(FATAL_ERROR "Submodule ${relative} did not export ${target}")
    endif()
endfunction()
