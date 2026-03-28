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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
/// @file
#ifndef VIDRE_CORE_EXCEPTION_HH
#define VIDRE_CORE_EXCEPTION_HH
#pragma once

/**@addtogroup api-core-exception
 * @details
 * @fileinfo
 *
 * There are many pro's and con's for using exceptions versus error return
 * types—which we shall not repeat here. We should note, though, that the
 * mantra that exceptions are generally slower is [untenable](https://www.youtube.com/watch?v=bY2FlayomlE).
 *
 * We shold advice, though, that Boost's exception
 * [annotations](https://www.boost.org/doc/libs/latest/libs/exception/doc/tutorial_transporting_data.html) are used and encouraged
 * for transporting terse and human readable information, which may add computational overhead.
 *
 * */
#include <vidre/core/config.hh>

#include <boost/exception/all.hpp>

#include <string>
#include <source_location>

namespace vidre {

    /**@addtogroup api-core-exception
     * */
    ///@{

    /** Exception info factory.
     * See Boost's ["Transporting of Arbitrary
     * Data"](https://www.boost.org/doc/libs/latest/libs/exception/doc/tutorial_transporting_data.html) for further information.
     * */
    template <class Tag, class T>
    using error_info = boost::error_info<Tag, T>;

    /** C-string error message.
     * */
    using throw_message = error_info<struct throw_message_, const char *>;

    /** Buffer size for messages.
     * @todo This should be in the config.hh!
     * */
#define VIDRE_EXCEPTION_BUFSIZE 1024

    /** Static error message.
     *
     * Some implementations passing error messages (for example std::error_code::message()) create
     * temporary strings which are not usable with exceptions. This class provides a mechanism
     * with static storage for passing such messages.
     *
     * Usage:
     * @code
     * const char *system_error::what() const noexcept {
     *     static thread_local static_error_message msg;
     *
     *     msg = ec_.message();
     *
     *     return msg;
     * }
     * @endcode
     *
     * */
    class VIDRE_CORE_VISIBILITY static_error_message {
    public:
        static_error_message() = default;

    protected:
        char buf_[VIDRE_EXCEPTION_BUFSIZE] = {};

    public:
        /** Implicit constant C-string conversion.
         * */
        operator const char *() const noexcept {
            return buf_;
        }

        /** Assign C-string.
         * */
        static_error_message &operator=(const char *s) noexcept {
            auto n = std::min(size_t(VIDRE_EXCEPTION_BUFSIZE - 1), strlen(s));
            std::copy_n(s, n, buf_);
            buf_[n] = 0;
            return *this;
        }

        /** Assign std::string.
         * */
        static_error_message &operator=(const std::string &s) noexcept {
            auto n = std::min(size_t(VIDRE_EXCEPTION_BUFSIZE - 1), s.size());
            std::copy_n(s.data(), n, buf_);
            buf_[n] = 0;
            return *this;
        }
    };

    /** Default VidRe exception.
     * */
    class VIDRE_CORE_VISIBILITY exception
        : public virtual boost::exception
        , public virtual std::exception {
    public:
        exception(const std::source_location location = std::source_location::current());
    };

    inline exception::exception(const std::source_location location) : boost::exception(), std::exception() {
#if VIDRE_DEVELOP
        (*this)                                                 //
            << boost::throw_function(location.function_name())  //
            << boost::throw_file(location.file_name())          //
            << boost::throw_line(location.line());
#endif
    }

    /** Exception originating from a std::error_code.
     * */
    class VIDRE_CORE_VISIBILITY system_error : public virtual exception {
    protected:
        const std::error_code ec_{};

    public:
        system_error(std::error_code ec, const std::source_location location = std::source_location::current())
            : exception(location), ec_(ec) {}

        const char *what() const noexcept override;
    };

    ///@}

}  // namespace vidre

#endif  // VIDRE_CORE_EXCEPTION_HH
