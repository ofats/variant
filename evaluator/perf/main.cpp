#include "evaluator/evaluator.h"

#include "benchmark/benchmark.h"

namespace {

constexpr char test_data[] = "1 + 2 * (3 - 5) ** 3 / 2 - 6 - cos(3) + sin(2)";

std::string create_big_data() {
    auto result = std::string{test_data};
    for (std::size_t i = 0; i < 13; ++i) {
        result += '+' + result;
    }
    return result;
}

const auto small_tree = evaler::parse(test_data);
const auto small_tree_dyn = evaler::convert_to_dynamic(small_tree);
const auto large_tree = evaler::parse(create_big_data());
const auto large_tree_dyn = evaler::convert_to_dynamic(large_tree);

}  // namespace

void BM_static_eval(benchmark::State& state) {
    for (auto _ : state) {
        evaler::eval(small_tree);
    }
}

void BM_dynamic_eval(benchmark::State& state) {
    for (auto _ : state) {
        small_tree_dyn->eval();
    }
}

void BM_static_eval_big(benchmark::State& state) {
    for (auto _ : state) {
        evaler::eval(large_tree);
    }
}

void BM_dynamic_eval_big(benchmark::State& state) {
    for (auto _ : state) {
        large_tree_dyn->eval();
    }
}

BENCHMARK(BM_static_eval);
BENCHMARK(BM_dynamic_eval);
BENCHMARK(BM_static_eval_big);
BENCHMARK(BM_dynamic_eval_big);

BENCHMARK_MAIN();
