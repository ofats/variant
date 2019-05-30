#include "static_evaluator/evaluator.h"

#include "catch/catch.h"

#include <iostream>

using namespace Catch::literals;
namespace st_eval = static_evaluator;

namespace {

const auto pi_num = std::cos(-1);

}  // namespace

TEST_CASE("Print test", "[static_evaluator]") {
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
    const auto node = st_eval::parse("1 +  2 + 4 / 2 + 8 *(1- 2) + 2**8");
    std::cout << st_eval::print(node);
    REQUIRE(st_eval::eval(node) == 253_a);
}

TEST_CASE("Simple test", "[static_evaluator]") {
    SECTION("Sum test") {
        const auto node = st_eval::parse("2 + 3");
        REQUIRE(st_eval::eval(node) == 5_a);
    }
    SECTION("Sub test") {
        const auto node = st_eval::parse("2 - 3");
        REQUIRE(st_eval::eval(node) == -1_a);
    }
    SECTION("Mul test") {
        const auto node = st_eval::parse("2 * 3");
        REQUIRE(st_eval::eval(node) == 6_a);
    }
    SECTION("Div test") {
        const auto node = st_eval::parse("3 / 2");
        REQUIRE(st_eval::eval(node) == 1.5_a);
    }
    SECTION("Pow test") {
        const auto node = st_eval::parse("2 ** 3");
        REQUIRE(st_eval::eval(node) == 8_a);
    }
    SECTION("Sin test") {
        auto node = st_eval::parse("sin(0)");
        REQUIRE(st_eval::eval(node) == 0_a);
        node = st_eval::parse("sin(3)");
        REQUIRE(st_eval::eval(node) == Approx(std::sin(3)));
    }
    SECTION("Cos test") {
        auto node = st_eval::parse("cos(0)");
        REQUIRE(st_eval::eval(node) == 1_a);
        node = st_eval::parse("cos(3)");
        REQUIRE(st_eval::eval(node) == Approx(std::cos(3)));
    }
    SECTION("Log test") {
        auto node = st_eval::parse("log(1)");
        REQUIRE(st_eval::eval(node) == 0_a);
        node = st_eval::parse("log(3)");
        REQUIRE(st_eval::eval(node) == Approx(std::log(3)));
    }
}
