enable_testing()

project(lib_ut)

set(SOURCE_FILES
    main.cpp)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} lib)

add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})
