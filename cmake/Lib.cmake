set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "bin")

# --------------------------------------------------------------------

set(LIBS_LIST "wheels;twist;sure;course-testing;fmt;function2")

if((CMAKE_BUILD_TYPE MATCHES Release) AND NOT TWIST_FAULTY)
    list(APPEND LIBS_LIST "mimalloc")
endif()

# --------------------------------------------------------------------

# Dependencies

macro(task_link_libraries)
    list(APPEND LIBS_LIST ${ARGV})
endmacro()

# --------------------------------------------------------------------

# Sources

macro(set_task_sources)
    prepend(TASK_SOURCES "${TASK_DIR}/" ${ARGV})
endmacro()

# --------------------------------------------------------------------

# Libraries

macro(add_dir_library DIR_NAME)
    # Optional lib target name (dir name by default)
    if (${ARGC} GREATER 1)
        set(LIB_NAME ${ARGV1})
    else()
        set(LIB_NAME ${DIR_NAME})
    endif()

    set(LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/${DIR_NAME})
    set(LIB_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

    project_log("Add dir library: ${LIB_NAME}")

    file(GLOB_RECURSE LIB_CXX_SOURCES ${LIB_DIR}/*.cpp)
    file(GLOB_RECURSE LIB_HEADERS ${LIB_DIR}/*.hpp ${LIB_DIR}/*.ipp)

    if (LIB_CXX_SOURCES)
        add_library(${LIB_NAME} STATIC ${LIB_CXX_SOURCES} ${LIB_HEADERS})
        target_include_directories(${LIB_NAME} PUBLIC ${LIB_INCLUDE_DIR})
        target_link_libraries(${LIB_NAME} ${LIBS_LIST})
    else()
        # header-only library
        add_library(${LIB_NAME} INTERFACE)
        target_include_directories(${LIB_NAME} INTERFACE ${LIB_INCLUDE_DIR})
        target_link_libraries(${LIB_NAME} INTERFACE ${LIBS_LIST})
    endif()

    # Append ${LIB_TARGET to LIBS_LIST
    # list(APPEND LIBS_LIST ${LIB_TARGET})
    # set(LIBS_LIST ${LIBS_LIST} PARENT_SCOPE)
endmacro()

# --------------------------------------------------------------------

# Custom target

function(add_task_dir_target NAME DIR_NAME)
    get_task_target(TARGET_NAME ${NAME})

    set(TARGET_DIR "${TASK_DIR}/${DIR_NAME}")
    file(GLOB_RECURSE TARGET_CXX_SOURCES ${TARGET_DIR}/*.cpp)

    add_task_executable(${TARGET_NAME} ${TARGET_CXX_SOURCES})
endfunction()

# --------------------------------------------------------------------

# Tests

macro(add_concur_test TARGET_NAME)
    prepend(TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/" ${ARGN})
    add_executable(${TARGET_NAME} ${TEST_SOURCES})
    target_link_libraries(${TARGET_NAME} pthread exe)

    # Append test to TEST_LIST
    list(APPEND TEST_LIST ${TARGET_NAME})
    set(TEST_LIST "${TEST_LIST}")
endmacro()

function(add_task_test_dir DIR_NAME)
    # Optional test target name (dir name by default)
    if (${ARGC} GREATER 1)
        set(BINARY_NAME ${ARGV1})
    else()
        set(BINARY_NAME ${DIR_NAME})
    endif()

    get_task_target(TEST_NAME ${BINARY_NAME})

    set(TEST_DIR "${TASK_DIR}/${DIR_NAME}")
    file(GLOB_RECURSE TEST_CXX_SOURCES ${TEST_DIR}/*.cpp)

    add_task_executable(${TEST_NAME} ${TEST_CXX_SOURCES})

    # Append test to TEST_LIST
    list(APPEND TEST_LIST ${TEST_NAME})
    set(TEST_LIST "${TEST_LIST}" PARENT_SCOPE)
endfunction()

function(add_task_all_tests_target)
    get_task_target(ALL_TESTS_TARGET "run_all_tests")
    run_chain(${ALL_TESTS_TARGET} ${TEST_LIST})
endfunction()

# --------------------------------------------------------------------

# Benchmark

function(add_task_benchmark BINARY_NAME)
    get_task_target(BENCH_NAME ${BINARY_NAME})

    prepend(BENCH_SOURCES "${TASK_DIR}/" ${ARGN})
    add_task_executable(${BENCH_NAME} ${BENCH_SOURCES})
    target_link_libraries(${BENCH_NAME} benchmark)

    if(${TOOL_BUILD})
        get_task_target(RUN_BENCH_TARGET "run_benchmark")
        add_custom_target(${RUN_BENCH_TARGET} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${BENCH_NAME})
        add_dependencies(${RUN_BENCH_TARGET} ${BENCH_NAME})
    endif()
endfunction()

# --------------------------------------------------------------------
