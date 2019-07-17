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
                data >>= prs::advance() >> prs::skip_spaces();
                result = binary_op<'+'>(std::move(result), t_nonterm(data));
                data >>= prs::skip_spaces();
                break;
            case '-':
                data >>= prs::advance() >> prs::skip_spaces();
                result = binary_op<'-'>(std::move(result), t_nonterm(data));
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
                data >>= prs::advance() >> prs::skip_spaces();
                result = binary_op<'*'>(std::move(result), s_nonterm(data));
                data >>= prs::skip_spaces();
                break;
            case '/':
                data >>= prs::advance() >> prs::skip_spaces();
                result = binary_op<'/'>(std::move(result), s_nonterm(data));
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
        data >>= prs::advance();
        if ('*' != peek(data)) {
            data = prs::unrecv(std::move(data));
            return result;
        }
        data >>= prs::advance() >> prs::skip_spaces();
        return binary_op<'*', '*'>{std::move(result), s_nonterm(data)};
    }
    return result;
}

calc_node f_nonterm(prs::input_data& data) {
    if ('(' == peek(data)) {
        data >>= prs::advance() >> prs::skip_spaces();
        auto result = e_nonterm(data);
        data >>= prs::advance_if<')'>() >> prs::skip_spaces();
        return result;
    }

    if ('s' == peek(data)) {
        data >>= prs::advance() >> prs::advance_if("in(") >> prs::skip_spaces();
        auto result = unary_op<math_func::sin>(e_nonterm(data));
        data >>= prs::advance_if<')'>() >> prs::skip_spaces();
        return result;
    }

    if ('c' == peek(data)) {
        data >>= prs::advance() >> prs::advance_if("os(") >> prs::skip_spaces();
        auto result = unary_op<math_func::cos>(e_nonterm(data));
        data >>= prs::advance_if<')'>() >> prs::skip_spaces();
        return result;
    }

    if ('l' == peek(data)) {
        data >>= prs::advance() >> prs::advance_if("og(") >> prs::skip_spaces();
        auto result = unary_op<math_func::log>(e_nonterm(data));
        data >>= prs::advance_if<')'>() >> prs::skip_spaces();
        return result;
    }

    return n_nonterm(data);
}

calc_node n_nonterm(prs::input_data& data) {
    bool is_negative = false;
    if ('+' == peek(data)) {
        data >>= prs::advance();
    } else if ('-' == peek(data)) {
        is_negative = true;
        data >>= prs::advance();
    }

    const auto real_part = [&data] {
        if (peek(data) != '.') {
            return 0.0;
        }
        data >>= prs::advance();
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
            data >>= prs::advance();
        }
        return result;
    };

    double result = 0.0;
    if ('0' == peek(data)) {
        data >>= prs::advance();
        result = real_part();
    } else if (std::isdigit(peek(data))) {
        while (std::isdigit(peek(data))) {
            const double digit = peek(data) - '0';
            result *= 10.0;
            result += digit;
            data >>= prs::advance();
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
