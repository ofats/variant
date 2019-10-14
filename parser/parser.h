#pragma once

#include "util/meta.h"

#include <algorithm>
#include <stdexcept>
#include <string>

namespace prs {

// Lightweight view over the string (similar to std::string_view).
// Field `cursor` point to current position in the string.
struct input_data {
    const std::string* input;
    std::size_t cursor = 0;
};

// Creates nice error message which contains visual information about current
// symbol in the input.
inline std::string make_fancy_error_log(const input_data& data) {
    return "Syntax error:\n\"" + *data.input + "\"\n" +
           std::string(data.cursor + 1, '~') + '^';
}

// Just returns symbol under the cursor.
inline char peek(const input_data& data) { return (*data.input)[data.cursor]; }

inline input_data unrecv(input_data&& data) {
    --data.cursor;
    return data;
}

// -------------------- TRANSFORMATION OPS --------------------

// Lambda wrapper to deal with ADL in operators.
template <class F>
struct callable {
    auto operator()(input_data&& data) const {
        return base::invoke(f, std::move(data));
    }
    F f;
};

template <class T>
struct is_tns : std::false_type {};

template <class F>
struct is_tns<callable<F>> : std::true_type {};

template <class T>
constexpr bool is_tns_v = is_tns<T>::value;

template <class F, class FDec = std::decay_t<F>>
auto make_callable(F&& f)
    -> std::enable_if_t<base::is_invocable_r_v<input_data, F, input_data&&>,
                        callable<FDec>> {
    return callable<FDec>{std::forward<F>(f)};
}

// Transformation applier.
template <class Tns>
std::enable_if_t<is_tns_v<std::decay_t<Tns>>, input_data> operator>>(
    input_data&& data, Tns&& t) {
    return base::invoke(std::forward<Tns>(t), std::move(data));
}

// In place transformation applier.
template <class Tns>
std::enable_if_t<is_tns_v<std::decay_t<Tns>>> operator>>=(input_data& data,
                                                          Tns&& t) {
    data = std::move(data) >> std::forward<Tns>(t);
}

// Transformations composition.
template <class A, class B,
          class = std::enable_if_t<is_tns_v<std::decay_t<A>> &&
                                   is_tns_v<std::decay_t<B>>>>
auto operator>>(A&& a, B&& b) {
    return make_callable(
        [a = std::forward<A>(a), b = std::forward<B>(b)](input_data&& data) {
            return std::move(data) >> a >> b;
        });
}

// -------------------- TRANSFORMATIONS --------------------

// Unconditional cursor advance.
template <std::size_t num = 1>
auto advance() {
    return make_callable([](input_data&& data) {
        data.cursor += num;
        return data;
    });
}

// Cursor will be advanced only if symbol under it equals `c`.
template <char c>
auto advance_if() {
    return make_callable([](input_data&& data) {
        if (c != (*data.input)[data.cursor]) {
            throw std::runtime_error{make_fancy_error_log(data) +
                                     "\nExpected '" + c + '\''};
        }
        ++data.cursor;
        return data;
    });
}

// Cursor will be repeatedly advanced
// until symbol under it stops being equal `c`.
template <char c>
auto advance_while() {
    return make_callable([](input_data&& data) {
        while (c == (*data.input)[data.cursor]) {
            ++data.cursor;
        }
        return data;
    });
}

// Cursor will be advanced only if symbol under it satisfies the predicate `F`.
template <class F,
          class = std::enable_if_t<base::is_invocable_r_v<bool, F, char>>>
auto advance_if(F&& f) {
    return make_callable([f = std::forward<F>(f)](input_data&& data) {
        if (!base::invoke(f, (*data.input)[data.cursor])) {
            throw std::runtime_error{make_fancy_error_log(data)};
        }
        ++data.cursor;
        return data;
    });
}

// Cursor will be repeatedly advanced
// until symbol under it stops satisfying the predicate `F`.
template <class F,
          class = std::enable_if_t<base::is_invocable_r_v<bool, F, char>>>
auto advance_while(F&& f) {
    return make_callable([f = std::forward<F>(f)](input_data&& data) {
        while (base::invoke(f, (*data.input)[data.cursor])) {
            ++data.cursor;
        }
        return data;
    });
}

// Cursor will be advanced only if the string under it equals `s`.
//
// We're assuming that argument to this call will always be know during compile
// time, i.e it would be string literal. So there wouldn't be dangling pointer.
template <std::size_t n>
auto advance_if(const char (&s)[n]) {
    // s inside the capture is decayed to `const char*`.
    return make_callable([s = s](input_data&& data) {
        if (!std::equal(s, s + n - 1, data.input->cbegin() + data.cursor)) {
            throw std::runtime_error{make_fancy_error_log(data) +
                                     "\nExpected \"" +
                                     std::string(s, s + n - 1) + '\"'};
        }
        data.cursor += n - 1;
        return data;
    });
}

// -------------------- PREDEFINED TRANSFORMATIONS --------------------

// Skips as many whitespace symbols as possible.
inline auto skip_spaces() {
    return advance_while([](const char c) -> bool { return std::isspace(c); });
}

}  // namespace prs
