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
#ifndef VIDRE_CORE_PROGRAM_HH
#define VIDRE_CORE_PROGRAM_HH
#pragma once

/**@addtogroup api-core-program
 * @details
 * @fileinfo
 *
 * Among other tools, here we provide a replacement for Qt's QApplication to
 * avoid the heavy framework that must only be used for graphical applications.
 *
 * */

#include <vidre/core/config.hh>
#include <vidre/util/filesystem.hh>
#include <vidre/util/variant.hh>

namespace vidre {

    /** Program related.
     * @ingroup api-core-program
     * */
    namespace program {

        /** Implementation details.
         * @ingroup api-core-details
         */
        namespace detail {

            /**@addtogroup api-core-details
             * */
            ///@{

            /** Simple, small key-variant pair map.
             *
             * @todo this could, as previously the variant, also go into a "util" header?
             * */
            class VIDRE_CORE_VISIBILITY tinymap : public std::vector<std::pair<std::string, variant>> {
            public:
                tinymap() = default;

            public:
                /** Set key-variant pair.
                 * */
                tinymap &set(const std::string &key, variant value);

                /** Get key.
                 * */
                variant get(const std::string &key);

                /** Check if key exists.
                 * */
                bool has(const std::string &key);
            };

            ///@}

        }  // namespace detail

        /**@addtogroup api-core-details
         * */
        ///@{

        /** Program invocation path.
         * @note This function must not be called before FIXME
         * */
        const fs::path &invocation();

        /** Program argument definition.
         *
         * The parser understands the following program arguments:
         *
         *      -h                  A single "short" option.
         *      -abc                Multiple short options.
         *      --help              A "long" option.
         *      -x <value>          A short option with an argument.
         *      --extra <string>    A "long" option with an argument.
         *      --extra=<string>
         *      --                  End of options (positional arguments follow).
         *
         * Note that values after options may not start with a dash ('-').
         *
         * For parsing, an argument is described by
         * ([] denotes an optional setting):
         *
         *      {
         *          <long-option>, [<short-option>],
         *          <brief>,
         *          [<argument-type>=argument::none],
         *          [<value-type>=variant::none],
         *          [<default>],
         *          [<callback>]
         *      }
         *
         * The fields in detail:
         *
         * The __long-option__ is always required, and, is also used as key in the parsed arguments
         * (see @ref vidre::arg::parser::parsed() ). Program arguments use "--<long-option>".
         *
         * The __short-option__ is a single, optional character. Program arguments use '-<short-option>',
         * for example "-h" for displaying help.
         *
         * The __brief__ description, generally required, appears in the help text.
         *
         * For the __argument-type__, see @ref vidre::arg::id, the default is @c none.
         *
         * If applicable, either the __value-type__ or the __default__ value can be set (see @ref vidre::arg::variant).
         *
         * In addition, a __callback__ function can be assigned, which gets called immediately after processing the option.
         *
         * The full constructor is:
         *
         *      {"<long-option>",<short-option>,"<brief>",<argument-type>,<value-type>,<default>,<callback>}
         *
         * The following constructors are supported:
         *
         *      {"<long-option>","<brief>"}
         *      {"<long-option>","<brief>",<callback>}
         *      {"<long-option>",<short-option>,"<brief>"}
         *      {"<long-option>","<brief>",<short-option>,<callback>}
         *
         *      {"<long-option>","<brief>",argument::required,[<value-type>=variant::cstring]}
         *      {"<long-option>",<short-option>,"<brief>",argument::required,[<value-type>=variant::cstring]}
         *
         *      {"<long-option>","<brief>",argument::optional,<default>}
         *      {"<long-option>",<short-option>,"<brief>",argument::optional,<default>}
         *
         *      {"<long-option>","<brief>",argument::positional,[<value-type>=variant::cstring]}
         *
         * Alternatives for positional arguments or parameters, respectively:
         *
         *      {"<long-option>","<brief>",argument::positional,[<value-type>=variant::cstring]}
         *
         * is the same as
         *
         *      argument::parameter{"<long-option>","<brief>",[<value-type>=variant::cstring]}
         *
         * @see FIXME
         * */
        struct VIDRE_CORE_VISIBILITY argument {
            struct parameter;
            class parser;

            /** Kind of argument.
             * */
            enum kind : int {
                flag,       ///< Pure flag without value (e.g. --help).
                optional,   ///< Option that may take a value (otherwise using the default).
                required,   ///< Option requiring a value (e.g. --log=debug).
                positional  ///< Argument is a positional parameter.
            };

            /** Type for callbacks.
             * Callbacks are immediately called during the parsing run, after possible
             * required (or optional) parameters are parsed.
             * */
            using callback_type = void (*)(parser &, const std::string &, variant);

            consteval argument() = default;
            consteval argument(const char *long_, char short_, const char *brief_, kind type_, variant::value vtype_,
                               variant &&defval, callback_type callback_) noexcept
                : long_option(long_)
                , short_option(short_)
                , brief(brief_)
                , type(type_)
                , value(vtype_)
                , default_value(defval)
                , callback(callback_) {}

            // argument::none
            consteval argument(const char *long_, const char *brief_) noexcept : long_option(long_), brief(brief_) {}
            consteval argument(const char *long_, char short_, const char *brief_) noexcept
                : long_option(long_), short_option(short_), brief(brief_) {}

