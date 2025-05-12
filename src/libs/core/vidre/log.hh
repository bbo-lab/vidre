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
 * Example (see @ref examples/core/log.cc):
 * @include examples/core/log.cc
 *
 * */

#include <vidre/core/config.hh>
#include <vidre/cpp/util.hh>

#include <limits>
#include <iostream>
#include <vector>

#include <cassert>

namespace vidre {

    /**@addtogroup api-core-log
     * */
    ///@{

    /** Message logging interface.
     *
     * This provides an interface for program-flow and general message logging.
     *
     * By default, log messages are emitted to @c std::clog but may also be recorded
     * in a file or a temporary buffer (see @c log::stream for further information).
     *
     * Typical usage (see @c log::level):
     * @code
     * log::warning() << "A warning.";
     * @endcode
     *
     * */
    class VIDRE_CORE_VISIBILITY log {
    public:
        class stream;

        /** Type for log message decorration (or ornament, respectively).
         * */
        enum class fret {
            plain,  ///< No decorration.
            color,  ///< Uses coloring for log messages (usually only on an interactive terminal).
            tag     ///< Uses tags (such as "[ERROR]") per log entry.
        };

        using message_type = std::string_view;  ///< String (or message, respectively) type.

        /** Log levels.
         *
         * The log levels are represented as individual classes,
         * so instead of using
         * @code
         * log(log::level::warning) << "A warning.";
         * @endcode
         * users must use
         * @code
         * log::warning() << "A warning.";
         * @endcode
         * which provide additional functionality (see esp. @c log::fatal and @c log::trace .
         *
         * */
        enum class level : int {
            fatal = -5,     ///< System is in an unusable state and the program will terminate.
            alert = -4,     ///< Immediate user interaction is required, usually when data corruption occurs.
            critical = -3,  ///< A critical error, such as hardware failure.
            error = -2,     ///< An application error occurred.
            warning = -1,   ///< Warning level, which may hint to future failure.
            notice = 0,     ///< A noteworthy condition occurred (also default log level).
            status = 1,     ///< A status message.
            info = 2,       ///< A purely informational message.
            debug = 3,      ///< Debugging information.
            trace = 4       ///< Tracing.
        };

        /**@internal Number of discernable log levels.
         * */
        constexpr static unsigned level_n = 10;

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

        /**@internal Maximum number of streams.
         * */
        constexpr static std::size_t max_streams = 128;

        /**@internal Minimum log level (as integer). */
        constexpr static int min_level = static_cast<int>(level::fatal);

        /**@internal Maximum log level (as integer).
         *
         * @note
         *      Tracing (which is only enabled for DEBUG builds) allows nesting, which, in turn,
         *      allows the respective log formatter to indent the trace.
         *
         * */
        constexpr static int max_level = std::numeric_limits<int>::max();

        /**@internal Default log level.
         * */
#if defined(DEBUG)
        constexpr static int default_level = max_level;
#elif defined(NDEBUG)
        constexpr static int default_level = 0;
#else
        constexpr static int default_level = static_cast<int>(level::info);
#endif

        /** End-of-line.
         * */
        constexpr static message_type endl = message_type({"\n", 1});

    public:
        template <typename T>
        class scope;

        /**@internal Add a stream.
         * @note add()/remove() do not handle life-time!
         * */
        template <typename T>
        requires std::derived_from<T, stream>
        static unsigned add(T &s) noexcept {
            assert(stream_.size() < max_streams);
            stream_.push_back(&s);
            return stream_.size() - 1;
        }

        /**@internal Remove a stream (by id).
         * */
        static void remove(unsigned id) noexcept;

        /**@internal Adjust stream threshold.
         * */
        static void setthreshold(unsigned id, level threshold) noexcept;

    protected:
        static std::vector<stream *> stream_;  ///< Global log streams.

    public:
        log() = default;

    protected:
        log(level lvl) noexcept : level_(static_cast<int>(lvl)) {}

    protected:
        int level_ = 0;      ///< Log level (for this logging object).
        bool first_ = true;  ///< True for first line of output, false otherwise.

    public:
        /** Emit a log message.
         * @param msg The log text.
         * @param depth Indentation depth (must only be used by @c log::trace).
         * */
        void emit(const message_type &msg, unsigned depth = 0) noexcept;

    private:
        friend class stream;
    };

    /** Log stream.
     *
     * FIXME
     *
     * */
    class VIDRE_CORE_VISIBILITY log::stream {
    public:
        /**@internal Per log-level decoration.
         * @see log::fret
         * */
        struct fret {
            message_type first;   ///< First line prefix.
            message_type prefix;  ///< Line prefix.
            message_type suffix;  ///< Line suffix.
        };

