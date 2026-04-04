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
#ifndef VIDRE_CORE_PLUGIN_DYNAMIC_HH
#define VIDRE_CORE_PLUGIN_DYNAMIC_HH
#pragma once

#include <vidre/core/config.hh>
#include <vidre/plugin/exception.hh>
#include <vidre/plugin/information.hh>

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

            namespace library {

                /**@addtogroup api-core-details
                 * */
                ///@{

#if defined(VIDRE_USING_DLFCN) or defined(DOXYGEN)
                /// Operating system abstraction for DLL module handle.
                using handle_type = void *;

                /// Operating system abstraction for DLL symbol handle.
                using symbol_type = void *;

                /// Operating system abstraction for DLL module modes.
                enum mode : int {
                    lazy = RTLD_LAZY,
                    now = RTLD_NOW,
                };

#elif defined(VIDRE_USING_LIBLOADERAPI)
                using handle_type = HMODULE;

                enum mode {
                    lazy = DONT_RESOLVE_DLL_REFERENCES,
                    now = 0
                };
#endif

                /** Open library.
                 * @return handle (nullptr on error) and error message (nullptr on success)
                 * */
                std::pair<handle_type, std::string> open(const char *filename, int flags) noexcept;

                /** Close library.
                 * @return error message on failure, nullptr otherwise
                 */
                std::pair<bool, std::string> close(handle_type handle) noexcept;

                /** Get library symbol by name.
                 * @return handle (nullptr on error) and error message (nullptr on success)
                 * */
                std::pair<symbol_type, std::string> symbol(handle_type handle, const char *symname) noexcept;

                ///@}

            }  // namespace library

        }  // namespace detail

        /**@addtogroup api-core-plugin
         * */
        ///@{

        /** Dynamically linked object plugin loader.
         * */
        class VIDRE_CORE_VISIBILITY dynamic {
        public:
            /** File extension for dynamic load libraries.
             * */
            static constexpr const char *extension = VIDRE_DLL_EXTENSION;

            /** Exception.
             * */
            class VIDRE_CORE_VISIBILITY exception : public virtual vidre::exception {
            protected:
                std::string msg_{};

            public:
                exception(const std::string &msg, const std::source_location location = std::source_location::current())
                    : vidre::exception(location), msg_(msg) {}

                const char *what() const noexcept override {
                    return msg_.c_str();
                }
            };

            /** DLL handler abstraction.
             *
             * This class should serve as an abstraction of dlopen (or the
             * respective OS equivalent). It's purpose is not only to load
             * modules, but may be also used for modules that depend on
             * proprietry libraries which are not distributed together with
             * VidRe.
             *
             * */
            class VIDRE_CORE_VISIBILITY library {
            public:
                using handle_type = detail::library::handle_type;
                using symbol_type = detail::library::symbol_type;

                enum mode : int {
                    lazy = detail::library::mode::lazy,  ///< Perform lazy binding.
                                                         ///< Resolve symbols only as the code that references them is executed.
                    now = detail::library::mode::now     ///< All undefined symbols in the shared object are resolved before
                                                         ///< constructor returns.
                };

            public:
                library() = default;

                library(const char *path, mode flags = mode::lazy) {
                    open(path, flags);
                }
                library(const std::string &path, mode flags = mode::lazy) : library(path.c_str(), flags) {}

                library(const library &other) = delete;
                library(library &&other) = delete;

                ~library() {
                    detail::library::close(handle_);
                }

            protected:
                handle_type handle_ = {};

            public:
                /** Open library by path.
                 * @throws plugin::exception
                 * */
                bool open(const char *path, mode flags = mode::lazy) {
                    auto lib = detail::library::open(path, flags);
                    handle_ = lib.first;
                    if (not handle_)
                        throw exception::runtime() << error::message(lib.second);
                    return handle_;
                }

                bool open(const std::string &path, mode flags = mode::lazy) {
                    return open(path.c_str(), flags);
                }

                /** Close library.
                 * */
                void close() noexcept {
                    detail::library::close(handle_);
                    handle_ = nullptr;
                }

                /** Check if valid.
                 * */
                bool isvalid() const noexcept {
                    return handle_ != nullptr;
                }

                /** Get library symbol. */
                symbol_type get(const std::string &symname) {
                    auto sym = detail::library::symbol(handle_, symname.c_str());
                    if (not sym.first)
                        throw exception::runtime() << error::message(sym.second);
                    return sym.first;
                }

                /** Get library symbol.
                 * */
                template <typename T>
                T get(const std::string &symname) {
                    auto sym = detail::library::symbol(handle_, symname.c_str());
                    return reinterpret_cast<T>(sym.first);
                }
            };

        public:
            dynamic() = default;

        protected:
            // FIXME path_
            library module_{};
            information *info_{};

        public:
            void load(const std::string &name);

            /** Check if valid.
             * */
            bool isvalid() const noexcept {
                return info_ != nullptr;
            }

            /** Get module information.
             * */
            const information info() const noexcept {
                return *info_;
            }
        };

        ///@}

    }  // namespace plugin

}  // namespace vidre

#endif  // VIDRE_CORE_PLUGIN_DYNAMIC_HH
