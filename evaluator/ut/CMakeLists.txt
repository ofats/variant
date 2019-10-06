enable_testing()

project(evaulator_ut)

set(SOURCE_FILES
    evaluator_ut.cpp)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} evaluator catch2)

add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})
