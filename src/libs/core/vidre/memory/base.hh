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
#include <vidre/error.hh>

#include <string_view>
#include <string>
#include <type_traits>
#include <cstring>
#include <algorithm>
#include <cassert>

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
        concept string_type =
            std::same_as<std::remove_cvref_t<T>, std::string_view> || std::same_as<std::remove_cvref_t<T>, const char *>;

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
            view(const char (&arr)[N]) noexcept : size_(N), data_(reinterpret_cast<const_pointer>(arr)) {}
            view(const std::string_view &str) noexcept : size_(str.size()), data_(reinterpret_cast<const_pointer>(str.data())) {}
            view(const std::string &str) noexcept : size_(str.size()), data_(reinterpret_cast<const_pointer>(str.data())) {}

            template <typename T>
            requires (not std::is_same_v<std::remove_cvref_t<T>, view> and
                      not std::is_same_v<std::remove_cvref_t<T>, std::string_view> and
                      not std::is_same_v<std::remove_cvref_t<T>, const char *> and not std::contiguous_iterator<T> and
                      std::is_trivially_copyable_v<T>)
            view(const T &value) noexcept : size_(sizeof(T)), data_(reinterpret_cast<const_pointer>(&value)) {}

        protected:
            size_type size_{};
            const_pointer data_{};

        public:
            /** Get size of stored data.
             * */
            constexpr size_type size() const noexcept {
                return size_;
            }

            constexpr const_pointer begin() const noexcept {
                return data_;
            }
            constexpr const_pointer end() const noexcept {
                return data_ + size_;
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

            /** Advance the view by a given offset.
             * @param offset The number of bytes to advance the view.
             * @return The updated view.
             * */
            view &operator+=(size_type offset) noexcept {
                if (offset > size_)
                    offset = size_;
                data_ += offset;
                size_ -= offset;
                return *this;
            }
        };

        /** Get new memory view with an offset.
         * @param v The original memory view.
         * @param offset The number of bytes to offset the view.
         * @return A new memory view starting at the given offset.
         * */
        inline view operator+(const view &v, size_type offset) noexcept {
            view result = v;
            result += offset;
            return result;
        }

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

        /** Area-or-view concept.
         * This concept checks if a type is either `area` or `view`.
         * */
        template <typename T>
        concept area_or_view = std::same_as<std::remove_cvref_t<T>, area> || std::same_as<std::remove_cvref_t<T>, view>;

        /** Copy data from source to destination.
         * @tparam Src The type of the source, which must be either `area` or `view`.
         * @param source The source memory view or area.
         * @param destination The destination memory area.
         * @return The number of bytes copied.
         *
         * This function copies data from the source memory view or area to the
         * destination memory area, up to the minimum of their sizes.
         *
         * */
        template <area_or_view Src>
        inline size_type copy(const Src &source, area destination) noexcept {
            const auto n = std::min(source.size(), destination.size());
            std::memcpy(destination.data(), source.data(), n);
            return n;
        }

        /** Byte-wise copy from a source area or view to a trivially copyable destination.
         * @tparam Src The type of the source, which must be either `area` or `view`.
         * @tparam Dst The type of the destination, which must be trivially copyable.
         * @param source The source memory area or view.
         * @param destination The destination object.
         * @return The number of bytes copied.
         *
         * This function copies data from the source memory area or view to the
         * trivially copyable destination object.
         *
         * @pre source.size() >= sizeof(Dst)
         * */
        template <area_or_view Src, typename Dst>
        requires (std::is_trivially_copyable_v<Dst> and not area_or_view<Dst>)
        inline size_type copy(const Src &source, Dst &destination) noexcept {
            assert(source.size() >= sizeof(Dst) && "source area or view is too small for the destination type");
            std::memcpy(&destination, source.data(), sizeof(Dst));
            return sizeof(Dst);
        }

        /** Byte-wise copy a trivially copyable source to a destination area.
         * @tparam T The type of the source, which must be trivially copyable.
         * @param source The source object.
         * @param destination The destination memory area.
         * @return The number of bytes copied.
         *
         * This function copies data from the trivially copyable source object to the
         * destination memory area.
         *
         * @pre destination.size() >= sizeof(T)
         * */
        template <typename T>
        requires (std::is_trivially_copyable_v<T> and not area_or_view<T>)
        inline size_type copy(const T &source, area destination) noexcept {
            assert(destination.size() >= sizeof(T) && "destination area is too small for the source type");
            std::memcpy(destination.data(), &source, sizeof(T));
            return sizeof(T);
        }

        ///@}

    }  // namespace memory

}  // namespace vidre

#endif  // VIDRE_CORE_MEMORY_BASE_HH
