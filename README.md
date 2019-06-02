# variant
Analogue of std::variant written in pure C++14.

Interface is 99% similar to std::variant: https://en.cppreference.com/w/cpp/utility/variant

## How to test it?
Firstly, you need to build it using one of these:
```
./build_debug.sh
```
For debug build, or:
```
./build_release.sh
```
For release build.
Then you need to run tests using `./run_tests_debug.sh` or `./run_tests_release.sh`.

## What else do we have?
Besides unittests, we have small library for expressions evaluation, located at `evaluator/`.

Expression trees represented by nested nodes, that are built on variants.
You can run `./build/Release/evaluator/bin/calculator` to use simple calculator program.

There is also a "dynamic" analogue of expression tree, that is built on abstract classes and virtual method calls.

You can benchmark both approaches by running `./build/Release/evaluator/perf/evaler_benchmark`.
