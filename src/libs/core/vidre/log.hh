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

#include <limits>
#include <vector>

#if defined(DEBUG)
#include <source_location>
#include <regex>
#endif

#include <cstdio>
#include <cassert>

namespace vidre {

    namespace lognew {

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

        /** Log-message packet.
         * */
        struct __vidre_packed__(8) VIDRE_CORE_VISIBILITY packet : public io::packet::basic {
            chrono::timestamp timestamp{};  ///< Timestamp.

            packet(level lvl, uint16_t stream = 0) noexcept     //
                : basic(to_underlying(io::packet::major::log),  //
                        to_underlying(lvl), stream, sizeof(timestamp)) {}

            io::result put(io::sink &f) const override {
                return basic::put(f) + f.put(timestamp);
            }

            /** Get log level.
             * @returns The log level of this message.
             * */
            level log_level() const noexcept {
                return static_cast<level>(type.minor);
            }

            /** Get the log message.
             * @returns A view of the log message.
             * */
            std::string_view message() noexcept {
                return std::string_view{reinterpret_cast<const char *>(this) + sizeof(packet), length - sizeof(timestamp)};
            }
        };

        ///@}

        namespace detail {
            /**@addtogroup api-core-log-details
             * */
            ///@{

            /** Default per-thread buffer size. */
#if not defined(VIDRE_CORE_LOG_BUFFER_SIZE)
#define VIDRE_CORE_LOG_BUFFER_SIZE 32768
#endif

            /** Per-thread message store.
             * */
            class VIDRE_CORE_VISIBILITY store {
            public:
                store() = default;
                store(const store &) = delete;
                store &operator=(const store &) = delete;
                store(store &&) = default;
                store &operator=(store &&) = default;

            protected:
                static thread_local io::detail::buffer buffer_;  ///< Per-thread buffer.
                lognew::packet *ptr_{};                          // Pointer to the current packet being constructed.

            public:
                /** Make packet. */
                lognew::packet *make(level lvl, uint16_t stream = 0) noexcept {
                    if ((buffer_.size - buffer_.used) < sizeof(lognew::packet))
                        return nullptr;

                    if (ptr_) {
                        // FIXME check if terminated...
                    }
                    ptr_ = reinterpret_cast<lognew::packet *>(buffer_.data + buffer_.used);
                    new (ptr_) lognew::packet(lvl, stream);
                    buffer_.used += sizeof(lognew::packet);
                    return ptr_;
                }

                /** Finalize packet.
                 * */
                void finalize() noexcept {}
            };

            ///@}

        }  // namespace detail

    }  // namespace lognew

    // ============================================================================
    // ============================================================================
    // ============================================================================
    // ============================================================================
    // ============================================================================
    // ============================================================================
    // ============================================================================
    // ============================================================================
    // ============================================================================
    // ============================================================================
    // ============================================================================
    // ============================================================================
    // ============================================================================
    // ============================================================================
    // ============================================================================
    // ============================================================================
    // ============================================================================

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

        /**@addtogroup api-core-log-details
         * */
        ///@{

        /** String (or message, respectively) type.
         * */
        using message_type = std::string_view;

        template <typename T>
        concept convertible_to_message_type = std::is_convertible_v<T, message_type>;

        template <typename Stream, typename T>
        concept functor = requires (Stream &outs, T &&t) {
            { t.operator()(outs) } -> std::same_as<Stream &>;
        };

        ///@}

        /**@addtogroup api-core-log-if
         * */
        ///@{

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
            trace = 4       ///< Tracing.
        };

        ///@internal
        constexpr static unsigned level_n = 10;                            ///< Number of discernable log levels.
        constexpr static int level_min = static_cast<int>(level::fatal);   ///< Minimum log level (as integer).
        constexpr static int level_max = std::numeric_limits<int>::max();  ///< Maximum log level (as integer).
#if defined(DEBUG) or defined(DOXYGEN)
        constexpr static int level_default = static_cast<int>(level::debug);  ///< Default log level.
#elif defined(NDEBUG)
        constexpr static int level_default = static_cast<int>(level::notice);
#else
        constexpr static int level_default = static_cast<int>(level::info);
