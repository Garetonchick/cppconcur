# --------------------------------------------------------------------

# Examples

macro(add_example DIR_NAME)
    project_log("Add example: ${DIR_NAME}")

    set(TARGET_NAME "example_${DIR_NAME}")
    set(TARGET_DIR "${CMAKE_CURRENT_SOURCE_DIR}/${DIR_NAME}")
    file(GLOB_RECURSE TARGET_CXX_SOURCES ${TARGET_DIR}/*.cpp)

    add_executable(${TARGET_NAME} ${TARGET_CXX_SOURCES})
    target_link_libraries(${TARGET_NAME} pthread exe)
endmacro()

# --------------------------------------------------------------------
