# variant
Analogue of std::variant written in pure C++14.

Interface is 99% similar to std::variant:
https://en.cppreference.com/w/cpp/utility/variant

## What else do we have?
Besides unittests, we have small library for expressions evaluation, located at
`evaluator/`.

Expression trees represented by nested nodes, that are built on variants.
You can run `bazel run evaluator:calculator` to use simple calculator program.

There is also a "dynamic" analogue of expression tree, that is built on abstract
classes and virtual method calls.

You can benchmark both approaches by running
`bazel run evaluator:evaluator_bench -c opt` (don't even try running debug
build binary, please).
