#pragma once

#include "matrix_ops.h"

#include <exception>

namespace base {

template <class... Ts>
class variant;

class bad_variant_access : public std::exception {};

namespace detail {

struct variant_accessor {
    template <std::size_t I, class... Ts>
    static base::type_pack_element_t<I, Ts...>& get(variant<Ts...>& v);

    template <std::size_t I, class... Ts>
    static const base::type_pack_element_t<I, Ts...>& get(
        const variant<Ts...>& v);

    template <std::size_t I, class... Ts>
    static base::type_pack_element_t<I, Ts...>&& get(variant<Ts...>&& v);

    template <std::size_t I, class... Ts>
    static const base::type_pack_element_t<I, Ts...>&& get(
        const variant<Ts...>&& v);
};

constexpr std::size_t variant_npos = -1;

template <class T, class... Ts>
constexpr std::size_t index_of_impl() {
    bool bs[] = {std::is_same<T, Ts>::value...};
    std::size_t result = variant_npos;
    std::size_t count = 0;
    for (std::size_t i = 0; i < sizeof...(Ts); ++i) {
        if (bs[i]) {
            ++count;
            result = i;
        }
    }
    return count == 1 ? result : variant_npos;
}

// Given some time `T` and type pack `Ts`, returns relative position of `T` in
// `Ts`. If `T` not presented in `Ts` or `T` presented in `Ts` more then once,
// returns `variant_npos`.
template <class T, class... Ts>
constexpr std::size_t index_of = index_of_impl<T, Ts...>();

static_assert(index_of<int, int, double> == 0, "");
static_assert(index_of<int, double> == variant_npos, "");
static_assert(index_of<int, int, double, int> == variant_npos, "");
static_assert(index_of<int> == variant_npos, "");

template <class F, class Indexes, class... Vs>
struct visit_concrete_result;

template <class F, std::size_t... indexes, class... Vs>
struct visit_concrete_result<F, std::index_sequence<indexes...>, Vs...>
    : base::invoke_result<F, decltype(variant_accessor::get<indexes>(
                                 std::declval<Vs>()))...> {};

template <class F, class Indexes, class... Vs>
using visit_concrete_result_t =
    base::subtype<visit_concrete_result<F, Indexes, Vs...>>;

template <class T, class... Ts>
constexpr bool types_are_same = base::conjunction_v<std::is_same<T, Ts>...>;

template <class F, class Indexes, class... Vs>
struct is_visitable_concrete;

template <class F, std::size_t... indexes, class... Vs>
struct is_visitable_concrete<F, std::index_sequence<indexes...>, Vs...>
    : base::is_invocable<F, decltype(variant_accessor::get<indexes>(
                                std::declval<Vs>()))...> {};

template <class F, bool typesAreSame, class... Vs>
struct visit_result_if_same
    : base::invoke_result<F, decltype(variant_accessor::get<0>(
                                 std::declval<Vs>()))...> {};

template <class F, class... Vs>
struct visit_result_if_same<F, false, Vs...> {};

template <class F, bool invokable, class IndexPacks, class... Vs>
struct visit_result_if_visitable {};

template <class F, class... IndexPacks, class... Vs>
struct visit_result_if_visitable<F, true, base::type_pack<IndexPacks...>, Vs...>
    : visit_result_if_same<
          F, types_are_same<visit_concrete_result_t<F, IndexPacks, Vs...>...>,
          Vs...> {};

template <class F, class IndexPacks, class... Vs>
struct visit_result_impl;

template <class F, class... IndexPacks, class... Vs>
struct visit_result_impl<F, base::type_pack<IndexPacks...>, Vs...>
    : visit_result_if_visitable<
          F,
          base::conjunction_v<is_visitable_concrete<F, IndexPacks, Vs...>...>,
          base::type_pack<IndexPacks...>, Vs...> {};

template <class F, class... Vs>
struct visit_result
    : visit_result_impl<
          F,
          decltype(matops::build_all_matrix_indexes(
              std::index_sequence<
                  base::template_parameters_count_v<std::decay_t<Vs>>...>{})),
          Vs...> {};

template <class F, class... Vs>
using visit_result_t = base::subtype<visit_result<F, Vs...>>;

template <class R, class FRef, class... VRefs, std::size_t... ids>
R unwrap_indexes(FRef f, VRefs... vs, std::index_sequence<ids...>) {
    return std::forward<FRef>(f)(
        variant_accessor::get<ids - 1>(std::forward<VRefs>(vs))...);
}

// Normal case (when no one variant is valueless by exception).
template <class R, class Indexes, bool valueless, class FRef, class... VRefs>
constexpr std::enable_if_t<!valueless, R> visit_concrete(FRef f, VRefs... vs) {
    return unwrap_indexes<R, FRef, VRefs...>(
        std::forward<FRef>(f), std::forward<VRefs>(vs)..., Indexes{});
}

// Valueless case (when some of variants is valueless by exception).
template <class R, class Indexes, bool valueless, class FRef, class... VRefs>
constexpr std::enable_if_t<valueless, R> visit_concrete(FRef, VRefs...) {
    throw bad_variant_access{};
}

// (size_t)(-1) + 1 == 0, so if one of indexes is 0, variant is valueless.
template <std::size_t... ids>
constexpr bool check_valueless(std::index_sequence<ids...>) {
    const bool bs[] = {(ids == 0)...};
    for (const bool b : bs) {
        if (b) {
            return true;
        }
    }
    return false;
}

template <class R, class F, class... Vs, class... IndexPacks>
R visit(F&& f, base::type_pack<IndexPacks...>, Vs&&... vs) {
    using handler_type = R (*)(F&&, Vs && ...);

    using FakeSizes = std::index_sequence<
        1 + base::template_parameters_count_v<std::decay_t<Vs>>...>;

    static constexpr handler_type handlers[] = {
        visit_concrete<R, IndexPacks, check_valueless(IndexPacks{}), F&&,
                       Vs&&...>...};

    const std::size_t idx =
        matops::normal_to_flat_index(FakeSizes{}, (vs.index() + 1)...);

    return handlers[idx](std::forward<F>(f), std::forward<Vs>(vs)...);
}

template <class R, class F, class T>
R call_if_same(F&& f, T&& a, T&& b) {
    return std::forward<F>(f)(std::forward<T>(a), std::forward<T>(b));
}

template <class R, class F, class T, class U>
R call_if_same(F&&, T&&, U&&) {  // Will never be called
    std::terminate();
}

}  // namespace detail

}  // namespace base
