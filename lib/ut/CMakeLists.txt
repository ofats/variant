enable_testing()

project(lib_ut)

set(SOURCE_FILES
    variant_ut.cpp)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} lib catch)

add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})
