#include "evaluator.h"

namespace dynamic_evaluator {
namespace detail {

std::unique_ptr<calc_node> e_nonterm(input_data& input) {
    auto result = t_nonterm(input);
    input = skip_spaces(std::move(input));
    while ('+' == peek(input) || '-' == peek(input)) {
        switch (peek(input)) {
            case '+':
                input = skip_spaces(next(std::move(input), '+'));
                result = std::make_unique<sum_node>(std::move(result),
                                                    t_nonterm(input));
                input = skip_spaces(std::move(input));
                break;
            case '-':
                input = skip_spaces(next(std::move(input), '-'));
                result = std::make_unique<sub_node>(std::move(result),
                                                    t_nonterm(input));
                input = skip_spaces(std::move(input));
                break;
        }
    }
    return result;
}

std::unique_ptr<calc_node> t_nonterm(input_data& input) {
    auto result = s_nonterm(input);
    input = skip_spaces(std::move(input));
    while ('*' == peek(input) || '/' == peek(input)) {
        switch (peek(input)) {
            case '*':
                input = skip_spaces(next(std::move(input), '*'));
                result = std::make_unique<mul_node>(std::move(result),
                                                    s_nonterm(input));
                input = skip_spaces(std::move(input));
                break;
            case '/':
                input = skip_spaces(next(std::move(input), '/'));
                result = std::make_unique<div_node>(std::move(result),
                                                    s_nonterm(input));
                input = skip_spaces(std::move(input));
                break;
        }
    }
    return result;
}

std::unique_ptr<calc_node> s_nonterm(input_data& input) {
    auto result = f_nonterm(input);
    input = skip_spaces(std::move(input));
    if ('*' == peek(input)) {
        input = next(std::move(input), '*');
        if ('*' != peek(input)) {
            input = unnext(std::move(input));
            return result;
        }
        input = skip_spaces(next(std::move(input), '*'));
        return std::make_unique<pow_node>(std::move(result), s_nonterm(input));
    }
    return result;
}

std::unique_ptr<calc_node> f_nonterm(input_data& input) {
    if ('(' == peek(input)) {
        input = skip_spaces(next(std::move(input), '('));
        auto result = e_nonterm(input);
        input = skip_spaces(next(std::move(input), ')'));
        return result;
    }

    if ('-' == peek(input)) {
        input = next(std::move(input), '-');
        return n_nonterm(input);
    }
    return p_nonterm(input);
}

std::unique_ptr<calc_node> p_nonterm(input_data& input) {
    const auto is_digit = [](const char c) { return std::isdigit(c); };
    if (!is_digit(peek(input))) {
        throw std::runtime_error{make_fancy_error_log(input) +
                                 "\nDigit expected"};
    }

    std::int64_t result = 0;
    while (is_digit(peek(input))) {
        const std::int64_t digit = peek(input) - '0';
        constexpr auto max_val = std::numeric_limits<std::int64_t>::max();
        if (result > max_val / 10) {
            throw std::runtime_error{make_fancy_error_log(input) +
                                     "\nSigned integer overflow"};
        }
        result *= 10;
        if (result > max_val - digit) {
            throw std::runtime_error{make_fancy_error_log(input) +
                                     "\nSigned integer overflow"};
        }
        result += digit;
        input = next(std::move(input), is_digit);
    }
    return std::make_unique<value_node>(static_cast<double>(result));
}

std::unique_ptr<calc_node> n_nonterm(input_data& input) {
    const auto is_digit = [](const char c) { return std::isdigit(c); };
    if (!is_digit(peek(input))) {
        throw std::runtime_error{make_fancy_error_log(input) +
                                 "\nDigit expected"};
    }

    std::int64_t result = 0;
    while (is_digit(peek(input))) {
        const std::int64_t digit = peek(input) - '0';
        constexpr auto min_val = std::numeric_limits<std::int64_t>::min();
        if (result < min_val / 10) {
            throw std::runtime_error{make_fancy_error_log(input) +
                                     "\nSigned integer underflow"};
        }
        result *= 10;
        if (result < min_val + digit) {
            throw std::runtime_error{make_fancy_error_log(input) +
                                     "\nSigned integer underflow"};
        }
        result += digit;
        input = next(std::move(input), is_digit);
    }
    return std::make_unique<value_node>(static_cast<double>(result));
}

}  // namespace detail
}  // namespace dynamic_evaluator
