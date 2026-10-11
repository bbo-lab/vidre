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
#ifndef VIDRE_CORE_UTIL_CHRONO_HH
#define VIDRE_CORE_UTIL_CHRONO_HH
#pragma once

/**@defgroup api-core-util-chrono Time utilities
 * @ingroup api-core-util
 * @details
 * @fileinfo
 *
 * Vidre, in general, uses UTC as the standard time reference, accompanied by a monotonic
 * clock for timestamping data. Here we provide utilities, based on the standard C++ chrono
 * library, to work with both UTC and monotonic time points.
 *
 * */

#include <vidre/core/config.hh>
#include <vidre/memory/base.hh>
#include <vidre/error.hh>

#include <chrono>

namespace vidre {

    /** Time utilities.
     * @ingroup api-core-util-chrono
     * */
    namespace chrono {

        class reference;

        /**@addtogroup api-core-util-chrono
         * */
        ///@{

        /** Default clock (UTC).
         * */
        using clock = std::chrono::utc_clock;

        /** Convert UTC to local clock time.
         * */
        inline auto local(const clock::time_point &t) noexcept {
            return std::chrono::zoned_time{std::chrono::current_zone(), clock::to_sys(t)};
        }

        /** Monotonic clock.
         *
         * The "monotonic" clock is used for measuring intervals and durations accurately,
         * independent of changes to the system clock.
         *
         * @see chrono::timestamp
         * @see chrono::reference
         * */
        using monotonic = std::chrono::steady_clock;

        /** Representation of a "timestamp".
         *
         * Timestamps are represented as a combination of seconds and nanoseconds,
         * allowing for high-resolution time measurements. The `now()` function
         * provides a convenient way to obtain the current time as a timestamp,
         * using a high-resolution, monotonic clock to ensure accuracy and consistency
         * across different platforms.
         *
         * @note Timestamps do NOT necessarily represent wall-clock time, but
         *       rather a monotonic time point that is useful for measuring intervals
         *       and durations.
         *
         * @see chrono::reference
         * */
        struct __vidre_packed__(8) VIDRE_CORE_VISIBILITY timestamp {
            uint64_t sec{};   ///< Seconds.
            uint64_t nsec{};  ///< Nanoseconds past the current second.

            operator memory::view() const noexcept {
                return {sizeof(timestamp), reinterpret_cast<const uint8_t *>(&sec)};
            }

            /** Get system representation as a monotonic time-point.
             * */
            operator monotonic::time_point() const noexcept {
                return monotonic::time_point{std::chrono::seconds{sec} + std::chrono::nanoseconds{nsec}};
            }

            /** Get current time as a timestamp.
             * @returns A timestamp (taken from a high-resolution, monotonic clock).
             * */
            static timestamp now() noexcept {
                auto now = monotonic::now();

                auto sec = std::chrono::time_point_cast<std::chrono::seconds>(now);
                auto nsec = std::chrono::duration_cast<std::chrono::nanoseconds>(now - sec);

                return {static_cast<uint64_t>(sec.time_since_epoch().count()),  //
                        static_cast<uint64_t>(nsec.count())};
            }

            /** Get the corresponding UTC time related to the given reference.
             * @see chrono::reference
             * */
            clock::time_point to_utc(const reference &) const noexcept;

            /** Compare two timestamps.
             * @returns A comparison result indicating the ordering of the timestamps.
             * */
            auto operator<=>(const timestamp &) const noexcept = default;
        };

        /** UTC time.
         *
         * This struct represents UTC time as seconds and nanoseconds, using a binary 
         * representation analogous to POSIX timespec, suitable for serialization.
         *
         * @note The official UTC epoch is 1 January 1972. The underlying [`std::chrono::utc_clock`](https://en.cppreference.com/cpp/chrono/utc_clock) 
         *       uses 1 January 1970 instead to be consistent with the POSIX time.
         *
         * */
        struct __vidre_packed__(8) VIDRE_CORE_VISIBILITY utctime {
            int64_t sec{};    ///< Seconds (since epoch, UTC).
            uint64_t nsec{};  ///< Nanoseconds past the current second.

            utctime() = default;
            utctime(const utctime &) = default;
            utctime &operator=(const utctime &) = default;
            utctime(utctime &&) = default;
            utctime &operator=(utctime &&) = default;

            utctime(const clock::time_point &tp) noexcept {
                auto sec_ = std::chrono::time_point_cast<std::chrono::seconds>(tp);
                auto nsec_ = std::chrono::duration_cast<std::chrono::nanoseconds>(tp - sec_);

                sec = static_cast<int64_t>(sec_.time_since_epoch().count());
                nsec = static_cast<uint64_t>(nsec_.count());
            }

            operator memory::view() const noexcept {
                return {sizeof(utctime), reinterpret_cast<const uint8_t *>(&sec)};
            }

            /** Get the current UTC timestamp.
             */
            static utctime now() noexcept {
                return clock::now();
            }

            /** Get system representation as a time point.
             * */
            operator clock::time_point() const noexcept {
                return clock::time_point(std::chrono::seconds(sec) + std::chrono::nanoseconds(nsec));
            }

            bool operator<=>(const utctime &) const noexcept = default;
        };

        /** Representation of an ISO 8601 date (UTC).
         *
         * @note The textual representation follows the fixed ISO 8601 format "YYYYMMDDTHHMMSS.SSSSSSZ".
         *
         * */
        struct __vidre_packed__(8) VIDRE_CORE_VISIBILITY isodate {
            char iso[32]{};  ///< ISO 8601 date string ("YYYYMMDDTHHMMSS.SSSSSSZ", padded).

            operator std::string_view() const noexcept {
                return std::string_view(iso, sizeof("YYYYMMDDTHHMMSS.SSSSSSZ") - 1);
            }

