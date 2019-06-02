#include "evaluator.h"

namespace evaler {

class value_node final : public dynamic_calc_node {
public:
    value_node(const double value) : value_(value) {}

    double eval() override { return value_; }

    std::string print(const int indent = 0) override {
        return std::string(indent, '\t') + std::to_string(value_) + '\n';
    }

private:
    double value_;
};

class binary_node : public dynamic_calc_node {
public:
    binary_node(std::unique_ptr<dynamic_calc_node> left_expr,
                std::unique_ptr<dynamic_calc_node> right_expr)
        : left_expr_(std::move(left_expr)),
          right_expr_(std::move(right_expr)) {}

protected:
    std::unique_ptr<dynamic_calc_node> left_expr_, right_expr_;
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

class pow_node final : public binary_node {
public:
    using binary_node::binary_node;

    double eval() override {
        return std::pow(this->left_expr_->eval(),
                        static_cast<std::int64_t>(this->right_expr_->eval()));
    }

    std::string print(const int indent = 0) override {
        return this->left_expr_->print(indent + 1) + std::string(indent, '\t') +
               "**\n" + this->right_expr_->print(indent + 1);
    }
};

class unary_node : public dynamic_calc_node {
public:
    unary_node(std::unique_ptr<dynamic_calc_node> expr) : expr_(std::move(expr)) {}

protected:
    std::unique_ptr<dynamic_calc_node> expr_;
};

class sin_node final : public unary_node {
public:
    using unary_node::unary_node;

    double eval() override { return std::sin(this->expr_->eval()); }

    std::string print(const int indent = 0) override {
        return std::string(indent, '\t') + "sin()" + '\n' +
               this->expr_->print(indent + 1);
    }
};

class cos_node final : public unary_node {
public:
    using unary_node::unary_node;

    double eval() override { return std::cos(this->expr_->eval()); }

    std::string print(const int indent = 0) override {
        return std::string(indent, '\t') + "cos()" + '\n' +
               this->expr_->print(indent + 1);
    }
};

class log_node final : public unary_node {
public:
    using unary_node::unary_node;

    double eval() override { return std::log(this->expr_->eval()); }

    std::string print(const int indent = 0) override {
        return std::string(indent, '\t') + "log()" + '\n' +
               this->expr_->print(indent + 1);
    }
};

std::unique_ptr<dynamic_calc_node> convert_to_dynamic(const calc_node& node) {
    struct visitor {
        auto operator()(const double value)
            -> std::unique_ptr<dynamic_calc_node> {
            return std::make_unique<value_node>(value);
        };
        auto operator()(const binary_op<'+'>& value)
            -> std::unique_ptr<dynamic_calc_node> {
            return std::make_unique<sum_node>(
                convert_to_dynamic(*value.left_expr),
                convert_to_dynamic(*value.right_expr));
        };
        auto operator()(const binary_op<'-'>& value)
            -> std::unique_ptr<dynamic_calc_node> {
            return std::make_unique<sub_node>(
                convert_to_dynamic(*value.left_expr),
                convert_to_dynamic(*value.right_expr));
        };
        auto operator()(const binary_op<'*'>& value)
            -> std::unique_ptr<dynamic_calc_node> {
            return std::make_unique<mul_node>(
                convert_to_dynamic(*value.left_expr),
                convert_to_dynamic(*value.right_expr));
        };
        auto operator()(const binary_op<'/'>& value)
            -> std::unique_ptr<dynamic_calc_node> {
            return std::make_unique<div_node>(
                convert_to_dynamic(*value.left_expr),
                convert_to_dynamic(*value.right_expr));
        };
        auto operator()(const binary_op<'*', '*'>& value)
            -> std::unique_ptr<dynamic_calc_node> {
            return std::make_unique<pow_node>(
                convert_to_dynamic(*value.left_expr),
                convert_to_dynamic(*value.right_expr));
        };
        auto operator()(const single_arg_func_op<math_func::sin>& value)
            -> std::unique_ptr<dynamic_calc_node> {
            return std::make_unique<sin_node>(convert_to_dynamic(*value.expr));
        }
        auto operator()(const single_arg_func_op<math_func::cos>& value)
            -> std::unique_ptr<dynamic_calc_node> {
            return std::make_unique<cos_node>(convert_to_dynamic(*value.expr));
        }
        auto operator()(const single_arg_func_op<math_func::log>& value)
            -> std::unique_ptr<dynamic_calc_node> {
            return std::make_unique<log_node>(convert_to_dynamic(*value.expr));
        }
    };
    return base::visit(visitor{}, node);
}

}  // namespace evaler
