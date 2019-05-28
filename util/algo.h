#pragma once

#include <type_traits>
#include <functional>

namespace base {

template <class T, class U, class BinaryOp = std::multiplies<T>>
std::enable_if_t<std::is_integral<U>::value && std::is_arithmetic<T>::value, T>
binpow(const T& value, const U degree, BinaryOp bin_op = {}) {
    if (degree < 0) {
        return T{1} / binpow(value, -degree);
    }
    if (0 == degree) {
        return T{1};
    }
    const auto result = binpow(bin_op(value, value), degree >> 1);
    if (degree & 1) {
        return bin_op(result, value);
    }
    return result;
}

}  // namespace base
