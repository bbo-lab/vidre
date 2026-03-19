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
#include <vidre/exception.hh>
#include <vidre/plugin/information.hh>

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

#if defined(VIDRE_USING_DLFCN)
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

        /** General plugin handler and base class.
         *
         * FIXME
         *
         * */
        class VIDRE_CORE_VISIBILITY handler {
        public:
        public:
            handler() = default;

        public:
            /** Load plugin by name.
             * @param name Module name with path (but no extension, e.g. "my/plugin").
             * */
            virtual void load(const std::string &name) = 0;
        };

        /** Dynamically linked object plugin loader.
         * */
        class VIDRE_CORE_VISIBILITY dynamic : public handler {
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
             * @endinternal
             *
             * */
            class VIDRE_CORE_VISIBILITY library {
            public:
                class VIDRE_CORE_VISIBILITY exception : public virtual vidre::exception {
                protected:
                    std::string msg_{};

                public:
                    exception(const char *msg, const std::source_location location = std::source_location::current())
                        : vidre::exception(location), msg_(msg) {}

                    const char *what() const noexcept override {
                        return msg_.c_str();
                    }
                };

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
                library(const char *path, mode flags = mode::lazy) {
                    open(path, flags);
                }
                library(const std::string &path, mode flags = mode::lazy) : library(path.c_str(), flags) {}

                library(const library &other) = delete;
                library(library &&other) = delete;

                ~library();

            protected:
                handle_type handle_ = {};
                const char *emsg_ = {};

            public:
                /** Open library by path.
                 * */
                bool open(const char *path, mode flags = mode::lazy) noexcept;
                bool open(const std::string &path, mode flags = mode::lazy) noexcept {
                    return open(path.c_str(), flags);
                }

                /** Close library.
                 * */
                void close() noexcept;

                /** Check if valid.
                 * */
                bool isvalid() const noexcept {
                    return handle_ != nullptr;
                }

                /** Get last error.
                 * */
                std::string error() const noexcept {
                    return emsg_;
                }

                /** Get library symbol. */
                handle_type get(const std::string &symname) noexcept;

                /** Get library symbol.
                 * */
                template <typename T>
                T get(const std::string &symname) {
                    auto sym = get(symname);
                    if (not sym)
                        throw exception(emsg_);
                    return reinterpret_cast<T>(sym);
                }
            };

        public:
            dynamic() = default;

        protected:
            // FIXME path_
            library module_{};
            information *info_{};

        public:
            void load(const std::string &name) override;

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
