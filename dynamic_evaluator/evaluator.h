#pragma once

#include "variant/variant.h"

#include <algorithm>
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
std::unique_ptr<calc_node> parse(const std::string& input);

}  // namespace dynamic_evaluator
