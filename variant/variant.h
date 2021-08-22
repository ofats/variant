#pragma once

#include "internal/variant_traits.h"

namespace base {

template <class T>
struct in_place_type_t {};

template <class T>
constexpr in_place_type_t<T> in_place_type;

template <std::size_t I>
struct in_place_index_t {};

template <std::size_t I>
constexpr in_place_index_t<I> in_place_index;

template <std::size_t I, class V>
struct variant_alternative;

template <std::size_t I, class... Ts>
struct variant_alternative<I, variant<Ts...>>
    : base::type_pack_element<I, Ts...> {};

template <std::size_t I, class V>
using variant_alternative_t = typename variant_alternative<I, V>::type;

template <class V>
struct variant_size;

template <class... Ts>
struct variant_size<variant<Ts...>>
    : base::template_parameters_count<variant<Ts...>> {};

template <class V>
constexpr std::size_t variant_size_v = variant_size<V>::value;

constexpr std::size_t variant_npos = detail::variant_npos;

template <class F, class... Vs,
          class Result = detail::visit_result_t<F&&, Vs&&...>>
constexpr Result visit(F&& f, Vs&&... vs) {
    constexpr auto matrixDimensionsSizes =
        std::index_sequence<1 + variant_size_v<std::decay_t<Vs>>...>{};

    return detail::visit<Result>(
        std::forward<F>(f),
        matops::build_all_matrix_indexes(matrixDimensionsSizes),
        std::forward<Vs>(vs)...);
}

template <class T, class... Ts, std::size_t index = detail::index_of<T, Ts...>>
constexpr std::enable_if_t<index != variant_npos, bool> holds_alternative(
    const variant<Ts...>& v) noexcept {
    return index == v.index();
}

template <class... Ts>
class variant {
    using T_0 = variant_alternative_t<0, variant>;

    static_assert(base::conjunction_v<base::negation<std::is_reference<Ts>>...>,
                  "variant type arguments cannot be references.");
    static_assert(
        base::conjunction_v<base::negation<std::is_same<Ts, void>>...>,
        "variant type arguments cannot be void.");
    static_assert(base::conjunction_v<base::negation<std::is_array<Ts>>...>,
                  "variant type arguments cannot be arrays.");
    static_assert(sizeof...(Ts) > 0, "variant type list cannot be empty.");

public:
    variant() noexcept(std::is_nothrow_default_constructible<T_0>::value) {
        static_assert(std::is_default_constructible<T_0>::value,
                      "First alternative must be default constructible");
        emplace_impl<0>();
    }

    variant(const variant& rhs) {
        if (!rhs.valueless_by_exception()) {
            forward_variant(rhs);
        }
    }

    variant(variant&& rhs) noexcept(
        base::conjunction_v<std::is_nothrow_move_constructible<Ts>...>) {
        if (!rhs.valueless_by_exception()) {
            forward_variant(std::move(rhs));
        }
    }

    template <class T, class TDec = std::decay_t<T>,
              std::size_t index = detail::index_of<TDec, Ts...>,
              class = std::enable_if_t<!std::is_same<TDec, variant>::value &&
                                       index != variant_npos>>
    variant(T&& value) {
        emplace_impl<index>(std::forward<T>(value));
    }

    template <class T, class... Args>
    explicit variant(in_place_type_t<T>, Args&&... args) {
        emplace_impl<detail::index_of<std::decay_t<T>, Ts...>>(
            std::forward<Args>(args)...);
    }

    template <std::size_t I, class... Args>
    explicit variant(in_place_index_t<I>, Args&&... args) {
        emplace_impl<I>(std::forward<Args>(args)...);
    }

    ~variant() { destroy(); }

