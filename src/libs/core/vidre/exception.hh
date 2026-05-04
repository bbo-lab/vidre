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
 * Within VidRe, exceptions are used for transporting terse error reporting
 * information, and, therefore, should not be used in fast code paths (use
 * @c std::optional and @c std::expected there).
 *
 * @{
 * @namespace vidre::error Exception and errors.
 * @defgroup api-core-exception-details Implementation details.
 * @}
 * */
#include <vidre/core/config.hh>
#include <vidre/util/shared.hh>

#include <source_location>
#include <system_error>
#include <string>
#include <expected>

#if not defined(VIDRE_EXCEPTION_STORE) or defined(DOXYGEN)
/** Size of per-thread storage for exceptions.
 * @ingroup api-core-exception-details
 * */
#define VIDRE_EXCEPTION_STORE (64UL * 1024UL)
#endif

namespace vidre {

    namespace error {

        /** Implementation details namespace.
         * @ingroup api-core-exception-details
         * */
        namespace detail {

            /**@addtogroup api-core-exception-details
             * */
            ///@{

            /** Thread-local storage for exceptions.
             * Even though we're wasting @c VIDRE_EXCEPTION_STORE bytes per
             * thread - this store should make exceptions mostly usable even under
             * out-of-memory conditions.
             * */
            class VIDRE_CORE_VISIBILITY store {
            public:
                using pointer = void *;
                using const_pointer = const void *;
                using size_type = std::size_t;

                static constexpr size_type granularity = sizeof(std::ptrdiff_t);

                struct entry {
                    entry *next;
                    size_type size;

                    entry() = default;
                    entry(pointer ptr, size_type sz) : next(reinterpret_cast<entry *>(ptr)), size(sz) {}

                    const char *addr() const noexcept {
                        return reinterpret_cast<const char *>(this);
                    }

                    const char *data() const noexcept {
                        return addr() + sizeof(entry);
                    }
                };

                static constexpr size_type max_available = VIDRE_EXCEPTION_STORE - sizeof(entry);

            public:
                store();

            protected:
                static thread_local size_type used_;
                static thread_local char mem_[VIDRE_EXCEPTION_STORE];

            public:
                /** Report free storage.
                 * @note This is for development only.
                 * */
                static size_type free() noexcept {
                    return (max_available - used_) * (used_ <= max_available);
                }

                /** Allocate @a n bytes.
                 * */
                static pointer allocate(size_t n) noexcept;

                /** Deallocate previously allocated pointer @a ptr.
                 * */
                static void deallocate(pointer ptr) noexcept;
            };

            /** Standard allocator.
             * @internal
             * With friendly help from DeepSeek.
             * @endinternal
             * */
            template <class T>
            struct allocator {
                using value_type = T;

                // For rebinding to other types (C++17 and later can omit this)
                template <typename U>
                struct rebind {
                    using other = allocator<U>;
                };

                // Default constructor (required)
                allocator() noexcept = default;

                // Copy constructor (required)
                template <typename U>
                allocator(const allocator<U> &) noexcept {}

                // Allocate memory
                T *allocate(std::size_t n) {
                    if (n > max_size()) {
                        throw std::bad_alloc();
                    }

                    // Call your static memory allocation function
                    void *ptr = store::allocate(n * sizeof(T));
                    if (!ptr) {
                        throw std::bad_alloc();
                    }
                    return static_cast<T *>(ptr);
                }

                // Deallocate memory
                void deallocate(T *ptr, std::size_t n) noexcept {
                    // Call your static deallocation function
                    store::deallocate(static_cast<typename store::pointer>(ptr));
                }

                // Optional: max_size
                std::size_t max_size() const noexcept {
                    return static_cast<std::size_t>(-1) / sizeof(T);
                }

                // Optional: equality operators
                friend bool operator==(const allocator &, const allocator &) noexcept {
                    return true;  // All instances are equivalent
                }

                friend bool operator!=(const allocator &, const allocator &) noexcept {
                    return false;
                }
            };

            template <typename T, typename U>
            bool operator==(const allocator<T> &, const allocator<U> &) noexcept {
                return true;
            }

            /** Deleter.
             * */
            template <class T>
            struct deleter {
                void operator()(T *p) const {
                    p->~T();
                    store::deallocate(p);
                }
            };

            ///@}

        }  // namespace detail

        /**@addtogroup api-core-exception
         * */
        ///@{

        /** Allocator for exception handling.
         * */
        template <typename T>
        using allocator = error::detail::allocator<T>;

        /** Standard type for error messages.
         * */
        using string = std::basic_string<char, std::char_traits<char>, error::detail::allocator<char>>;

        class diagnostic;

        ///@}

    }  // namespace error

    namespace error {

        namespace detail {

            /**@addtogroup api-core-exception-details
             * */
            ///@{

            /** Supplemental information for exceptions.
             * */
            class VIDRE_CORE_VISIBILITY supplement {
            public:
                supplement() = default;
                virtual ~supplement() {}

            public:
                /** Individual tag.
                 * */
                virtual const char *tag() noexcept {
                    return nullptr;
                }

                /** Make the message.
                 * */
                virtual void make(vidre::error::string &) noexcept {}
            };

            ///@}

        }  // namespace detail

    }  // namespace error

    class exception;

    /**@addtogroup api-core-exception
     * */
    ///@{

    /** Abort program with a message.
     * */
    [[noreturn]] void abort_with_message(const char *msg);

    ///@}

    /** Representation of two values: an expected value of type T, or an unexpected value of type E.
     * @see <a href="https://en.cppreference.com/w/cpp/utility/expected.html">std::expected on CppReference</a>
     * */
    template <class T, class E>
    using expected = std::expected<T, E>;

    /** Representation of an unexpected value as stored in @ref vidre::expected.
     * @see <a href="https://en.cppreference.com/w/cpp/utility/expected/unexpected.html">std::unexpected on CppReference</a>
     * */
    template <class E>
    using unexpected = std::unexpected<E>;

    namespace error {

        /**@addtogroup api-core-exception
         * */
        ///@{

        /** Base class for supplemental exception info.
         * */
        class VIDRE_CORE_VISIBILITY info : public error::detail::supplement {
        public:
            using shared_type = shared<error::detail::supplement,  //
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

    /**@addtogroup api-core-exception
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

        /**@addtogroup api-core-exception
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

#endif  // VIDRE_CORE_EXCEPTION_HH
