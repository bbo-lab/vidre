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
 * moment of writing). Its major purpose is to facilitate complex configuration
 * scripts of devices, such as cameras, where, for example, setting or
 * retrieving the frame rate involves setting several registers (note, that
 * the USB3Vision standard implements this using XML files).
 *
 * @see @ref examples/core/script.cc
 *
 * */

#include <vidre/core/config.hh>
#include <delua.hpp>

#include <filesystem>

namespace vidre {

    /**@addtogroup api-core-script
     * */
    ///@{

    /** Scripting.
     * */
    struct script {
        /** Script interface.
         *
         * The script interface is based on <a href="https://github.com:jsawbbo/delua">Delua</a>,
         * which provides a low-level C++ interface to Lua (version 5.4 at the moment
         * of writing). We add a number of high-level methods for convenience.
         *
         * @see vidre::script::g
         * @see @ref examples/core/script.cc
         *
         * */
        class VIDRE_CORE_VISIBILITY interface : public lua::state {
        public:
            class stack;

        public:
            interface() = default;
            ~interface() {
                if (L)
                    close();
            }

        protected:
            interface(void (*init)(interface &)) : lua::state(lua::state::newstate()) {
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

            /** Execute a string.
             * Example:
             * @snippet examples/core/script.cc hello-world
             * */
            lua::status exec(const std::string &s);

            /** Require library.
             * @returns lua::status::ok or lua::status::yield, otherwise an exception is thrown
             * */
            void require(const std::string &lib);

        private:
            friend struct vidre::script;
        };

        /** Global script state.
         * @see @ref examples/core/script.cc
         * */
        static interface g;
    };

    /** Stack scope.
     * This class facilitates automatic Lua stack cleanup by scope.
     * 
     * Example:
     * @snippet examples/core/script.cc stack-top
     * */
    class VIDRE_CORE_VISIBILITY script::interface::stack {
    public:
        stack() = delete;
        stack(interface &L) : L_(L), top_(L.gettop()) {}
        ~stack() {
            L_.settop(top_);
        }

    protected:
        interface &L_;
        index_type top_;

    public:
        /** Get number of elements on stack. */
        size_type size() const noexcept {
            return size_type(L_.gettop() - top_);
        }
    };

    ///@}

}  // namespace vidre

#endif  // VIDRE_CORE_SCRIPT_HH
