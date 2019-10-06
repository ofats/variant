enable_testing()

project(evaluator)

set(HEADER_FILES
    evaluator.h)

set(SOURCE_FILES
    dynamic.cpp
    parsing.cpp)

add_library(${PROJECT_NAME} ${SOURCE_FILES} ${HEADER_FILES})

target_link_libraries(${PROJECT_NAME} parser variant util)

if (NOT CMAKE_BUILD_TYPE MATCHES Debug)
  add_subdirectory(perf)
endif()

add_subdirectory(bin)
add_subdirectory(ut)
