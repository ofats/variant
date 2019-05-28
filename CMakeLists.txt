cmake_minimum_required(VERSION 3.7)
enable_testing()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS_DEBUG "-O0 -fno-omit-frame-pointer -g -fsanitize=address -Wall -Wextra -Werror")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -Wall -Wextra -Werror -g")

project(variant_lib)

include_directories(./)

add_subdirectory(variant)
add_subdirectory(static_evaluator)
add_subdirectory(dynamic_evaluator)
add_subdirectory(catch)
