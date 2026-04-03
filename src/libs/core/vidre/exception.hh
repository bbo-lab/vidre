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
 * */
#include <vidre/core/config.hh>
#include <vidre/util/shared.hh>

#include <source_location>
#include <string>
#include <expected>

#if not defined(VIDRE_EXCEPTION_STORE) or defined(DOXYGEN)
/** Size of per-thread storage for exceptions.
 * @ingroup api-core-details
 * */
#define VIDRE_EXCEPTION_STORE (64UL * 1024UL)
#endif

namespace vidre {

    namespace detail {

        /** Exception details.
         * @ingroup api-core-details
         * */
        namespace exception {

            /**@addtogroup api-core-details
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
             * FIXME
             * */
            template <class T>
            struct deleter {
                void operator()(T *p) const {
                    p->~T();
                    store::deallocate(p);
                }
            };

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

                /** The message.
                 * */
                virtual const char *message() noexcept {
                    return nullptr;
                }
            };

            ///@}

        }  // namespace exception

    }  // namespace detail

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

    /** Exception and error namespace.
     * @ingroup api-core-exception
     * */
    namespace error {

        /**@addtogroup api-core-exception
         * */
        ///@{

        /** Allocator for exception handling.
         * */
        template <typename T>
        using allocator = detail::exception::allocator<T>;

        /** Standard type for error messages.
         * */
        using message = std::basic_string<char, std::char_traits<char>, detail::exception::allocator<char>>;

        class diagnostic;

        /** FIXME */
        class VIDRE_CORE_VISIBILITY info : public detail::exception::supplement {
        public:
            using shared_type = shared<detail::exception::supplement,  //
                                       detail::exception::allocator<detail::exception::supplement>>;

        public:
            info() = default;
            virtual ~info() {}

        protected:
            shared_type next_ = {};

        public:
            template <class Info>
            requires std::derived_from<Info, detail::exception::supplement>
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

                    next_ = shared_type(ptr, detail::exception::deleter<Info>());
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
            const char *message() noexcept override {
                abort_with_message("internal error: must not be used directly");
            };

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
        exception() noexcept = default;

        // exception(const exception &) noexcept = default;
        // exception &operator=(const exception &) noexcept = default;
        // exception(exception &&) noexcept = default;
        // exception &operator=(exception &&) noexcept = default;

        virtual ~exception() {}

    public:
        virtual const char *what();

    public:
        class runtime;
        class logic;
        // FIXME
    };

    /** Add supplemental information to exception.
     * */
    template <class E, class Info>
    requires (std::derived_from<E, exception> and std::derived_from<Info, error::info>)
    E &operator<<(E &e, Info &&info) noexcept {
        e.attach(std::move(info));
        return e;
    }

    /** Runtime exception.
     * */
    class exception::runtime : public virtual exception {
    public:
        runtime() noexcept = default;
        virtual ~runtime() {}

    public:
        const char *what() override;
    };

    /** Logic error.
     * */
    class exception::logic : public virtual exception {
    public:
        logic() noexcept = default;
        virtual ~logic() {}

    public:
        const char *what() override;
    };

    ///@}

    namespace error {

        /**@addtogroup api-core-exception
         * */
        ///@{

        /** FIXME */
        class VIDRE_CORE_VISIBILITY diagnostic {
        public:
            diagnostic() = delete;

            template <class E>
            requires std::derived_from<E, exception>
            diagnostic(E &e) noexcept : e_(&e) {}

        protected:
            exception *e_;

        public:
            void show() noexcept;
        };

        class VIDRE_CORE_VISIBILITY location : public virtual info {
        public:
            location(const std::source_location &src = std::source_location::current()) noexcept : src_(src) {}
            virtual ~location() {}

        protected:
            std::source_location src_{};

        public:
            const char *tag() noexcept override {
                return "source";
            }

            /** The message.
             * */
            const char *message() noexcept override;
        };

        ///@}

    }  // namespace error

}  // namespace vidre

#endif  // VIDRE_CORE_EXCEPTION_HH
