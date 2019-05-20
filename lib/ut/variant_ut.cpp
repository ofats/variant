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
}
