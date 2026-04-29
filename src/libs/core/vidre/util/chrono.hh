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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
/// @file
#ifndef VIDRE_CORE_UTIL_CHRONO_HH
#define VIDRE_CORE_UTIL_CHRONO_HH
#pragma once

/**@defgroup api-core-util-chrono Time utilities
 * @ingroup api-core-util
 * @details
 * @fileinfo
 * */

#include <vidre/core/config.hh>
#include <vidre/exception.hh>

#include <chrono>

namespace vidre {

    /** Time utilities.
     * @ingroup api-core-util-chrono
     * */
    namespace chrono {

        /**@addtogroup api-core-util-chrono
         * */
        ///@{

        /** Representation of a "timestamp".
         * Timestamps are represented as a combination of seconds and nanoseconds,
         * allowing for high-resolution time measurements. The `now()` function
         * provides a convenient way to obtain the current time as a timestamp,
         * using a high-resolution, monotonic clock to ensure accuracy and consistency
         * across different platforms.
         *
         * @note Timestamps do NOT necessarily represent wall-clock time, but
         *       rather a monotonic time point that is useful for measuring intervals
         *       and durations.
         * */
        struct __vidre_packed__(8) VIDRE_CORE_VISIBILITY timestamp {
            uint64_t sec{};   ///< Seconds.
            uint64_t nsec{};  ///< Nanoseconds past the current second.

            /** Get current time as a timestamp.
             * @returns A timestamp (taken from a high-resolution, monotonic clock).
             * */
            static timestamp now() noexcept {
                auto now = std::chrono::steady_clock::now();

                auto sec = std::chrono::time_point_cast<std::chrono::seconds>(now);
                auto nsec = std::chrono::duration_cast<std::chrono::nanoseconds>(now - sec);

                return {static_cast<uint64_t>(sec.time_since_epoch().count()),  //
                        static_cast<uint64_t>(nsec.count())};
            }
        };

        /** Representation of an ISO 8601 date.
         * */
        struct __vidre_packed__(8) VIDRE_CORE_VISIBILITY isodate {
            char iso[32]{};  ///< ISO 8601 date string ("YYYYMMDDTHHMMSS.SSSZ\0\0\0").

            /** Get current time as an ISO 8601 date string.
             * @returns An ISO 8601 date string representing the current time in UTC.
             * */
            static isodate now() noexcept;

            /** Parse an ISO 8601 date string.
             * @todo implement parsing from ISO 8601 date string (see
             * https://chat.deepseek.com/a/chat/s/53fa0e95-8e7a-46a4-9170-7ea1fa20f055)
             */
            static std::chrono::time_point<std::chrono::system_clock> parse(const std::string_view &);
        };

        /** Simple timer.
         * */
        class VIDRE_CORE_VISIBILITY timer {
        public:
            using clock = std::chrono::steady_clock;
            using timepoint = clock::time_point;

            using nanoseconds = std::chrono::duration<double, std::nano>;
            using microseconds = std::chrono::duration<double, std::micro>;
            using milliseconds = std::chrono::duration<double, std::milli>;
            using seconds = std::chrono::duration<double>;

        public:
            timer() = default;

        protected:
            timepoint start_{};  ///< Start time.
            timepoint stop_{};   ///< Stop time.

        public:
            /** Start (or continue) the timer.
             * */
            void start() {
                start_ = clock::now();
            }

            /** Stop the timer.
             * */
            void stop() {
                stop_ = clock::now();
            }

            /** Resume the timer.
             * */
            void resume() {
                start_ = clock::now() - (stop_ - start_);
            }

            /** Get the elapsed time since the timer was started.
             * @tparam Duration The duration type to return (default: microseconds).
             * */
            template <class Duration = microseconds>
            double elapsed() {
                return Duration(clock::now() - start_).count();
            }

            /** Get the duration between the start and stop times.
             * @tparam Duration The duration type to return (default: microseconds).
             * */
            template <class Duration = microseconds>
            double duration() {
                return Duration(stop_ - start_).count();
            }

            template <class Duration = microseconds, class T>
            requires (std::integral<T> or std::floating_point<T>)
            double duration(T n) {
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