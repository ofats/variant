#pragma once

#include "variant_traits.h"

template <class T>
struct TInPlaceType {}; // aka std::in_place_type_t

template <class T>
constexpr TInPlaceType<T> IN_PLACE_TYPE; // aka std::in_place_type

template <std::size_t I>
struct TInPlaceIndex {}; // aka std::in_place_index_t

template <std::size_t I>
constexpr TInPlaceIndex<I> IN_PLACE_INDEX; // aka std::in_place_index

template <std::size_t I, class V>
struct TVariantAlternative;

template <std::size_t I, class... Ts>
struct TVariantAlternative<I, TVariant<Ts...>> : meta::type_pack_element<I, Ts...> {};

template <std::size_t I, class V>
using TVariantAlternativeT =
    typename TVariantAlternative<I, V>::type;  // aka std::variant_alternative_t

template <class V>
using TVariantSize = NPrivate::TSize<V>; // aka std::variant_size

template <class V>
constexpr std::size_t VARIANT_SIZE_V = TVariantSize<V>::value; // aka std::variant_size_v

constexpr std::size_t VARIANT_NPOS = NPrivate::T_NPOS; // aka std::variant_npos

template <class F, class... Vs>
constexpr decltype(auto) Visit(F&& f, Vs&&... vs) {
    constexpr auto matrixDimensionsSizes =
        std::index_sequence<1 + VARIANT_SIZE_V<std::decay_t<Vs>>...>{};

    constexpr auto matrixSize = matops::matrix_size(matrixDimensionsSizes);

    return NPrivate::VisitImpl(
        std::forward<F>(f),
        matops::build_all_matrix_indexes(matrixDimensionsSizes),
        std::forward<Vs>(vs)...);
}

template <class T, class... Ts>
constexpr std::enable_if_t<NPrivate::TIndexOf<T, Ts...>::value != VARIANT_NPOS,
                           bool>
HoldsAlternative(const TVariant<Ts...>& v) noexcept {
    return NPrivate::TIndexOf<T, Ts...>::value == v.index();
}

template <class... Ts>
class TVariant {
    template <class T>
    using TIndex = NPrivate::TIndexOf<std::decay_t<T>, Ts...>;

    using T_0 = TVariantAlternativeT<0, TVariant>;

    static_assert(NPrivate::TTypeTraits<Ts...>::TNoRefs::value,
                  "TVariant type arguments cannot be references.");
    static_assert(NPrivate::TTypeTraits<Ts...>::TNoVoids::value,
                  "TVariant type arguments cannot be void.");
    static_assert(NPrivate::TTypeTraits<Ts...>::TNoArrays::value,
                  "TVariant type arguments cannot be arrays.");
    static_assert(NPrivate::TTypeTraits<Ts...>::TNotEmpty::value,
                  "TVariant type list cannot be empty.");

public:
    TVariant() noexcept(std::is_nothrow_default_constructible<T_0>::value) {
        static_assert(std::is_default_constructible<T_0>::value,
                      "First alternative must be default constructible");
        EmplaceImpl<0>();
    }

    TVariant(const TVariant& rhs) {
        if (!rhs.valueless_by_exception()) {
            ForwardVariant(rhs);
        }
    }

    TVariant(TVariant&& rhs) noexcept(
        meta::conjunction<std::is_nothrow_move_constructible<Ts>...>::value) {
        if (!rhs.valueless_by_exception()) {
            ForwardVariant(std::move(rhs));
        }
    }

    template <class T, class = std::enable_if_t<!std::is_same<std::decay_t<T>, TVariant>::value>>
    TVariant(T&& value) {
        EmplaceImpl<TIndex<T>::value>(std::forward<T>(value));
    }

    template <class T, class... TArgs>
    explicit TVariant(TInPlaceType<T>, TArgs&&... args) {
        EmplaceImpl<TIndex<T>::value>(std::forward<TArgs>(args)...);
    }

    template <std::size_t I, class... TArgs>
    explicit TVariant(TInPlaceIndex<I>, TArgs&&... args) {
        EmplaceImpl<I>(std::forward<TArgs>(args)...);
    }

    ~TVariant() {
        Destroy();
    }

