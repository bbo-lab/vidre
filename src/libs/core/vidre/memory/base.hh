/* VidRe
 * Copyright (C) 2026 Max-Planck-Institute f. Neurobiol. of Behavior — caesar, Bonn, Germany
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
#ifndef VIDRE_CORE_MEMORY_BASE_HH
#define VIDRE_CORE_MEMORY_BASE_HH
#pragma once

/**@addtogroup api-core-memory-base
 * @details
 * @fileinfo
 * */

#include <vidre/core/config.hh>
#include <vidre/cpp/util.hh>

#include <string_view>
#include <string>
#include <type_traits>

namespace vidre {

    namespace memory {

        /**@addtogroup api-core-memory-base
         * */
        ///@{

        /** Type of memory size. */
        using size_type = std::size_t;
        /** Type of pointer differences. */
        using difference_type = std::make_signed_t<size_type>;

        /** Default memory value type.
         * @note VidRe fundamental purpose is to record and process image data,
         *       which is commonly represented as byte arrays. Therefore, the default
         *       value type for memory operations is set to uint8_t, which represents
         *       an unsigned byte.
         * */
        using value_type = uint8_t;

        /** Pointer to memory. */
        using pointer = value_type *;
        /** Pointer to const memory. */
        using const_pointer = const value_type *;

        /** Check if type is a string type.
         * */
        template <typename T>
        concept string_type = std::same_as<std::remove_cvref_t<T>, std::string_view> || std::same_as<std::remove_cvref_t<T>, const char *>;

        /** Memory view.
         *
         * Akin to the STL's std::string_view or std::span, this class provides a non-owning
         * view on a contiguous memory region.
         *
         * */
        class VIDRE_CORE_VISIBILITY view {
        public:
            constexpr view() = default;

            constexpr view(view &&other) = default;
            constexpr view &operator=(view &&other) = default;
            constexpr view(const view &other) = default;
            constexpr view &operator=(const view &other) = default;

            constexpr view(size_type n, pointer ptr) noexcept : size_(n), data_(ptr) {}
            constexpr view(size_type n, const_pointer ptr) noexcept : size_(n), data_(ptr) {}

            view(size_type n, const char *ptr) noexcept : size_(n), data_(reinterpret_cast<const_pointer>(ptr)) {}
            template <std::size_t N>
            view(const char (&arr)[N]) noexcept : size_(N - 1), data_(reinterpret_cast<const_pointer>(arr)) {}
            view(const std::string_view &str) noexcept : size_(str.size()), data_(reinterpret_cast<const_pointer>(str.data())) {}
            view(const std::string &str) noexcept : size_(str.size()), data_(reinterpret_cast<const_pointer>(str.data())) {}

            template <typename T>
            requires (not string_type<T> and std::contiguous_iterator<T>)
            view(const T &value) noexcept : size_(sizeof(T)), data_(reinterpret_cast<pointer>(&value)) {}

        protected:
            size_type size_{};
            const_pointer data_{};

        public:
            /** Get size of stored data.
             * */
            constexpr size_type size() const noexcept {
                return size_;
            }

            /** Get pointer to data.
             * */
            ///@{
            constexpr const_pointer data() const noexcept {
                return data_;
            }

            constexpr operator const_pointer() const noexcept {
                return data_;
            }

            constexpr operator const void *() const noexcept {
                return static_cast<const void *>(data_);
            }
            ///@}
        };

        /** Writable memory view.
         * This class extends the basic view by allowing write access to the underlying memory.
         * It is designed for scenarios where mutable access to the data is required, such as
         * when processing or modifying image data in-place.
         * */
        class VIDRE_CORE_VISIBILITY area {
        public:
            constexpr area() = default;

            constexpr area(area &&other) = default;
            constexpr area &operator=(area &&other) = default;
            constexpr area(const area &other) = default;
            constexpr area &operator=(const area &other) = default;

            constexpr area(size_type n, pointer ptr) noexcept : size_(n), data_(ptr) {}

        protected:
            size_type size_{};
            pointer data_{};

        public:
            /** Get size of stored data.
             * */
            constexpr size_type size() const noexcept {
                return size_;
            }

            /** Get pointer to data.
             * */
            ///@{
            constexpr pointer data() noexcept {
                return data_;
            }

            constexpr operator pointer() noexcept {
                return data_;
            }

            constexpr const_pointer data() const noexcept {
                return data_;
            }

            constexpr operator const_pointer() const noexcept {
                return data_;
            }

            constexpr operator const void *() const noexcept {
                return static_cast<const void *>(data_);
            }

            constexpr operator void *() noexcept {
                return static_cast<void *>(data_);
            }
            ///@}
        };



        ///@}

    }  // namespace memory

}  // namespace vidre

#endif  // VIDRE_CORE_MEMORY_BASE_HH
