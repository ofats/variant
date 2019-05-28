enable_testing()

project(parser)

set(HEADER_FILES
    parser.h)

set(SOURCE_FILES
    parser.cpp)

add_library(${PROJECT_NAME} ${SOURCE_FILES} ${HEADER_FILES})

target_link_libraries(${PROJECT_NAME} variant)
