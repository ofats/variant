#include "static_evaluator/evaluator.h"

#include "catch/catch.h"

#include <iostream>

TEST_CASE("Print test", "[static_evaluator]") {
    namespace st_eval = static_evaluator;
    st_eval::calc_node node = 5.0;
    auto result = st_eval::print(node);
    std::cout << result;

    st_eval::calc_node sum_node{IN_PLACE_TYPE<st_eval::binary_op<'+'>>,
                                std::make_unique<st_eval::calc_node>(2.0),
                                std::make_unique<st_eval::calc_node>(3.0)};

    result = st_eval::print(sum_node);
    std::cout << result;
}

TEST_CASE("Parse test", "[static_evaluator]") {
    namespace st_eval = static_evaluator;
    const auto node = st_eval::parse("1 +  2 + 4 / 2 + 8 *(1- 2)");
    std::cout << st_eval::print(node);
    using namespace Catch::literals;
    REQUIRE(st_eval::eval(node) == -3_a);
}
