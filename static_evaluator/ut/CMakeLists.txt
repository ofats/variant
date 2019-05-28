enable_testing()

project(static_evaulator_ut)

set(SOURCE_FILES
    evaluator_ut.cpp)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} static_evaluator catch)

add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})