        /**@internal Number of predefined frets.
         * @see log::fret
         * */
        constexpr static std::size_t fret_n = 3;

        /**@internal The decorations (type/level).
         * @see log::fret
         * */
        static const fret *frets[fret_n];

    public:
        stream() = default;
        stream(level lvl) noexcept : threshold_{static_cast<int>(lvl)} {}
        stream(log::fret decor) noexcept : fret_{static_cast<int>(decor)} {}
        stream(level lvl, log::fret decor) noexcept : threshold_{static_cast<int>(lvl)}, fret_{static_cast<int>(decor)} {}
        virtual ~stream() = default;

    protected:
        int threshold_ = log::default_level;  ///< Log threshold for this stream.
        int fret_ = 0;                        ///< Decoration for this stream.

    public:
        /** Get this streams log-level threshold.
         * */
        int threshold() const noexcept {
            return threshold_;
        }

        /** Set this streams log-level threshold.
         * */
        void setthreshold(level threshold) noexcept {
            threshold_ = static_cast<int>(threshold);
        }

        /** Write text to this log stream.
         * */
        virtual void write(const message_type &msg) noexcept {
            std::clog << msg;
        }

    private:
        friend class log;
    };

    ///@cond
    inline void log::setthreshold(unsigned id, level threshold) noexcept {
        if (id == 0)
            return;
        else if (id >= stream_.size())
            return;
        else
            stream_[id]->setthreshold(threshold);
    }
    ///@endcond

    /** Scoped log streaming.
     *
     * FIXME
     *
     * */
    template <typename T>
    class VIDRE_CORE_VISIBILITY log::scope {
    public:
        template <typename... Args>
        scope(Args &&...args) : stream_(args...) {
            id_ = log::add(&stream_);
        }
        ~scope() {
            log::remove(id_);
        }

    protected:
        T stream_;
        unsigned id_;
    };

    ///@}

    /** Fatal log message.
     * */
    class VIDRE_CORE_VISIBILITY log::fatal : log {
    public:
        fatal() noexcept : log(log::level::fatal) {}
        ~fatal() {
            std::terminate();
        }

        fatal &operator<<(const message_type &msg) noexcept {
            emit(msg);
            return *this;
        }

        template <typename V>
        requires std::formattable<std::remove_cvref_t<V>, char>
        fatal &operator<<(const V &value) noexcept {
            emit(std::format("{}", value));
            return *this;
        }
    };

    ///@cond
#define VIDRE_CORE_LOG_IMPL(T)                                  \
    class VIDRE_CORE_VISIBILITY log::T : log {                  \
    public:                                                     \
        T() noexcept : log(log::level::T) {}                    \
        T &operator<<(const message_type &msg) noexcept {       \
            emit(msg);                                          \
            return *this;                                       \
        }                                                       \
        template <typename V>                                   \
        requires std::formattable<std::remove_cvref_t<V>, char> \
        T &operator<<(const V &value) noexcept {                \
            emit(std::format("{}", value));                     \
            return *this;                                       \
        }                                                       \
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
        debug &operator<<(const message_type &) noexcept {
            return *this;
        }
        template <typename V>
        requires std::formattable<std::remove_cvref_t<V>, char>
        fatal &operator<<(const V &value) noexcept {
            return *this;
        }
    };
#else
    VIDRE_CORE_LOG_IMPL(debug);
#endif

    ///@endcond

#if defined(DEBUG) or defined(DOXYGEN)
    /** Log trace.
     *
     * FIXME
     *
     * */
    class VIDRE_CORE_VISIBILITY log::trace : log {
    public:
        trace() noexcept {}
        trace &operator<<(const message_type &msg) noexcept {
            emit(msg);
            return *this;
        }
        template <typename V>
        requires std::formattable<std::remove_cvref_t<V>, char>
        trace &operator<<(const V &value) noexcept {
            emit(std::format("{}", value));
            return *this;
        }
    };
#else
    class VIDRE_CORE_VISIBILITY log::trace : log {
    public:
        trace() noexcept {}
        trace &operator<<(const message_type &) noexcept {
            return *this;
        }
        template <typename V>
        requires std::formattable<std::remove_cvref_t<V>, char>
        trace &operator<<(const V &value) noexcept {
            return *this;
        }
    };
#endif

}  // namespace vidre

#endif  // VIDRE_CORE_LOG_HH
