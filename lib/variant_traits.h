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

template <class F, class... Vs>
using TReturnType = decltype(
    std::declval<F>()(TVariantAccessor::Get<0>(std::declval<Vs>())...));

template <class ReturnType, class FRef, class... VRefs, std::size_t... ids>
ReturnType UnwrapIndexes(FRef f, VRefs... vs, std::index_sequence<ids...>) {
    return std::forward<FRef>(f)(
        TVariantAccessor::Get<ids>(std::forward<VRefs>(vs))...);
}

template <class ReturnType, class Indexes, class FRef, class... VRefs>
ReturnType VisitImplImpl(FRef f, VRefs... vs) {
    return UnwrapIndexes<ReturnType, FRef, VRefs...>(
        std::forward<FRef>(f), std::forward<VRefs>(vs)..., Indexes{});
}

template <class... Ts>
struct TTypePack {};

template <std::size_t... szs, class... Ids>
constexpr std::size_t EvalMatrixIndex(std::index_sequence<szs...>, Ids... ids) {
    constexpr std::size_t n = sizeof...(ids);
    constexpr std::size_t sizes[] = {szs...};
    const std::size_t indexes[] = {(std::size_t)ids...};
    std::size_t result = 0;
    for (std::size_t i = 0; i < n; ++i) {
        result *= sizes[n - i - 1];
        result += indexes[n - i - 1];
    }
    return result;
}

static_assert(EvalMatrixIndex(std::index_sequence<3, 3>{}, 0, 0) == 0, "");
static_assert(EvalMatrixIndex(std::index_sequence<3, 3>{}, 1, 0) == 1, "");
static_assert(EvalMatrixIndex(std::index_sequence<3, 3>{}, 1, 2) == 7, "");
static_assert(EvalMatrixIndex(std::index_sequence<3, 3, 3>{}, 1, 2, 1) == 16,
              "");

template <class F, class... Vs, class... IndexPacks>
decltype(auto) VisitImpl(F&& f, TTypePack<IndexPacks...>, Vs&&... vs) {
    using ReturnType = TReturnType<F&&, Vs&&...>;
    using LambdaType = ReturnType (*)(F&&, Vs&&...);
    constexpr LambdaType handlers[] = {
        VisitImplImpl<ReturnType, IndexPacks, F&&, Vs&&...>...};
    const std::size_t idx = EvalMatrixIndex(
        std::index_sequence<TSize<std::decay_t<Vs>>::value...>{},
        TVariantAccessor::Index(vs)...);
    return handlers[idx](std::forward<F>(f), std::forward<Vs>(vs)...);
}

template <class ReturnType, class F, class T>
ReturnType CallIfSame(F&& f, T&& a, T&& b) {
    return std::forward<F>(f)(std::forward<T>(a), std::forward<T>(b));
}

template <class ReturnType, class F, class T, class U>
ReturnType CallIfSame(F&&, T&&, U&&) { // Will never be called
    std::terminate();
}

template <std::size_t... szs>
constexpr std::size_t EvalMatrixSize() {
    const std::size_t sizes[] = {szs...};
    std::size_t result = 1;
    for (const std::size_t s : sizes) {
        result *= s;
    }
    return result;
}

template <>
constexpr std::size_t EvalMatrixSize() {
    return 0;
}

static_assert(EvalMatrixSize<3, 3, 3>() == 27, "");
static_assert(EvalMatrixSize<3>() == 3, "");
static_assert(EvalMatrixSize<0>() == 0, "");
static_assert(EvalMatrixSize<>() == 0, "");

template <std::size_t, std::size_t... acc>
constexpr auto EvalMatrixIndexes(std::index_sequence<>,
                                 std::index_sequence<acc...> result = {}) {
    return result;
}

template <std::size_t index, std::size_t size, std::size_t... sizes,
          std::size_t... acc>
constexpr auto EvalMatrixIndexes(std::index_sequence<size, sizes...>,
                                 std::index_sequence<acc...> = {}) {
    return EvalMatrixIndexes<index / size>(
        std::index_sequence<sizes...>{},
        std::index_sequence<acc..., index % size>{});
}

static_assert(
    std::is_same<decltype(EvalMatrixIndexes<0>(std::index_sequence<3, 3, 3>{})),
                 std::index_sequence<0, 0, 0>>::value,
    "");

static_assert(std::is_same<decltype(EvalMatrixIndexes<26>(
                               std::index_sequence<3, 3, 3>{})),
                           std::index_sequence<2, 2, 2>>::value,
              "");

template <std::size_t... indexes, class Sizes>
constexpr auto EvalMatrixIndexesFor(std::index_sequence<indexes...>,
                                    Sizes sizes) {
    return TTypePack<decltype(EvalMatrixIndexes<indexes>(sizes))...>{};
}

// 3x3 matrix test.
static_assert(
    std::is_same<decltype(EvalMatrixIndexesFor(std::make_index_sequence<9>{},
                                               std::index_sequence<3, 3>{})),
                 TTypePack<std::index_sequence<0, 0>, std::index_sequence<1, 0>,
                           std::index_sequence<2, 0>, std::index_sequence<0, 1>,
                           std::index_sequence<1, 1>, std::index_sequence<2, 1>,
                           std::index_sequence<0, 2>, std::index_sequence<1, 2>,
                           std::index_sequence<2, 2>>>::value,
    "");

// 3x3x3 matrix test.
static_assert(
    std::is_same<
        decltype(EvalMatrixIndexesFor(std::make_index_sequence<27>{},
                                      std::index_sequence<3, 3, 3>{})),
        TTypePack<std::index_sequence<0, 0, 0>, std::index_sequence<1, 0, 0>,
                  std::index_sequence<2, 0, 0>, std::index_sequence<0, 1, 0>,
                  std::index_sequence<1, 1, 0>, std::index_sequence<2, 1, 0>,
                  std::index_sequence<0, 2, 0>, std::index_sequence<1, 2, 0>,
                  std::index_sequence<2, 2, 0>, std::index_sequence<0, 0, 1>,
                  std::index_sequence<1, 0, 1>, std::index_sequence<2, 0, 1>,
                  std::index_sequence<0, 1, 1>, std::index_sequence<1, 1, 1>,
                  std::index_sequence<2, 1, 1>, std::index_sequence<0, 2, 1>,
                  std::index_sequence<1, 2, 1>, std::index_sequence<2, 2, 1>,
                  std::index_sequence<0, 0, 2>, std::index_sequence<1, 0, 2>,
                  std::index_sequence<2, 0, 2>, std::index_sequence<0, 1, 2>,
                  std::index_sequence<1, 1, 2>, std::index_sequence<2, 1, 2>,
                  std::index_sequence<0, 2, 2>, std::index_sequence<1, 2, 2>,
                  std::index_sequence<2, 2, 2>>>::value,
    "");

}  // namespace NPrivate
