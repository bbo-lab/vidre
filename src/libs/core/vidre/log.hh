/* VidRe
 * Copyright (C) 2025-2026 Max-Planck-Institute f. Neurobiol. of Behavior — caesar, Bonn, Germany
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
 * FIXME note about thread-safety, etc.
 *
 * @note VidRe in general only uses a single newline character ('\\n') to
 *       emit or determine an end-of-line (see also @ref log::endl).
 *
 * The logging interface also allows to stream its output to different locations
 * (see @c log::io::stream) with styling (see @c log::io::style) where each
 * stream may have it's own log threshold (see @c log::threshold).
 *
 * By default, all logging is sent to @c stderr (id 0).
 *
 * Example (see @ref examples/core/log.cc):
 * @include examples/core/log.cc
 *
 * @{
 * @namespace vidre::log
 * @}
 * */
/**@defgroup api-core-log-if Interface
 * @ingroup api-core-log
 * @brief General logging.
 *
 * The general log interface used within the framework follows the convention:
 * @code
 * log::LEVEL() << "message output" << log::endl;
 * @endcode
 * where @c LEVEL may be any of the existing @ref log::level "log levels" and may
 * be used as if @c stderr would be used directly.
 *
 * Special conditions apply to the following levels:
 * - @ref log::fatal terminates the program when the object is destructed.
 * - @ref log::debug is only supported (i.e. functional) when initially compiled
 *        without @c NDEBUG.
 * - @ref log::trace is only available when explicitly compiled with @c DEBUG.
 *
 * */
/**@defgroup api-core-log-io Output
 * @ingroup api-core-log
 * @brief Log streaming facilities.
 *
 * @{
 * @namespace vidre::log::io
 * @}
 * */
/**@defgroup api-core-log-details Implementation details
 * @ingroup api-core-log
 * @brief Internal implementation details.
 *
 * @{
 * @namespace vidre::log::detail
 * @}
 * */

#include <vidre/core/config.hh>
#include <vidre/cpp/util.hh>
#include <vidre/cpp/format.hh>
#include <vidre/io/types.hh>
#include <vidre/io/packet.hh>
#include <vidre/util/chrono.hh>
#include <vidre/sync/events.hh>
#include <vidre/error.hh>

#include <ankerl/unordered_dense.h>

#include <limits>
#include <vector>
#include <string>
#include <string_view>

#if defined(DEBUG)
#include <source_location>
#include <regex>
#endif

#include <cstdio>
#include <cassert>

namespace vidre {

    namespace log {
        // forward declarations
        class trace;

        /* TODO:
        - the log system needs to be transformed to use the packet system
        - check, as std::string has the zero-termination, if we need to include it in the packet data
        - what about color output?
        */

        /**@addtogroup api-core-log-if
         * */
        ///@{

        /** Log levels.
         * @internal
         * Internally, log events are passed as "raw" data - so, the log level is
         * represented in the TLV type as minor number.
         * @see @ref vidre::storage::raw
         * @see vidre::storage::raw::version_type
         * @endinternal
         * */
        enum class level : uint16_t {
            fatal = 0,     ///< System is in an unusable state and the program will terminate.
            alert = 1,     ///< Immediate user interaction is required, usually when data corruption occurs.
            critical = 2,  ///< A critical error, such as hardware failure.
            error = 3,     ///< An application error occurred.
            warning = 4,   ///< Warning level, which may hint to future failure.
            user = 5,      ///< A user message (such as an experimental note).
            notice = 6,    ///< A noteworthy condition occurred.
            status = 7,    ///< A status message.
            info = 8,      ///< A purely informational message.
            debug = 9,     ///< Debugging information.
            trace = 10     ///< Tracing.
        };

        ///@internal
        constexpr static unsigned level_n = 11;                                      ///< Number of discernable log levels.
        constexpr static unsigned level_min = to_underlying(level::fatal);           ///< Minimum log level (as integer).
        constexpr static unsigned level_max = std::numeric_limits<uint16_t>::max();  ///< Maximum log level (as integer).
#if defined(DOXYGEN)
        constexpr static unsigned level_default = defined_at_compile_time;  ///< Default log level.
#elif defined(DEBUG)
        constexpr static unsigned level_default = to_underlying(level::debug);
#elif defined(NDEBUG)
        constexpr static unsigned level_default = to_underlying(level::notice);
#else
        constexpr static unsigned level_default = to_underlying(level::info);
#endif
        ///@endinternal

        /** End-of-line.
         * */
        constexpr static std::string_view endl = {"\n", 1};

        /**@internal Indentation.
         * */
        constexpr static std::string_view indent_s = {"    ", 4};

        /** Log-message packet.
         * @note In contrast to other packets, the log packet is not "groupable",
         *       instead, the minor packet type indicates the log level.
         * */
        class VIDRE_CORE_VISIBILITY packet : public virtual io::packet {
        protected:
            chrono::timestamp timestamp = chrono::timestamp::now();  ///< Timestamp.
                                                                     /// @see chrono::reference
            std::string message{};                                   ///< Log message (variable length).

