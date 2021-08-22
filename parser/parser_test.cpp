#include "parser.h"

#include "catch2/catch_all.hpp"

using namespace Catch::literals;

TEST_CASE("Basic test", "[parser]") {
  const auto str = std::string{"abcd"};
  auto data = prs::input_data{&str};
  REQUIRE(prs::peek(data) == str.front());
}

TEST_CASE("Transformation static test", "[parser]") {
  SECTION("Wrong signature and no callable wrapper") {
    auto tns = [](prs::input_data&&) { return false; };
    STATIC_REQUIRE(!prs::is_tns_v<decltype(tns)>);
  }
  SECTION("No callable wrapper") {
    auto tns2 = [](prs::input_data&& data) { return data; };
    STATIC_REQUIRE(!prs::is_tns_v<decltype(tns2)>);
  }
  SECTION("Valid transformation") {
    auto tns2 = prs::make_callable([](prs::input_data&& data) { return data; });
    STATIC_REQUIRE(prs::is_tns_v<decltype(tns2)>);
  }
}

TEST_CASE("Advance test", "[parser]") {
  SECTION("Advance by number") {
    const auto str = std::string{"abcd"};
    auto data = prs::input_data{&str} >> prs::advance<2>();
    REQUIRE(prs::peek(data) == str[2]);
  }

  SECTION("Advance by specific char(s)") {
    const auto str = std::string{"aaabbbaaa"};
    auto data = prs::input_data{&str} >> prs::advance_if<'a'>();
    REQUIRE(data.cursor == 1);
    REQUIRE_THROWS(data >>= prs::advance_if<'b'>());

    data >>= prs::advance_while<'a'>();
    REQUIRE(prs::peek(data) == 'b');

    data >>= prs::advance_while<'a'>();
    REQUIRE(prs::peek(data) == 'b');
  }

  SECTION("Advance by predicate") {
    const auto is_alpha = [](const char c) -> bool { return std::isalpha(c); };
    const auto is_punct = [](const char c) -> bool { return std::ispunct(c); };

    const auto str = std::string{"difji[][-=+()agbo*&^$#%@!"};
    auto data = prs::input_data{&str} >> prs::advance_if(is_alpha);
    REQUIRE(is_alpha(prs::peek(data)));
    REQUIRE_THROWS(data >>= prs::advance_if(is_punct));

    data >>= prs::advance_while(is_alpha);
    REQUIRE(is_punct(prs::peek(data)));
    data >>= prs::advance_if(is_punct);
    REQUIRE(is_punct(prs::peek(data)));
  }
}

TEST_CASE("Tokens parse test") {
  const auto str = std::string{"my   name\tis \n\tbatman"};
  auto data = prs::input_data{&str};
  REQUIRE(prs::peek(data) == 'm');
  REQUIRE_NOTHROW(data >>= prs::advance_if("my") >> prs::skip_spaces());

  REQUIRE(prs::peek(data) == 'n');
  REQUIRE_NOTHROW(data >>= prs::advance_if("name") >> prs::skip_spaces());

  REQUIRE(prs::peek(data) == 'i');
  REQUIRE_NOTHROW(data >>= prs::advance_if("is") >> prs::skip_spaces());

  REQUIRE(prs::peek(data) == 'b');
  REQUIRE_NOTHROW(data >>= prs::advance_if("batman") >> prs::skip_spaces());
  REQUIRE(prs::peek(data) == '\0');
}
