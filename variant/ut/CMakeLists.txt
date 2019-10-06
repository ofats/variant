enable_testing()

project(variant_ut)

set(SOURCE_FILES
    variant_ut.cpp)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} variant catch2)

add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})
