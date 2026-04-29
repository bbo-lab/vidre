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
#include "chrono.hh"

using namespace vidre;

chrono::isodate chrono::isodate::make(clock::time_point tp) noexcept {
    isodate result;

    // Convert to a year-month-day representation
    auto days = std::chrono::floor<std::chrono::days>(tp);
    auto ymd = std::chrono::year_month_day(days);
    // auto time_of_day = tp - days;

    // // Extract components
    // auto year = static_cast<int>(ymd.year());
    // auto month = static_cast<unsigned>(ymd.month());
    // auto day = static_cast<unsigned>(ymd.day());

    // auto hour = std::chrono::duration_cast<std::chrono::hours>(time_of_day);
    // time_of_day -= hour;
    // auto minute = std::chrono::duration_cast<std::chrono::minutes>(time_of_day);
    // time_of_day -= minute;
    // auto second = std::chrono::duration_cast<std::chrono::seconds>(time_of_day);
    // time_of_day -= second;
    // auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(time_of_day);

    // // Format directly
    // std::format_to_n(result.iso, sizeof(result.iso) - 1, "{:04d}{:02d}{:02d}T{:02d}{:02d}{:02d}.{:03d}Z", year, month, day,
    //                  hour.count(), minute.count(), second.count(), ms.count());

    // result.iso[31] = '\0';
    return result;
}

// static std::chrono::system_clock::time_point from(const char* s, size_t n) noexcept {
//     std::string_view sv(s, n);

//     // Parse with milliseconds (optional)
//     std::chrono::sys_time<std::chrono::milliseconds> tp;

//     // Try with milliseconds first
//     std::from_chars_result result;
//     int year, month, day, hour, minute, second, ms = 0;

//     // Use std::from_chars for each component (most efficient)
//     auto parse_comp = [&](auto& val, size_t pos, size_t len) -> bool {
//         auto [ptr, ec] = std::from_chars(sv.data() + pos, sv.data() + pos + len, val);
//         return ec == std::errc();
//     };

//     if (parse_comp(year, 0, 4) && parse_comp(month, 4, 2) && parse_comp(day, 6, 2) &&
//         sv[8] == 'T' &&
//         parse_comp(hour, 9, 2) && parse_comp(minute, 11, 2) && parse_comp(second, 13, 2)) {

//         size_t pos = 15;
//         if (pos < sv.size() && sv[pos] == '.') {
//             pos++;
//             auto [ptr, ec] = std::from_chars(sv.data() + pos, sv.data() + pos + 3, ms);
//             if (ec == std::errc()) {
//                 int digits = ptr - (sv.data() + pos);
//                 while (digits++ < 3) ms *= 10;
//                 pos += 3;
//             }
//         }

//         if (pos < sv.size() && sv[pos] == 'Z') {
//             using namespace std::chrono;
//             return sys_days{year{year}/month{month}/day{day}} +
//                    hours{hour} + minutes{minute} + seconds{second} + milliseconds{ms};
//         }
//     }

//     return {};
// }

chrono::clock::time_point chrono::isodate::parse(const std::string_view &) {
    // TODO implement parsing from ISO 8601 date string (see
    // https://chat.deepseek.com/a/chat/s/53fa0e95-8e7a-46a4-9170-7ea1fa20f055)

    throw exception::logic() << error::message("invalid format");

    return chrono::clock::now();
}