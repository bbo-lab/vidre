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
#ifndef VIDRE_BASE_MODULE_LOADER_HH
#define VIDRE_BASE_MODULE_LOADER_HH

#include <vidre/base/config.hh>

#include <system_error>
#include <dlfcn.h>

namespace vidre {

    namespace module {
        /**@addtogroup api-base-module
         * */
        ///@{

        /** Shared object (module).
         * */
        class VIDRE_BASE_VISIBILITY shared {
        public:
            enum options {
                lazy = RTLD_LAZY,
                now = RTLD_NOW
            };

        public:
            shared() = default;
            shared(const char *name, options opt = lazy) {
                open(name, opt);
            }

        protected:
            void *dll_ = {};

        public:
            /** Check if valid.
             * */
            bool valid() const noexcept {
                return dll_ != nullptr;
            }

            /** Open share library.
             * */
            std::error_code open(const char *name, options opt = lazy) {
                if (dll_)
                    return std::make_error_code(std::errc::already_connected);

                dll_ = dlopen(name, opt);
                if (dll_)
                    return {};
                else
                    return std::make_error_code(std::errc(errno));
            }

            /** Close shared library.
             * */
            void close() {
                if (dll_)
                    dlclose(dll_);
                dll_ = nullptr;
            }

            /** Retrieve a symbol.
             * */
            template <typename T>
            T *get(const char *name) {
                return reinterpret_cast<T *>(dlsym(dll_, name));
            }
        };


#define VIDRE_SHARED_MODULE(name) #name ".so"

        ///@}
    }  // namespace module

}  // namespace vidre

#endif  // VIDRE_BASE_MODULE_LOADER_HH
