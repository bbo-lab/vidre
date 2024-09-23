/* VidRe
 * Copyright (C) 2024 Max-Planck-Institute f. Neurobiol. of Behavior — caesar, Bonn, Germany
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
// @file
#pragma once
#ifndef VIDRE_CORE_PLUGIN_DYNAMIC_HH
#define VIDRE_CORE_PLUGIN_DYNAMIC_HH

#include <vidre/core/config.hh>

#include <system_error>
#include <dlfcn.h>

namespace vidre {

    namespace plugin {
        /**@addtogroup api-core-plugin
         * */
        ///@{

        /** Dynamically linked object plugin loader.
         *
         * TBD there will also be a plugin::python and maybe others,
         *     so, this class should actually be derived from a virtual
         *     loader class (and register itself as a loader in some other
         *     handler class, that find, list, searches, etc... all
         *     available modules)
         *
         * */
        class VIDRE_CORE_VISIBILITY dynamic {
        public:
            static constexpr const char *extension = VIDRE_DLL_EXTENSION;

            /** DLL handler abstraction.
             *
             * This class should serve as an abstraction of dlopen (or the
             * respective OS equivalent). It's purpose is not only to load
             * modules, but may be also used for modules that depend on
             * proprietry libraries which are not distributed together with
             * VidRe.
             *
             * @hidden
             * In the case of propriertry libraries the module should not
             * just fail to load. Instead, it should, itself, be able to
             * detect, if it is required and instead of failing report
             * detailed instructions on how to install the correction version
             * of the proprietry software VidRe was compiled for. This
             * can be for example achieved by using the pattern:
             * @code
             * const char * (*foreign_version_function)() = {};
             * //...
             * void *foreign_lib = {};
             *
             * void init() {
             *     foreign_lib = dlopen("libforeign" VIDRE_DLL_EXTENSION);
             *     if (not foreign_lib) {
             *         // ... report ..
             *         return;
             *     }
             *     foreign_version_function = (const char *(*)()) dlsym(foreign_lib, "version_function");
             *     if (not foreign_version_function) {
             *         // ... report ..
             *         return;
             *     }
             *
             *     // ... check version ...
             *     // etc.
             * }
             * @endcode
             * Another method would be to provide a secondary "analysis module".
             * A third method could be to have two plugins, the first loads the
             * the actual plugin.
             *
             * */
            class VIDRE_CORE_VISIBILITY library {
            public:
                library() = default;
                ~library();

            protected:
                void *handle_ = {};
                std::error_code ecode_ = {};

            public:
                /** Check if valid. */
                bool isvalid() const noexcept {
                    return handle_ != nullptr;
                }

                // ...
            };

        public:
            dynamic() = default;

        protected:
            // FIXME

        public:
        };

        ///@}
    }  // namespace plugin

}  // namespace vidre

#endif  // VIDRE_CORE_PLUGIN_DYNAMIC_HH
