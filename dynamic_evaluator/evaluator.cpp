#include "evaluator.h"

#include "parser/parser.h"

namespace dynamic_evaluator {

std::unique_ptr<calc_node> e_nonterm(prs::input_data& input);
std::unique_ptr<calc_node> t_nonterm(prs::input_data& input);
std::unique_ptr<calc_node> s_nonterm(prs::input_data& input);
std::unique_ptr<calc_node> f_nonterm(prs::input_data& input);
std::unique_ptr<calc_node> p_nonterm(prs::input_data& input);
std::unique_ptr<calc_node> n_nonterm(prs::input_data& input);

std::unique_ptr<calc_node> e_nonterm(prs::input_data& input) {
    auto result = t_nonterm(input);
    input = prs::skip_spaces(std::move(input));
    while ('+' == prs::peek(input) || '-' == prs::peek(input)) {
        switch (prs::peek(input)) {
            case '+':
                input = prs::skip_spaces(prs::next(std::move(input), '+'));
                result = std::make_unique<sum_node>(std::move(result),
                                                    t_nonterm(input));
                input = prs::skip_spaces(std::move(input));
                break;
            case '-':
                input = prs::skip_spaces(prs::next(std::move(input), '-'));
                result = std::make_unique<sub_node>(std::move(result),
                                                    t_nonterm(input));
                input = prs::skip_spaces(std::move(input));
                break;
        }
    }
    return result;
}

std::unique_ptr<calc_node> t_nonterm(prs::input_data& input) {
    auto result = s_nonterm(input);
    input = prs::skip_spaces(std::move(input));
    while ('*' == prs::peek(input) || '/' == prs::peek(input)) {
        switch (prs::peek(input)) {
            case '*':
                input = prs::skip_spaces(prs::next(std::move(input), '*'));
                result = std::make_unique<mul_node>(std::move(result),
                                                    s_nonterm(input));
                input = prs::skip_spaces(std::move(input));
                break;
            case '/':
                input = prs::skip_spaces(prs::next(std::move(input), '/'));
                result = std::make_unique<div_node>(std::move(result),
                                                    s_nonterm(input));
                input = prs::skip_spaces(std::move(input));
                break;
        }
    }
    return result;
}

std::unique_ptr<calc_node> s_nonterm(prs::input_data& input) {
    auto result = f_nonterm(input);
    input = prs::skip_spaces(std::move(input));
    if ('*' == prs::peek(input)) {
        input = prs::next(std::move(input), '*');
        if ('*' != prs::peek(input)) {
            input = prs::unnext(std::move(input));
            return result;
        }
        input = prs::skip_spaces(prs::next(std::move(input), '*'));
        return std::make_unique<pow_node>(std::move(result), s_nonterm(input));
    }
    return result;
}

std::unique_ptr<calc_node> f_nonterm(prs::input_data& input) {
    if ('(' == prs::peek(input)) {
        input = prs::skip_spaces(prs::next(std::move(input), '('));
        auto result = e_nonterm(input);
        input = prs::skip_spaces(prs::next(std::move(input), ')'));
        return result;
    }

    if ('-' == prs::peek(input)) {
        input = prs::next(std::move(input), '-');
        return n_nonterm(input);
    }
    return p_nonterm(input);
}

std::unique_ptr<calc_node> p_nonterm(prs::input_data& input) {
    const auto is_digit = [](const char c) { return std::isdigit(c); };
    if (!is_digit(prs::peek(input))) {
        throw std::runtime_error{make_fancy_error_log(input) +
                                 "\nDigit expected"};
    }

    std::int64_t result = 0;
    while (is_digit(prs::peek(input))) {
        const std::int64_t digit = prs::peek(input) - '0';
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
        input = prs::next(std::move(input), is_digit);
    }
    return std::make_unique<value_node>(static_cast<double>(result));
}

std::unique_ptr<calc_node> n_nonterm(prs::input_data& input) {
    const auto is_digit = [](const char c) { return std::isdigit(c); };
    if (!is_digit(prs::peek(input))) {
        throw std::runtime_error{make_fancy_error_log(input) +
                                 "\nDigit expected"};
    }

    std::int64_t result = 0;
    while (is_digit(prs::peek(input))) {
        const std::int64_t digit = prs::peek(input) - '0';
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
        input = prs::next(std::move(input), is_digit);
    }
    return std::make_unique<value_node>(static_cast<double>(result));
}

std::unique_ptr<calc_node> parse(const std::string& input) {
    auto data = prs::skip_spaces(prs::input_data{&input, 0});
    auto result = e_nonterm(data);
    if (data.cursor != input.size()) {
        throw std::runtime_error{prs::make_fancy_error_log(data) +
                                 "\nUnexpected symbol"};
    }
    return result;
}

}  // namespace dynamic_evaluator
