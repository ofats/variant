#include "lib/variant.h"

#include "catch/catch.h"

TEST_CASE("Smoking test", "[variant]") {
    using TVar = TVariant<int, double, std::string>;
    auto v = TVariant<int, double, std::string>{};
    REQUIRE(HoldsAlternative<int>(v));
    REQUIRE(0 == v.index());

    SECTION("Changing value") {
        v = 1.5;
        REQUIRE(HoldsAlternative<double>(v));
        REQUIRE(1 == v.index());
    }

    SECTION("Visitation") {
        Visit([](auto&& value) {
            REQUIRE(std::is_same<decltype(value), int&>::value);
            value = 1;
        }, v);
        REQUIRE(1 == Get<0>(v));
    }

    SECTION("Visitation of rvalue") {
        Visit([](auto&& value) {
            REQUIRE(std::is_same<decltype(value), int&&>::value);
        }, std::move(v));
    }

    auto sumVisiter = [](auto&& value) -> TVar { return value + value; };

    v = 3;
    v = Visit(sumVisiter, std::move(v));
    REQUIRE(Get<0>(v) == 6);

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

    REQUIRE_THROWS_AS(v.emplace<1>(), std::runtime_error);

    REQUIRE(v.valueless_by_exception());
    REQUIRE(VARIANT_NPOS == v.index());

    REQUIRE_THROWS_AS(Get<0>(v), TBadVariantAccess);
    REQUIRE(nullptr == GetIf<0>(&v));
    REQUIRE_THROWS_AS(Visit([](auto&&) {}, v), TBadVariantAccess);

    v = 5;
    REQUIRE(HoldsAlternative<int>(v));
    REQUIRE(0 == v.index());
    REQUIRE(5 == Get<int>(v));
}
