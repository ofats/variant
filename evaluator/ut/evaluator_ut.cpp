#include "evaluator/evaluator.h"

#include "catch/catch.h"

#include <iostream>

using namespace Catch::literals;

namespace {

const auto pi_num = std::cos(-1);

}  // namespace

TEST_CASE("Print test", "[evaluator]") {
    evaler::calc_node node = 5.0;
    auto result = evaler::print(node);
    std::cout << result;

    evaler::calc_node sum_node{IN_PLACE_TYPE<evaler::binary_op<'+'>>,
                                std::make_unique<evaler::calc_node>(2.0),
                                std::make_unique<evaler::calc_node>(3.0)};

    result = evaler::print(sum_node);
    std::cout << result;
}

TEST_CASE("Parse test", "[evaluator]") {
    const auto node =
        evaler::parse("1 +  2 + 4 / 2 + 8 *(1- 2) + 2**8 + sin(0) - cos(0)");
    std::cout << evaler::print(node);
    REQUIRE(evaler::eval(node) == 252_a);
    const auto dyn_node = evaler::convert_to_dynamic(node);
    REQUIRE(evaler::print(node) == dyn_node->print());
    REQUIRE(dyn_node->eval() == 252_a);
}

TEST_CASE("Simple test", "[evaluator]") {
    SECTION("Sum test") {
        const auto node = evaler::parse("2 + 3");
        REQUIRE(evaler::eval(node) == 5_a);
    }
    SECTION("Sub test") {
        const auto node = evaler::parse("2 - 3");
        REQUIRE(evaler::eval(node) == -1_a);
    }
    SECTION("Mul test") {
        const auto node = evaler::parse("2 * 3");
        REQUIRE(evaler::eval(node) == 6_a);
    }
    SECTION("Div test") {
        const auto node = evaler::parse("3 / 2");
        REQUIRE(evaler::eval(node) == 1.5_a);
    }
    SECTION("Pow test") {
        const auto node = evaler::parse("2 ** 3");
        REQUIRE(evaler::eval(node) == 8_a);
    }
    SECTION("Sin test") {
        auto node = evaler::parse("sin(0)");
        REQUIRE(evaler::eval(node) == 0_a);
        node = evaler::parse("sin(3)");
        REQUIRE(evaler::eval(node) == Approx(std::sin(3)));
    }
    SECTION("Cos test") {
        auto node = evaler::parse("cos(0)");
        REQUIRE(evaler::eval(node) == 1_a);
        node = evaler::parse("cos(3)");
        REQUIRE(evaler::eval(node) == Approx(std::cos(3)));
    }
    SECTION("Log test") {
        auto node = evaler::parse("log(1)");
        REQUIRE(evaler::eval(node) == 0_a);
        node = evaler::parse("log(3)");
        REQUIRE(evaler::eval(node) == Approx(std::log(3)));
    }
}
