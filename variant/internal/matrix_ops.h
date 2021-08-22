#pragma once

#include <utility>

#include "util/meta.h"

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

namespace detail {

// Constexpr version of std::array (aka std::array from c++17)
template <class T, std::size_t n>
struct array {
  constexpr const T& operator[](std::size_t i) const { return data_[i]; }
  constexpr T& operator[](std::size_t i) { return data_[i]; }
  T data_[n];
};

template <std::size_t index, std::size_t... sizes>
constexpr auto flat_to_normal_index(std::index_sequence<sizes...>) {
  const std::size_t dims[] = {sizes...};
  auto result = array<std::size_t, sizeof...(sizes)>{};
  auto cur = index;
  for (std::size_t i = 0; i < sizeof...(sizes); ++i) {
    result[i] = cur % dims[i];
    cur /= dims[i];
  }
  return result;
}

template <std::size_t index, class Sizes, std::size_t... ids>
constexpr auto flat_to_normal_index_helper(Sizes sizes,
                                           std::index_sequence<ids...>) {
  constexpr auto result = flat_to_normal_index<index>(sizes);
  return std::index_sequence<result[ids]...>{};
}

}  // namespace detail

// Given matrix dimensions and index in corresponding flattened matrix,
// computes normal index in that matrix.
//
// Example:
// 0 -> {3, 3, 3} -> flat_to_normal_index -> {0, 0, 0}
// 26 -> {3, 3, 3} -> flat_to_normal_index -> {2, 2, 2}
template <std::size_t index, std::size_t... sizes>
constexpr auto flat_to_normal_index(std::index_sequence<sizes...> is) {
  return detail::flat_to_normal_index_helper<index>(
      is, std::make_index_sequence<sizeof...(sizes)>{});
}

static_assert(std::is_same<decltype(flat_to_normal_index<0>(
                               std::index_sequence<3, 3, 3>{})),
                           std::index_sequence<0, 0, 0>>::value,
              "");

static_assert(std::is_same<decltype(flat_to_normal_index<26>(
                               std::index_sequence<3, 3, 3>{})),
                           std::index_sequence<2, 2, 2>>::value,
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
  return base::type_pack<decltype(
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
        base::type_pack<std::index_sequence<0, 0>, std::index_sequence<1, 0>,
                        std::index_sequence<2, 0>, std::index_sequence<0, 1>,
                        std::index_sequence<1, 1>, std::index_sequence<2, 1>,
                        std::index_sequence<0, 2>, std::index_sequence<1, 2>,
                        std::index_sequence<2, 2>>>::value,
    "");

// 3x3x3 matrix test.
static_assert(
    std::is_same<
        decltype(build_all_matrix_indexes(std::index_sequence<3, 3, 3>{})),
        base::type_pack<
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
