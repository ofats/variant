add_subdirectory(catch2)

# Disabling benchmark in Debug
if (NOT CMAKE_BUILD_TYPE MATCHES Debug)
  set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "Suppressing benchmark's tests" FORCE)
  add_subdirectory(benchmark)
endif()
