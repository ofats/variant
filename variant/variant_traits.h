#pragma once

#include "matrix_ops.h"

#include <exception>

template <class... Ts>
class TVariant;

class TBadVariantAccess : public std::exception {};

namespace NPrivate {

struct TVariantAccessor {
    template <std::size_t I, class... Ts>
    static base::type_pack_element_t<I, Ts...>& Get(TVariant<Ts...>& v);

    template <std::size_t I, class... Ts>
    static const base::type_pack_element_t<I, Ts...>& Get(
        const TVariant<Ts...>& v);

    template <std::size_t I, class... Ts>
    static base::type_pack_element_t<I, Ts...>&& Get(TVariant<Ts...>&& v);

    template <std::size_t I, class... Ts>
    static const base::type_pack_element_t<I, Ts...>&& Get(
        const TVariant<Ts...>&& v);

    template <class... Ts>
    static constexpr std::size_t Index(const TVariant<Ts...>& v) noexcept;
};

constexpr std::size_t T_NPOS = -1;

template <class X, class... Ts>
constexpr std::size_t IndexOfImpl() {
    bool bs[] = {std::is_same<X, Ts>::value...};
    std::size_t result = T_NPOS;
    std::size_t count = 0;
    for (std::size_t i = 0; i < sizeof...(Ts); ++i) {
        if (bs[i]) {
            ++count;
            if (T_NPOS == result) {
                result = i;
            }
        }
    }
    return count == 1 ? result : T_NPOS;
}

static_assert(IndexOfImpl<int, int, double>() == 0, "");
static_assert(IndexOfImpl<int, double>() == T_NPOS, "");
static_assert(IndexOfImpl<int, int, double, int>() == T_NPOS, "");
static_assert(IndexOfImpl<int>() == T_NPOS, "");

template <class X, class... Ts>
struct TIndexOf : std::integral_constant<std::size_t, IndexOfImpl<X, Ts...>()> {
};

template <class F, class Indexes, class... Vs>
struct TSingleReturnType;

template <class F, std::size_t... indexes, class... Vs>
struct TSingleReturnType<F, std::index_sequence<indexes...>, Vs...>
    : base::invoke_result<F, decltype(TVariantAccessor::Get<indexes>(
                                 std::declval<Vs>()))...> {};

template <class F, class Indexes, class... Vs>
using TSingleReturnTypeT = base::subtype<TSingleReturnType<F, Indexes, Vs...>>;

template <class T, class... Ts>
constexpr bool TypesAreSame() {
    return base::conjunction<std::is_same<T, Ts>...>::value;
}

template <class F, class Indexes, class... Vs>
struct TIsInvocable;

template <class F, std::size_t... indexes, class... Vs>
struct TIsInvocable<F, std::index_sequence<indexes...>, Vs...>
    : base::is_invocable<F, decltype(TVariantAccessor::Get<indexes>(
                                std::declval<Vs>()))...> {};

template <class F, bool typesAreSame, class... Vs>
struct TReturnTypeIfSameResults
    : base::invoke_result<F, decltype(TVariantAccessor::Get<0>(
                                 std::declval<Vs>()))...> {};

template <class F, class... Vs>
struct TReturnTypeIfSameResults<F, false, Vs...> {};

template <class F, bool invokable, class IndexPacks, class... Vs>
struct TReturnTypeIfInvocable {};

template <class F, class... IndexPacks, class... Vs>
struct TReturnTypeIfInvocable<F, true, base::type_pack<IndexPacks...>, Vs...>
    : TReturnTypeIfSameResults<
          F, TypesAreSame<TSingleReturnTypeT<F, IndexPacks, Vs...>...>(),
          Vs...> {};

template <class F, class IndexPacks, class... Vs>
struct TReturnTypeImpl;

template <class F, class... IndexPacks, class... Vs>
struct TReturnTypeImpl<F, base::type_pack<IndexPacks...>, Vs...>
    : TReturnTypeIfInvocable<
          F, base::conjunction_v<TIsInvocable<F, IndexPacks, Vs...>...>,
          base::type_pack<IndexPacks...>, Vs...> {};

template <class F, class... Vs>
struct TReturnType
    : TReturnTypeImpl<F,
                      decltype(matops::build_all_matrix_indexes(
                          std::index_sequence<base::template_parameters_count_v<
                              std::decay_t<Vs>>...>{})),
                      Vs...> {};

template <class F, class... Vs>
using TReturnTypeT = base::subtype<TReturnType<F, Vs...>>;

template <class ReturnType, class FRef, class... VRefs, std::size_t... ids>
ReturnType UnwrapIndexes(FRef f, VRefs... vs, std::index_sequence<ids...>) {
    return std::forward<FRef>(f)(
        TVariantAccessor::Get<ids>(std::forward<VRefs>(vs))...);
}

// Normal case (when no one variant is valueless by exception).
template <class ReturnType, class Indexes, bool inBoundaries, class FRef,
          class... VRefs>
constexpr std::enable_if_t<inBoundaries, ReturnType> VisitConcrete(
    FRef f, VRefs... vs) {
    return UnwrapIndexes<ReturnType, FRef, VRefs...>(
        std::forward<FRef>(f), std::forward<VRefs>(vs)..., Indexes{});
}

// Boundaries case (when some of variants is valueless by exception).
template <class ReturnType, class Indexes, bool inBoundaries, class FRef,
          class... VRefs>
constexpr std::enable_if_t<!inBoundaries, ReturnType> VisitConcrete(FRef,
                                                                    VRefs...) {
    throw TBadVariantAccess{};
}

template <class F, class... Vs, class... IndexPacks>
auto Visit(F&& f, base::type_pack<IndexPacks...>, Vs&&... vs)
    -> TReturnTypeT<F&&, Vs&&...> {
    using ReturnType = TReturnTypeT<F&&, Vs&&...>;
    using LambdaType = ReturnType (*)(F&&, Vs && ...);

    using RealSizes = std::index_sequence<
        base::template_parameters_count_v<std::decay_t<Vs>>...>;
    using FakeSizes = std::index_sequence<
        1 + base::template_parameters_count_v<std::decay_t<Vs>>...>;

    static constexpr LambdaType handlers[] = {
        VisitConcrete<ReturnType, IndexPacks,
                      matops::check_boundaries(IndexPacks{}, RealSizes{}), F&&,
                      Vs&&...>...};

    const std::size_t idx = matops::normal_to_flat_index(
        FakeSizes{}, TVariantAccessor::Index(vs)...);

    return handlers[idx](std::forward<F>(f), std::forward<Vs>(vs)...);
}

template <class ReturnType, class F, class T>
ReturnType CallIfSame(F&& f, T&& a, T&& b) {
    return std::forward<F>(f)(std::forward<T>(a), std::forward<T>(b));
}

template <class ReturnType, class F, class T, class U>
ReturnType CallIfSame(F&&, T&&, U&&) {  // Will never be called
    std::terminate();
}

}  // namespace NPrivate
