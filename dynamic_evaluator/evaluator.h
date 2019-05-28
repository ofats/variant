#pragma once

#include "variant/variant.h"

#include <algorithm>
#include <cassert>
#include <memory>

namespace dynamic_evaluator {

class calc_node {
public:
    virtual ~calc_node() = default;
    virtual double eval() = 0;
    virtual std::string print(const int indent = 0) = 0;
};

class value_node final : public calc_node {
public:
    value_node(const double value) : value_(value) {}

    double eval() override {
        return value_;
    }

    std::string print(const int indent = 0) override {
        return std::string(indent, '\t') + std::to_string(value_) + '\n';
    }

private:
    double value_;
};

class binary_node : public calc_node {
public:
    binary_node(std::unique_ptr<calc_node> left_expr,
                std::unique_ptr<calc_node> right_expr)
        : left_expr_(std::move(left_expr)),
          right_expr_(std::move(right_expr)) {}

protected:
    std::unique_ptr<calc_node> left_expr_, right_expr_;
};

class sum_node final : public binary_node {
public:
    using binary_node::binary_node;

    double eval() override {
        return this->left_expr_->eval() + this->right_expr_->eval();
    }

    std::string print(const int indent = 0) override {
        return this->left_expr_->print(indent + 1) + std::string(indent, '\t') +
               "+\n" + this->right_expr_->print(indent + 1);
    }
};

class sub_node final : public binary_node {
public:
    using binary_node::binary_node;

    double eval() override {
        return this->left_expr_->eval() - this->right_expr_->eval();
    }

    std::string print(const int indent = 0) override {
        return this->left_expr_->print(indent + 1) + std::string(indent, '\t') +
               "-\n" + this->right_expr_->print(indent + 1);
    }
};

class mul_node final : public binary_node {
public:
    using binary_node::binary_node;

    double eval() override {
        return this->left_expr_->eval() * this->right_expr_->eval();
    }

    std::string print(const int indent = 0) override {
        return this->left_expr_->print(indent + 1) + std::string(indent, '\t') +
               "*\n" + this->right_expr_->print(indent + 1);
    }
};

class div_node final : public binary_node {
public:
    using binary_node::binary_node;

    double eval() override {
        return this->left_expr_->eval() / this->right_expr_->eval();
    }

    std::string print(const int indent = 0) override {
        return this->left_expr_->print(indent + 1) + std::string(indent, '\t') +
               "/\n" + this->right_expr_->print(indent + 1);
    }
};

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

class pow_node final : public binary_node {
public:
    using binary_node::binary_node;

    double eval() override {
        return binpow(this->left_expr_->eval(), this->right_expr_->eval());
    }

    std::string print(const int indent = 0) override {
        return this->left_expr_->print(indent + 1) + std::string(indent, '\t') +
               "**\n" + this->right_expr_->print(indent + 1);
    }
};

// `E` -> `E` + `T` | `E` - `T` | `T`
// `T` -> `T` * `S` | `T` / `S` | `F`
// `S` -> `F` ** `S` | `F`
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

std::unique_ptr<calc_node> e_nonterm(input_data& input);
std::unique_ptr<calc_node> t_nonterm(input_data& input);
std::unique_ptr<calc_node> s_nonterm(input_data& input);
std::unique_ptr<calc_node> f_nonterm(input_data& input);
std::unique_ptr<calc_node> p_nonterm(input_data& input);
std::unique_ptr<calc_node> n_nonterm(input_data& input);

}  // namespace detail

inline std::unique_ptr<calc_node> parse(const std::string& input) {
    auto data = detail::skip_spaces(detail::input_data{&input, 0});
    auto result = detail::e_nonterm(data);
    if (data.cursor != input.size()) {
        throw std::runtime_error{detail::make_fancy_error_log(data) +
                                 "\nUnexpected symbol"};
    }
    return result;
}

}  // namespace dynamic_evaluator