        public:
            packet() = delete;
            packet(const packet &) = delete;
            packet &operator=(const packet &) = delete;
            packet(packet &&) = default;
            packet &operator=(packet &&) = default;
            virtual ~packet() = default;

            packet(level lvl, uint16_t stream = 0) noexcept          //
                : io::packet(to_underlying(io::packet::major::log),  //
                             to_underlying(lvl), stream, sizeof(timestamp))
                , timestamp{chrono::timestamp::now()} {
                message.reserve(256);  // pre-allocate some space for the message to avoid frequent reallocations
                update();
            }

            packet(const header &hdr_) noexcept : io::packet(hdr_) {
                message.reserve(256);  // pre-allocate some space for the message to avoid frequent reallocations
            }

            template <typename... Args>
            static shared<packet> make(Args &&...args) {
                return std::make_shared<packet>(std::forward<Args>(args)...);
            }

            void update() noexcept override {
                hdr.length = sizeof(timestamp) + message.size();
            }

            io::result emit(io::sink &f) override {
                return io::packet::emit(f)                                                           //
                       + f.put(timestamp)                                                            //
                       + f.put({message.size(), reinterpret_cast<const uint8_t *>(message.data())})  //
                       + pad(f);
            }

            /** Get log level.
             * @returns The log level of this message.
             * */
            template <std::endian E = std::endian::native>
            log::level getlevel() const noexcept {
                return static_cast<log::level>(hdr.type.minor);
            }

        private:
            friend class handler;
        };

        /** Log message output.
         * This class represents the implementation of the log message packet
         * source (fan out), but also provides a static interface for setting
         * global log parameters.
         *
         * @see io::source
         * */
        class VIDRE_CORE_VISIBILITY output : public virtual io::source {
        public:
            /** Global maximum threshold level for log messages.
             * This threshold determines the maximum log level that will be processed.
             * Messages with a level higher than this threshold will be ignored.
             *
             * For DEBUG builds, the default is `level::trace`, for NDEBUG builds
             * the default is `level::notice`, otherwise it is `level::status`.
             * */
            static log::level maxthreshold;

        public:
            output() = default;

            output(const output &) = delete;
            output &operator=(const output &) = delete;
            output(output &&) = delete;
            output &operator=(output &&) = delete;

            virtual ~output() = default;

        protected:
            void send(shared<packet> msg) noexcept;

        public:
            cap getcap() const noexcept override {
                return {cap::packet | cap::multi, {}};
            }

        protected:
            status attach(shared<io::sink> other, cap::flag_type cap) override;
            status detach(const io::sink *snk) override;

        private:
            friend class handler;
            friend class trace;
        };

        ///@}

        /**@addtogroup api-core-log-details
         * */
        ///@{

        /** Log message functor concept.
         * @tparam Stream The output stream type.
         * @tparam T The functor type.
         * */
        template <typename Stream, typename T>
        concept functor = requires (Stream &outs, T &&t) {
            { t.operator()(outs) } -> std::same_as<Stream &>;
        } or requires (Stream &outs, T &&t) {
            { t(outs) } -> std::same_as<Stream &>;
        };

        /** Log message handler.
         *
         * This class handles all log messages. For readability it is recommended to use
         * the individual implementations (see @c log::fatal, etc.).
         *
         * Typical usage:
         * @code{.cpp}
         * log::error() << "An error occurred.";
         * @endcode
         * or
         * @code{.cpp}
         * log::error err;
         * err << "An error occurred.\n";
         * err << "And this is what went wrong: ...\n";
         * @endcode
         *
         * @note The log message (or packet, respectively) is emitted when log object goes out of scope.
         * */
        class VIDRE_CORE_VISIBILITY handler {
        public:
            handler() = delete;

            handler(const handler &) = delete;
            handler &operator=(const handler &) = delete;
            handler(handler &&) = delete;
            handler &operator=(handler &&) = delete;

            ~handler() {
                emit();
            }

            /** Constructor.
             * @param lvl The log level.
             * */
            handler(level lvl) noexcept : msg_{packet::make(lvl, 0)} {};

        protected:
            shared<packet> msg_{};  ///< Log message packet.

        protected:
            /** Packet emission method.
             * This method is responsible for emitting the log message packet.
             * It is called automatically when the log handler goes out of scope.
             * */
            void emit() noexcept;

            /** Write message to the packet buffer.
             * @param msg The message to write.
             * */
            void write(const std::string_view &msg) noexcept {
                msg_->message += msg;
            }

        public:
            handler &operator<<(const std::string_view &msg) noexcept {
                write(msg);
                return *this;
            }

            template <typename T>
            requires functor<handler, T>
            handler &operator<<(T &&fn) noexcept {
                return fn(*this);
            }

            template <typename V>
            requires (not std::is_convertible_v<V, std::string_view> and std::formattable<std::remove_cvref_t<V>, char>)
            handler &operator<<(const V &value) noexcept {
                msg_->message += std::format("{}", value);
                return *this;
            }
        };

