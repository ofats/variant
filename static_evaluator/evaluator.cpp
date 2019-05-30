#include "evaluator.h"

#include "parser/parser.h"

namespace static_evaluator {

calc_node e_nonterm(prs::input_data& data);
calc_node t_nonterm(prs::input_data& data);
calc_node s_nonterm(prs::input_data& data);
calc_node f_nonterm(prs::input_data& data);
calc_node p_nonterm(prs::input_data& data);
calc_node n_nonterm(prs::input_data& data);

calc_node e_nonterm(prs::input_data& data) {
    auto result = t_nonterm(data);
    data >>= prs::skip_spaces();
    while ('+' == peek(data) || '-' == peek(data)) {
        switch (peek(data)) {
            case '+':
                data >>= prs::next<'+'>() >> prs::skip_spaces();
                result.emplace<binary_op<'+'>>(
                    std::make_unique<calc_node>(std::move(result)),
                    std::make_unique<calc_node>(t_nonterm(data)));
                data >>= prs::skip_spaces();
                break;
            case '-':
                data >>= prs::next<'-'>() >> prs::skip_spaces();

                result.emplace<binary_op<'-'>>(
                    std::make_unique<calc_node>(std::move(result)),
                    std::make_unique<calc_node>(t_nonterm(data)));
                data >>= prs::skip_spaces();
                break;
        }
    }
    return result;
}

calc_node t_nonterm(prs::input_data& data) {
    auto result = s_nonterm(data);
    data >>= prs::skip_spaces();
    while ('*' == peek(data) || '/' == peek(data)) {
        switch (peek(data)) {
            case '*':
                data >>= prs::next<'*'>() >> prs::skip_spaces();
                result.emplace<binary_op<'*'>>(
                    std::make_unique<calc_node>(std::move(result)),
                    std::make_unique<calc_node>(s_nonterm(data)));
                data >>= prs::skip_spaces();
                break;
            case '/':
                data >>= prs::next<'/'>() >> prs::skip_spaces();
                result.emplace<binary_op<'/'>>(
                    std::make_unique<calc_node>(std::move(result)),
                    std::make_unique<calc_node>(s_nonterm(data)));
                data >>= prs::skip_spaces();
                break;
        }
    }
    return result;
}

calc_node s_nonterm(prs::input_data& data) {
    auto result = f_nonterm(data);
    data >>= prs::skip_spaces();
    if ('*' == peek(data)) {
        data >>= prs::next<'*'>();
        if ('*' != peek(data)) {
            data = prs::unnext(std::move(data));
            return result;
        }
        data >>= prs::next<'*'>() >> prs::skip_spaces();
        return binary_op<'*', '*'>{
            std::make_unique<calc_node>(std::move(result)),
            std::make_unique<calc_node>(s_nonterm(data))};
    }
    return result;
}

calc_node f_nonterm(prs::input_data& data) {
    if ('(' == peek(data)) {
        data >>= prs::next<'('>() >> prs::skip_spaces();
        auto result = e_nonterm(data);
        data >>= prs::next<')'>() >> prs::skip_spaces();
        return result;
    }

    if ('s' == peek(data)) {
        data >>= prs::next("sin(") >> prs::skip_spaces();
        auto result = single_arg_func_op<math_func::sin>(
            std::make_unique<calc_node>(e_nonterm(data)));
        data >>= prs::next<')'>() >> prs::skip_spaces();
        return result;
    }

    if ('c' == peek(data)) {
        data >>= prs::next("cos(") >> prs::skip_spaces();
        auto result = single_arg_func_op<math_func::cos>(
            std::make_unique<calc_node>(e_nonterm(data)));
        data >>= prs::next<')'>() >> prs::skip_spaces();
        return result;
    }

    if ('l' == peek(data)) {
        data >>= prs::next("log(") >> prs::skip_spaces();
        auto result = single_arg_func_op<math_func::log>(
            std::make_unique<calc_node>(e_nonterm(data)));
        data >>= prs::next<')'>() >> prs::skip_spaces();
        return result;
    }

    if ('-' == peek(data)) {
        data >>= prs::next<'-'>();
        return n_nonterm(data);
    }
    return p_nonterm(data);
}

calc_node p_nonterm(prs::input_data& data) {
    const auto is_digit = [](const char c) { return std::isdigit(c); };
    if (!is_digit(peek(data))) {
        throw std::runtime_error{prs::make_fancy_error_log(data) +
                                 "\nDigit expected"};
    }

    std::int64_t result = 0;
    while (is_digit(peek(data))) {
        const std::int64_t digit = peek(data) - '0';
        constexpr auto max_val = std::numeric_limits<std::int64_t>::max();
        if (result > max_val / 10) {
            throw std::runtime_error{prs::make_fancy_error_log(data) +
                                     "\nSigned integer overflow"};
        }
        result *= 10;
        if (result > max_val - digit) {
            throw std::runtime_error{prs::make_fancy_error_log(data) +
                                     "\nSigned integer overflow"};
        }
        result += digit;
        data = prs::next(std::move(data), is_digit);
    }
    return calc_node{static_cast<double>(result)};
}

calc_node n_nonterm(prs::input_data& data) {
    const auto is_digit = [](const char c) { return std::isdigit(c); };
    if (!is_digit(peek(data))) {
        throw std::runtime_error{prs::make_fancy_error_log(data) +
                                 "\nDigit expected"};
    }

    std::int64_t result = 0;
    while (is_digit(peek(data))) {
        const std::int64_t digit = peek(data) - '0';
        constexpr auto min_val = std::numeric_limits<std::int64_t>::min();
        if (result < min_val / 10) {
            throw std::runtime_error{prs::make_fancy_error_log(data) +
                                     "\nSigned integer underflow"};
        }
        result *= 10;
        if (result < min_val + digit) {
            throw std::runtime_error{prs::make_fancy_error_log(data) +
                                     "\nSigned integer underflow"};
        }
        result += digit;
        data = prs::next(std::move(data), is_digit);
    }
    return calc_node{static_cast<double>(result)};
}

calc_node parse(const std::string& input) {
    auto data = prs::input_data{&input, 0} >> prs::skip_spaces();
    auto result = e_nonterm(data);
    if (data.cursor != input.size()) {
        throw std::runtime_error{prs::make_fancy_error_log(data) +
                                 "\nUnexpected symbol"};
    }
    return result;
}

}  // namespace static_evaluator