    TVariant& operator=(const TVariant& rhs) {
        if (rhs.valueless_by_exception()) {
            if (!valueless_by_exception()) {
                DestroyImpl();
                Index_ = ::TVariantSize<TVariant>::value;
            }
        } else if (index() == rhs.index()) {
            Visit([](auto& dst, const auto& src) {
                NPrivate::CallIfSame<void>([](auto& x, const auto& y) {
                    x = y;
                }, dst, src);
            }, *this, rhs);
        } else {
            *this = TVariant{rhs};
        }
        return *this;
    }

    TVariant& operator=(TVariant&& rhs) {
        if (rhs.valueless_by_exception()) {
            if (!valueless_by_exception()) {
                DestroyImpl();
                Index_ = ::TVariantSize<TVariant>::value;
            }
        } else if (index() == rhs.index()) {
            Visit([](auto& dst, auto& src) -> void {
                NPrivate::CallIfSame<void>([](auto& x, auto& y) {
                    x = std::move(y);
                }, dst, src);
            }, *this, rhs);
        } else {
            Destroy();
            try {
                ForwardVariant(std::move(rhs));
            } catch (...) {
                Index_ = ::TVariantSize<TVariant>::value;
                throw;
            }
        }
        return *this;
    }

    template <class T>
    std::enable_if_t<!std::is_same<std::decay_t<T>, TVariant>::value,
                     TVariant&>
    operator=(T&& value) {
        if (HoldsAlternative<std::decay_t<T>>(*this)) {
            *ReinterpretAs<TIndex<std::decay_t<T>>::value>() = std::forward<T>(value);
        } else {
            emplace<T>(std::forward<T>(value));
        }
        return *this;
    }

    // -------------------- OBSERVERS --------------------

    constexpr std::size_t index() const noexcept {
        return valueless_by_exception() ? VARIANT_NPOS : Index_;
    }

    constexpr bool valueless_by_exception() const noexcept {
        return Index_ == ::TVariantSize<TVariant>::value;
    }

    // -------------------- MODIFIERS --------------------

    template <std::size_t I, class... Args>
    std::enable_if_t<std::is_constructible<TVariantAlternativeT<I, TVariant>,
                                           Args...>::value,
                     TVariantAlternativeT<I, TVariant>>&
    emplace(Args&&... args) {
        Destroy();
        try {
            return EmplaceImpl<I>(std::forward<Args>(args)...);
        } catch (...) {
            Index_ = sizeof...(Ts);
            throw;
        }
    };

    template <class T, class... Args>
    auto emplace(Args&&... args)
        -> decltype(emplace<TIndex<T>::value>(std::forward<Args>(args)...)) {
        return emplace<TIndex<T>::value>(std::forward<Args>(args)...);
    };

    void swap(TVariant& rhs) {
        if (!valueless_by_exception() || !rhs.valueless_by_exception()) {
            if (index() == rhs.index()) {
                Visit([](auto& a, auto& b) -> void {
                    NPrivate::CallIfSame<void>([](auto& x, auto& y) {
                        DoSwap(x, y);
                    }, a, b);
                }, *this, rhs);
            } else {
                std::swap(*this, rhs);
            }
        }
    }

private:
    void Destroy() noexcept {
        if (!valueless_by_exception()) {
            DestroyImpl();
        }
    }

    void DestroyImpl() noexcept {
        Visit([](auto& value) {
            using T = std::decay_t<decltype(value)>;
            value.~T();
        }, *this);
    }

    template <std::size_t I, class... TArgs>
    TVariantAlternativeT<I, TVariant>& EmplaceImpl(TArgs&&... args) {
        new (&Storage_) TVariantAlternativeT<I, TVariant>(std::forward<TArgs>(args)...);
        Index_ = I;
        return *ReinterpretAs<I>();
    }

    template <class Variant>
    void ForwardVariant(Variant&& rhs) {
        Visit([&](auto&& value) {
            new (this) TVariant(std::forward<decltype(value)>(value));
        }, std::forward<Variant>(rhs));
    }

private:
    friend struct NPrivate::TVariantAccessor;

    template <std::size_t I>
    TVariantAlternativeT<I, TVariant>* ReinterpretAs() noexcept {
        return reinterpret_cast<TVariantAlternativeT<I, TVariant>*>(&Storage_);
    }

    template <std::size_t I>
    const TVariantAlternativeT<I, TVariant>* ReinterpretAs() const noexcept {
        return reinterpret_cast<const TVariantAlternativeT<I, TVariant>*>(&Storage_);
    }

private:
    std::size_t Index_ = ::TVariantSize<TVariant>::value;
    std::aligned_union_t<0, Ts...> Storage_;
};

