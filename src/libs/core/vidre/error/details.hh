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
#ifndef VIDRE_CORE_ERROR_DETAILS_HH
#define VIDRE_CORE_ERROR_DETAILS_HH
#pragma once

#include <vidre/core/config.hh>

#include <new>
#include <string>
#include <atomic>
#include <functional>

#if not defined(VIDRE_EXCEPTION_STORE) or defined(DOXYGEN)
/** Size of per-thread storage for exceptions.
 * @ingroup api-core-error-details
 * */
#define VIDRE_EXCEPTION_STORE (64UL * 1024UL)
#endif

namespace vidre {

    namespace error {

        /** Implementation details namespace.
         * @ingroup api-core-error-details
         * */
        namespace detail {

            /**@addtogroup api-core-error-details
             * */
            ///@{

            /** Shared ownership smart pointer with custom allocator support.
             *
             * Implements reference-counted shared ownership similar to std::shared_ptr,
             * but with full control over memory allocation through a custom allocator.
             * The control block (reference counts + object) is allocated as a single
             * contiguous block using the `rebind`ed allocator type.
             *
             * @tparam T The type of the managed object
             * @tparam Allocator The allocator type for both the control block and object
             *
             * ## Strong vs. Weak References
             *
             * This implementation currently supports **strong references** only. A complete
             * implementation would also include weak references:
             *
             * ### Strong References (this class)
             * - Keep the managed object alive
             * - Control block is destroyed when the LAST strong reference dies
             * - Can access the managed object directly (operator->, operator*)
             * - Example: `shared<Foo> sp1 = make_shared<Foo>();` // strong count = 1
             *
             * ### Weak References (not yet implemented)
             * - Observe the object without keeping it alive
             * - Don't prevent object destruction
             * - Must be converted to strong reference before access (lock())
             * - Prevent control block deallocation even after object destruction
             * - Example: `weak<Foo> wp = sp1;` // weak count = 1, strong count unchanged
             *
             * ## Reference Count Behavior
             *
             * ```
             * Strong count > 0: Object is alive and accessible
             * Strong count = 0: Object is destroyed, control block may persist if weak count > 0
             * Both counts = 0: Control block is deallocated
             * ```
             *
             * ## Memory Layout
             *
             * ```
             * ┌─────────────────────────────────────┐
             * │  counted_object (control block)     │
             * ├─────────────────────────────────────┤
             * │  std::atomic<size_t> strong_count   │
             * │  std::atomic<size_t> weak_count     │
             * │  T object (constructed in-place)    │
             * └─────────────────────────────────────┘
             * ```
             *
             * The control block and object are allocated as one unit, improving cache
             * locality and reducing allocation overhead compared to separate allocations.
             *
             * ## Thread Safety
             *
             * - Reference count operations are atomic and thread-safe
             * - Access to the managed object is NOT automatically thread-safe
             * - Concurrent reads of the same object are safe if the object allows it
             * - Concurrent reads and writes require external synchronization
             *
             * ## Usage Example
             *
             * ```cpp
             * // Using default allocator
             * auto sp1 = make_shared<std::string>("Hello");
             *
             * // Using custom static allocator
             * using MyAlloc = StaticAllocator<std::string>;
             * auto sp2 = make_shared<std::string, MyAlloc>("World");
             *
             * // Reference counting behavior
             * auto sp3 = sp2;           // Strong count: 2
             * std::cout << sp3.use_count(); // Prints: 2
             *
             * sp3.reset();              // Strong count: 1 (object still alive)
             * sp2.reset();              // Strong count: 0 (object destroyed)
             * ```
             *
             * @note Weak reference support would require:
             *       - A separate `weak<T>` class
             *       - `lock()` method to obtain strong reference
             *       - `expired()` to check if object is gone
             *       - Proper handling of weak count in control block destruction
             *
             * @see make_shared Factory function for convenient creation
             * @see std::shared_ptr Standard equivalent with more features
             *
             * @internal
             * Implementation notes:
             * - Uses empty base optimization via stateless allocator when possible
             * - Memory ordering: relaxed for increments (no synchronization needed),
             *   acq_rel for decrements (ensures proper cleanup visibility)
             * - The allocator is stored by value (copied) - suitable for stateless allocators
             *
             * With friendly help from DeepSeek (code and documentation).
             * @endinternal
             *
             * @todo Code/documentation copied (only refactored names) directly from DeepSeek, needs audit!
             * @todo This will only be ever used here, there's a lot to distill away...
             * */
            template <typename T, typename Allocator>
            class error_shared {
            // note: With C++-26, std::shared_ptr will support heterogeneous allocators.
            public:
                using allocator_type = Allocator;

