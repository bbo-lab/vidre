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

#include <utility>
#include <filesystem>

namespace vidre {

    /** Scripting.
     * @ingroup api-core-script */
    namespace script {
        /**@addtogroup api-core-script
         * */
        ///@{

        /** Scripting engine.
         *
         * The scripting engine is based on <a href="https://github.com:jsawbbo/delua">Delua</a>,
         * which provides a low-level C++ interface to Lua (version 5.4 at the moment
         * of writing). We add a number of high-level methods for convenience.
         *
         * @see vidre::script::g
         * @see @ref examples/core/script.cc
         *
         * */
        class VIDRE_CORE_VISIBILITY engine : public lua::state {
        public:
            class stack;

        public:
            engine() = delete;

            engine(engine &&other) : lua::state(std::exchange(other.L, nullptr)) {}
            engine(const engine &) = delete;

            /** Create new state with initialization function.
             * */
            engine(void (*init)(engine &)) : lua::state(lua::state::newstate()) {
                openlibs();

                if (init)
                    init(*this);
            }

            ~engine() {
                if (L)
                    close();
            }

        protected:
            /** Helper to clear package.path or package.cpath. */
            void clearpackage(const std::string &field);

            /** Helper to add to package.path or package.cpath. */
            void addpackage(const std::string &path, const std::string &field);

        public:
            /** Clear loader path.
             * @param field     "cpath" or "path", if omitted, both.
             * */
            void clearpath(const std::string &field = std::string{}) {
                if (field.empty()) {
                    clearpackage("path");
                    clearpackage("cpath");
                }
                else
                    clearpackage(field);
            }

            /** Add loader path.
             * @param path   Filesystem path.
             * @param field  "cpath" or "path"
             * */
            void addpath(const std::filesystem::path &path, const std::string &field) {
                addpackage(path, field);
            }

            /** Execute a string.
             * Example:
             * @snippet examples/core/script.cc hello-world
             * */
            lua::status exec(const std::string &s);

            /** Require library.
             * @param modname   Module to load.
             * */
            void require(const std::string &modname);
        };

        /** Global script state.
         * @see @ref examples/core/script.cc
         * */
        extern engine g;

        /** Stack scope.
         *
         * Lua is a stack based scripting engine requiring proper stack handling.
         * To simplify the stack handling this class provides a scoped stack
         * cleanup.
         *
         * Example:
         * @snippet examples/core/script.cc stack-top
         * */
        class VIDRE_CORE_VISIBILITY engine::stack {
        public:
            using index_type = engine::index_type;
            using size_type = engine::size_type;

        public:
            stack() = delete;
            stack(engine &L) : L_(L), top_(L.gettop()) {}
            ~stack() {
                L_.settop(top_);
            }

        protected:
            engine &L_;
            index_type top_;

        public:
            /** Get number of elements on stack. */
            size_type size() const noexcept {
                return size_type(L_.gettop() - top_);
            }
        };

        ///@}

        /** Analyze a Lua exception.
         * @param thr   The Lua thread.
         * @param chunk Optional chunk name.
         * @return line in chunk if given.
         * @return -1 otherwise.
         * */
        int analyze(lua::state thr, const std::string &chunk = {});

    }  // namespace script

}  // namespace vidre

#endif  // VIDRE_CORE_SCRIPT_HH