template <class... Ts>
bool operator==(const TVariant<Ts...>& a, const TVariant<Ts...>& b) {
    if (a.index() != b.index()) {
        return false;
    }
    return a.valueless_by_exception() || Visit([](const auto& a, const auto& b) {
        return NPrivate::CallIfSame<bool>([](const auto& x, const auto& y) {
            return x == y;
        }, a, b);
    }, a, b);
}

template <class... Ts>
bool operator!=(const TVariant<Ts...>& a, const TVariant<Ts...>& b) {
    // The standard forces us to call operator!= for values stored in variants.
    // So we cannot reuse operator==.
    if (a.index() != b.index()) {
        return true;
    }
    return !a.valueless_by_exception() && Visit([](const auto& a, const auto& b) {
        return NPrivate::CallIfSame<bool>([](const auto& x, const auto& y) {
            return x != y;
        }, a, b);
    }, a, b);
}

template <class... Ts>
bool operator<(const TVariant<Ts...>& a, const TVariant<Ts...>& b) {
    if (b.valueless_by_exception()) {
        return false;
    }
    if (a.valueless_by_exception()) {
        return true;
    }
    if (a.index() == b.index()) {
        return Visit([](const auto& a, const auto& b) {
            return NPrivate::CallIfSame<bool>([](const auto& x, const auto& y) {
                return x < y;
            }, a, b);
        }, a, b);
    }
    return a.index() < b.index();
}

template <class... Ts>
bool operator>(const TVariant<Ts...>& a, const TVariant<Ts...>& b) {
    // The standard forces as to call operator> for values stored in variants.
    // So we cannot reuse operator< and operator==.
    if (a.valueless_by_exception()) {
        return false;
    }
    if (b.valueless_by_exception()) {
        return true;
    }
    if (a.index() == b.index()) {
        return Visit([](const auto& a, const auto& b) -> bool {
            return NPrivate::CallIfSame<bool>([](const auto& x, const auto& y) {
                return x > y;
            }, a, b);
        }, a, b);
    }
    return a.index() > b.index();
}

template <class... Ts>
bool operator<=(const TVariant<Ts...>& a, const TVariant<Ts...>& b) {
    // The standard forces as to call operator> for values stored in variants.
    // So we cannot reuse operator< and operator==.
    if (a.valueless_by_exception()) {
        return true;
    }
    if (b.valueless_by_exception()) {
        return false;
    }
    if (a.index() == b.index()) {
        return Visit([](const auto& a, const auto& b) {
            return NPrivate::CallIfSame<bool>([](const auto& x, const auto& y) {
                return x <= y;
            }, a, b);
        }, a, b);
    }
    return a.index() < b.index();
}

template <class... Ts>
bool operator>=(const TVariant<Ts...>& a, const TVariant<Ts...>& b) {
    // The standard forces as to call operator> for values stored in variants.
    // So we cannot reuse operator< and operator==.
    if (b.valueless_by_exception()) {
        return true;
    }
    if (a.valueless_by_exception()) {
        return false;
    }
    if (a.index() == b.index()) {
        return Visit([](const auto& a, const auto& b) {
            return NPrivate::CallIfSame<bool>([](const auto& x, const auto& y) {
                return x >= y;
            }, a, b);
        }, a, b);
    }
    return a.index() > b.index();
}


namespace NPrivate {

template <std::size_t I, class V>
decltype(auto) GetImpl(V&& v) {
    if (I != v.index()) {
        throw TBadVariantAccess{};
    }
    return NPrivate::TVariantAccessor::Get<I>(std::forward<V>(v));
}

}  // namespace NPrivate

// -------------------- GET BY INDEX --------------------

template <std::size_t I, class... Ts>
meta::type_pack_element_t<I, Ts...>& Get(TVariant<Ts...>& v) {
    return NPrivate::GetImpl<I>(v);
}

template <std::size_t I, class... Ts>
const meta::type_pack_element_t<I, Ts...>& Get(const TVariant<Ts...>& v) {
    return NPrivate::GetImpl<I>(v);
}

template <std::size_t I, class... Ts>
meta::type_pack_element_t<I, Ts...>&& Get(TVariant<Ts...>&& v) {
    return NPrivate::GetImpl<I>(std::move(v));
}

