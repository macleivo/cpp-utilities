/*
 * Copyright 2023 Marcus Leivo
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#include <type_traits>
#include <utility>
#include <vector>
namespace {
template <typename T>
struct add_const {
    using type = const T;
};

template <typename T>
struct add_const<const T> {
    using type = T;
};

template <typename T>
constexpr typename add_const<T>::type& as_const(T& t) noexcept {
    return as_const(t);
}
} // namespace

namespace mleivo {
template <typename T>
class propagate_const {
    T m_ptr{nullptr};
    using element_type = std::remove_reference_t<decltype(*std::declval<T&>())>;

    template <typename U>
    struct is_propagate_const : std::false_type {};

    template <typename U>
    struct is_propagate_const<propagate_const<U>> : std::true_type {};

    template <typename U>
    static constexpr auto is_propagate_const_v = is_propagate_const<U>::value;

public:
    constexpr propagate_const() = default;
    constexpr propagate_const(propagate_const&& p) = default;

    template <typename U, typename std::enable_if_t<
                              std::is_constructible_v<T, U> and not std::is_convertible<U, T>::value, int> = 0>
    explicit constexpr propagate_const(propagate_const<U>&& u) : m_ptr(std::move<U>(u.get())) {
    }

    template <typename U,
              typename std::enable_if_t<std::is_constructible_v<T, U> and std::is_convertible<U, T>::value, int> = 1>
    constexpr propagate_const(propagate_const<U>&& u) : m_ptr(std::move<U>(u.get())) {
    }

    template <typename U, typename std::enable_if_t<std::is_constructible_v<T, U> and not is_propagate_const_v<U>
                                                        and not std::is_convertible_v<U, T>,
                                                    int> = 0>
    constexpr propagate_const(U&& u) : m_ptr(std::forward<U>(u)) {
    }

    template <typename U, typename std::enable_if_t<std::is_constructible_v<T, U> and not is_propagate_const_v<U>
                                                        and std::is_convertible_v<U, T>,
                                                    int> = 1>
    constexpr propagate_const(U&& u) : m_ptr(std::forward<U>(u)) {
    }

    propagate_const(const propagate_const&) = delete;

    constexpr element_type* get() {
        return m_ptr;
    }
    constexpr const element_type* get() const {
        return m_ptr;
    }

    constexpr element_type* operator->() {
        return m_ptr;
    }
    constexpr const element_type* operator->() const {
        return m_ptr;
    }

    constexpr element_type& operator*() {
        return *m_ptr;
    }
    constexpr const element_type& operator*() const {
        return *m_ptr;
    }

    explicit constexpr operator bool() const {
        return get() != nullptr;
    }
};
} // namespace mleivo