    variant& operator=(const variant& rhs) {
        if (rhs.valueless_by_exception()) {
            if (!valueless_by_exception()) {
                destroy_impl();
                index_ = variant_npos;
            }
        } else if (index() == rhs.index()) {
            visit(
                [](auto& dst, const auto& src) {
                    detail::call_if_same<void>(
                        [](auto& x, const auto& y) { x = y; }, dst, src);
                },
                *this, rhs);
        } else {
            *this = variant{rhs};
        }
        return *this;
    }

    variant& operator=(variant&& rhs) {
        if (rhs.valueless_by_exception()) {
            if (!valueless_by_exception()) {
                destroy_impl();
                index_ = variant_npos;
            }
        } else if (index() == rhs.index()) {
            visit(
                [](auto& dst, auto& src) -> void {
                    detail::call_if_same<void>(
                        [](auto& x, auto& y) { x = std::move(y); }, dst, src);
                },
                *this, rhs);
        } else {
            destroy();
            try {
                forward_variant(std::move(rhs));
            } catch (...) {
                index_ = variant_npos;
                throw;
            }
        }
        return *this;
    }

    template <class T, class TDec = std::decay_t<T>>
    std::enable_if_t<!std::is_same<TDec, variant>::value, variant&> operator=(
        T&& value) {
        if (holds_alternative<TDec>(*this)) {
            *reinterpret_as<detail::index_of<TDec, Ts...>>() =
                std::forward<T>(value);
        } else {
            emplace<T>(std::forward<T>(value));
        }
        return *this;
    }

    // -------------------- OBSERVERS --------------------

    constexpr std::size_t index() const noexcept { return index_; }

    constexpr bool valueless_by_exception() const noexcept {
        return index_ == variant_npos;
    }

    // -------------------- MODIFIERS --------------------

    template <std::size_t I, class... Args>
    std::enable_if_t<std::is_constructible<variant_alternative_t<I, variant>,
                                           Args...>::value,
                     variant_alternative_t<I, variant>>&
    emplace(Args&&... args) {
        destroy();
        try {
            return emplace_impl<I>(std::forward<Args>(args)...);
        } catch (...) {
            index_ = variant_npos;
            throw;
        }
    }

    template <class T, class... Args,
              std::size_t index = detail::index_of<T, Ts...>>
    auto emplace(Args&&... args)
        -> decltype(emplace<index>(std::forward<Args>(args)...)) {
        return emplace<index>(std::forward<Args>(args)...);
    }

    void swap(variant& rhs) {
        if (!valueless_by_exception() || !rhs.valueless_by_exception()) {
            if (index() == rhs.index()) {
                visit(
                    [](auto& a, auto& b) -> void {
                        detail::call_if_same<void>(
                            [](auto& x, auto& y) { std::swap(x, y); }, a, b);
                    },
                    *this, rhs);
            } else {
                std::swap(*this, rhs);
            }
        }
    }

private:
    void destroy() noexcept {
        if (!valueless_by_exception()) {
            destroy_impl();
        }
    }

    void destroy_impl() noexcept {
        visit(
            [](auto& value) {
                using T = std::decay_t<decltype(value)>;
                value.~T();
            },
            *this);
    }

    template <std::size_t I, class... Args>
    variant_alternative_t<I, variant>& emplace_impl(Args&&... args) {
        new (&storage_)
            variant_alternative_t<I, variant>(std::forward<Args>(args)...);
        index_ = I;
        return *reinterpret_as<I>();
    }

    template <class Variant>
    void forward_variant(Variant&& rhs) {
        visit(
            [&](auto&& value) {
                new (this) variant(std::forward<decltype(value)>(value));
            },
            std::forward<Variant>(rhs));
    }

private:
    friend struct detail::variant_accessor;

    template <std::size_t I>
    variant_alternative_t<I, variant>* reinterpret_as() noexcept {
        return reinterpret_cast<variant_alternative_t<I, variant>*>(&storage_);
    }