            private:
                struct counted_object {
                    std::atomic<size_t> strong_count;
                    std::atomic<size_t> weak_count;  // currently unused, would allow a weak implementation
                    T *object;

                    counted_object(T *ptr) : strong_count(1), weak_count(0), object(ptr) {}
                };

                using CountedObjectAllocator = typename std::allocator_traits<Allocator>::template rebind_alloc<counted_object>;

                counted_object *object_{};

                Allocator allocator{};
                std::function<void(T *)> deleter;

            private:
                // Helper to destroy the counted object
                void destroy_counted_object() noexcept {
                    if (object_) {
                        // Manually destroy the object and counted object
                        deleter(object_->object);
                        object_->~counted_object();

                        // Deallocate memory using the allocator
                        CountedObjectAllocator counted_object_allocator(allocator);
                        counted_object_allocator.deallocate(object_, 1);
                    }
                }

                void release() noexcept {
                    if (!object_)
                        return;

                    // Decrement strong count
                    size_t old_count = object_->strong_count.fetch_sub(1, std::memory_order_acq_rel);

                    if (old_count == 1) {
                        // No more strong references, destroy the object
                        destroy_counted_object();
                    }
                }

            public:
                error_shared() noexcept {}

                error_shared(T *ptr) noexcept {
                    deleter = [allocator = allocator](T *ptr) mutable {
                        ptr->~T();
                        allocator.deallocate(ptr, 1);
                    };

                    CountedObjectAllocator counted_object_allocator(allocator);
                    object_ = counted_object_allocator.allocate(1);

                    new (object_) counted_object(ptr);
                }

                template <class Y, class Deleter>
                requires std::derived_from<Y, T>
                error_shared(Y *ptr, Deleter d) noexcept {
                    deleter = [d = std::move(d)](T *ptr) mutable {
                        d(dynamic_cast<Y *>(ptr));
                    };

                    CountedObjectAllocator counted_object_allocator(allocator);
                    object_ = counted_object_allocator.allocate(1);

                    new (object_) counted_object(ptr);
                }

                error_shared(const error_shared &other) noexcept : object_(other.object_), allocator(other.allocator), deleter(other.deleter) {
                    if (object_) {
                        object_->strong_count.fetch_add(1, std::memory_order_relaxed);
                    }
                }

                error_shared(error_shared &&other) noexcept
                    : object_(other.object_), allocator(std::move(other.allocator)), deleter(other.deleter) {
                    other.object_ = nullptr;
                }

                error_shared &operator=(const error_shared &other) noexcept {
                    if (this != &other) {
                        // Decrement current reference
                        release();

                        // Copy other
                        object_ = other.object_;
                        allocator = other.allocator;
                        deleter = other.deleter;

                        if (object_) {
                            object_->strong_count.fetch_add(1, std::memory_order_relaxed);
                        }
                    }
                    return *this;
                }

                error_shared &operator=(error_shared &&other) noexcept {
                    if (this != &other) {
                        release();

                        object_ = other.object_;
                        allocator = std::move(other.allocator);
                        deleter = other.deleter;

                        other.object_ = nullptr;
                    }
                    return *this;
                }

                ~error_shared() {
                    release();
                }

                void swap(error_shared &other) noexcept {
                    std::swap(object_, other.object_);
                    std::swap(allocator, other.allocator);
                    std::swap(deleter, other.deleter);
                }

            public:
                T *operator->() noexcept {
                    return object_->object;
                }

                const T *operator->() const noexcept {
                    return object_->object;
                }

                T &operator*() noexcept {
                    return *object_->object;
                }

                const T &operator*() const noexcept {
                    return *object_->object;
                }

                T *get() noexcept {
                    return object_ ? object_->object : nullptr;
                }

                const T *get() const noexcept {
                    return object_ ? object_->object : nullptr;
                }

                size_t use_count() const noexcept {
                    return object_ ? object_->strong_count.load(std::memory_order_acquire) : 0;
                }

                bool unique() const noexcept {
                    return use_count() == 1;
                }

                void reset() noexcept {
                    release();
                    object_ = nullptr;
                }
            };

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
                static pointer allocate(std::size_t n) noexcept;

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

        /**@addtogroup api-core-error-exceptions
         * */
        ///@{

        /** Allocator for exception handling.
         * */
        template <typename T>
        using allocator = error::detail::allocator<T>;

        /** Standard type for error messages.
         * */
        using string = std::basic_string<char, std::char_traits<char>, error::detail::allocator<char>>;

        // forward declaration
        class diagnostic;

        ///@}

        namespace detail {

            /**@addtogroup api-core-error-details
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

}  // namespace vidre

#endif  // VIDRE_CORE_ERROR_DETAILS_HH
