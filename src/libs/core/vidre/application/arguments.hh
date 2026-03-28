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
#ifndef VIDRE_CORE_APPLICATION_ARGUMENTS_HH
#define VIDRE_CORE_APPLICATION_ARGUMENTS_HH
#pragma once

#include <vidre/core/config.hh>
#include <vidre/settings/json.hh>

namespace vidre {

    /** Application arguments and parsing.
     * @ingroup api-core-app */
    namespace arg {

        /**@addtogroup api-core-app
         * */
        ///@{

        class parser;

        /** Variant.
         * */
        using variant = settings::json;

        /** Program argument description.
         * @code
         * { "long-option", '<short>', flags, callback, default, brief}
         * @endcode
         *
         * @code
         * { "long-option", brief}
         * { "long-option", '<short>', brief}
         * { "long-option", '<short>', callback, brief}
         *
         * { "long-option", '<short>', flags, brief}
         * { "long-option", '<short>', flags, callback, brief}
         * { "long-option", '<short>', default, brief}
         * { "long-option", '<short>', flags, default, brief}
         * @endcode
         * */
        class VIDRE_CORE_VISIBILITY option {
        public:
            /** Option flags.
             * */
            enum flags {
                no_argument,
                optional_argument,
                requires_argument,
                positional_argument
            };

            /** Argument parser callback type.
             * */
            using callback_type = int (*)(parser &);

        public:
            constexpr option() = default;

            constexpr option(const char *olong, const char *desc) : long_option(olong), brief(desc) {}
            constexpr option(const char *olong, char oshort, const char *desc)
                : long_option(olong), short_option(oshort), brief(desc) {}
            constexpr option(const char *olong, char oshort, callback_type cb, const char *desc)
                : long_option(olong), short_option(oshort), callback(cb), brief(desc) {}

        protected:
            const char *long_option{};
            char short_option{};
            flags flags_ = no_argument;
            callback_type callback{};

            variant value{};

            const char *brief{};

        public:
            // FIXME

        private:
            friend class parser;
        };

        /** Argument parser.
         * FIXME
         * */
        class VIDRE_CORE_VISIBILITY parser {
        public:
            constexpr parser(std::initializer_list<option> init) : opts_(init) {}

        protected:
            std::initializer_list<option> opts_{};
            std::filesystem::path invocation_{};
            std::vector<std::string> unparsed_{};

        protected:
            std::string make_option_help(const arg::option &opt);
            const option &find_short(char c);
            void handle(const option &);

        public:
            void parse(int argc, const char **argv);

            int help();
        };

        ///@}

    }  // namespace arg

}  // namespace vidre

#endif  // VIDRE_CORE_APPLICATION_ARGUMENTS_HH
