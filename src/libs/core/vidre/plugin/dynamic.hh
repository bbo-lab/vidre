/* VidRe
 * Copyright (C) 2024-2025 Max-Planck-Institute f. Neurobiol. of Behavior — caesar, Bonn, Germany
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
#ifndef VIDRE_CORE_PLUGIN_DYNAMIC_HH
#define VIDRE_CORE_PLUGIN_DYNAMIC_HH
#pragma once

#include <vidre/core/config.hh>

#include <system_error>

#if defined(_MSC_VER)
#include <libloaderapi.h>
#define VIDRE_USING_LIBLOADERAPI
#else
#include <dlfcn.h>
#define VIDRE_USING_DLFCN
#endif

namespace vidre {

    namespace plugin {

        namespace detail {
            /**@addtogroup api-core-details
             * */
            ///@{

#if defined VIDRE_USING_DLFCN
            /// Operating system abstraction for DLL module handle.
            using handle_type = void *;

            /// Operating system abstraction for DLL symbol handle.
            using symbol_type = void *;

            /// Operating system abstraction for DLL module modes.
            enum mode {
                lazy = RTLD_LAZY,
                now = RTLD_NOW,
            };

            // FIXME inline handle_type libopen(const char *filename, int flags) noexcept {}
            // FIXME inline void libclose(handle_type handle) noexcept {}
            // FIXME inline symbol_type libget(handle_type handle, const char *symbol) noexcept {}
            // FIXME inline const char *liberror(handle_type handle); ???
#elif defined(VIDRE_USING_LIBLOADERAPI)
            using handle_type = HMODULE;

            enum mode {
                lazy = DONT_RESOLVE_DLL_REFERENCES,
                now = 0
            };
#endif

            ///@}
        }  // namespace detail

        /**@addtogroup api-core-plugin
         * */
        ///@{

        /** Dynamically linked object plugin loader.
         *
         * @TBD{there will also be a plugin::python and maybe others,
         *     so, this class should actually be derived from a virtual
         *     loader class (and register itself as a loader in some other
         *     handler class, that finds, lists, searches, etc... all
         *     available modules)}
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
             * @internal
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
                using handle_type = detail::handle_type;

                enum mode {
                    lazy = detail::mode::lazy,  ///< Perform lazy binding.
                                                ///< Resolve symbols only as the code that references them is executed.
                    now =
                        detail::mode::now  ///< All undefined symbols in the shared object are resolved before constructor returns.
                };

            public:
                library() = default;
                library(const char *path, int flags = mode::lazy);
                ~library();

            protected:
                handle_type handle_ = {};
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
