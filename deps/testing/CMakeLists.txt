project(course-testing)

set(SOURCES
        source/course/test/twist/stress.cpp
        source/course/test/twist/randomize.cpp)

if (TWIST_SIM)
    list(APPEND SOURCES
            source/course/test/twist/sim/result.cpp)

    if (TWIST_SIM_ISOLATION)
        list(APPEND SOURCES
                source/course/test/twist/model.cpp)
    endif()
endif()

add_library(course-testing STATIC ${SOURCES})

target_include_directories(course-testing PUBLIC source)
target_link_libraries(course-testing twist wheels)