#endif
        ///@endinternal

        /** Log message handler.
         *
         * This class handles logging and, as well, is the base class for
         * the individual implementations (see @c log::fatal, etc.);
         *
         * @ingroup api-core-log-details */
        class VIDRE_CORE_VISIBILITY handler {
        public:
            handler() = default;
            handler(level lvl) : level_(static_cast<int>(lvl)) {};

        protected:
            int level_ = level_default;  ///< Log level for this object.

        private:
            char last_ = 0;  ///<@internal Last emitted character.

        protected:
            /** Emit text.
             * @note
             *      The message is parsed so that FIXME
             * */
            void emit(const message_type &txt, unsigned = 0) noexcept;

        public:
            handler &operator<<(const vidre::log::message_type &msg) noexcept {
                emit(msg);
                return *this;
            }

            template <typename T>
            requires functor<handler, T>
            handler &operator<<(T &&fn) noexcept {
                return fn(*this);
            }

            template <typename V>
            requires (not convertible_to_message_type<V> and std::formattable<std::remove_cvref_t<V>, char>)
            handler &operator<<(const V &value) noexcept {
                emit(std::format("{}", value));
                return *this;
            }
        };

        /** End-of-line.
         * */
        constexpr static message_type endl = {"\n", 1};

        /**@internal Indentation.
         * */
        constexpr static message_type indent_s = {"    ", 4};

        ///@}

        inline namespace io {

            /**@addtogroup api-core-log-io
             * */
            ///@{

            /** Output style.
             * */
            struct style {
                /** Style enumeration.
                 * */
                enum type : uint8_t {
                    plain,   ///< Plain, undecorated output.
                    tagged,  ///< As the name suggests (i.e. first line tag, following lines indented).
                    color    ///< Colored output (uses terminal escape sequences).
                };
            };

            /** Default log stream.
             *
             * This class implements logging to @c stderr, but also, serves a
             * base class for implementing individual log streamers.
             *
             * Developers wishing to implement their own logic should study
             * @c log::io::file.
             *
             * */
            class VIDRE_CORE_VISIBILITY stream {
            protected:
                /**@internal List of log streams.
                 * */
                static std::vector<io::stream *> channels;

                /**@internal Maximum log level in channels.
                 * */
                static int maxthreshold;

            public:
                /**@internal Add a log stream.
                 * */
                template <typename S>
                requires std::derived_from<S, stream>
                static unsigned add(S &channel) noexcept {
                    unsigned id = channels.size();
                    channels.push_back(&channel);
                    if (channel.threshold_ > maxthreshold)
                        maxthreshold = channel.threshold_;
                    return id;
                }

                /**@internal Remove a log stream by channel id.
                 * @see vidre::log::scope<>
                 * */
                static void remove(unsigned chanid) noexcept;

                /** Get threshold.
                 * */
                static level threshold(unsigned chanid) noexcept;

                /** Set threshold.
                 * */
                static void threshold(unsigned chanid, int lvl) noexcept;
                static void threshold(unsigned chanid, level lvl) noexcept {
                    threshold(chanid, static_cast<int>(lvl));
                }

            public:
                stream() = default;
                stream(level th) noexcept : threshold_(static_cast<int>(th)) {}
                stream(style::type sty) noexcept : style_(sty) {}
                stream(level th, style::type sty) noexcept : threshold_(static_cast<int>(th)), style_(sty) {}

                virtual ~stream() {
                    if (chanid_ >= 0)
                        remove(chanid_);
                }

            protected:
                int threshold_ = level_default;     ///< Log threshold for this stream.
                style::type style_ = style::plain;  ///< Output style.
                int chanid_ = -1;                   ///< Channel ID (if av.).

            public:
                /** Write text to this log stream.
                 *
                 * @note
                 *      This method only writes text to the stream. All logic
                 *      for styling is handled by the vidre::log::handler class.
                 *
                 * */
                virtual void put(const message_type &msg) noexcept;

            private:
                friend class vidre::log::handler;
                friend class vidre::log::trace;
            };

            /** File logger.
             * */
            class VIDRE_CORE_VISIBILITY file : public stream {
            public:
                file(const std::string_view filename, level th = static_cast<level>(level_default),
                     style::type sty = style::tagged) noexcept;
                virtual ~file() {
                    if (outs_ != nullptr)
                        ::fclose(outs_);
                }

            protected:
                FILE *outs_ = nullptr;

            protected:
                /** Write header.
                 *
                 * This is, at the moment of writing:
                 *
                 *      [[[=== UTC ===]]]
                 *
                 * where "UTC" is the actual ISO 8601 UTC Time.
                 */
                void header() noexcept;

            public:
                void put(const message_type &msg) noexcept override {
                    if (outs_)
                        fwrite(msg.data(), msg.size(), 1, outs_);
                }
            };

            /** Log to buffer.
             * */
            class VIDRE_CORE_VISIBILITY string : public stream {
            public:
                string(level th = static_cast<level>(level_default), style::type sty = style::color) noexcept;

            protected:
                std::string buf_ = {};

            public:
                void put(const message_type &msg) noexcept override {
                    buf_.append(msg);
                }

                /** Get buffer content.
                 * */
                message_type get() noexcept {
                    return buf_;
                }
            };

            ///@}

        }  // namespace io

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

        /** Helper for log-level classes.
         * @ingroup api-core-log-details */
#define VIDRE_CORE_LOG_IMPL(T)                       \
    class VIDRE_CORE_VISIBILITY T : public handler { \
    public:                                          \
        T() noexcept : handler(level::T) {}          \
    };

        /**@addtogroup api-core-log-if
         * */
        ///@{

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
         * Tracing is expensive but can give valuable information about the
         * program flow. Therefore,
         *
         * */
        class VIDRE_CORE_VISIBILITY trace : public handler {
        protected:
            /**@internal Internal stack when tracing.
             * */
            static std::vector<std::source_location> stack;

            /**@internal Pattern for tracing.
             * */
            static std::regex match;

            /**@internal Source filename.
             * */
            static message_type file_name() noexcept {
                message_type name = stack.back().file_name();
                auto pos = name.find("vidre/");
                if (pos == message_type::npos)
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
            std::size_t follow_ = 0;  ///< Indentation depth (0 counts as not traced).

        protected:
            void emit(const message_type &txt) noexcept {
                if (io::stream::maxthreshold < level_)
                    return;

                if (follow_ == 0)
                    return;

                handler::emit(txt, follow_ - 1);
            }

        public:
            trace &operator<<(const vidre::log::message_type &msg) noexcept {
                emit(msg);
                return *this;
            }

            template <typename V>
            requires (not convertible_to_message_type<V> and std::formattable<std::remove_cvref_t<V>, char>)
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

    }  // namespace log

}  // namespace vidre

#endif  // VIDRE_CORE_LOG_HH
