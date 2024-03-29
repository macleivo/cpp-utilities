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
#pragma once

#include <algorithm>
#include <numeric>
#include <utility>

#include <tuple>
#include <vector>

namespace mleivo::pipes {
namespace detail {
template <typename CallT, typename... Args>
struct wrapper {
    using mleivo_pipe = std::true_type;

    std::tuple<Args...> m_t;
    constexpr wrapper(Args... args) : m_t(std::forward<Args>(args)...) {
    }

    template <typename ContainerT>
    constexpr decltype(auto) operator()(ContainerT&& container) {
        std::apply(
            [&](auto&&... args) {
                using std::begin;
                using std::end;
                CallT::call(begin(container), end(container), std::forward<decltype(args)>(args)...);
            },
            std::move(m_t));
        return std::forward<decltype(container)>(container);
    }
};

template <typename CallT, typename... Args>
struct ret_wrapper {
    using mleivo_pipe_ret = std::true_type;

    std::tuple<Args...> m_t;
    constexpr ret_wrapper(Args... args) : m_t(std::forward<Args>(args)...) {
    }

    template <typename ContainerT>
    constexpr auto operator()(ContainerT&& container) {
        return std::apply(
            [&](auto&&... args) {
                using std::begin;
                using std::end;
                return CallT::call(begin(container), end(container), std::forward<decltype(args)>(args)...);
            },
            std::move(m_t));
    }
};
} // namespace detail

#define MLEIVO_STL_WRAPPER(FUNCTION_NAME)                                                                              \
    namespace detail {                                                                                                 \
    struct FUNCTION_NAME {                                                                                             \
        template <typename... Args>                                                                                    \
        static constexpr decltype(auto) call(Args&&... args) {                                                         \
            std::FUNCTION_NAME(std::forward<decltype(args)>(args)...);                                                 \
        }                                                                                                              \
    };                                                                                                                 \
    }                                                                                                                  \
                                                                                                                       \
    template <typename... Args>                                                                                        \
    constexpr decltype(auto) FUNCTION_NAME(Args&&... args) {                                                           \
        return detail::wrapper<detail::FUNCTION_NAME, decltype(args)...>{std::forward<decltype(args)>(args)...};       \
    };

#define MLEIVO_STL_WRAPPER_RET(FUNCTION_NAME)                                                                          \
    namespace detail {                                                                                                        \
    struct FUNCTION_NAME {                                                                                      \
        template <typename... Args>                                                                                    \
        static constexpr auto call(Args&&... args) {                                                                   \
            return std::FUNCTION_NAME(std::forward<decltype(args)>(args)...);                                          \
        }                                                                                                              \
    };                                                                                                                 \
    }                                                                                                                  \
                                                                                                                       \
    template <typename... Args>                                                                                        \
    constexpr auto FUNCTION_NAME(Args&&... args) {                                                                     \
        return detail::ret_wrapper<detail::FUNCTION_NAME, decltype(args)...>(std::forward<decltype(args)>(args)...);   \
    }

MLEIVO_STL_WRAPPER(for_each)
MLEIVO_STL_WRAPPER(reverse)
MLEIVO_STL_WRAPPER(sort)

MLEIVO_STL_WRAPPER_RET(max_element)
MLEIVO_STL_WRAPPER_RET(accumulate)

#undef MLEIVO_STL_WRAPPER
#undef MLEIVO_STL_WRAPPER_RET
} // namespace mleivo::pipes

template <typename ContainerT, typename CallablePipeT,
          typename = typename std::remove_cv_t<std::remove_reference_t<CallablePipeT>>::mleivo_pipe>
decltype(auto) constexpr operator|(ContainerT&& container, CallablePipeT&& f) {
    return f(std::forward<ContainerT>(container));
}

template <typename ContainerT, typename CallablePipeT,
          typename = typename std::remove_cv_t<std::remove_reference_t<CallablePipeT>>::mleivo_pipe_ret>
auto constexpr operator|(ContainerT&& container, CallablePipeT&& f) {
    return f(std::forward<ContainerT>(container));
}
