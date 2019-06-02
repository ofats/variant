enable_testing()

project(util)

set(HEADER_FILES
    meta.h)

set(SOURCE_FILES
    util.cpp)

add_library(${PROJECT_NAME} ${SOURCE_FILES} ${HEADER_FILES})
