#pragma once

#include "parser/parser.h"
#include "variant/variant.h"

#include <algorithm>
#include <cassert>
#include <memory>

namespace static_evaluator {

template <char... signs>
struct binary_op;

using calc_node = TVariant<double, binary_op<'+'>, binary_op<'-'>,
                           binary_op<'*'>, binary_op<'/'>, binary_op<'*', '*'>>;

template <char... signs>
struct binary_op {
    binary_op(std::unique_ptr<calc_node> a, std::unique_ptr<calc_node> b)
        : left_expr(std::move(a)), right_expr(std::move(b)) {}
    binary_op(binary_op&&) noexcept = default;
    binary_op& operator=(binary_op&&) noexcept = default;

    std::unique_ptr<calc_node> left_expr, right_expr;
};

// `E` -> `E` + `T` | `E` - `T` | `T`
// `T` -> `T` * `S` | `T` / `S` | `F`
// `S` -> `F` ** `S` | `F`
// `F` -> `P` | - 'N' | ( `E` )

namespace detail {

calc_node e_nonterm(prs::input_data& input);
calc_node t_nonterm(prs::input_data& input);
calc_node s_nonterm(prs::input_data& input);
calc_node f_nonterm(prs::input_data& input);
calc_node p_nonterm(prs::input_data& input);
calc_node n_nonterm(prs::input_data& input);

inline double binpow(const double num, const std::int64_t st) {
    if (st < 0) {
        return 1.0 / binpow(num, -st);
    }
    if (0 == st) {
        return 1.0;
    }
    double result = binpow(num * num, st >> 1);
    if (st & 1) {
        result *= num;
    }
    return result;
}

}  // namespace detail

inline calc_node parse(const std::string& input) {
    auto data = prs::skip_spaces(prs::input_data{&input, 0});
    auto result = detail::e_nonterm(data);
    if (data.cursor != input.size()) {
        throw std::runtime_error{prs::make_fancy_error_log(data) +
                                 "\nUnexpected symbol"};
    }
    return result;
}

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
    auto operator()(const binary_op<'*', '*'>& value) {
        return print(*value.left_expr, indent + 1) + std::string(indent, '\t') +
               "**" + '\n' + print(*value.right_expr, indent + 1);
    }
};

inline std::string print(const calc_node& n, const int indent) {
    auto vis = print_visitor{indent};
    return Visit(vis, n);
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
        auto operator()(const binary_op<'*', '*'>& value) {
            return detail::binpow(eval(*value.left_expr),
                                  eval(*value.right_expr));
        };
    };
    return Visit(eval_visitor{}, n);
}

}  // namespace static_evaluator
