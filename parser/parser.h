#pragma once

#include "util/meta.h"

#include <algorithm>
#include <string>
#include <stdexcept>

namespace prs {

struct input_data {
    const std::string* input;
    std::size_t cursor;
};

inline std::string make_fancy_error_log(const input_data& data) {
    return "Syntax error:\n\"" + *data.input + "\"\n" +
           std::string(data.cursor + 1, '~') + '^';
}

inline char peek(const input_data& data) { return (*data.input)[data.cursor]; }

template <char c>
input_data next_if(input_data&& data) {
    if (c != (*data.input)[data.cursor]) {
        throw std::runtime_error{make_fancy_error_log(data) + "\nExpected '" +
                                 c + '\''};
    }
    ++data.cursor;
    return data;
}

template <std::size_t n>
input_data next_if(input_data&& data, const char* s) {
    if (!std::equal(s, s + n, data.input->cbegin() + data.cursor)) {
        throw std::runtime_error{make_fancy_error_log(data) + "\nExpected \"" +
                                 std::string(s, s + n) + '\"'};
    }
    data.cursor += n;
    return data;
}

template <class F>
std::enable_if_t<base::is_invocable_r_v<bool, F&&, char>, input_data> next_if(
    input_data&& data, F&& f) {
    if (!std::forward<F>(f)((*data.input)[data.cursor])) {
        throw std::runtime_error{make_fancy_error_log(data)};
    }
    ++data.cursor;
    return data;
}

inline input_data unnext(input_data&& data) {
    --data.cursor;
    return data;
}

inline input_data skip_spaces(input_data&& data) {
    while (std::isspace((*data.input)[data.cursor])) {
        ++data.cursor;
    }
    return data;
}

template <class T>
constexpr bool is_tns_v = base::is_invocable_r_v<input_data, T, input_data&&>;

// Transformation applier.
template <class Tns>
std::enable_if_t<is_tns_v<Tns>, input_data> operator>>(input_data&& data,
                                                       Tns t) {
    return t(std::move(data));
}

// In place transformation applier.
template <class Tns>
std::enable_if_t<is_tns_v<Tns>> operator>>=(input_data& data, Tns t) {
    data = std::move(data) >> t;
}

// Transformations composition.
template <class A, class B,
          class = std::enable_if_t<is_tns_v<A> && is_tns_v<B>>>
auto operator>>(A a, B b) {
    return [a = std::move(a), b = std::move(b)](input_data&& data) {
        return std::move(data) >> a >> b;
    };
}

// -------------------- SKIP SPACES --------------------

inline auto skip_spaces() {
    return [](input_data&& data) { return skip_spaces(std::move(data)); };
}

// -------------------- NEXT --------------------

template <char c>
auto next_if() {
    return [](input_data&& data) { return next_if<c>(std::move(data)); };
}

// We're assuming that argument to this call will always be know during compile
// time, i.e it would be string literal. So there wouldn't be dangling pointer.
template <std::size_t n>
auto next_if(const char (&s)[n]) {
    // s inside the capture is decayed to `const char*`.
    return [s = s](input_data&& data) { return next_if<n - 1>(std::move(data), s); };
}

template <class F,
          class = std::enable_if_t<base::is_invocable_r_v<bool, F, char>>>
auto next_if(F&& f) {
    return [f = std::move(f)](input_data&& data) {
        return next_if(std::move(data), f);
    };
}

inline auto next_if_digit() {
    return next_if([](const char c) -> bool { return std::isdigit(c); });
}

template <std::size_t num = 1>
auto next() {
    return [](input_data&& data) {
        data.cursor += num;
        return data;
    };
}

}  // namespace prs
