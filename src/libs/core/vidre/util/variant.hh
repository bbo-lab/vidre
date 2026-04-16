/* VidRe
 * Copyright (C) 2026 Max-Planck-Institute f. Neurobiol. of Behavior — caesar, Bonn, Germany
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
/// @file
#ifndef VIDRE_CORE_UTIL_VARIANT_HH
#define VIDRE_CORE_UTIL_VARIANT_HH
#pragma once

/**@defgroup api-core-util-variant Variants
 * @ingroup api-core-util
 * @details
 * @fileinfo
 * */

#include <vidre/core/config.hh>
#include <vidre/exception.hh>

#include <variant>

namespace vidre {

    /**@addtogroup api-core-util-variant
     * */
    ///@{

    /** A simple variant.
     *
     * This variant is, except for std::string, constant evaluated. Internally
     * it stores: @c long, @c unsigned long, @c double, <tt>const char *</tt>,
     * and, @c std::string.
     * */
    class VIDRE_CORE_VISIBILITY variant {
    public:
        /** Internally stored value type.
         * */
        enum value : unsigned {
            none,     ///< The variant is "empty".
            natural,  ///< Natural (ie. unsigned) number.
            integer,  ///< Signed integer.
            number,   ///< Floating-point number.
            cstring,  ///< Constant C-string.
            string,   ///< A string (@c std::string).
        };

        /** Helper for deducing the type.
         * */
        template <typename T>
        struct deduce {
            static constexpr value id = none;
            using type = void;
        };

    public:
        consteval variant() = default;

        constexpr variant(variant &&other) = default;
        constexpr variant &operator=(variant &&) = default;
        constexpr variant(const variant &other) = default;
        constexpr variant &operator=(const variant &) = default;

        constexpr variant(unsigned long v) noexcept : data_{v}, type_{natural} {}
        constexpr variant(unsigned int v) noexcept : data_{(unsigned long)v}, type_{natural} {}
        constexpr variant(unsigned short v) noexcept : data_{(unsigned long)v}, type_{natural} {}
        constexpr variant(long v) noexcept : data_{v}, type_{integer} {}
        constexpr variant(int v) noexcept : data_{(long)v}, type_{integer} {}
        constexpr variant(short v) noexcept : data_{(long)v}, type_{integer} {}
        constexpr variant(double v) noexcept : data_{v}, type_{number} {}
        constexpr variant(const char *v) noexcept : data_{v}, type_{cstring} {}
        variant(const std::string &v) noexcept : data_{v}, type_{string} {}

    protected:
        std::variant<unsigned long, long, double, const char *, std::string> data_{};
        value type_{none};

    public:
        /** Get the value type.
         * */
        constexpr value type() const noexcept {
            return type_;
        }

        /** Check if "empty".
         * */
        constexpr bool empty() const noexcept {
            return (type_ == none);
        }

        /** Get value.
         * */
        template <typename T>
        constexpr operator T() const;

        /** Assign value.
         * @note The value must be of the same type.
         * */
        template <typename T>
        constexpr variant &operator=(const T &);
        template <typename T>
        constexpr variant &operator=(const T *);
    };

    ///@cond
    // FIXME the following needs to go into "implementation details"
    template <>
    struct variant::deduce<unsigned long> {
        static constexpr value id = natural;
        using type = unsigned long;
    };

    template <>
    struct variant::deduce<unsigned int> {
        static constexpr value id = natural;
        using type = unsigned long;
    };

    template <>
    struct variant::deduce<unsigned short> {
        static constexpr value id = natural;
        using type = unsigned long;
    };

    template <>
    struct variant::deduce<long> {
        static constexpr value id = integer;
        using type = long;
    };

    template <>
    struct variant::deduce<int> {
        static constexpr value id = integer;
        using type = long;
    };

    template <>
    struct variant::deduce<short> {
        static constexpr value id = integer;
        using type = long;
    };

    template <>
    struct variant::deduce<char> {
        static constexpr value id = integer;
        using type = long;
    };

    template <>
    struct variant::deduce<double> {
        static constexpr value id = number;
        using type = double;
    };

    template <>
    struct variant::deduce<float> {
        static constexpr value id = number;
        using type = double;
    };

    template <>
    struct variant::deduce<const char *> {
        static constexpr value id = cstring;
        using type = const char *;
    };

    template <>
    struct variant::deduce<std::string> {
        static constexpr value id = string;
        using type = std::string;
    };

    template <typename T>
    inline constexpr variant::operator T() const {
        if (deduce<T>::id != type_) {
            if (std::is_constant_evaluated())
                throw "implicit type conversion is not supported";
            else
                throw exception::logic() << error::message("implicit type conversion is not supported");
        }

        return static_cast<T>(std::get<typename deduce<T>::type>(data_));
    }

    template <>
    inline constexpr variant::operator const char *() const {
        switch (type_) {
            case cstring:
                return std::get<const char *>(data_);
            case string:
                return std::get<std::string>(data_).c_str();
            default:
                if (std::is_constant_evaluated())
                    throw "implicit type conversion is not supported";
                else
                    throw exception::logic() << error::message("implicit type conversion is not supported");
        }
    }

    template <>
    inline variant::operator std::string() const {
        switch (type_) {
            case cstring:
                return std::get<const char *>(data_);
            case string:
                return std::get<std::string>(data_);
            default:
                throw exception::logic() << error::message("implicit type conversion is not supported");
        }
    }

    template <typename T>
    inline constexpr variant &variant::operator=(const T &v) {
        if (deduce<T>::id != type_) {
            if (std::is_constant_evaluated())
                throw "implicit type conversion is not supported";
            else
                throw exception::logic() << error::message("implicit type conversion is not supported");
        }

        data_ = static_cast<typename deduce<T>::type>(v);

        return *this;
    }

    template <>
    inline constexpr variant &variant::operator=(const char *v) {
        switch (type_) {
            case cstring:
                data_ = v;
                break;
            case string:
                data_ = std::string(v);
                break;
            default:
                if (std::is_constant_evaluated())
                    throw "implicit type conversion is not supported";
                else
                    throw exception::logic() << error::message("implicit type conversion is not supported");
        }

        return *this;
    }

    template <>
    inline constexpr variant &variant::operator=(const std::string &v) {
        switch (type_) {
            case cstring:
                type_ = string;
                [[fallthrough]];
            case string:
                data_ = v;
                break;
            default:
                if (std::is_constant_evaluated())
                    throw "implicit type conversion is not supported";
                else
                    throw exception::logic() << error::message("implicit type conversion is not supported");
        }

        return *this;
    }
    ///@endcond

    ///@}

}  // namespace vidre

#endif  // VIDRE_CORE_UTIL_VARIANT_HH
