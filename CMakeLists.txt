cmake_minimum_required(VERSION 3.20)

project(cppconcur)


include(cmake/Logging.cmake)
include(cmake/Ccache.cmake)
include(cmake/Helpers.cmake)
include(cmake/CheckCompiler.cmake)
include(cmake/CompileOptions.cmake)
include(cmake/Sanitize.cmake)
include(cmake/Platform.cmake)
include(cmake/Development.cmake)
include(cmake/FetchContent.cmake)

add_subdirectory(deps)

# This must be included after deps so that
# third party code is not affected
include(cmake/Werror.cmake)
include(cmake/Lib.cmake)
include(cmake/Test.cmake)

add_subdirectory(src)
add_subdirectory(examples)
add_subdirectory(tests)
