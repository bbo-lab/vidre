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
#ifndef VIDRE_CORE_PLUGIN_TYPES_HH
#define VIDRE_CORE_PLUGIN_TYPES_HH
#pragma once

/**@addtogroup api-core-plugin-category
 * @details
 * @fileinfo
 *
 * VidRe supports several types of plugins, which are categorized by their purpose
 * and/or technical implementation.
 *
 * FIXME
 *
 * */

#include <vidre/core/config.hh>

#include <string_view>

namespace vidre {

    namespace plugin {

        /**@addtogroup api-core-plugin-category
         * */
        ///@{

        /** Plugin category.
         *
         * The plugin category only describes the technical implementation of a plugin,
         * not its purpose or capabilities - only distinguishing how it is loaded.
         * This also requires that the plugin description is formatted in a specific way.
         *
         * */
        struct category {
            enum class type {
                native,  ///< Compiled and dynamically-linked loadable module.
                script,  ///< Script-based plugin.
                program  ///< An executable program that is called by VidRe.
            };

            static std::string_view name(type t) noexcept;
            static type id(std::string_view name) noexcept;
        };

        /** Plugin interface.
         *
         * The "interface" describes the "data flow" capabilities of a plugin, i.e.
         * whether it produces and/or consumes data. This is relevant for distinguishing
         * between "source", "sink" and "pipe" plugins, which can be used in different
         * contexts and have different requirements for their implementation.
         *
         * */
        struct interface {
            enum class type {
                other,   ///< Other plugin type (e.g. GUI).
                sink,    ///< Plugin that consumes data (e.g. storage).
                source,  ///< Plugin that produces data (e.g. acquisition).
                pipe     ///< Plugin that both consumes and produces data (e.g. processing).
            };

            static std::string_view name(type t) noexcept;
            static type id(std::string_view name) noexcept;
        };

        /** Communication endpoint(s).
         *
         * Plugins that produce or consume data require some form of communication 
         * with the rest of the system, which can be implemented in different ways. 
         * These communication "endpoints" are categorized by their technical 
         * implementation and require specific configuration parameters, 
         * described in FIXME.
         *
         * */
        struct endpoint {
            enum class hint {
                none,    ///< No external endpoint (self-contained, e.g., GUI hooks).
                stream,  ///< Standard VidRe data stream (@see @ref api-core-storage-io).
                stdio,   ///< Standard input/output of the program process.
                memory,  ///< Shared memory region (identifier in config).
                fifo,    ///< Named pipe (path in config).
                socket   ///< A socket (address in config).
                         ///  With few exceptions, plugins will only use local IPC. There,
                         ///  in general, Unix domain sockets are preferred. However, to
                         ///  support remote communication, especially when the daemon
                         ///  runs on a headless system, TCP/IP sockets can also be used
                         ///  for remote control. We recommend, though, using SSH tunnels
                         ///  for secure communication in that case.
            };

            static std::string_view name(hint t) noexcept;
            static hint id(std::string_view name) noexcept;
        };

        ///@}

    }  // namespace plugin

}  // namespace vidre

#endif  // VIDRE_CORE_PLUGIN_TYPES_HH
