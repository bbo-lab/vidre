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

/**@addtogroup api-core-details
 * */
///@{

/** Default alignment.
 * @todo check, which architectures
 * */
#ifdef __aarch64__
#define VIDRE_DEFAULT_ALIGNMENT alignof(double)
#else
#define VIDRE_DEFAULT_ALIGNMENT alignof(int)
#endif

///@}

namespace vidre {

    namespace storage {

        /**@addtogroup api-core-storage
         * */
        ///@{

        /** Dynamic storage constraint.
         * */
        static constexpr std::size_t dynamic = std::size_t(-1);

        /** Default alignment.
         * */
        static constexpr std::size_t alignment = VIDRE_DEFAULT_ALIGNMENT;

        /** Generic data storage file.
         *
         * The "generic" data storage format consists of a <tt>common</tt> file header
         * followed by a series of chunks. Each chunk is composed of frame <tt>data</tt>
         * combined with header or tail <tt>info</tt>.
         * 
         * The class(es) provide the infrastructure to create a specific implementation.
         *
         * */
        class VIDRE_CORE_VISIBILITY generic {
        public:
            class common;
            class info;
            class data;

        public:
            generic() = default;

        protected:

        public:
            
        };

        ///@}

    }  // namespace storage

}  // namespace vidre

#endif  // VIDRE_CORE_STORAGE_GENERIC_HH
