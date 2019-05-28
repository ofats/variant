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

inline input_data next(input_data&& data, const char c) {
    if (c != (*data.input)[data.cursor]) {
        throw std::runtime_error{make_fancy_error_log(data) + "\nExpected '" +
                                 c + '\''};
    }
    ++data.cursor;
    return data;
}

template <std::size_t n>
input_data next(input_data&& data, const char (&s)[n]) {
    if (!std::count(s, s + n, (*data.input)[data.cursor])) {
        throw std::runtime_error{make_fancy_error_log(data) +
                                 "\nExpected one of {" + s + '}'};
    }
    ++data.cursor;
    return data;
}

template <class F>
std::enable_if_t<base::is_invocable_v<F, char>, input_data> next(
    input_data&& data, F f) {
    if (!f((*data.input)[data.cursor])) {
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

}  // namespace prs
