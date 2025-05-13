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
 * VidRe provides a program-flow and message logging interface that supports
 * several severity levels (including nested tracing):
 *
 * - @c log::fatal (which also terminates the program),
 * - @c log::alert,
 * - @c log::critical,
 * - @c log::error,
 * - @c log::warning,
 * - @c log::notice,
 * - @c log::status,
 * - @c log::info,
 * - @c log::debug, and,
 * - @c log::trace,
 *
 * (the latter are compile-time enabled only if NDEBUG is not defined and
 * DEBUG is defined, respectively, at compile-time). These classes my be used
 * for single or block-wise output, such as
 * @code
 * log::notice() << "A notice.\nMore information for this notice.\n";
 * @endcode
 * vs.
 * @code
 * log::notice notice;
 * notice << "A notice.\n";
 * notice << "More information for this notice.\n";
 * @endcode
 *
 * @note VidRe in general only uses a single newline character ('\\n') to
 *      emit or determine an end-of-line (see @c log::endl).
 *
 * The logging interface also allows to stream its output to different locations
 * (see @c log::io::stream) with markup (see @c log::io::markup) where each
 * stream may have it's own log threshold (see @c log::threshold).
 *
 * By default, all logging is sent to @c std::clog (id 0).
 *
 * Example (see @ref examples/core/log.cc):
 * @include examples/core/log.cc
 *
 * @defgroup api-core-log-if Interface
 * @ingroup api-core-log
 *
 * @defgroup api-core-log-io In-/output
 * @ingroup api-core-log
 *
 * @defgroup api-core-log-details Implementation details
 * @ingroup api-core-log
 * */

#include <vidre/core/config.hh>
#include <vidre/cpp/util.hh>

#include <limits>
#include <iostream>
#include <vector>

#include <cassert>

namespace vidre {

    /** Message logging interface.
     * @ingroup api-core-log */
    namespace log {

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
        inline namespace io {
            class stream;
        }  // namespace io

        /** String (or message, respectively) type.
         * @ingroup api-core-log-details */
        using message_type = std::string_view;

        /** Log levels.
         * @ingroup api-core-log-if */
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

        constexpr static unsigned level_n = 10;                            ///<@internal Number of discernable log levels.
        constexpr static int level_min = static_cast<int>(level::fatal);   ///<@internal Minimum log level (as integer).
        constexpr static int max_level = std::numeric_limits<int>::max();  ///<@internal Maximum log level (as integer).
#if defined(DEBUG)
        constexpr static int level_default = max_level;  ///<@internal Default log level.
#elif defined(NDEBUG)
        constexpr static int level_default = 0;
#else
        constexpr static int level_default = static_cast<int>(level::info);
#endif

        /** Log message handler.
         *
         * This class handles logging and, as well, is the base class for
         * the individual implementations (see @c log::fatal, etc.);
         *
         * @ingroup api-core-log-details */
        class VIDRE_CORE_VISIBILITY handler {
        public:
            handler() = default;
            handler(level lvl) {};

        protected:
            int level_ = level_default; ///< Log level for this object.



        };

        /**@addtogroup api-core-log-if
         * */
        ///@{

        /** End-of-line.
         * */
        constexpr static message_type endl = {"\n", 1};

        ///@}

        /** Logging IO.
         * @ingroup api-core-log
         * */
        inline namespace io {

            // FIXME

        }  // namespace io

        /**@addtogroup api-core-log-if
         * */
        ///@{

        /** Fatal log message.
         * */
        class VIDRE_CORE_VISIBILITY fatal : handler {
        public:
            fatal() noexcept : handler(log::level::fatal) {}
            ~fatal() {
                std::terminate();
            }

            fatal &operator<<(const message_type &msg) noexcept {
                // emit(msg);
                return *this;
            }

            template <typename V>
            requires std::formattable<std::remove_cvref_t<V>, char>
            fatal &operator<<(const V &value) noexcept {
                // emit(std::format("{}", value));
                return *this;
            }
        };

        ///@}

        ///@cond

        ///@endcond

    }  // namespace log

#if 0
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


        /**@internal Maximum number of streams.
         * */
        constexpr static std::size_t max_streams = 128;


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
        int level_ = 0;  ///< Log level (for this logging object).

    private:
        bool first_ = true;    ///< True for first line of output, false otherwise.
        bool newline_ = true;  ///< True when last output finished with a newline.

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
     * This class implements logging to @c std::clog, but also, serves a fundament to
     * implement individual log streamers. FIXME
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
        int threshold_ = log::level_default;  ///< Log threshold for this stream.
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

#endif

}  // namespace vidre

#endif  // VIDRE_CORE_LOG_HH
