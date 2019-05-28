enable_testing()

project(dynamic_evaluator)

set(HEADER_FILES
    evaluator.h)

set(SOURCE_FILES
    evaluator.cpp)

add_library(${PROJECT_NAME} ${SOURCE_FILES} ${HEADER_FILES})

target_link_libraries(${PROJECT_NAME} variant parser)

add_subdirectory(bin)
add_subdirectory(ut)
