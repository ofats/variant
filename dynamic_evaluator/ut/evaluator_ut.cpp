#include "dynamic_evaluator/evaluator.h"

#include "catch/catch.h"

#include <iostream>

using namespace Catch::literals;
namespace dyn_eval = dynamic_evaluator;

TEST_CASE("Print test", "[dynamic_evaluator]") {
    const auto node = std::make_unique<dyn_eval::value_node>(5.0);
    auto result = node->print();
    std::cout << result;

    const auto sum_node = std::make_unique<dyn_eval::sum_node>(
        std::make_unique<dyn_eval::value_node>(2.0),
        std::make_unique<dyn_eval::value_node>(3.0));

    result = sum_node->print();
    std::cout << result;
}

TEST_CASE("Parse test", "[dynamic_evaluator]") {
    const auto node = dyn_eval::parse("1 +  2 + 4 / 2 + 8 *(1- 2) + 2**8");
    std::cout << node->print();
    REQUIRE(node->eval() == 253_a);
}

TEST_CASE("Simple test", "[dynamic_evaluator]") {
    SECTION("Sum test") {
        const auto node = dyn_eval::parse("2 + 3");
        REQUIRE(node->eval() == 5_a);
    }
    SECTION("Sub test") {
        const auto node = dyn_eval::parse("2 - 3");
        REQUIRE(node->eval() == -1_a);
    }
    SECTION("Mul test") {
        const auto node = dyn_eval::parse("2 * 3");
        REQUIRE(node->eval() == 6_a);
    }
    SECTION("Div test") {
        const auto node = dyn_eval::parse("3 / 2");
        REQUIRE(node->eval() == 1.5_a);
    }
    SECTION("Pow test") {
        const auto node = dyn_eval::parse("2 ** 3");
        REQUIRE(node->eval() == 8_a);
    }
}
