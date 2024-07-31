include(FetchContent)

# --------------------------------------------------------------------

# Offline mode (uncomment next line to enable)
# set(FETCHCONTENT_FULLY_DISCONNECTED ON)

# set(FETCHCONTENT_QUIET OFF)

# --------------------------------------------------------------------

# Libraries

# --------------------------------------------------------------------

# fmt with println

project_log("FetchContent: fmt")

FetchContent_Declare(
        fmt
        GIT_REPOSITORY https://github.com/fmtlib/fmt.git
        GIT_TAG 10.2.1
)
FetchContent_MakeAvailable(fmt)

# --------------------------------------------------------------------

# Unique Function

project_log("FetchContent: function2")

FetchContent_Declare(
        function2
        GIT_REPOSITORY https://github.com/Naios/function2.git
        GIT_TAG 4.2.2
)
FetchContent_MakeAvailable(function2)

# --------------------------------------------------------------------

project_log("FetchContent: wheels")

FetchContent_Declare(
        wheels
        SOURCE_DIR "${CMAKE_SOURCE_DIR}/deps/wheels"
)
FetchContent_MakeAvailable(wheels)

# --------------------------------------------------------------------

project_log("FetchContent: sure")

FetchContent_Declare(
        sure
        SOURCE_DIR "${CMAKE_SOURCE_DIR}/deps/sure"
)
FetchContent_MakeAvailable(sure)

# --------------------------------------------------------------------

project_log("FetchContent: twist")

FetchContent_Declare(
        twist
        SOURCE_DIR "${CMAKE_SOURCE_DIR}/deps/twist"
)
FetchContent_MakeAvailable(twist)

# --------------------------------------------------------------------

project_log("FetchContent: tinyfiber")

FetchContent_Declare(
        tinyfiber
        GIT_REPOSITORY https://gitlab.com/Lipovsky/tinyfiber.git
        GIT_TAG 55c3f2d98ec68156c15bbac48bc734288ebd58b6
)
FetchContent_MakeAvailable(tinyfiber)

# --------------------------------------------------------------------

project_log("FetchContent: asio")

FetchContent_Declare(
        asio
        GIT_REPOSITORY https://github.com/chriskohlhoff/asio.git
        GIT_TAG asio-1-29-0
)
FetchContent_MakeAvailable(asio)

add_library(asio INTERFACE)
target_include_directories(asio INTERFACE ${asio_SOURCE_DIR}/asio/include)

# --------------------------------------------------------------------

# Memory allocation

if((CMAKE_BUILD_TYPE MATCHES Release) AND NOT TWIST_FAULTY)
    project_log("FetchContent: mimalloc")

    FetchContent_Declare(
            mimalloc
            GIT_REPOSITORY https://github.com/microsoft/mimalloc
            GIT_TAG master
    )
    FetchContent_MakeAvailable(mimalloc)

endif()

# --------------------------------------------------------------------

project_log("FetchContent: moodycamel")

FetchContent_Declare(
        moodycamel
        GIT_REPOSITORY https://github.com/cameron314/concurrentqueue
        GIT_TAG master
)
FetchContent_MakeAvailable(moodycamel)