            // callback
            consteval argument(const char *long_, const char *brief_, callback_type callback_) noexcept
                : long_option(long_), brief(brief_), callback(callback_) {}
            consteval argument(const char *long_, char short_, const char *brief_, callback_type callback_) noexcept
                : long_option(long_), short_option(short_), brief(brief_), callback(callback_) {}

            // other
            consteval argument(const char *long_, const char *brief_, kind type_, variant::value vtype_ = variant::cstring) noexcept
                : long_option(long_), brief(brief_), type(type_), value(vtype_) {}
            consteval argument(const char *long_, char short_, const char *brief_, kind type_,
                               variant::value vtype_ = variant::cstring) noexcept
                : long_option(long_), short_option(short_), brief(brief_), type(type_), value(vtype_) {}

            consteval argument(const char *long_, const char *brief_, kind type_, variant defval_) noexcept
                : long_option(long_), brief(brief_), type(type_), value(defval_.type()), default_value(defval_) {}
            consteval argument(const char *long_, char short_, const char *brief_, kind type_, variant defval_) noexcept
                : long_option(long_)
                , short_option(short_)
                , brief(brief_)
                , type(type_)
                , value(defval_.type())
                , default_value(defval_) {}

            const char *long_option{};             ///< The "long" option, also used as key (required).
            char short_option{};                   ///< The "short" option (optional).
            const char *brief{};                   ///< Brief description of the option (required).
            argument::kind type = argument::flag;  ///< Option type.

            variant::value value = variant::cstring;  ///< Expected value type (if applicable).
            variant default_value{};                  ///< Default value (if applicable).
            callback_type callback{};                 ///< Callback function.
        };

        /** Alternative construction of positional arguments.
         * @see vidre::arg::option
         * */
        struct VIDRE_CORE_VISIBILITY argument::parameter : public argument {
            parameter() = delete;

            consteval parameter(const char *long_, const char *brief_, variant::value vtype_ = variant::cstring) noexcept
                : argument(long_, brief_, argument::positional, vtype_) {}
            consteval parameter(const char *long_, const char *brief_, variant defval_) noexcept
                : argument(long_, brief_, argument::positional, defval_) {}
        };

        /** Program options.
         * @see @ref example/core/program.cc
         * */
        template <size_t N>
        class options {
        public:
            options() = delete;

            options(options &&) = default;
            options(const options &) = delete;

            /** Constructor.
             * @param brief         Short, brief program description.
             * @param description   Long program description.
             * @param options       Program options.
             */
            consteval options(const char *brief_, const char *description_, std::initializer_list<argument> args_)
                : brief(brief_), description(description_), size(N) {
                if (args_.size() != N)
                    throw "size deduction failure";
                std::copy(args_.begin(), args_.end(), args);
            }

        protected:
            const char *brief{};        ///< Brief program description (see @ref examples/core/app.cc).
            const char *description{};  ///< Program usage text.
            size_t size{};              ///< Number of options.
            argument args[N]{};         ///< Options buffer (allowing constant evaluation).

        private:
            friend class argument::parser;
        };

        // dedcution guide
        template <size_t N>
        options(const char *, const char *, const argument (&)[N]) -> options<N>;

        template <size_t N>
        void parse(options<N> &opts, int argc, const char **argv) {
            // FIXME
        }

        /** FIXME
         * @see @ref example/core/program.cc
         * */
        class argument::parser {
        public:
            using map = detail::tinymap;

        public:
            parser() = default;
        };

#if 0
        /** Argument parser.
         * @see @ref examples/core/app.cc
         * @todo It might be better to split the options definition into a part,
         *       that can be constant evaluated - and have the actual parser in
         *       a separate class (so that max_options becomes superfluous).
         * */
        class parser {
        public:
            /** Maximum number of option descriptions, this parser can hold. */
            static constexpr size_t max_options = VIDRE_MAX_OPTIONS;

        public:
            parser() = delete;

            parser(parser &&) = default;
            parser(const parser &) = delete;

            /** Constructor.
             * @param brief         Short, brief program description.
             * @param description   Long program description.
             * @param options       Program options.
             */
            consteval parser(const char *brief, const char *description, std::initializer_list<option> options)
                : brief_(brief), description_(description), opts_() {
                if (options.size() > max_options)
                    throw "too many options - consider raising the maximum (VIDRE_MAX_OPTIONS)";
                std::copy(options.begin(), options.end(), opts_);
            }

        protected:
            const char *brief_{};         ///< Brief program description (see @ref examples/core/app.cc).
            const char *description_{};   ///< Program usage text.
            option opts_[max_options]{};  ///< Options buffer (allowing constant evaluation).

            static std::filesystem::path invocation_;   ///< Program invocation path/file.
            static map parsed_;                         ///< Parsed options.
            static std::vector<std::string> unparsed_;  ///< Unparsed options.

        protected:
            void handle(const option &);

            const arg::option *find_short(char c);
            const arg::option *find_long(const std::string &name);

        public:
            /** Parse program invocation arguments.
             * */
            void parse(int argc, const char **argv);

            /** Get list of unparsed elements.
             * */
            auto const &unparsed() const noexcept {
                return unparsed_;
            }

            /** Get list of parsed elements.
             * */
            auto const &parsed() const noexcept {
                return parsed_;
            }

            /** Show contents after parsing.
             * @note This method is intended for debugging purposes only.
             * */
            void show() const noexcept;

            /** Help text generator.
             * */
            int help();
        };
#endif

        ///@}

    }  // namespace program

}  // namespace vidre

#endif  // VIDRE_CORE_APPLICATION_HH
