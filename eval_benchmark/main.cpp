#include "benchmark/benchmark.h"

#include "dynamic_evaluator/evaluator.h"
#include "static_evaluator/evaluator.h"

namespace {

constexpr char test_data[] = "1 + 2 * (3 - 5) ** 3 / 2 - 6";

std::string create_big_data() {
    auto result = std::string{test_data};
    for (std::size_t i = 0; i < 13; ++i) {
        result += '+' + result;
    }
    return result;
}

}  // namespace

void BM_static_eval(benchmark::State& state) {
    auto tree = static_evaluator::parse(test_data);
    for (auto _ : state) {
        static_evaluator::eval(tree);
    }
}

void BM_dynamic_eval(benchmark::State& state) {
    auto tree = dynamic_evaluator::parse(test_data);
    for (auto _ : state) {
        tree->eval();
    }
}

void BM_static_eval_big(benchmark::State& state) {
    auto tree = static_evaluator::parse(create_big_data());
    for (auto _ : state) {
        static_evaluator::eval(tree);
    }
}

void BM_dynamic_eval_big(benchmark::State& state) {
    auto tree = dynamic_evaluator::parse(create_big_data());
    for (auto _ : state) {
        tree->eval();
    }
}

BENCHMARK(BM_static_eval);
BENCHMARK(BM_dynamic_eval);
BENCHMARK(BM_static_eval_big);
BENCHMARK(BM_dynamic_eval_big);

BENCHMARK_MAIN();
