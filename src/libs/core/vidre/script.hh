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
#ifndef VIDRE_CORE_SCRIPT_HH
#define VIDRE_CORE_SCRIPT_HH
#pragma once

/**@addtogroup api-core-script
 * @details
 * @fileinfo
 *
 * VidRe provides scripting functionality through Lua (version 5.4 at the
 * moment of writing). Its maor purpose is to facilitate complex configuration
 * scripts of devices, such as cameras, where
 *
 * @internal
 *
 * @endinternal
 *
 * */

#include <vidre/core/config.hh>
#include <delua.hpp>

#include <filesystem>

namespace vidre {

    /**@addtogroup api-core-script
     * */
    ///@{

    /** FIXME
     * */
    class VIDRE_CORE_VISIBILITY script {
    public:
        /** Low-level interface.
         * */
        class VIDRE_CORE_VISIBILITY impl : public lua::state {
        public:
            class stack;

        public:
            impl() = default;
            ~impl() {
                if (L)
                    close();
            }

        protected:
            impl(void (*init)(impl &)) : lua::state(lua::state::newstate()) {
                luaL_openlibs(L);  // FIXME this should be in lua::state

                if (init)
                    init(*this);
            }

        public:
            /** Add a dynamic library loader path.
             * */
            void addpath(const std::filesystem::path &path);

            /** Add a script loader path.
             * */
            void addcpath(const std::filesystem::path &path);

            /** Run a script.
             * */
            void run(const std::string &scr);

            /** Require library.
             * */
            void require(const std::string &lib);

        private:
            friend class vidre::script;
        };

        /** Global VidRe state.
         * */
        static impl g;

    public:
    public:
    };

    /** Stack scope.
     * FIXME
     * */
    class VIDRE_CORE_VISIBILITY script::impl::stack {
    public:
        stack() = delete;
        stack(impl &L) : L_(L), top_(L.gettop()) {}
        ~stack() {
            L_.settop(top_);
        }

    protected:
        impl &L_;
        index_type top_;

    public:
        /** Get number of elements on stack. */
        size_type size() const noexcept {
            return size_type(L_.gettop()-top_);
        }
    };

    ///@}

}  // namespace vidre

#endif  // VIDRE_CORE_SCRIPT_HH
