cmake_minimum_required(VERSION 3.7)

project(variant_lib)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(./)

# FIXME: CMAKE_BUILD_TYPE=Release must imply -DNDEBUG implicitly
if (NOT CMAKE_BUILD_TYPE MATCHES Debug)
  add_definitions(-DNDEBUG)
  # Enabling benchmarks
  add_subdirectory(bench)
endif()

# Enabling tests
enable_testing()
add_subdirectory(test)

add_subdirectory(deps)

add_subdirectory(evaluator)
add_subdirectory(parser)
add_subdirectory(util)
add_subdirectory(variant)
