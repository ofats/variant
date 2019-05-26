#pragma once

#include "meta.h"

#include <utility>

namespace matops {

// Given matrix dimensions and normal index in that matrix,
// computes index in corresponding flattened matrix.
//
// Example:
// {3, 3, 3} -> {1, 2, 1} -> normal_to_flat_index -> 16
// {3, 3} -> {1, 2} -> normal_to_flat_index -> 7
template <std::size_t... szs, class... Ids>
constexpr std::size_t normal_to_flat_index(std::index_sequence<szs...>,
                                           Ids... ids) {
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

static_assert(normal_to_flat_index(std::index_sequence<3, 3>{}, 0, 0) == 0, "");
static_assert(normal_to_flat_index(std::index_sequence<3, 3>{}, 1, 0) == 1, "");
static_assert(normal_to_flat_index(std::index_sequence<3, 3>{}, 1, 2) == 7, "");
static_assert(normal_to_flat_index(std::index_sequence<3, 3, 3>{}, 1, 2, 1) ==
                  16,
              "");

// Given matrix dimensions and index in corresponding flattened matrix,
// computes normal index in that matrix.
//
// Example:
// 0 -> {3, 3, 3} -> flat_to_normal_index -> {0, 0, 0}
// 26 -> {3, 3, 3} -> flat_to_normal_index -> {2, 2, 2}
template <std::size_t, std::size_t... acc>
constexpr auto flat_to_normal_index(std::index_sequence<>,
                                    std::index_sequence<acc...> result = {}) {
    return result;
}

template <std::size_t index, std::size_t size, std::size_t... sizes,
          std::size_t... acc>
constexpr auto flat_to_normal_index(std::index_sequence<size, sizes...>,
                                 std::index_sequence<acc...> = {}) {
    return flat_to_normal_index<index / size>(
        std::index_sequence<sizes...>{},
        std::index_sequence<acc..., index % size>{});
}

static_assert(std::is_same<decltype(flat_to_normal_index<0>(
                               std::index_sequence<3, 3, 3>{})),
                           std::index_sequence<0, 0, 0>>::value,
              "");

static_assert(std::is_same<decltype(flat_to_normal_index<26>(
                               std::index_sequence<3, 3, 3>{})),
                           std::index_sequence<2, 2, 2>>::value,
              "");

// Given matrix dimensions and index in that matrix,
// determines whether index is pointing inside that matrix,
// i.e each index is less than size of corresponding dimension.
//
// Example:
// {1, 1, 1} -> {3, 3, 3} -> check_boundaries -> true
// {1, 3, 1} -> {3, 3, 3} -> check_boundaries -> false
// {5, 5, 5} -> {3, 3, 3} -> check_boundaries -> false
template <std::size_t... ids, std::size_t... szs>
constexpr bool check_boundaries(std::index_sequence<ids...>,
                                std::index_sequence<szs...>) {
    const bool bs[] = {ids < szs...};
    bool result = true;
    for (const bool b : bs) {
        result &= b;
    }
    return result;
}

static_assert(check_boundaries(std::index_sequence<>{},
                               std::index_sequence<>{}),
              "");
static_assert(check_boundaries(std::index_sequence<1, 2, 1>{},
                               std::index_sequence<3, 3, 3>{}),
              "");
static_assert(!check_boundaries(std::index_sequence<1, 2, 3>{},
                                std::index_sequence<3, 3, 3>{}),
              "");

// Given matrix dimensions, calculates matrix size,
// i.e multiplies all the dimensions.
//
// Example:
// {3, 3, 3} -> matrix_size -> 27
// {3, 3} -> matrix_size -> 9
template <std::size_t... szs>
constexpr std::size_t matrix_size(std::index_sequence<szs...>) {
    const std::size_t sizes[] = {szs...};
    std::size_t result = 1;
    for (const std::size_t s : sizes) {
        result *= s;
    }
    return result;
}
constexpr std::size_t matrix_size(std::index_sequence<>) { return 0; }

static_assert(matrix_size(std::index_sequence<3, 3, 3>{}) == 27, "");
static_assert(matrix_size(std::index_sequence<3>{}) == 3, "");
static_assert(matrix_size(std::index_sequence<0>{}) == 0, "");
static_assert(matrix_size(std::index_sequence<>{}) == 0, "");

namespace detail {

template <std::size_t... indexes, class Sizes>
constexpr auto build_all_matrix_indexes(std::index_sequence<indexes...>,
                                        Sizes sizes) {
    return meta::type_pack<decltype(
        matops::flat_to_normal_index<indexes>(sizes))...>{};
}

}  // namespace detail

// Given matrix dimensions, builds list of all the indexes in that matrix in
// lexicographical order.
//
// Example:
// {3, 3} -> build_all_matrix_indexes ->
// {{0, 0}, {1, 0}, {2, 0},
//  {0, 1}, {1, 1}, {2, 1},
//  {0, 2}, {1, 2}, {2, 2}}
template <class Sizes>
constexpr auto build_all_matrix_indexes(Sizes sizes) {
    return detail::build_all_matrix_indexes(
        std::make_index_sequence<matrix_size(sizes)>{}, sizes);
}

// 3x3 matrix test.
static_assert(
    std::is_same<
        decltype(build_all_matrix_indexes(std::index_sequence<3, 3>{})),
        meta::type_pack<std::index_sequence<0, 0>, std::index_sequence<1, 0>,
                        std::index_sequence<2, 0>, std::index_sequence<0, 1>,
                        std::index_sequence<1, 1>, std::index_sequence<2, 1>,
                        std::index_sequence<0, 2>, std::index_sequence<1, 2>,
                        std::index_sequence<2, 2>>>::value,
    "");

// 3x3x3 matrix test.
static_assert(
    std::is_same<
        decltype(build_all_matrix_indexes(std::index_sequence<3, 3, 3>{})),
        meta::type_pack<
            std::index_sequence<0, 0, 0>, std::index_sequence<1, 0, 0>,
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

}  // namespace matops
