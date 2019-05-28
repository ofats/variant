enable_testing()

project(variant)

set(HEADER_FILES
    matrix_ops.h
    variant.h
    variant_traits.h)

set(SOURCE_FILES
    variant.cpp)

add_library(${PROJECT_NAME} ${SOURCE_FILES} ${HEADER_FILES})

target_link_libraries(${PROJECT_NAME} util)

add_subdirectory(ut)
