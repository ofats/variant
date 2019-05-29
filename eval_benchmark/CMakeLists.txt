enable_testing()

project(eval_benchmark)

set(SOURCE_FILES
    main.cpp)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} static_evaluator dynamic_evaluator benchmark)
