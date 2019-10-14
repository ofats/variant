enable_testing()

project(parser_ut)

set(SOURCE_FILES
    ../../parser/parser_ut.cpp)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} parser catch2)

add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})
