#include "variant.h"

#include "catch2/catch_all.hpp"

TEST_CASE("Smoking test", "[variant]") {
    using var_t = base::variant<int, double, std::string>;
    auto v = var_t{};
    REQUIRE(base::holds_alternative<int>(v));
    REQUIRE(0 == v.index());

    SECTION("Changing value") {
        v = 1.5;
        REQUIRE(base::holds_alternative<double>(v));
        REQUIRE(1 == v.index());
    }

    SECTION("Visitation") {
        base::visit(
            [](auto&& value) {
                REQUIRE(std::is_same<decltype(value), int&>::value);
                value = 1;
            },
            v);
        REQUIRE(1 == base::get<0>(v));
    }

    SECTION("Visitation of rvalue") {
        base::visit(
            [](auto&& value) {
                REQUIRE(std::is_same<decltype(value), int&&>::value);
            },
            std::move(v));
    }

    auto sumVisiter = [](auto&& value) -> var_t { return value + value; };

    v = 3;
    v = base::visit(sumVisiter, std::move(v));
    REQUIRE(base::get<int>(v) == 6);
    REQUIRE(base::get<0>(v) == 6);
    REQUIRE(*base::get_if<int>(&v) == 6);
    REQUIRE(*base::get_if<0>(&v) == 6);

    v = std::string{"abc"};
    v = base::visit([](auto&& value) -> var_t { return value + value; },
                    std::move(v));
    REQUIRE(base::get<2>(v) == "abcabc");

    SECTION("Variant copy") {
        auto tmp = v;
        REQUIRE(tmp.index() == v.index());
        REQUIRE(tmp == v);
        REQUIRE(base::get<std::string>(tmp) == "abcabc");
        tmp.emplace<1>(0.5);
        v = tmp;
        REQUIRE(base::holds_alternative<double>(v));
    }
}

TEST_CASE("Valueless by exception test", "[variant]") {
    struct TThrowOnConstruct {
        TThrowOnConstruct() { throw std::runtime_error{"TThrowOnConstruct"}; }
    };

    using var_t = base::variant<int, TThrowOnConstruct>;

    var_t v;
    REQUIRE(base::holds_alternative<int>(v));
    REQUIRE(0 == v.index());
    REQUIRE(0 == base::get<0>(v));

    SECTION("Variant would become valueless after emplace") {
        REQUIRE_THROWS_AS(v.emplace<1>(), std::runtime_error);

        REQUIRE(v.valueless_by_exception());
        REQUIRE(base::variant_npos == v.index());

        REQUIRE_THROWS_AS(base::get<0>(v), base::bad_variant_access);
        REQUIRE(nullptr == base::get_if<0>(&v));
        REQUIRE_THROWS_AS(base::visit([](auto&&) {}, v),
                          base::bad_variant_access);
    }

    var_t v2;
    REQUIRE_THROWS_AS(v2.emplace<1>(), std::runtime_error);

    SECTION("Swap would work even if one of variants in valueless") {
        v.swap(v2);
        REQUIRE(v.valueless_by_exception());
        REQUIRE(base::holds_alternative<int>(v2));
    }
}

namespace {

template <class Arg, class F>
constexpr bool CheckCallable(F&&) {
    return base::is_invocable_v<F&&, Arg>;
}

#define WELL_FORMED(TYPE, VAR, EXP) \
    CheckCallable<TYPE>([](auto VAR) -> decltype(EXP) {})

#define ILL_FORMED(TYPE, VAR, EXP) !WELL_FORMED(TYPE, VAR, EXP)

}  // namespace

TEST_CASE("SFINAE-friendliness test", "[variant]") {
    using var_t = base::variant<int, double, char, char>;

    SECTION("Check converting constructor") {
        REQUIRE(WELL_FORMED(int, v, var_t{v}));
        REQUIRE(ILL_FORMED(char, v, var_t{v}));
        REQUIRE(ILL_FORMED(std::string, v, var_t{v}));
    }

    SECTION("Check base::get and base::get_if by index") {
        // Argument to base::get and base::get_if functions must be less then
        // number of variant alternatives
        REQUIRE(WELL_FORMED(var_t, v, base::get<2>(v)));
        REQUIRE(WELL_FORMED(var_t, v, base::get_if<2>(&v)));
        REQUIRE(ILL_FORMED(var_t, v, base::get<5>(v)));
        REQUIRE(ILL_FORMED(var_t, v, base::get_if<5>(&v)));
    }

    SECTION("Check base::get and base::get_if by type") {
        // Type must be presented in altenatives list exactly once
        REQUIRE(WELL_FORMED(var_t, v, base::get<int>(v)));
        REQUIRE(WELL_FORMED(var_t, v, base::get<double>(v)));
        REQUIRE(ILL_FORMED(var_t, v, base::get<char>(v)));
        REQUIRE(ILL_FORMED(var_t, v, base::get<int*>(v)));

        REQUIRE(WELL_FORMED(var_t, v, base::get_if<int>(&v)));
        REQUIRE(WELL_FORMED(var_t, v, base::get_if<double>(&v)));
        REQUIRE(ILL_FORMED(var_t, v, base::get_if<char>(&v)));
        REQUIRE(ILL_FORMED(var_t, v, base::get_if<int*>(&v)));
    }

    SECTION("Check base::holds_alternative") {
        // Type must be presented in altenatives list exactly once
        REQUIRE(WELL_FORMED(var_t, v, base::holds_alternative<int>(v)));
        REQUIRE(WELL_FORMED(var_t, v, base::holds_alternative<double>(v)));
        REQUIRE(ILL_FORMED(var_t, v, base::holds_alternative<char>(v)));
        REQUIRE(ILL_FORMED(var_t, v, base::holds_alternative<int*>(v)));
    }

    using var2_t = base::variant<int, std::string>;

    SECTION("Check emplace") {
        // Choosen alternative must be constructible from arguments
        REQUIRE(WELL_FORMED(var2_t, v, v.template emplace<int>(5)));
        REQUIRE(WELL_FORMED(var2_t, v, v.template emplace<0>(5)));
        REQUIRE(ILL_FORMED(var2_t, v, v.template emplace<int>(nullptr)));
        REQUIRE(ILL_FORMED(var2_t, v, v.template emplace<0>(nullptr)));

        REQUIRE(WELL_FORMED(var2_t, v, v.template emplace<std::string>("abc")));
        REQUIRE(WELL_FORMED(var2_t, v, v.template emplace<1>("abc")));
        REQUIRE(ILL_FORMED(var2_t, v,
                           v.template emplace<std::string>((void*)nullptr)));
        REQUIRE(ILL_FORMED(var2_t, v, v.template emplace<1>((void*)nullptr)));

        REQUIRE(ILL_FORMED(var2_t, v, v.template emplace<2>()));
        REQUIRE(ILL_FORMED(var2_t, v, v.template emplace<double>()));
    }

    SECTION("Check base::visit") {
        // Invocation to visit must be a valid expression of the same type and
        // value category, for all combinations of alternative types of all
        // variants.
        auto nothingOp = [](auto &&) -> void {};
        REQUIRE(WELL_FORMED(var_t, v, base::visit(nothingOp, v)));
        auto returnOp = [](auto&& value) { return value; };
        REQUIRE(ILL_FORMED(var_t, v, base::visit(returnOp, v)));
        auto notAllTypesOp = [](int) {};
        REQUIRE(ILL_FORMED(var2_t, v, base::visit(notAllTypesOp, v)));
    }
}