        /** Helper for log-level classes.
         * @ingroup api-core-log-details */
#define VIDRE_CORE_LOG_IMPL(T)                       \
    class VIDRE_CORE_VISIBILITY T : public handler { \
    public:                                          \
        T() noexcept : handler(level::T) {}          \
    };

        /** Null sink.
         * @ingroup api-core-log-details */
        class VIDRE_CORE_VISIBILITY null {
        public:
            null() = default;
            template <typename V>
            null &operator<<(const V &value) noexcept {
                return *this;
            }
        };

        ///@}

        /**@addtogroup api-core-log-if
         * */
        ///@{

        /** Log message source.
         * */
        extern shared<output> source;

        /** Fatal log message.
         * @note
         *      The destructor here calls @c std::terminate.
         * */
        class VIDRE_CORE_VISIBILITY fatal : public handler {
        public:
            fatal() noexcept : handler(log::level::fatal) {}
            [[noreturn]] ~fatal() {
                std::terminate();
            }
        };

        /** Alert (requiring user interaction).
         * @see log::level::alert */
        VIDRE_CORE_LOG_IMPL(alert);
        /** Critical condition (such as hardware failure).
         * @see log::level::critical */
        VIDRE_CORE_LOG_IMPL(critical);
        /** Errors.
         * @see log::level::error */
        VIDRE_CORE_LOG_IMPL(error);
        /** Warnings.
         * @see log::level::warning */
        VIDRE_CORE_LOG_IMPL(warning);
        /** Noteworthy condition.
         * @see log::level::notice */
        VIDRE_CORE_LOG_IMPL(notice);
        /** Status message.
         * @see log::level::status */
        VIDRE_CORE_LOG_IMPL(status);
        /** Purely informational.
         * @see log::level::info */
        VIDRE_CORE_LOG_IMPL(info);

#if defined(NDEBUG) or defined(DOXYGEN)
        /** Debug message.
         * */
        class VIDRE_CORE_VISIBILITY debug {
        public:
            debug() = default;

            template <typename V>
            debug &operator<<(const V &value) noexcept {
                return *this;
            }
        };
#else
        VIDRE_CORE_LOG_IMPL(debug);
#endif

#if defined(DEBUG) or defined(DOXYGEN)
        /** Log trace.
         *
         * Tracing is costly and should be used judiciously; it is only enabled when
         * the @c DEBUG macro is defined during compilation.
         *
         * @note In contrast to the default handling, trace messages (or packets) are
         *       emitted per line.
         *
         * @see pattern()
         * */
        class VIDRE_CORE_VISIBILITY trace : public handler {
        protected:
            /**@internal Internal stack when tracing.
             * */
            static thread_local std::vector<std::source_location> stack;

            /**@internal Pattern for tracing.
             * */
            static std::regex match;

            /**@internal Source filename.
             * */
            static std::string_view file_name() noexcept {
                std::string_view name = stack.back().file_name();
                auto pos = name.find("vidre/");
                if (pos == std::string_view::npos)
                    return name;
                return {name.data() + pos, name.size() - pos};
            }

            /**@internal Source line. */
            static auto line() noexcept {
                return stack.back().line();
            }

            /**@internal Source column. */
            static auto column() noexcept {
                return stack.back().column();
            }

            /**@internal Source function. */
            static auto function_name() noexcept {
                return stack.back().function_name();
            }

        public:
            /** Set wildcard pattern (what to trace).
             * The wildcard pattern is matched with the calling functions
             * source file name. If matched, anything called from there has
             * active tracing.
             * */
            static void pattern(std::string_view pat);

        public:
            trace(std::source_location source = std::source_location::current()) noexcept;
            ~trace();

        protected:
            std::string message_;     ///< Temporary message buffer.
            std::size_t follow_ = 0;  ///< Indentation depth (0 counts as not traced).

        protected:
            void emit(bool last = false) noexcept;

            void emit(const std::string_view &txt) noexcept;

        public:
            trace &operator<<(const std::string_view &msg) noexcept {
                emit(msg);
                return *this;
            }

            template <typename V>
            requires (not std::is_convertible_v<V, std::string_view> and std::formattable<std::remove_cvref_t<V>, char>)
            trace &operator<<(const V &value) noexcept {
                emit(std::format("{}", value));
                return *this;
            }
        };
#else
        class VIDRE_CORE_VISIBILITY trace : public null {
        public:
            static void pattern(std::string_view pat) {}
        };
#endif

        /** Setup log streams.
         *
         * This command intended for use with command line options.
         *
         * @todo show example
         * */
        void setup(const std::string &def);

        ///@}

        // /** Output style.
        //  * */
        // struct style {
        //     /** Style enumeration.
        //      * */
        //     enum type : uint8_t {
        //         plain,   ///< Plain, undecorated output.
        //         tagged,  ///< As the name suggests (i.e. first line tag, following lines indented).
        //         color    ///< Colored output (uses terminal escape sequences).
        //     };
        // };

    }  // namespace log

}  // namespace vidre

#endif  // VIDRE_CORE_LOG_HH
