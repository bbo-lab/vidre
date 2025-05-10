/* VidRe
 * Copyright (C) 2025 Max-Planck-Institute f. Neurobiol. of Behavior — caesar, Bonn, Germany
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
#ifndef VIDRE_CORE_LOG_HH
#define VIDRE_CORE_LOG_HH
#pragma once

/**@addtogroup api-core-log
 * @details
 * @fileinfo
 *
 * */

#include <vidre/core/config.hh>

#include <limits>
#include <iostream>
#include <vector>

#include <cassert>

namespace vidre {

    /**@addtogroup api-core-log
     * */
    ///@{

    /** Message logging.
     *
     * FIXME
     *
     * */
    class VIDRE_CORE_VISIBILITY log {
    public:
        class stream;

        class fatal;
        class alert;
        class critical;
        class error;
        class warning;
        class notice;
        class status;
        class info;
        class debug;
        class trace;

        using string_type = std::string_view;

        /** Log levels.
         * */
        enum class level : int {
            fatal = -5,     ///< System is in an unusable state and the program will terminate.
            alert = -4,     ///< Immediate user interaction is required, usually when data corruption occurs.
            critical = -3,  ///< A critical error, such as hardware failure.
            error = -2,     ///< An application error occurred.
            warning = -1,   ///< Warning level, which may hint to future failure.
            notice = 0,     ///< A noteworthy condition occurred.
            status = 1,     ///< A status message.
            info = 2,       ///< A purely informational message.
            debug = 3,      ///< Debugging information.
            trace = 4,      ///< Tracing.
        };

        // static const string_type prefix

        /** Minimum log level (as integer). */
        constexpr static int min_level = static_cast<int>(level::fatal);

        /** Default log level. */
        constexpr static int default_level = static_cast<int>(level::notice);

        /** End-of-line.
         * */
        constexpr static string_type eol = string_type({"\n", 1});

        /** Maximum log level (as integer).
         *
         * @note
         *      Tracing (which is only enabled for DEBUG builds) allows nesting, which, in turn,
         *      allows the respective log formatter to indent the trace.
         *
         * */
        constexpr static int max_level = std::numeric_limits<int>::max();

    public:
        /** Set (global) logging threshold.
         * */
        static void threshold(level lvl) noexcept {
            threshold_ = static_cast<int>(lvl);
            assert(threshold_ >= 0);
        }

    protected:
        static int threshold_;                 ///< Global log threshold.
        static std::vector<stream *> stream_;  ///< Log streams.

    public:
        log() = default;
        ~log();

    protected:
        log(level lvl) noexcept : level_(static_cast<int>(lvl)) {}

    protected:
        int level_ = default_level;  ///< Log level (for this logging object).

    public:
        /** Emit a log message.
         * */
        void emit(const string_type &msg) noexcept;

        /** Emit indented log message.
         * */
        void emit(unsigned depth, const string_type &msg) noexcept;

    public:
        log &operator<<(const string_type &msg) noexcept {
            emit(msg);
            return *this;
        }
    };

    /** Log stream.
     *
     * FIXME
     *
     * */
    class VIDRE_CORE_VISIBILITY log::stream {
    public:
        stream() = default;
        virtual ~stream() = default;

    public:
        virtual void emit(const string_type &msg) noexcept {
            std::clog << msg;
        }
    };

    ///@}

    class VIDRE_CORE_VISIBILITY log::fatal : log {
    public:
        fatal() noexcept : log(log::level::fatal) {}
        ~fatal() {
            std::terminate();
        }

        fatal &operator<<(const string_type &msg) noexcept {
            emit(msg);
            return *this;
        }
    };

#define VIDRE_CORE_LOG_IMPL(T)                           \
    class VIDRE_CORE_VISIBILITY log::T : log {           \
    public:                                              \
        T() noexcept : log(log::level::T) {}             \
        T &operator<<(const string_type &msg) noexcept { \
            emit(msg);                                   \
            return *this;                                \
        }                                                \
    };

    VIDRE_CORE_LOG_IMPL(alert);
    VIDRE_CORE_LOG_IMPL(critical);
    VIDRE_CORE_LOG_IMPL(error);
    VIDRE_CORE_LOG_IMPL(warning);
    VIDRE_CORE_LOG_IMPL(notice);
    VIDRE_CORE_LOG_IMPL(status);
    VIDRE_CORE_LOG_IMPL(info);

#if defined(NDEBUG)
    class VIDRE_CORE_VISIBILITY log::debug : log {
    public:
        debug() noexcept {}
        debug &operator<<(const string_type &) noexcept {
            return *this;
        }
    };
#else
    VIDRE_CORE_LOG_IMPL(debug);
#endif

#if defined(DEBUG)
    // FIXME
    class VIDRE_CORE_VISIBILITY log::trace : log {
    public:
        trace() noexcept {}
        trace &operator<<(const string_type &) noexcept {
            return *this;
        }
    };
#else
    class VIDRE_CORE_VISIBILITY log::trace : log {
    public:
        trace() noexcept {}
        trace &operator<<(const string_type &) noexcept {
            return *this;
        }
    };
#endif

}  // namespace vidre

#endif  // VIDRE_CORE_LOG_HH