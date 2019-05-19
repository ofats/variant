#pragma once

#include <type_traits>
#include <utility>

namespace NPrivate {

template <class... Bs>
constexpr bool ConjunctionImpl() {
    bool bs[] = {Bs::value...};
    for (const bool b : bs) {
        if (!b) {
            return false;
        }
    }
    return true;
}

}  // namespace NPrivate

template <class... Bs>
using TConjunction = std::integral_constant<
    bool, NPrivate::ConjunctionImpl<Bs...>()>;  // aka std::conjunction

template <class B>
using TNegation = std::integral_constant<bool, !B::value>;  // aka std::negation

template <class T>
using TSubtype = typename T::type;

namespace NPrivate {

template <std::size_t I, class T>
struct TIndexedType {
    using type = T;
};

template <class, class... Ts>
struct TIndexedTypes;

template <std::size_t... Is, class... Ts>
struct TIndexedTypes<std::index_sequence<Is...>, Ts...> {
    struct type : TIndexedType<Is, Ts>... {};
};

template <class... Ts>
using TIndexedTypesFor =
    typename TIndexedTypes<std::index_sequence_for<Ts...>, Ts...>::type;

template <std::size_t I, class T>
constexpr TIndexedType<I, T> GetIndexedType(TIndexedType<I, T>);

}  // namespace NPrivate

// Metafunction taking type pack and some index.
// Return type located on required index in that pack.
template <std::size_t I, class... Ts>
struct TTypePackElement {
    using type = TSubtype<decltype(
        NPrivate::GetIndexedType<I>(NPrivate::TIndexedTypesFor<Ts...>{}))>;
};

// Shortcut for typename TTypePackElement::type.
template <std::size_t I, class... Ts>
using TTypePackElementT = typename TTypePackElement<I, Ts...>::type;