            operator memory::view() const noexcept {
                return {sizeof(isodate), reinterpret_cast<const uint8_t *>(&iso)};
            }

            /** Get current time as an ISO 8601 date string.
             * @returns An ISO 8601 date string representing the current time in UTC.
             * */
            static isodate make(clock::time_point tp) noexcept;

            /** Now.
             * @returns The current time as an ISO 8601 date string.
             * */
            static isodate now() noexcept {
                return make(clock::now());
            }

            /** Parse an ISO 8601 date string.
             * */
            static clock::time_point parse(const std::string_view &);

            /** Get the time point represented by this ISO 8601 date string.
             * @returns A time point corresponding to the ISO 8601 date string.
             * */
            operator clock::time_point() const noexcept {
                return parse(iso);
            }

            /** Comparison.
             * @param other The other ISO 8601 date to compare with.
             * @returns The result of the three-way comparison between this and the other ISO 8601 date.
             * */
            auto operator<=>(const isodate &other) const noexcept {
                return static_cast<clock::time_point>(*this) <=> static_cast<clock::time_point>(other);
            }
        };

        /** Clock reference.
         *
         * This class, storing both, wall UTC time and monotonic time, establishes
         * a relationship between these two time domains on the _local_ machine. 
         *
         * It should be noted that this reference also allows aligning time measurments
         * from different machines, assuming their clocks are reasonably synchronized,
         * though for precise alignment, additional synchronization mechanisms may be 
         * required.
         *
         * */
        class VIDRE_CORE_VISIBILITY reference {
        public:
            reference() = default;

            reference(const reference &) = default;
            reference &operator=(const reference &) = default;
            reference(reference &&) = default;
            reference &operator=(reference &&) = default;

            reference(clock::time_point clock, monotonic::time_point monotonic) noexcept : clock_(clock), monotonic_(monotonic) {}
            reference(utctime utc, timestamp ts) noexcept : clock_(utc), monotonic_(ts) {}
            reference(isodate iso, timestamp ts) noexcept : clock_(iso), monotonic_(ts) {}

            ~reference() = default;

        protected:
            clock::time_point clock_{};          ///< Reference point for UTC time.
            monotonic::time_point monotonic_{};  ///< Reference point for monotonic time.

        public:
            /** "Set" the reference clock.
             *
             * This sets the reference clock to the current time.
             *
             * */
            void set() noexcept {
                clock_ = clock::now();
                monotonic_ = monotonic::now();
            }

            operator clock::time_point() const noexcept {
                return clock_;
            }

            operator monotonic::time_point() const noexcept {
                return monotonic_;
            }

            /** Get the reference clock.
             * @tparam T The clock type to return (either `chrono::clock` or `chrono::monotonic`).
             * @returns The reference clock of the specified type.
             * */
            template <class T>
            requires (std::same_as<T, clock> or std::same_as<T, monotonic>)
            auto get() const noexcept;
        };

        template <>
        inline auto reference::get<clock>() const noexcept {
            return clock_;
        }

        template <>
        inline auto reference::get<monotonic>() const noexcept {
            return monotonic_;
        }

        inline clock::time_point timestamp::to_utc(const reference &ref) const noexcept {
            return ref.get<clock>() - ref.get<monotonic>().time_since_epoch() + std::chrono::seconds(sec) +
                   std::chrono::nanoseconds(nsec);
        }

        /** Simple timer.
         * */
        class VIDRE_CORE_VISIBILITY timer {
        public:
            using clock = monotonic;
            using time_point = clock::time_point;

            using nanoseconds = std::chrono::duration<double, std::nano>;
            using microseconds = std::chrono::duration<double, std::micro>;
            using milliseconds = std::chrono::duration<double, std::milli>;
            using seconds = std::chrono::duration<double>;

        public:
            timer(const timer &) = delete;
            timer &operator=(const timer &) = delete;
            timer(timer &&) = default;
            timer &operator=(timer &&) = default;

            timer(bool start = false) noexcept {
                if (start)
                    this->start();
            }

        protected:
            time_point start_{};  ///< Start time.
            time_point stop_{};   ///< Stop time.

        public:
            /** Start (or continue) the timer.
             * */
            timer &start() {
                start_ = clock::now();
                return *this;
            }

            /** Stop the timer.
             * */
            timer &stop() {
                stop_ = clock::now();
                return *this;
            }

            /** Resume the timer.
             * */
            timer &resume() {
                start_ = clock::now() - (stop_ - start_);
                return *this;
            }

            /** Get the elapsed time since the timer was started.
             * @tparam Duration The duration type to return (default: microseconds).
             * */
            template <class Duration = microseconds>
            double elapsed() const noexcept {
                return Duration(clock::now() - start_).count();
            }

            /** Get the duration between the start and stop times.
             * @tparam Duration The duration type to return (default: microseconds).
             * */
            template <class Duration = microseconds>
            double duration() const noexcept {
                return Duration(stop_ - start_).count();
            }

            template <class Duration = microseconds, class T>
            requires (std::integral<T> or std::floating_point<T>)
            double duration(T n) const noexcept {
                return duration<Duration>() / static_cast<double>(n);
            }
        };

        ///@}

    }  // namespace chrono

}  // namespace vidre

///@cond
#include <ostream>
namespace std {
    inline ostream &operator<<(ostream &os, const vidre::chrono::isodate &date) {
        // ISO 8601 format with milliseconds always takes exactly 28 characters
        // plus null terminator = 29 total (YYYYMMDDTHHMMSS.SSSZ)
        // Write the exact length to avoid scanning for null terminator
        return os.write(date.iso, 28);  // 28 characters, no null terminator
    }
}  // namespace std
///@endcond

#endif  //  VIDRE_CORE_UTIL_CHRONO_HH
