#pragma once

#include "variant/variant.h"

#include <cmath>
#include <memory>

namespace evaler {

// -------------------- NODES --------------------

// Enumeration describing math functions.
enum class math_func { sin, cos, log };

template <math_func>
struct single_arg_func_op;

// Node for representing binary operators, i.e `+`, `-`, etc.
template <char... signs>
struct binary_op;

// Base node for calculation tree representation.
using calc_node = TVariant<
    double, binary_op<'+'>, binary_op<'-'>, binary_op<'*'>, binary_op<'/'>,
    binary_op<'*', '*'>, single_arg_func_op<math_func::sin>,
    single_arg_func_op<math_func::cos>, single_arg_func_op<math_func::log>>;

template <math_func>
struct single_arg_func_op {
    single_arg_func_op(std::unique_ptr<calc_node> arg) : expr(std::move(arg)) {}
    single_arg_func_op(single_arg_func_op&&) = default;
    single_arg_func_op& operator=(single_arg_func_op&&) = default;

    std::unique_ptr<calc_node> expr;
};

template <char... signs>
struct binary_op {
    binary_op(std::unique_ptr<calc_node> a, std::unique_ptr<calc_node> b)
        : left_expr(std::move(a)), right_expr(std::move(b)) {}
    binary_op(binary_op&&) = default;
    binary_op& operator=(binary_op&&) = default;

    std::unique_ptr<calc_node> left_expr, right_expr;
};

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
        return print(*value.left_expr, indent + 1) + std::string(indent, '\t') +
               std::string(1, sign) + '\n' +
               print(*value.right_expr, indent + 1);
    }
    auto operator()(const binary_op<'*', '*'>& value) {
        return print(*value.left_expr, indent + 1) + std::string(indent, '\t') +
               "**" + '\n' + print(*value.right_expr, indent + 1);
    }
    auto operator()(const single_arg_func_op<math_func::sin>& value) {
        return std::string(indent, '\t') + "sin()" + '\n' +
               print(*value.expr, indent + 1);
    }
    auto operator()(const single_arg_func_op<math_func::cos>& value) {
        return std::string(indent, '\t') + "cos()" + '\n' +
               print(*value.expr, indent + 1);
    }
    auto operator()(const single_arg_func_op<math_func::log>& value) {
        return std::string(indent, '\t') + "log()" + '\n' +
               print(*value.expr, indent + 1);
    }
};

}  // namespace detail

inline std::string print(const calc_node& n, const int indent) {
    auto vis = detail::print_visitor{indent};
    return Visit(vis, n);
}

// -------------------- EVALUATION --------------------

// Goes through the calculation tree and returns the result.
inline double eval(const calc_node& n) {
    struct visitor {
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
            return std::pow(eval(*value.left_expr), eval(*value.right_expr));
        };
        auto operator()(const single_arg_func_op<math_func::sin>& value) {
            return std::sin(eval(*value.expr));
        }
        auto operator()(const single_arg_func_op<math_func::cos>& value) {
            return std::cos(eval(*value.expr));
        }
        auto operator()(const single_arg_func_op<math_func::log>& value) {
            return std::log(eval(*value.expr));
        }
    };
    return Visit(visitor{}, n);
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
