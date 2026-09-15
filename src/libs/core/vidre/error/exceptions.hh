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
#ifndef VIDRE_CORE_ERROR_EXCEPTIONS_HH
#define VIDRE_CORE_ERROR_EXCEPTIONS_HH
#pragma once

#include <vidre/core/config.hh>
#include <vidre/error/details.hh>
#include <vidre/error/flow.hh>

#include <source_location>
#include <system_error>
#include <string>

namespace vidre {

    // forward declaration
    class exception;

    namespace error {

        /**@addtogroup api-core-error-exceptions
         * */
        ///@{

        /** Base class for supplemental exception info.
         * */
        class VIDRE_CORE_VISIBILITY info : public error::detail::supplement {
        public:
            using shared_type = detail::error_shared<error::detail::supplement,  //
                                                     error::detail::allocator<error::detail::supplement>>;

        public:
            info() = default;
            virtual ~info() {}

        protected:
            shared_type next_ = {};

        public:
            template <class Info>
            requires std::derived_from<Info, error::detail::supplement>
            void attach(Info &&inf) {
                if (next_.get()) {
                    auto nextptr = dynamic_cast<info *>(next_.get());
                    nextptr->attach(std::move(inf));
                }
                else {
                    using Allocator = typename std::allocator_traits<shared_type::allocator_type>::template rebind_alloc<Info>;
                    Allocator allocator;

                    Info *ptr = allocator.allocate(1);
                    new (ptr) Info(std::move(inf));

                    next_ = shared_type(ptr, error::detail::deleter<Info>());
                }
            }

        public:
            /** Individual tag.
             * */
            const char *tag() noexcept override {
                // should not happen...
                return "<>";
            }

            /** The message.
             * */
            void make(error::string &) noexcept override {};

        private:
            friend class diagnostic;
        };

        ///@}

    }  // namespace error

    /**@addtogroup api-core-error-exceptions
     * */
    ///@{

    /** General exception.
     * */
    class VIDRE_CORE_VISIBILITY exception : public virtual error::info {
    public:
        exception(const std::source_location &src = std::source_location::current()) noexcept;
        virtual ~exception() {}

    public:
        virtual const char *what() const noexcept;

        exception &operator<<(const char *msg) noexcept;

    public:
        class logic;
        class internal;
        class runtime;
        class argument;
        class memory;
    };

    /** Add supplemental information to exception.
     * */
    template <class E, class Info>
    requires (std::derived_from<E, exception> and std::derived_from<Info, error::info>)
    E operator<<(E &&e, Info &&info) noexcept {
        e.attach(std::move(info));
        return std::move(e);
    }

    template <class E, class Info>
    requires (std::derived_from<E, exception> and std::derived_from<Info, error::info>)
    E &operator<<(E &e, Info &&info) noexcept {
        e.attach(std::move(info));
        return e;
    }

    /** Logic error.
     * */
    class VIDRE_CORE_VISIBILITY exception::logic : public virtual exception {
    public:
        logic(const std::source_location &src = std::source_location::current()) noexcept : exception(src) {}
        virtual ~logic() {}

    public:
        const char *what() const noexcept override;
    };

    /** Internal error.
     * */
    class VIDRE_CORE_VISIBILITY exception::internal : public virtual exception {
    public:
        internal(const std::source_location &src = std::source_location::current()) noexcept : exception(src) {}
        virtual ~internal() {}

    public:
        const char *what() const noexcept override;
    };

    /** Runtime exception.
     * */
    class VIDRE_CORE_VISIBILITY exception::runtime : public virtual exception {
    public:
        runtime(const std::source_location &src = std::source_location::current()) noexcept : exception(src) {}
        virtual ~runtime() {}

    public:
        const char *what() const noexcept override;
    };

    /** Argument error.
     * */
    class VIDRE_CORE_VISIBILITY exception::argument : public virtual exception::runtime {
    public:
        argument(const std::source_location &src = std::source_location::current()) noexcept : runtime(src) {}
        virtual ~argument() {}

