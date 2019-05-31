#include "evaluator.h"

#include "parser/parser.h"

#include <iostream>

namespace evaler {

calc_node e_nonterm(prs::input_data& data);
calc_node t_nonterm(prs::input_data& data);
calc_node s_nonterm(prs::input_data& data);
calc_node f_nonterm(prs::input_data& data);
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

    return n_nonterm(data);
}

calc_node n_nonterm(prs::input_data& data) {
    bool is_negative = false;
    if ('+' == peek(data)) {
        data >>= prs::next<'+'>();
    } else if ('-' == peek(data)) {
        is_negative = true;
        data >>= prs::next<'-'>();
    }

    const auto real_part = [&data] {
        if (peek(data) != '.') {
            return 0.0;
        }
        data >>= prs::next<'.'>();
        if (!std::isdigit(peek(data))) {
            throw std::runtime_error{prs::make_fancy_error_log(data) +
                                     "\nDigit expected"};
        }
        double result = 0.0;
        double current_pos = 0.1;
        while (std::isdigit(peek(data))) {
            const double digit = peek(data) - '0';
            result += current_pos * digit;
            current_pos *= 0.1;
            data >>= prs::next_digit();
        }
        return result;
    };

    double result = 0.0;
    if ('0' == peek(data)) {
        data >>= prs::next<'0'>();
        result = real_part();
    } else if (std::isdigit(peek(data))) {
        while (std::isdigit(peek(data))) {
            const double digit = peek(data) - '0';
            result *= 10.0;
            result += digit;
            data >>= prs::next_digit();
        }
        result += real_part();
    } else {
        throw std::runtime_error{prs::make_fancy_error_log(data) +
                                 "\nDigit expected"};
    }
    data >>= prs::skip_spaces();
    return (is_negative ? -result : result);
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

}  // namespace evaler
