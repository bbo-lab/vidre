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
#ifndef VIDRE_CORE_UTIL_SHARED_HH
#define VIDRE_CORE_UTIL_SHARED_HH
#pragma once

/**@defgroup api-core-util-shared Shared data structures.
 * @ingroup api-core-util
 * @details
 * @fileinfo
 *
 * */

#include <vidre/core/config.hh>

#include <atomic>
#include <cstddef>
#include <type_traits>
#include <memory>
#include <functional>

namespace vidre {

    /**@addtogroup api-core-util-shared
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
     * @todo Code/documentatoin copied (only refactored names) directly from DeepSeek, needs audit!
     * */
    template <typename T, typename Allocator = std::allocator<T>>
    class shared {
    public:
        using allocator_type = Allocator;

    private:
        /** Internal object representation.
         * */
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

    public:
        shared() noexcept {}

        shared(T *ptr) noexcept {
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
        shared(Y *ptr, Deleter d) noexcept {
            deleter = [d = std::move(d)](T *ptr) mutable {
                d(dynamic_cast<Y *>(ptr));
            };

            CountedObjectAllocator counted_object_allocator(allocator);
            object_ = counted_object_allocator.allocate(1);

            new (object_) counted_object(ptr);
        }

        shared(const shared &other) noexcept : object_(other.object_), allocator(other.allocator), deleter(other.deleter) {
            if (object_) {
                object_->strong_count.fetch_add(1, std::memory_order_relaxed);
            }
        }

        shared(shared &&other) noexcept : object_(other.object_), allocator(std::move(other.allocator)), deleter(other.deleter) {
            other.object_ = nullptr;
        }

        shared &operator=(const shared &other) noexcept {
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

        shared &operator=(shared &&other) noexcept {
            if (this != &other) {
                release();

                object_ = other.object_;
                allocator = std::move(other.allocator);
                deleter = other.deleter;
                
                other.object_ = nullptr;
            }
            return *this;
        }

        ~shared() {
            release();
        }

    public:
        T *operator->() noexcept {
            return &object_->object;
        }

        const T *operator->() const noexcept {
            return &object_->object;
        }

        T &operator*() noexcept {
            return object_->object;
        }

        const T &operator*() const noexcept {
            return object_->object;
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

        void swap(shared &other) noexcept {
            std::swap(object_, other.object_);
            std::swap(allocator, other.allocator);
            std::swap(deleter, other.deleter);
        }

    private:
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
    };

    /** Factory function for creating shared objects.
     *
     * Constructs a shared object of type T with the given allocator and arguments.
     * Memory for the control block and object is allocated using the `rebind`ed
     * allocator type.
     *
     * @tparam T The type of object to create
     * @tparam Allocator The allocator type (deduced from parameter if provided)
     * @tparam Args The argument types for T's constructor
     * @param args Arguments to forward to T's constructor
     * @return A shared object managing the newly created instance
     *
     * @example
     * ```cpp
     * // With default allocator
     * auto p1 = make_shared<MyClass>(42, "example");
     *
     * // With custom allocator
     * auto p2 = make_shared<MyClass, StaticAllocator<MyClass>>(42, "example");
     *
     * // Allocator with state (if needed)
     * MyAllocator alloc(some_state);
     * auto p3 = make_shared<MyClass, MyAllocator>(42, "example");
     * // Note: allocator is copied, not moved
     * ```
     *
     * @note The allocator parameter is currently not taken as a function argument.
     *       To pass a stateful allocator, consider this alternative:
     *       ```cpp
     *       shared<T, MyAllocator> p(allocator_arg, alloc, args...);
     *       ```
     */
    // FIXME
    // template <typename T, typename Allocator = std::allocator<T>, typename... Args>
    // shared<T, Allocator> make_shared(Args &&...args) {
    //     return shared<T, Allocator>(std::forward<Args>(args)...);
    // }

    ///@}

}  // namespace vidre

#endif  //
