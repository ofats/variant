#pragma once

#include "variant/variant.h"

#include <cmath>
#include <memory>

namespace evaler {

// -------------------- NODES --------------------

// Enumeration describing math functions.
enum class math_func { sin, cos, log };

template <math_func>
struct unary_op;

// Node for representing binary operators, i.e `+`, `-`, etc.
template <char... signs>
struct binary_op;

// Base node for calculation tree representation.
using calc_node =
    base::variant<double, binary_op<'+'>, binary_op<'-'>, binary_op<'*'>,
                  binary_op<'/'>, binary_op<'*', '*'>, unary_op<math_func::sin>,
                  unary_op<math_func::cos>, unary_op<math_func::log>>;

template <math_func>
struct unary_op {
    unary_op(calc_node&& arg);

    std::unique_ptr<calc_node> expr;
};

namespace detail {

struct binary_op_impl;

}  // namespace detail

template <char... signs>
struct binary_op {
    binary_op(calc_node&& a, calc_node&& b);

    std::unique_ptr<detail::binary_op_impl> impl;
};

namespace detail {

struct binary_op_impl {
    binary_op_impl(calc_node&& a, calc_node&& b)
        : left(std::move(a)), right(std::move(b)) {}

    calc_node left, right;
};

}  // namespace detail

template <math_func func>
unary_op<func>::unary_op(calc_node&& arg)
    : expr(std::make_unique<calc_node>(std::move(arg))) {}

template <char... signs>
binary_op<signs...>::binary_op(calc_node&& a, calc_node&& b)
    : impl(std::make_unique<detail::binary_op_impl>(std::move(a),
                                                    std::move(b))) {}

// -------------------- PARSING --------------------

// Symbols '`', '|' are reserved
//
// `E` -> `E` + `T` | `E` - `T` | `T`
// `T` -> `T` * `S` | `T` / `S` | `F`
// `S` -> `F` ** `S` | `F`
// `F` -> ( `E` ) | sin( `E` ) | cos( `E` ) | log( `E` ) | `N`
// `N` -> `(0|[+-]?[1-9][0-9]*)(\.[0-9]+)?`
calc_node parse(const std::string& input);

// -------------------- PRINTING --------------------

// Prints calculation tree in human readable form.
// `indent` equals number of tablulations before every line the output.
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
        return print(value.impl->left, indent + 1) + std::string(indent, '\t') +
               std::string(1, sign) + '\n' +
               print(value.impl->right, indent + 1);
    }
    auto operator()(const binary_op<'*', '*'>& value) {
        return print(value.impl->left, indent + 1) + std::string(indent, '\t') +
               "**" + '\n' + print(value.impl->right, indent + 1);
    }
    auto operator()(const unary_op<math_func::sin>& value) {
        return std::string(indent, '\t') + "sin()" + '\n' +
               print(*value.expr, indent + 1);
    }
    auto operator()(const unary_op<math_func::cos>& value) {
        return std::string(indent, '\t') + "cos()" + '\n' +
               print(*value.expr, indent + 1);
    }
    auto operator()(const unary_op<math_func::log>& value) {
        return std::string(indent, '\t') + "log()" + '\n' +
               print(*value.expr, indent + 1);
    }
};

}  // namespace detail

inline std::string print(const calc_node& n, const int indent) {
    auto vis = detail::print_visitor{indent};
    return base::visit(vis, n);
}

// -------------------- EVALUATION --------------------

// Goes through the calculation tree and returns the result.
inline double eval(const calc_node& n) {
    struct visitor {
        auto operator()(const double value) { return value; };
        auto operator()(const binary_op<'+'>& value) {
            return eval(value.impl->left) + eval(value.impl->right);
        };
        auto operator()(const binary_op<'-'>& value) {
            return eval(value.impl->left) - eval(value.impl->right);
        };
        auto operator()(const binary_op<'*'>& value) {
            return eval(value.impl->left) * eval(value.impl->right);
        };
        auto operator()(const binary_op<'/'>& value) {
            return eval(value.impl->left) / eval(value.impl->right);
        };
        auto operator()(const binary_op<'*', '*'>& value) {
            return std::pow(eval(value.impl->left), eval(value.impl->right));
        };
        auto operator()(const unary_op<math_func::sin>& value) {
            return std::sin(eval(*value.expr));
        }
        auto operator()(const unary_op<math_func::cos>& value) {
            return std::cos(eval(*value.expr));
        }
        auto operator()(const unary_op<math_func::log>& value) {
            return std::log(eval(*value.expr));
        }
    };
    return base::visit(visitor{}, n);
}

// -------------------- DYNAMIC PART --------------------

// Abstract class that provides an interface for working with calculation tree.
// Alternative to `calc_node`.
class dynamic_calc_node {
public:
    virtual ~dynamic_calc_node() = default;
    virtual double eval() = 0;
    virtual std::string print(const int indent = 0) = 0;
};

// Converts `calc_node` to `dynamic_calc_node`, i.e creates dynamic
// representation of the calculation tree.
std::unique_ptr<dynamic_calc_node> convert_to_dynamic(const calc_node& node);

}  // namespace evaler
