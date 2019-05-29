#include "variant/variant.h"

#include "catch/catch.h"

TEST_CASE("Smoking test", "[variant]") {
    using TVar = TVariant<int, double, std::string>;
    auto v = TVar{};
    REQUIRE(HoldsAlternative<int>(v));
    REQUIRE(0 == v.index());

    SECTION("Changing value") {
        v = 1.5;
        REQUIRE(HoldsAlternative<double>(v));
        REQUIRE(1 == v.index());
    }

    SECTION("Visitation") {
        Visit(
            [](auto&& value) {
                REQUIRE(std::is_same<decltype(value), int&>::value);
                value = 1;
            },
            v);
        REQUIRE(1 == Get<0>(v));
    }

    SECTION("Visitation of rvalue") {
        Visit(
            [](auto&& value) {
                REQUIRE(std::is_same<decltype(value), int&&>::value);
            },
            std::move(v));
    }

    auto sumVisiter = [](auto&& value) -> TVar { return value + value; };

    v = 3;
    v = Visit(sumVisiter, std::move(v));
    REQUIRE(Get<int>(v) == 6);
    REQUIRE(Get<0>(v) == 6);
    REQUIRE(*GetIf<int>(&v) == 6);
    REQUIRE(*GetIf<0>(&v) == 6);

    v = std::string{"abc"};
    v = Visit([](auto&& value) -> TVar { return value + value; }, std::move(v));
    REQUIRE(Get<2>(v) == "abcabc");

    SECTION("Variant copy") {
        auto tmp = v;
        REQUIRE(tmp.index() == v.index());
        REQUIRE(tmp == v);
        REQUIRE(Get<std::string>(tmp) == "abcabc");
        tmp.emplace<1>(0.5);
        v = tmp;
        REQUIRE(HoldsAlternative<double>(v));
    }
}

TEST_CASE("Valueless by exception test", "[variant]") {
    struct TThrowOnConstruct {
        TThrowOnConstruct() { throw std::runtime_error{"TThrowOnConstruct"}; }
    };

    using TVar = TVariant<int, TThrowOnConstruct>;

    TVar v;
    REQUIRE(HoldsAlternative<int>(v));
    REQUIRE(0 == v.index());
    REQUIRE(0 == Get<0>(v));

    SECTION("Variant would become valueless after emplace") {
        REQUIRE_THROWS_AS(v.emplace<1>(), std::runtime_error);

        REQUIRE(v.valueless_by_exception());
        REQUIRE(VARIANT_NPOS == v.index());

        REQUIRE_THROWS_AS(Get<0>(v), TBadVariantAccess);
        REQUIRE(nullptr == GetIf<0>(&v));
        REQUIRE_THROWS_AS(Visit([](auto&&) {}, v), TBadVariantAccess);
    }

    TVar v2;
    REQUIRE_THROWS_AS(v2.emplace<1>(), std::runtime_error);

    SECTION("Swap would work even if one of variants in valueless") {
        v.swap(v2);
        REQUIRE(v.valueless_by_exception());
        REQUIRE(HoldsAlternative<int>(v2));
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
    using TVar = TVariant<int, double, char, char>;

    SECTION("Check converting constructor") {
        REQUIRE(WELL_FORMED(int, v, TVar{v}));
        REQUIRE(ILL_FORMED(char, v, TVar{v}));
        REQUIRE(ILL_FORMED(std::string, v, TVar{v}));
    }

    SECTION("Check Get and GetIf by index") {
        // Argument to Get and GetIf functions must be less then number of
        // variant alternatives
        REQUIRE(WELL_FORMED(TVar, v, Get<2>(v)));
        REQUIRE(WELL_FORMED(TVar, v, GetIf<2>(&v)));
        REQUIRE(ILL_FORMED(TVar, v, Get<5>(v)));
        REQUIRE(ILL_FORMED(TVar, v, GetIf<5>(&v)));
    }

    SECTION("Check Get and GetIf by type") {
        // Type must be presented in altenatives list exactly once
        REQUIRE(WELL_FORMED(TVar, v, Get<int>(v)));
        REQUIRE(WELL_FORMED(TVar, v, Get<double>(v)));
        REQUIRE(ILL_FORMED(TVar, v, Get<char>(v)));
        REQUIRE(ILL_FORMED(TVar, v, Get<int*>(v)));

        REQUIRE(WELL_FORMED(TVar, v, GetIf<int>(&v)));
        REQUIRE(WELL_FORMED(TVar, v, GetIf<double>(&v)));
        REQUIRE(ILL_FORMED(TVar, v, GetIf<char>(&v)));
        REQUIRE(ILL_FORMED(TVar, v, GetIf<int*>(&v)));
    }

    SECTION("Check HoldsAlternative") {
        // Type must be presented in altenatives list exactly once
        REQUIRE(WELL_FORMED(TVar, v, HoldsAlternative<int>(v)));
        REQUIRE(WELL_FORMED(TVar, v, HoldsAlternative<double>(v)));
        REQUIRE(ILL_FORMED(TVar, v, HoldsAlternative<char>(v)));
        REQUIRE(ILL_FORMED(TVar, v, HoldsAlternative<int*>(v)));
    }

    using TVar2 = TVariant<int, std::string>;

    SECTION("Check emplace") {
        // Choosen alternative must be constructible from arguments
        REQUIRE(WELL_FORMED(TVar2, v, v.template emplace<int>(5)));
        REQUIRE(WELL_FORMED(TVar2, v, v.template emplace<0>(5)));
        REQUIRE(ILL_FORMED(TVar2, v, v.template emplace<int>(nullptr)));
        REQUIRE(ILL_FORMED(TVar2, v, v.template emplace<0>(nullptr)));

        REQUIRE(WELL_FORMED(TVar2, v, v.template emplace<std::string>("abc")));
        REQUIRE(WELL_FORMED(TVar2, v, v.template emplace<1>("abc")));
        REQUIRE(ILL_FORMED(TVar2, v,
                           v.template emplace<std::string>((void*)nullptr)));
        REQUIRE(ILL_FORMED(TVar2, v, v.template emplace<1>((void*)nullptr)));

        REQUIRE(ILL_FORMED(TVar2, v, v.template emplace<2>()));
        REQUIRE(ILL_FORMED(TVar2, v, v.template emplace<double>()));
    }

    SECTION("Check Visit") {
        // Invocation to visit must be a valid expression of the same type and
        // value category, for all combinations of alternative types of all
        // variants.
        auto nothingOp = [](auto &&) -> void {};
        REQUIRE(WELL_FORMED(TVar, v, Visit(nothingOp, v)));
        auto returnOp = [](auto&& value) { return value; };
        REQUIRE(ILL_FORMED(TVar, v, Visit(returnOp, v)));
        auto notAllTypesOp = [](int) {};
        REQUIRE(ILL_FORMED(TVar2, v, Visit(notAllTypesOp, v)));
    }
}
