#pragma once

#include "variant/variant.h"

#include <algorithm>
#include <cassert>
#include <memory>

namespace static_evaluator {

template <char sign>
struct binary_op;

using calc_node = TVariant<double, binary_op<'+'>, binary_op<'-'>,
                           binary_op<'*'>, binary_op<'/'>>;

template <char sign>
struct binary_op {
    binary_op(std::unique_ptr<calc_node> a, std::unique_ptr<calc_node> b)
        : left_expr(std::move(a)), right_expr(std::move(b)) {}
    binary_op(binary_op&&) noexcept = default;
    binary_op& operator=(binary_op&&) noexcept = default;

    std::unique_ptr<calc_node> left_expr, right_expr;
};

std::string print(const calc_node& n, const int indent = 0);

struct print_visitor {
    print_visitor(const int indent) : indent(indent) {}
    const int indent;

    auto operator()(double value) {
        return std::string(indent, '\t') + std::to_string(value) + '\n';
    }
    template <char sign>
    auto operator()(const binary_op<sign>& value) {
        return print(*value.left_expr, indent + 1) + std::string(indent, '\t') +
               std::string(1, sign) + '\n' +
               print(*value.right_expr, indent + 1);
    }
};

inline std::string print(const calc_node& n, const int indent) {
    auto vis = print_visitor{indent};
    return Visit(vis, n);
}

// `E` -> `E` + `T` | `E` - `T` | `T`
// `T` -> `T` * `F` | `T` / `F` | `F`
// `F` -> `P` | - 'N' | ( `E` )

namespace detail {

struct input_data {
    const std::string* input;
    std::size_t cursor;
};

inline std::string make_fancy_error_log(const input_data& data) {
    return "Syntax error:\n\"" + *data.input + "\"\n" +
           std::string(data.cursor + 1, '~') + '^';
}

inline char peek(const input_data& data) {
    return (*data.input)[data.cursor];
}

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
std::enable_if_t<meta::is_invocable_v<F, char>, input_data> next(
    input_data&& data, F f) {
    if (!f((*data.input)[data.cursor])) {
        throw std::runtime_error{make_fancy_error_log(data)};
    }
    ++data.cursor;
    return data;
}

inline input_data skip_spaces(input_data&& data) {
    while (std::isspace((*data.input)[data.cursor])) {
        ++data.cursor;
    }
    return data;
}

calc_node e_nonterm(input_data& input);
calc_node t_nonterm(input_data& input);
calc_node f_nonterm(input_data& input);
calc_node p_nonterm(input_data& input);
calc_node n_nonterm(input_data& input);

}  // namespace detail

inline calc_node parse(const std::string& input) {
    auto data = detail::skip_spaces(detail::input_data{&input, 0});
    auto result = detail::e_nonterm(data);
    if (data.cursor != input.size()) {
        throw std::runtime_error{detail::make_fancy_error_log(data) +
                                 "\nUnexpected symbol"};
    }
    return result;
}

inline double eval(const calc_node& n) {
    struct eval_visitor {
        auto operator()(const double value) { return value; };
        auto operator()(const binary_op<'+'>& value) {
            return eval(*value.left_expr) + eval(*value.right_expr);
        };
        auto operator()(const binary_op<'-'>& value) {
            return eval(*value.left_expr) - eval(*value.right_expr);
        };
        auto operator()(const binary_op<'*'>& value) {
            return eval(*value.left_expr) * eval(*value.right_expr);
        };
        auto operator()(const binary_op<'/'>& value) {
            return eval(*value.left_expr) / eval(*value.right_expr);
        };
    };
    return Visit(eval_visitor{}, n);
}

}  // namespace static_evaluator
