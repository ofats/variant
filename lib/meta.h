#pragma once

#include <type_traits>
#include <utility>

namespace meta {

// aka std::void_t
template <class...>
using void_t = void;

template <class...>
struct type_pack {};

namespace detail {

template <class F, class Args, class = void>
struct is_invocable : std::false_type {};

template <class F, class... Args>
struct is_invocable<
    F, type_pack<Args...>,
    void_t<decltype(std::declval<F>()(std::declval<Args>()...))>>
    : std::true_type {};

}  // namespace detail

// aka std::is_invocable
template <class F, class... Args>
using is_invocable = detail::is_invocable<F, type_pack<Args...>>;

// aka std::is_invocable_v
template <class F, class... Args>
constexpr bool is_invocable_v = is_invocable<F, Args...>::value;

namespace detail {

template <class... Bs>
constexpr bool conjunction_impl() {
    bool bs[] = {Bs::value...};
    for (const bool b : bs) {
        if (!b) {
            return false;
        }
    }
    return true;
}

}  // namespace detail

// aka std::conjunction
template <class... Bs>
using conjunction =
    std::integral_constant<bool, detail::conjunction_impl<Bs...>()>;

// aka std::negation
template <class B>
using negation = std::integral_constant<bool, !B::value>;

template <class T>
using subtype = typename T::type;

namespace detail {

template <std::size_t I, class T>
struct indexed_type {
    using type = T;
};

template <class, class... Ts>
struct indexed_types;

template <std::size_t... Is, class... Ts>
struct indexed_types<std::index_sequence<Is...>, Ts...> {
    struct type : indexed_type<Is, Ts>... {};
};

template <class... Ts>
using indexed_types_for =
    typename indexed_types<std::index_sequence_for<Ts...>, Ts...>::type;

template <std::size_t I, class T>
constexpr indexed_type<I, T> get_indexed_type(indexed_type<I, T>);

template <std::size_t I, bool index_in_boundaries, class... Ts>
struct type_pack_element_impl {
    using type = subtype<decltype(
        get_indexed_type<I>(detail::indexed_types_for<Ts...>{}))>;
};

template <std::size_t I, class... Ts>
struct type_pack_element_impl<I, false, Ts...> {};

}  // namespace detail

// Metafunction taking type pack and some index.
// Returns type located on required position in that pack.
template <std::size_t I, class... Ts>
struct type_pack_element
    : detail::type_pack_element_impl<I, (I < sizeof...(Ts)), Ts...> {};

// Shortcut for typename TTypePackElement::type.
template <std::size_t I, class... Ts>
using type_pack_element_t = typename type_pack_element<I, Ts...>::type;

static_assert(
    std::is_same<type_pack_element_t<1, int, char, double>, char>::value, "");

}  // namespace meta
