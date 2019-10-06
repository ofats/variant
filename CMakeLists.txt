cmake_minimum_required(VERSION 3.7)
enable_testing()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS_DEBUG "-O0 -fno-omit-frame-pointer -g -fsanitize=address -Wall -Wextra -Werror")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -Wall -Wextra -Werror -g")

# FIXME: CMAKE_BUILD_TYPE=Release must imply -DNDEBUG implicitly
if (NOT CMAKE_BUILD_TYPE MATCHES Debug)
  add_definitions(-DNDEBUG)
endif()

project(variant_lib)

include_directories(./)
# include_directories(./benchmark/include)

add_subdirectory(deps)

add_subdirectory(catch)
add_subdirectory(evaluator)
add_subdirectory(parser)
add_subdirectory(util)
add_subdirectory(variant)
