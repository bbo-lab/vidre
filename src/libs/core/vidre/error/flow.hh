/* VidRe
 * Copyright (C) 2024-2026 Max-Planck-Institute f. Neurobiol. of Behavior — caesar, Bonn, Germany
 * Author(s):
 *      Jürgen "George" Sawinski¹
 *
 * Affiliations:
 * ¹ MPI f. Neurobiol. of Behavior — caesar, Bonn, Germany
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
/// @file
#ifndef VIDRE_CORE_ERROR_FLOW_HH
#define VIDRE_CORE_ERROR_FLOW_HH
#include <memory>
#include <string_view>
#pragma once

/**@addtogroup api-core-error-flow
 * @details
 * @fileinfo
 * */

#include <vidre/core/config.hh>
#include <vidre/error/details.hh>

#include <system_error>
#include <expected>
#include <format>

namespace vidre {

    /**@addtogroup api-core-error-flow
     * */
    ///@{

    /** Abort program with a message.
     * FIXME
     * */
    [[noreturn]] void abort_with_message(const char *msg);

    /** Program flow status.
     *
     * This is a simple wrapper around a boolean value, to make it more explicit
     * that a function returns a status (i.e. success or failure) rather than a
     * value.
     *
     * @see `result<>`
     * */
    class VIDRE_CORE_VISIBILITY status {
    public:
        enum class type {
            ok = 0,
            bad = -1
        };

        constexpr static type ok = type::ok;    ///< Short for "status::type::ok".
        constexpr static type bad = type::bad;  ///< Short for "status::type::bad".

    public:
        /** Construct from status type.
         * @param s The status type (default: @ref bad).
         * */
        status(type s = bad) : status_(s) {}

        status(const status &) = default;
        status &operator=(const status &) = default;
        status(status &&) noexcept = default;
        status &operator=(status &&) noexcept = default;

        /** Construct from signed integer error code.
         *
         * This constructor enables seamless integration with C-style functions
         * that return negative values to indicate errors (e.g., POSIX functions,
         * many C library functions).
         *
         * @tparam T A signed integral type. Only @c int, @c long, and @c long long
         *           are accepted to avoid ambiguity with character types and other
         *           integral types that might have different semantic meanings.
         * @param value The integer value to interpret:
         *        - If value < 0: status is @ref bad (error)
         *        - If value >= 0: status is @ref ok (success)
         *
         * Example:
         * @code
         * // With a C function returning negative on error:
         * int c_result = some_c_function();
         * status s(c_result);
         * if (!s) {
         *     // handle error
         * }
         * @endcode
         *
         * @note This constructor is intentionally not explicit to allow
         *       natural integration with C APIs. Only @c int, @c long, and
         *       @c long long are accepted to prevent accidental conversions
         *       from character types, @c bool, or @c ptrdiff_t.
         *
         * @see @ref status::status(status_type)
         * */
        template <typename T>
        requires (std::same_as<T, int> || std::same_as<T, long> || std::same_as<T, long long>)
        status(T value) : status_(value < 0 ? bad : ok) {}

    protected:
        type status_ = bad;

    public:
        /** Convert the status to a boolean value.
         * @return true if the status is `ok`, false otherwise.
         * Example:
         * @code
         * if (some_function() == status::bad) {
         *     // handle error
         * }
         * @endcode
         * which is equivalent to
         * @code
         * if (not some_function()) {
         *     // handle error
         * }
         * @endcode
         * */
        operator bool() const noexcept {
            return status_ == ok;
        }

        /** Implicit conversion to status type.
         * @return The status type (`ok` or `bad`).
         * */
        operator type() const noexcept {
            return status_;
        }

        /** Boolean negation.
         * @return true if the status is bad, false otherwise.
         * */
        bool operator!() const noexcept {
            return status_ == bad;
        }

        /** String conversion.
         * @return A string representation of the status (`"ok"`, `"bad"`, or `"unknown"`).
         * */
        constexpr operator std::string_view() const noexcept {
            switch (status_) {
                case ok:
                    return "ok";
                case bad:
                    return "bad";
                default:
                    return "unknown";
            }
        }
    };

    template <typename T, typename E = std::errc>
    class result;

    /** A result type that either contains a value or an error.
     * @tparam T The type of the successful result.
     * @tparam E The type of the error (default: `std::errc`).
     *
     * This is a wrapper around std::expected that provides a simpler,
     * more intuitive interface.
     */
    template <typename T, typename E>
    requires (not std::is_void_v<T>)
    class result<T, E> {
    public:
        using value_type = T;
        using error_type = E;

    public:
        constexpr result() = default;
        constexpr result(const T &value) : impl_(value) {}
        constexpr result(T &&value) : impl_(std::move(value)) {}
        constexpr result(const E &error) : impl_(std::unexpected(error)) {}
        constexpr result(E &&error) : impl_(std::unexpected(std::move(error))) {}

        template <typename G>
        requires std::constructible_from<E, const G &>
        constexpr result(const std::unexpected<G> &error) : impl_(std::unexpected<E>(E(error.error()))) {}

        template <typename G>
        requires std::constructible_from<E, G>
        constexpr result(std::unexpected<G> &&error) : impl_(std::unexpected<E>(E(std::move(error.error())))) {}

    private:
        std::expected<T, E> impl_{};

    public:
        /** Implicit conversion to status.
         * */
        constexpr operator status::type() const noexcept {
            return (error() == E{}) ? status::ok : status::bad;
        }

        // /** Implicit conversion to error.
        //  * */
        // constexpr operator E() const noexcept {
        //     return impl_.error_or(E{});
        // }

        /** Dereference value.
         * */
        constexpr T &operator*() &
        requires (not std::is_void_v<T>)
        {
            return impl_.value();
        }
        /** Dereference value (const).
         * */
        constexpr const T &operator*() const & {
            return impl_.value();
        }

        /** Get error (if applicable).
         * */
        constexpr E error() const noexcept {
            return impl_.error_or(E{});
        }

    public:
        constexpr std::expected<T, E> &expected() {
            return impl_;
        }
        constexpr const std::expected<T, E> &expected() const {
            return impl_;
        }
    };

    template <typename E>
    class result<void, E> {
    public:
        using value_type = void;
        using error_type = E;

    public:
        constexpr result() = default;
        constexpr result(const E &error) : impl_(std::unexpected(error)) {}
        constexpr result(E &&error) : impl_(std::unexpected(std::move(error))) {}

        template <typename G>
        requires std::constructible_from<E, const G &>
        constexpr result(const std::unexpected<G> &error) : impl_(std::unexpected<E>(E(error.error()))) {}

        template <typename G>
        requires std::constructible_from<E, G>
        constexpr result(std::unexpected<G> &&error) : impl_(std::unexpected<E>(E(std::move(error.error())))) {}

    private:
        std::expected<void, E> impl_{};

    public:
        /** Implicit conversion to status.
         */
        constexpr operator status::type() const noexcept {
            return impl_.has_value() ? status::ok : status::bad;
        }

        /** Implicit conversion to error.
         */
        constexpr operator E() const {
            return impl_.error_or(E{});
        }

        /** Get error (if applicable).
         */
        constexpr E error() const {
            return impl_.error_or(E{});
        }

    public:
        constexpr std::expected<void, E> &expected() {
            return impl_;
        }

        constexpr const std::expected<void, E> &expected() const {
            return impl_;
        }
    };

    template <class E = std::errc>
    using unexpected = std::unexpected<E>;

    ///@}

}  // namespace vidre

#endif  // VIDRE_CORE_ERROR_FLOW_HH
