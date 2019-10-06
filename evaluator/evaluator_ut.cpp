#include "evaluator.h"

#include "catch2/catch.hpp"

#include <iostream>

using namespace Catch::literals;

namespace {

const auto pi_num = std::cos(-1);

}  // namespace

TEST_CASE("Print test", "[evaluator]") {
    evaler::calc_node node = 5.0;
    auto result = evaler::print(node);
    std::cout << result;

    evaler::calc_node sum_node{base::in_place_type<evaler::binary_op<'+'>>, 2.0,
                               3.0};

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
    SECTION("Positive integer") {
        const auto node = evaler::parse("2");
        REQUIRE(evaler::eval(node) == 2_a);
    }
    SECTION("Negative integer") {
        const auto node = evaler::parse("-2");
        REQUIRE(evaler::eval(node) == -2_a);
    }
    SECTION("Positive double") {
        const auto node = evaler::parse("2.5");
        REQUIRE(evaler::eval(node) == 2.5_a);
    }
    SECTION("Negative double") {
        const auto node = evaler::parse("-2.5");
        REQUIRE(evaler::eval(node) == -2.5_a);
    }
    SECTION("Sum") {
        const auto node = evaler::parse("2 + 3");
        REQUIRE(evaler::eval(node) == 5_a);
    }
    SECTION("Sub") {
        const auto node = evaler::parse("2 - 3");
        REQUIRE(evaler::eval(node) == -1_a);
    }
    SECTION("Mul") {
        const auto node = evaler::parse("2 * 3");
        REQUIRE(evaler::eval(node) == 6_a);
    }
    SECTION("Div") {
        const auto node = evaler::parse("3 / 2");
        REQUIRE(evaler::eval(node) == 1.5_a);
    }
    SECTION("Pow") {
        const auto node = evaler::parse("2 ** 3");
        REQUIRE(evaler::eval(node) == 8_a);
    }
    SECTION("Sin") {
        auto node = evaler::parse("sin(0)");
        REQUIRE(evaler::eval(node) == 0_a);
        node = evaler::parse("sin(3)");
        REQUIRE(evaler::eval(node) == Approx(std::sin(3)));
    }
    SECTION("Cos") {
        auto node = evaler::parse("cos(0)");
        REQUIRE(evaler::eval(node) == 1_a);
        node = evaler::parse("cos(3)");
        REQUIRE(evaler::eval(node) == Approx(std::cos(3)));
        node = evaler::parse("cos(3.1415926)");
        REQUIRE(evaler::eval(node) == -1_a);
    }
    SECTION("Log") {
        auto node = evaler::parse("log(1)");
        REQUIRE(evaler::eval(node) == 0_a);
        node = evaler::parse("log(2.71828)");
        REQUIRE(evaler::eval(node) == 1_a);
    }
}
