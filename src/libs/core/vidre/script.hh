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
#include <vidre/exception.hh>
#include <delua.hpp>

#include <filesystem>

namespace vidre {

    /**@addtogroup api-core-script
     * */
    ///@{

    /** Scripting.
     * */
    struct script {
        class exception;
        class stack;

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

        protected:
            /** Helper to clear package.path or package.cpath. */
            void clearpackage(const std::string &field);

            /** Helper to add to package.path or package.cpath. */
            void addpackage(const std::string &path, const std::string &field);

        public:
            /** Clear loader path.
             * @param field "cpath" or "path"
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
             * @returns lua::status::ok or lua::status::yield, otherwise an exception is thrown
             * */
            void require(const std::string &lib);

        private:
            friend struct vidre::script;
        };

        using index_type = interface::index_type;
        using size_type = interface::size_type;

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
    class VIDRE_CORE_VISIBILITY script::stack {
    public:
        using index_type = interface::index_type;
        using size_type = interface::size_type;

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

    class VIDRE_CORE_VISIBILITY script::exception : public virtual vidre::exception {
    public:
        exception(const std::string &message, const std::source_location location = std::source_location::current())
            : vidre::exception(location), error_(message) {}
        exception(script::interface &L = script::g, const std::source_location location = std::source_location::current())
            : vidre::exception(location) {
            getinfo(L);
        }

        const char *what() const noexcept override {
            return error_.c_str();
        }

    protected:
        std::string error_{};         ///< Lua error message.
        std::string chunk_{};         ///< Chunk name.
        script::size_type line_{};    ///< Line.
        script::size_type column_{};  ///< Column.

    protected:
        void getinfo(script::interface &L);
    };

    ///@}

}  // namespace vidre

#endif  // VIDRE_CORE_SCRIPT_HH