template <std::size_t I, class... Ts>
const meta::type_pack_element_t<I, Ts...>&& Get(const TVariant<Ts...>&& v) {
    return NPrivate::GetImpl<I>(std::move(v));
}

// -------------------- GET BY TYPE --------------------

template <class T, class... Ts>
auto Get(TVariant<Ts...>& v)
    -> decltype(Get<NPrivate::TIndexOf<T, Ts...>::value>(v)) {
    return Get<NPrivate::TIndexOf<T, Ts...>::value>(v);
}

template <class T, class... Ts>
auto Get(const TVariant<Ts...>& v)
    -> decltype(Get<NPrivate::TIndexOf<T, Ts...>::value>(v)) {
    return Get<NPrivate::TIndexOf<T, Ts...>::value>(v);
}

template <class T, class... Ts>
auto Get(TVariant<Ts...>&& v)
    -> decltype(Get<NPrivate::TIndexOf<T, Ts...>::value>(std::move(v))) {
    return Get<NPrivate::TIndexOf<T, Ts...>::value>(std::move(v));
}

template <class T, class... Ts>
auto Get(const TVariant<Ts...>&& v)
    -> decltype(Get<NPrivate::TIndexOf<T, Ts...>::value>(std::move(v))) {
    return Get<NPrivate::TIndexOf<T, Ts...>::value>(std::move(v));
}

// -------------------- GET IF BY INDEX --------------------

template <std::size_t I, class... Ts>
std::add_pointer_t<meta::type_pack_element_t<I, Ts...>> GetIf(
    TVariant<Ts...>* v) noexcept {
    return v != nullptr && I == v->index()
               ? &NPrivate::TVariantAccessor::Get<I>(*v)
               : nullptr;
}

template <std::size_t I, class... Ts>
std::add_pointer_t<const meta::type_pack_element_t<I, Ts...>> GetIf(
    const TVariant<Ts...>* v) noexcept {
    return v != nullptr && I == v->index()
               ? &NPrivate::TVariantAccessor::Get<I>(*v)
               : nullptr;
}

// -------------------- GET IF BY TYPE --------------------

template <class T, class... Ts>
auto GetIf(TVariant<Ts...>* v) noexcept
    -> decltype(GetIf<NPrivate::TIndexOf<T, Ts...>::value>(v)) {
    return GetIf<NPrivate::TIndexOf<T, Ts...>::value>(v);
}

template <class T, class... Ts>
auto GetIf(const TVariant<Ts...>* v) noexcept
    -> decltype(GetIf<NPrivate::TIndexOf<T, Ts...>::value>(v)) {
    return GetIf<NPrivate::TIndexOf<T, Ts...>::value>(v);
}

struct TMonostate {};

constexpr bool operator<(TMonostate, TMonostate) noexcept {
    return false;
}
constexpr bool operator>(TMonostate, TMonostate) noexcept {
    return false;
}
constexpr bool operator<=(TMonostate, TMonostate) noexcept {
    return true;
}
constexpr bool operator>=(TMonostate, TMonostate) noexcept {
    return true;
}
constexpr bool operator==(TMonostate, TMonostate) noexcept {
    return true;
}
constexpr bool operator!=(TMonostate, TMonostate) noexcept {
    return false;
}

namespace NPrivate {

template <std::size_t I, class... Ts>
meta::type_pack_element_t<I, Ts...>& TVariantAccessor::Get(TVariant<Ts...>& v) {
    return *v.template ReinterpretAs<I>();
}

template <std::size_t I, class... Ts>
const meta::type_pack_element_t<I, Ts...>& TVariantAccessor::Get(
    const TVariant<Ts...>& v) {
    return *v.template ReinterpretAs<I>();
}

template <std::size_t I, class... Ts>
meta::type_pack_element_t<I, Ts...>&& TVariantAccessor::Get(TVariant<Ts...>&& v) {
    return std::move(*v.template ReinterpretAs<I>());
}

template <std::size_t I, class... Ts>
const meta::type_pack_element_t<I, Ts...>&& TVariantAccessor::Get(
    const TVariant<Ts...>&& v) {
    return std::move(*v.template ReinterpretAs<I>());
}

template <class... Ts>
constexpr std::size_t TVariantAccessor::Index(const TVariant<Ts...>& v) noexcept {
    return v.Index_;
}

}  // namespace NPrivate
