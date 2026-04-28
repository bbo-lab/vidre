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

chrono::isodate chrono::isodate::now() noexcept {
    isodate result;

    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);

    std::tm utc_tm;
#ifdef _WIN32
    gmtime_s(&utc_tm, &time_t_now);
#else
    gmtime_r(&time_t_now, &utc_tm);
#endif

    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count() % 1000;

    std::format_to_n(result.iso, sizeof(result.iso) - 1, "{:04d}{:02d}{:02d}T{:02d}{:02d}{:02d}.{:03d}Z", utc_tm.tm_year + 1900,
                     utc_tm.tm_mon + 1, utc_tm.tm_mday, utc_tm.tm_hour, utc_tm.tm_min, utc_tm.tm_sec, ms);

    result.iso[31] = '\0';
    return result;
}