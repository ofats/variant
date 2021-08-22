#include "benchmark/benchmark.h"
#include "evaluator.h"

namespace {

constexpr char test_data[] = "1 + 2 * (3 - 5) ** 3 / 2 - 6 - cos(3) + sin(2)";

[[maybe_unused]] std::string create_big_data() {
  auto result = std::string{test_data};
  for (std::size_t i = 0; i < 13; ++i) {
    result += '+' + result;
  }
  return result;
}

const auto small_tree = evaler::parse("42");
const auto small_tree_dyn = evaler::convert_to_dynamic(small_tree);
const auto tree = evaler::parse(test_data);
const auto tree_dyn = evaler::convert_to_dynamic(tree);
const auto large_tree = evaler::parse(create_big_data());
const auto large_tree_dyn = evaler::convert_to_dynamic(large_tree);

}  // namespace

void make_stupid_arr() {
  auto v = std::vector<int>(8 * 1024 * 1024, 1);
  auto res = std::transform(v.cbegin(), v.cend(), v.begin(),
                            [](int a) { return a + 7; });
  benchmark::DoNotOptimize(res);
}

void BM_static_eval_small(benchmark::State& state) {
  make_stupid_arr();
  for (auto _ : state) {
    benchmark::DoNotOptimize(evaler::eval(small_tree));
  }
}

void BM_dynamic_eval_small(benchmark::State& state) {
  make_stupid_arr();
  for (auto _ : state) {
    benchmark::DoNotOptimize(small_tree_dyn->eval());
  }
}

void BM_static_eval(benchmark::State& state) {
  make_stupid_arr();
  for (auto _ : state) {
    benchmark::DoNotOptimize(evaler::eval(tree));
  }
}

void BM_dynamic_eval(benchmark::State& state) {
  make_stupid_arr();
  for (auto _ : state) {
    benchmark::DoNotOptimize(tree_dyn->eval());
  }
}

void BM_static_eval_big(benchmark::State& state) {
  make_stupid_arr();
  for (auto _ : state) {
    benchmark::DoNotOptimize(evaler::eval(large_tree));
  }
}

void BM_dynamic_eval_big(benchmark::State& state) {
  make_stupid_arr();
  for (auto _ : state) {
    benchmark::DoNotOptimize(large_tree_dyn->eval());
  }
}

BENCHMARK(BM_static_eval_small);
BENCHMARK(BM_dynamic_eval_small);
BENCHMARK(BM_static_eval);
BENCHMARK(BM_dynamic_eval);
BENCHMARK(BM_static_eval_big);
BENCHMARK(BM_dynamic_eval_big);

BENCHMARK_MAIN();
