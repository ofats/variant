#pragma once

#include "type_traits.h"

#include <exception>

template <class... Ts>
class TVariant;

class TBadVariantAccess : public std::exception {};

namespace NPrivate {

template <std::size_t I, class V>
struct TAlternative;

template <std::size_t I, class... Ts>
struct TAlternative<I, TVariant<Ts...>> {
    using type = TTypePackElementT<I, Ts...>;
};

template <std::size_t I, class V>
using TAlternativeType = typename TAlternative<I, V>::type;

template <class V>
struct TSize;

template <class... Ts>
struct TSize<TVariant<Ts...>> : std::integral_constant<std::size_t, sizeof...(Ts)> {};

struct TVariantAccessor {
    template <std::size_t I, class... Ts>
    static TAlternativeType<I, TVariant<Ts...>>& Get(TVariant<Ts...>& v);

    template <std::size_t I, class... Ts>
    static const TAlternativeType<I, TVariant<Ts...>>& Get(const TVariant<Ts...>& v);

    template <std::size_t I, class... Ts>
    static TAlternativeType<I, TVariant<Ts...>>&& Get(TVariant<Ts...>&& v);

    template <std::size_t I, class... Ts>
    static const TAlternativeType<I, TVariant<Ts...>>&& Get(const TVariant<Ts...>&& v);

    template <class... Ts>
    static constexpr std::size_t Index(const TVariant<Ts...>& v) noexcept;
};

constexpr std::size_t T_NPOS = -1;

template <class X, class... Ts>
constexpr std::size_t IndexOfImpl() {
    bool bs[] = {std::is_same<X, Ts>::value...};
    for (std::size_t i = 0; i < sizeof...(Ts); ++i) {
        if (bs[i]) {
            return i;
        }
    }
    return T_NPOS;
}

template <class X, class... Ts>
struct TIndexOf : std::integral_constant<std::size_t, IndexOfImpl<X, Ts...>()> {};

template <class X, class V>
struct TAlternativeIndex;

template <class X, class... Ts>
struct TAlternativeIndex<X, TVariant<Ts...>> : TIndexOf<X, Ts...> {};

template <class... Ts>
struct TTypeTraits {
    using TNoRefs = TConjunction<TNegation<std::is_reference<Ts>>...>;
    using TNoVoids = TConjunction<TNegation<std::is_same<Ts, void>>...>;
    using TNoArrays = TConjunction<TNegation<std::is_array<Ts>>...>;
    using TNotEmpty = std::integral_constant<bool, (sizeof...(Ts) > 0)>;
};

template <class FRef, class VRef, std::size_t I = 0>
using TReturnType = decltype(
    std::declval<FRef>()(TVariantAccessor::Get<I>(std::declval<VRef>())));

template <class FRef, class VRef, std::size_t... Is>
constexpr bool CheckReturnTypes(std::index_sequence<Is...>) {
    using R = TReturnType<FRef, VRef>;
    return TConjunction<std::is_same<R, TReturnType<FRef, VRef, Is>>...>::value;
}

template <class ReturnType, std::size_t I, class FRef, class VRef>
ReturnType VisitImplImpl(FRef f, VRef v) {
    return std::forward<FRef>(f)(TVariantAccessor::Get<I>(std::forward<VRef>(v)));
}

template <class ReturnType, class FRef, class VRef>
ReturnType VisitImplFail(FRef, VRef) {
    throw TBadVariantAccess{};
}

template <class F, class V, std::size_t... Is>
decltype(auto) VisitImpl(F&& f, V&& v, std::index_sequence<Is...>) {
    using FRef = decltype(std::forward<F>(f));
    using VRef = decltype(std::forward<V>(v));
    using ReturnType = TReturnType<FRef, VRef>;
    using LambdaType = ReturnType (*)(FRef, VRef);
    static constexpr LambdaType handlers[] = {
        VisitImplImpl<ReturnType, Is, FRef, VRef>...,
        VisitImplFail<ReturnType, FRef, VRef>};
    return handlers[TVariantAccessor::Index(v)](std::forward<F>(f), std::forward<V>(v));
}

template <class F, class V>
void VisitWrapForVoid(F&& f, V&& v, std::true_type) {
    // We need to make special wrapper when return type equals void
    auto l = [&](auto&& x) {
        std::forward<F>(f)(std::forward<decltype(x)>(x));
        return 0;
    };
    VisitImpl(l, std::forward<V>(v), std::make_index_sequence<TSize<std::decay_t<V>>::value>{});
}

template <class F, class V>
decltype(auto) VisitWrapForVoid(F&& f, V&& v, std::false_type) {
    return VisitImpl(std::forward<F>(f),
                     std::forward<V>(v),
                     std::make_index_sequence<TSize<std::decay_t<V>>::value>{});
}

// Can be simplified with c++17: IGNIETFERRO-982
template <class Ret, class F, class T, class U>
std::enable_if_t<std::is_same<std::decay_t<T>, std::decay_t<U>>::value,
Ret> CallIfSame(F f, T&& a, U&& b) {
    return f(std::forward<T>(a), std::forward<U>(b));
}

template <class Ret, class F, class T, class U>
std::enable_if_t<!std::is_same<std::decay_t<T>, std::decay_t<U>>::value,
Ret> CallIfSame(F, T&&, U&&) { // Will never be called
    std::terminate();
}

}  // namespace NPrivate
