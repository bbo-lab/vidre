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

#include <cstring>

using namespace vidre;

chrono::isodate chrono::isodate::make(clock::time_point tp) noexcept {
    isodate result;
    
    // Get current UTC time directly
    auto now = std::chrono::utc_clock::to_sys(tp);
    
    // Convert to a year-month-day representation
    auto days = std::chrono::floor<std::chrono::days>(now);
    auto ymd = std::chrono::year_month_day{days};
    auto time_of_day = now - days;
    
    // Extract components
    auto year = static_cast<int>(ymd.year());
    auto month = static_cast<unsigned>(ymd.month());
    auto day = static_cast<unsigned>(ymd.day());
    
    auto hour = std::chrono::duration_cast<std::chrono::hours>(time_of_day);
    time_of_day -= hour;
    auto minute = std::chrono::duration_cast<std::chrono::minutes>(time_of_day);
    time_of_day -= minute;
    auto second = std::chrono::duration_cast<std::chrono::seconds>(time_of_day);
    time_of_day -= second;
    auto us = std::chrono::round<std::chrono::microseconds>(time_of_day);
    
    // Format directly
    std::format_to_n(result.iso, sizeof(result.iso), 
                     "{:04d}{:02d}{:02d}T{:02d}{:02d}{:02d}.{:06d}Z",
                     year, month, day, 
                     hour.count(), minute.count(), second.count(), 
                     us.count());
    
    return result;
}

chrono::clock::time_point chrono::isodate::parse(const std::string_view &sv) {
    if (sv.size() < 15) return {};
    
    auto parse_int = [&](size_t pos, size_t len) -> std::optional<int> {
        int value = 0;
        auto [ptr, ec] = std::from_chars(sv.data() + pos, sv.data() + pos + len, value);
        if (ec != std::errc()) return std::nullopt;
        return value;
    };
    
    // parse components
    auto y = parse_int(0, 4);
    auto m = parse_int(4, 2);
    auto d = parse_int(6, 2);
    
    if (!y or !m or !d or sv[8] != 'T') return {};
    
    auto H = parse_int(9, 2);
    auto M = parse_int(11, 2);
    auto S = parse_int(13, 2);
    
    if (!H or !M or !S) return {};
    
    // parse microseconds
    int us = 0;
    size_t pos = 15;
    if (pos < sv.size() && sv[pos] == '.') {
        pos++;
        auto [ptr, ec] = std::from_chars(sv.data() + pos, sv.data() + pos + 6, us);
        if (ec == std::errc()) {
            // if fewer than 6 digits, scale up to microseconds
            int digits = ptr - (sv.data() + pos);
            while (digits++ < 6) us *= 10;
            pos += 6;
        }
    }
    
    // expected trailing 'Z'
    if (pos >= sv.size() || sv[pos] != 'Z') return {};
    
    // Construct time_point using C++20 chrono
    std::chrono::year_month_day ymd{std::chrono::year{*y}, std::chrono::month{(unsigned) *m}, std::chrono::day{(unsigned) *d}};
    auto tp = std::chrono::sys_days{ymd} + std::chrono::hours{*H} + std::chrono::minutes{*M} + 
              std::chrono::seconds{*S} + std::chrono::microseconds{us};
    
    return std::chrono::utc_clock::from_sys(tp);
}