#pragma once

#include "util/algo.h"
#include "variant/variant.h"

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
calc_node parse(const std::string& input);

std::string print(const calc_node& n, const int indent = 0);

namespace detail {

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

}  // namespace detail

inline std::string print(const calc_node& n, const int indent) {
    auto vis = detail::print_visitor{indent};
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
            return base::binpow(
                eval(*value.left_expr),
                static_cast<std::int64_t>(eval(*value.right_expr)));
        };
    };
    return Visit(eval_visitor{}, n);
}

}  // namespace static_evaluator
