project(evaluator)

set(HEADER_FILES
    evaluator.h)

set(SOURCE_FILES
    dynamic.cpp
    parsing.cpp)

add_library(${PROJECT_NAME} ${SOURCE_FILES} ${HEADER_FILES})

target_link_libraries(${PROJECT_NAME} parser variant util)

add_subdirectory(bin)