    public:
        const char *what() const noexcept override;
    };

    /** Out-of-memory error.
     * */
    class VIDRE_CORE_VISIBILITY exception::memory : public virtual exception::runtime {
    public:
        memory(const std::source_location &src = std::source_location::current()) noexcept : runtime(src) {}
        virtual ~memory() {}

    public:
        const char *what() const noexcept override;
    };

    ///@}

    namespace error {

        /**@addtogroup api-core-error-exceptions
         * */
        ///@{

        /** Exception diagnostic.
         * */
        class VIDRE_CORE_VISIBILITY diagnostic {
        public:
            diagnostic() = delete;

            template <class E>
            requires std::derived_from<E, exception>
            diagnostic(E &e) noexcept : e_(&e) {}

        protected:
            exception *e_;

        public:
            string dump() noexcept;
        };

        /** Source code location info.
         * */
        class VIDRE_CORE_VISIBILITY location : public virtual info {
        public:
            location(const std::source_location &src = std::source_location::current()) noexcept
                : function_(src.function_name()), filename_(src.file_name()), line_(src.line()), column_(src.column()) {}

            location(const char *filename, uint_least32_t line = 0, uint_least32_t column = 0) noexcept
                : filename_(filename), line_(line), column_(column) {}

            location(const char *function, const char *filename, uint_least32_t line = 0, uint_least32_t column = 0) noexcept
                : function_(function), filename_(filename), line_(line), column_(column) {}

            virtual ~location() {}

        protected:
            string function_{};
            string filename_{};
            uint_least32_t line_{};
            uint_least32_t column_{};

        public:
            const char *tag() noexcept override {
                return "source";
            }

            /** The message.
             * */
            void make(error::string &) noexcept override;
        };

        /** Error message.
         * */
        class VIDRE_CORE_VISIBILITY message : public virtual info {
        public:
            message(const char *msg) noexcept : msg_(msg) {}
            message(const std::string &msg) noexcept : msg_(msg) {}
            message(const string &msg) noexcept : msg_(msg) {}
            virtual ~message() {}

        protected:
            string msg_{};

        public:
            const char *tag() noexcept override {
                return "message";
            }

            /** The message.
             * */
            void make(error::string &emsg) noexcept override;
        };

        /** General error code.
         * */
        class VIDRE_CORE_VISIBILITY code : public virtual info {
        public:
            code(std::error_code ec) noexcept : ec_(ec) {}
            virtual ~code() {}

        protected:
            std::error_code ec_{};

        public:
            const char *tag() noexcept override {
                return "code";
            }

            /** The message.
             * */
            void make(error::string &emsg) noexcept override;
        };

        /** Generic error.
         * */
        class VIDRE_CORE_VISIBILITY errc : public virtual code {
        public:
            errc(std::errc ec) noexcept : code(std::make_error_code(ec)) {}
            virtual ~errc() {}

        public:
            const char *tag() noexcept override {
                return "errc";
            }
        };

        /** System error.
         * */
        class VIDRE_CORE_VISIBILITY system : public virtual code {
        public:
#if VEDRE_HOST_WINDOWS
            system() noexcept : code(std::error_code(GetLastError(), std::system_category())) {}
#else
            system() noexcept : code(std::make_error_code(static_cast<std::errc>(errno))) {}
#endif
            virtual ~system() {}

        public:
            const char *tag() noexcept override {
                return "system";
            }
        };

        ///@}

    }  // namespace error

    inline exception &exception::operator<<(const char *msg) noexcept {
        return *this << error::message(msg);
    }

    inline exception::exception(const std::source_location &src) noexcept {
#if defined(VIDRE_DEVELOP)
        *this << error::location(src);
#endif
    }

}  // namespace vidre

#endif  // VIDRE_CORE_ERROR_EXCEPTIONS_HH
