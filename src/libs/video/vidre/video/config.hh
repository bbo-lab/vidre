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
#ifndef VIDRE_VIDEO_CONFIG_HH
#define VIDRE_VIDEO_CONFIG_HH
#pragma once

#include <vidre/config.hh>

/**@addtogroup api-video-details
 * */
///@{

/** Declaration specification for import/export for the video library.
 *
 * @see VIDRE_CORE_VISIBILITY
 * */
#if not defined(VIDRE_VIDEO_VISIBILITY) or defined(DOXYGEN)
#define VIDRE_VIDEO_VISIBILITY VIDRE_API_IMPORT
#endif

///@}

#endif  // VIDRE_VIDEO_CONFIG_HH