    template <std::size_t I>
    const variant_alternative_t<I, variant>* reinterpret_as() const noexcept {
        return reinterpret_cast<const variant_alternative_t<I, variant>*>(
            &storage_);
    }

private:
    std::size_t index_ = variant_npos;
    std::aligned_union_t<0, Ts...> storage_;
};

template <class... Ts>
bool operator==(const variant<Ts...>& a, const variant<Ts...>& b) {
    if (a.index() != b.index()) {
        return false;
    }
    return a.valueless_by_exception() ||
           visit(
               [](const auto& a, const auto& b) {
                   return detail::call_if_same<bool>(
                       [](const auto& x, const auto& y) { return x == y; }, a,
                       b);
               },
               a, b);
}

template <class... Ts>
bool operator!=(const variant<Ts...>& a, const variant<Ts...>& b) {
    // The standard forces us to call operator!= for values stored in variants.
    // So we cannot reuse operator==.
    if (a.index() != b.index()) {
        return true;
    }
    return !a.valueless_by_exception() &&
           visit(
               [](const auto& a, const auto& b) {
                   return detail::call_if_same<bool>(
                       [](const auto& x, const auto& y) { return x != y; }, a,
                       b);
               },
               a, b);
}

template <class... Ts>
bool operator<(const variant<Ts...>& a, const variant<Ts...>& b) {
    if (b.valueless_by_exception()) {
        return false;
    }
    if (a.valueless_by_exception()) {
        return true;
    }
    if (a.index() == b.index()) {
        return visit(
            [](const auto& a, const auto& b) {
                return detail::call_if_same<bool>(
                    [](const auto& x, const auto& y) { return x < y; }, a, b);
            },
            a, b);
    }
    return a.index() < b.index();
}

template <class... Ts>
bool operator>(const variant<Ts...>& a, const variant<Ts...>& b) {
    // The standard forces us to call operator> for values stored in variants.
    // So we cannot reuse operator< and operator==.
    if (a.valueless_by_exception()) {
        return false;
    }
    if (b.valueless_by_exception()) {
        return true;
    }
    if (a.index() == b.index()) {
        return visit(
            [](const auto& a, const auto& b) -> bool {
                return detail::call_if_same<bool>(
                    [](const auto& x, const auto& y) { return x > y; }, a, b);
            },
            a, b);
    }
    return a.index() > b.index();
}

template <class... Ts>
bool operator<=(const variant<Ts...>& a, const variant<Ts...>& b) {
    // The standard forces us to call operator> for values stored in variants.
    // So we cannot reuse operator< and operator==.
    if (a.valueless_by_exception()) {
        return true;
    }
    if (b.valueless_by_exception()) {
        return false;
    }
    if (a.index() == b.index()) {
        return visit(
            [](const auto& a, const auto& b) {
                return detail::call_if_same<bool>(
                    [](const auto& x, const auto& y) { return x <= y; }, a, b);
            },
            a, b);
    }
    return a.index() < b.index();
}

template <class... Ts>
bool operator>=(const variant<Ts...>& a, const variant<Ts...>& b) {
    // The standard forces as to call operator> for values stored in variants.
    // So we cannot reuse operator< and operator==.
    if (b.valueless_by_exception()) {
        return true;
    }
    if (a.valueless_by_exception()) {
        return false;
    }
    if (a.index() == b.index()) {
        return visit(
            [](const auto& a, const auto& b) {
                return detail::call_if_same<bool>(
                    [](const auto& x, const auto& y) { return x >= y; }, a, b);
            },
            a, b);
    }
    return a.index() > b.index();
}

namespace detail {

template <std::size_t I, class V>
decltype(auto) get_impl(V&& v) {
    if (I != v.index()) {
        throw bad_variant_access{};
    }
    return detail::variant_accessor::get<I>(std::forward<V>(v));
}

}  // namespace detail

// -------------------- GET BY INDEX --------------------

template <std::size_t I, class... Ts>
base::type_pack_element_t<I, Ts...>& get(variant<Ts...>& v) {
    return detail::get_impl<I>(v);
}

template <std::size_t I, class... Ts>
const base::type_pack_element_t<I, Ts...>& get(const variant<Ts...>& v) {
    return detail::get_impl<I>(v);
}

template <std::size_t I, class... Ts>
base::type_pack_element_t<I, Ts...>&& get(variant<Ts...>&& v) {
    return detail::get_impl<I>(std::move(v));
}

template <std::size_t I, class... Ts>
const base::type_pack_element_t<I, Ts...>&& get(const variant<Ts...>&& v) {
    return detail::get_impl<I>(std::move(v));
}

// -------------------- GET BY TYPE --------------------

template <class T, class... Ts, std::size_t index = detail::index_of<T, Ts...>>
auto get(variant<Ts...>& v) -> decltype(get<index>(v)) {
    return get<index>(v);
}

template <class T, class... Ts, std::size_t index = detail::index_of<T, Ts...>>
auto get(const variant<Ts...>& v) -> decltype(get<index>(v)) {
    return get<index>(v);
}

template <class T, class... Ts, std::size_t index = detail::index_of<T, Ts...>>
auto get(variant<Ts...>&& v) -> decltype(get<index>(std::move(v))) {
    return get<index>(std::move(v));
}

template <class T, class... Ts, std::size_t index = detail::index_of<T, Ts...>>
auto get(const variant<Ts...>&& v) -> decltype(get<index>(std::move(v))) {
    return get<index>(std::move(v));
}

// -------------------- GET IF BY INDEX --------------------

template <std::size_t I, class... Ts>
std::add_pointer_t<base::type_pack_element_t<I, Ts...>> get_if(
    variant<Ts...>* v) noexcept {
    return v != nullptr && I == v->index()
               ? &detail::variant_accessor::get<I>(*v)
               : nullptr;
}

template <std::size_t I, class... Ts>
std::add_pointer_t<const base::type_pack_element_t<I, Ts...>> get_if(
    const variant<Ts...>* v) noexcept {
    return v != nullptr && I == v->index()
               ? &detail::variant_accessor::get<I>(*v)
               : nullptr;
}

// -------------------- GET IF BY TYPE --------------------

template <class T, class... Ts, std::size_t index = detail::index_of<T, Ts...>>
auto get_if(variant<Ts...>* v) noexcept -> decltype(get_if<index>(v)) {
    return get_if<index>(v);
}

template <class T, class... Ts, std::size_t index = detail::index_of<T, Ts...>>
auto get_if(const variant<Ts...>* v) noexcept -> decltype(get_if<index>(v)) {
    return get_if<index>(v);
}

struct monostate {};

constexpr bool operator<(monostate, monostate) noexcept { return false; }
constexpr bool operator>(monostate, monostate) noexcept { return false; }
constexpr bool operator<=(monostate, monostate) noexcept { return true; }
constexpr bool operator>=(monostate, monostate) noexcept { return true; }
constexpr bool operator==(monostate, monostate) noexcept { return true; }
constexpr bool operator!=(monostate, monostate) noexcept { return false; }

namespace detail {

template <std::size_t I, class... Ts>
base::type_pack_element_t<I, Ts...>& variant_accessor::get(variant<Ts...>& v) {
    return *v.template reinterpret_as<I>();
}

template <std::size_t I, class... Ts>
const base::type_pack_element_t<I, Ts...>& variant_accessor::get(
    const variant<Ts...>& v) {
    return *v.template reinterpret_as<I>();
}

template <std::size_t I, class... Ts>
base::type_pack_element_t<I, Ts...>&& variant_accessor::get(
    variant<Ts...>&& v) {
    return std::move(*v.template reinterpret_as<I>());
}

template <std::size_t I, class... Ts>
const base::type_pack_element_t<I, Ts...>&& variant_accessor::get(
    const variant<Ts...>&& v) {
    return std::move(*v.template reinterpret_as<I>());
}

}  // namespace detail

}  // namespace base
