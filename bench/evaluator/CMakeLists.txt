project(evaluator_bench)

set(SOURCE_FILES
    ../../evaluator/evaluator_bench.cpp)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} evaluator benchmark)
