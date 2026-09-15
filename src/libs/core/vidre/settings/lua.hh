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
#ifndef VIDRE_CORE_SETTINGS_LUA_HH
#define VIDRE_CORE_SETTINGS_LUA_HH
#pragma once

/**@addtogroup api-core-settings-lua
 * @details
 * @fileinfo
 *
 * The use of Lua is intended for complex configuration scripts, such as 
 * cameras. Cameras, for example, to set the exposure time, need to
 * adjust several camera registers. The USB3Vision standard has solved
 * the issue using an XML description the incorporates "computation" nodes,
 * allowing a generic interface for basically any camera without a specific
 * compiled implementation. 
 *
 * */

#include <vidre/core/config.hh>
#include <vidre/script.hh>
#include <vidre/error.hh>

#include <string>
#include <vector>

namespace vidre {

    namespace settings {

        /**@addtogroup api-core-settings-lua
         * */
        ///@{

        /** Configuration node.
         *
         * Configuration nodes represent entries in the configuration tree.
         * These can be sub-nodes or values, such as strings or numbers.
         *
         * @internal
         * Any node is, internally, is kept in the Lua registry.
         * FIXME
         * @endinternal
         *
         * @see @ref examples/core/settings.cc
         * */
        class VIDRE_CORE_VISIBILITY node {
        protected:
        public:
            node() = default;

            node(node &&other)
                : type_(other.type_)
                , ref_(std::exchange(other.ref_, lua::reference::nil))
                , parent_(other.parent_)
                , field_(other.field_) {}
            node(const node &) = delete;

            node(const node &parent, const std::string &field) : parent_(parent.ref_), field_(field) {}

            ~node() {
                unref();
            }

        protected:
            lua::type type_ = lua::type::none;                  ///< The Lua type this node represents.
            lua::state::index_type ref_ = lua::reference::nil;  ///< Lua reference of this node.

        private:
            lua::state::index_type parent_ = lua::reference::nil;  ///< Parent table.
            std::string field_{};                                  ///< Key in parent table.

        protected:
            /** Check if reference is nil.
             * */
            bool isnil() const noexcept {
                return ref_ == lua::reference::nil;
            }

            /** Get the reference.
             * */
            void ref() {
                type_ = script::g.type(-1);
                ref_ = script::g.ref(lua::registry);
            }

            /** Unreference table.
             * */
            void unref() {
                if (not isnil()) {
                    script::g.unref(ref_);
                    ref_ = lua::reference::nil;
                }
            }

            /** Push the table on the stack.
             * */
            lua::type push() const {
                return script::g.rawgeti(lua::registry, ref_);
            }

            /** Get an entry on the stack by name.
             * */
            lua::type get(const std::string &field) const;

        public:
            /** Get sub-node by @a field key.
             * */
            node operator[](const char *field) const {
                get(field);
                node retval(*this, field);
                retval.ref();
                return retval;
            }
            node operator[](const std::string &field) const {
                return operator[](field.c_str());
            }

            /** Implicit assignment.
             * */
            template <typename T>
            node &operator=(const T &);
            template <typename T>
            node &operator=(const T *);

            /** Implicit type conversion for values.
             * @see @ref examples/core/settings.cc
             * */
            template <typename T>
            operator T() const;

            /** Check if node is a table.
             * */
            bool istable() const noexcept {
                return type_ == lua::type::table;
            }

            /** Get Lua type behind this node.
             * */
            lua::type type() const noexcept {
                return type_;
            }

            /** Get list of keys.
             * */
            std::vector<std::string> keys() const;
        };

#if not defined(DOXYGEN)
        template <>
        inline node::operator bool() const {
            auto t = push();
            if (t != lua::type::boolean)
                throw exception::logic() << error::message("expected a number");
            auto retval = script::g.toboolean(-1);
            script::g.pop();
            return retval;
        }

        template <>
        inline node::operator lua::integer() const {
            push();
            if (not script::g.isinteger(-1))
                throw exception::logic() << error::message("expected an integer");
            auto retval = script::g.tointeger(-1);
            script::g.pop();
            return retval;
        }

        template <>
        inline node::operator lua::number() const {
            auto t = push();
            if (t != lua::type::number)
                throw exception::logic() << error::message("expected a number");
            auto retval = script::g.tonumber(-1);
            script::g.pop();
            return retval;
        }

        template <>
        inline node::operator std::string() const {
            auto t = push();
            if (t != lua::type::string)
                throw exception::logic() << error::message("expected a string");
            auto retval = script::g.tostring(-1);
            script::g.pop();
            return retval;
        }

        template <>
        inline node::operator const char *() const {
            auto t = push();
            if (t != lua::type::string)
                throw exception::logic() << error::message("expected a string");
            auto retval = script::g.tostring(-1);
            script::g.pop();
            return retval;
        }

        template <>
        inline node &node::operator=(const std::string &value) {
            if (type_ != lua::type::string)
                throw exception::logic() << error::message("invalid assignment: must be a string");
            if (parent_ == lua::reference::nil)
                throw exception::logic() << error::message("invalid assignment: unconnected node");

            script::g.rawgeti(lua::registry, parent_);
            script::g.pushlstring(value.data(), value.size());
            script::g.setfield(-2, field_.c_str());

            return *this;
        }

        template <>
        inline node &node::operator=(const char *value) {
            if (type_ != lua::type::string)
                throw exception::logic() << error::message("invalid assignment: must be a string");
            if (parent_ == lua::reference::nil)
                throw exception::logic() << error::message("invalid assignment: unconnected node");

            script::g.rawgeti(lua::registry, parent_);
            script::g.pushstring(value);
            script::g.setfield(-2, field_.c_str());

            return *this;
        }
#endif

        /** Configuration file.
         * @todo needs a dump() function
         * */
        class VIDRE_CORE_VISIBILITY file : public node {
        public:
            file() = default;

        public:
            /** Load configuration file.
             * */
            void load(const std::filesystem::path &file, bool force = false);

            /** Save configuration.
             * */
            void save();
            void save(const std::filesystem::path &file);
        };

        ///@}

    }  // namespace settings

}  // namespace vidre

#endif  // VIDRE_CORE_SETTINGS_LUA_HH
