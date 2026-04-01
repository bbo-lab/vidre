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
#ifndef VIDRE_CORE_STORAGE_GENERIC_HH
#define VIDRE_CORE_STORAGE_GENERIC_HH
#pragma once

#include <vidre/core/config.hh>

namespace vidre {

    namespace storage {

        /**@addtogroup api-core-storage
         * */
        ///@{

        /** Dynamic storage constraint.
         * FIXME
         * */
        static constexpr std::size_t dynamic = std::size_t(-1);

        /** Generic data storage file.
         *
         * The "generic" data storage format consists of a common file header
         * following by a series of data chunks. Each data chunk is composed
         * of a chunk header followed by actual data.
         *
         * */
        class VIDRE_CORE_VISIBILITY generic {
        public:
            class common;
            class header;
            class data;

        public:
        };

        ///@}

    }  // namespace storage

}  // namespace vidre

#endif  // VIDRE_CORE_STORAGE_GENERIC_HH
