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
#include <vidre/exception.hh>

#include <array>

#if not defined(VIDRE_MAX_OPTIONS)
/** Maximum number of options that can be passed to @c vidre::arg::parser.
 * @ingroup api-core-app
 * */
#define VIDRE_MAX_OPTIONS 16
#endif

namespace vidre {

    /** Application arguments and parsing.
     * @ingroup api-core-app */
    namespace arg {

        /** Implementation details.
         * @ingroup api-core-details
         */
        namespace detail {

            /**@addtogroup api-core-details
             * */
            ///@{

            // FIXME see deduce below, should be here...

            ///@}

        }  // namespace detail

        /**@addtogroup api-core-app
         * */
        ///@{

        class parser;

        /** A variant.
         *
         * This variant is, except for std::string, constant evaluated, allowing
         * the parser options also to be constant evaluated.
         *
         * @todo this could also go into a "util" header?
         * */
        class VIDRE_CORE_VISIBILITY variant {
        public:
            /** Internally stored value types.
             * */
            enum value_type : unsigned {
                none,     ///< The variant is "empty".
                natural,  ///< Natural (ie. unsigned) number.
                integer,  ///< Signed integer.
                number,   ///< Floating-point number.
                cstring,  ///< Constant C-string.
                string,   ///< A string (@c std::string).
            };

            /** FIXME
             * */
            template <typename T>
            struct deduce {
                static constexpr value_type type = none;
            };

        public:
            consteval variant() = default;

            constexpr variant(variant &&other) = default;
            constexpr variant &operator=(variant &&) = default;
            constexpr variant(const variant &other) = default;
            constexpr variant &operator=(const variant &) = default;

            constexpr variant(unsigned long v) noexcept : data_{v}, type_{natural} {}
            constexpr variant(unsigned int v) noexcept : data_{(unsigned long)v}, type_{natural} {}
            constexpr variant(unsigned short v) noexcept : data_{(unsigned long)v}, type_{natural} {}
            constexpr variant(long v) noexcept : data_{v}, type_{integer} {}
            constexpr variant(int v) noexcept : data_{(long)v}, type_{integer} {}
            constexpr variant(short v) noexcept : data_{(long)v}, type_{integer} {}
            constexpr variant(double v) noexcept : data_{v}, type_{number} {}
            constexpr variant(const char *v) noexcept : data_{v}, type_{cstring} {}
            variant(const std::string &v) noexcept : data_{v}, type_{string} {}

        protected:
            std::variant<unsigned long, long, double, const char *, std::string> data_{};
            value_type type_{none};

        public:
            /** Get the value type.
             * */
            constexpr value_type type() const noexcept {
                return type_;
            }

            /** Check if "empty".
             * */
            constexpr bool empty() const noexcept {
                return (type_ == none);
            }

            /** Get value.
             * */
            template <typename T>
            constexpr operator T() const;

            /** Assign value.
             * @note The value must be of the same type.
             * */
            template <typename T>
            constexpr variant &operator=(const T &);
            template <typename T>
            constexpr variant &operator=(const T *);
        };

#if not defined(DOXYGEN)
        // FIXME the following needs to go into "implementation details"
        template <>
        struct variant::deduce<unsigned long> {
            static constexpr value_type id = natural;
            using type = unsigned long;
        };

        template <>
        struct variant::deduce<unsigned int> {
            static constexpr value_type id = natural;
            using type = unsigned long;
        };

        template <>
        struct variant::deduce<unsigned short> {
            static constexpr value_type id = natural;
            using type = unsigned long;
        };

        template <>
        struct variant::deduce<long> {
            static constexpr value_type id = integer;
            using type = long;
        };

        template <>
        struct variant::deduce<int> {
            static constexpr value_type id = integer;
            using type = long;
        };

        template <>
        struct variant::deduce<short> {
            static constexpr value_type id = integer;
            using type = long;
        };

        template <>
        struct variant::deduce<char> {
            static constexpr value_type id = integer;
            using type = long;
        };

        template <>
        struct variant::deduce<double> {
            static constexpr value_type id = number;
            using type = double;
        };

        template <>
        struct variant::deduce<float> {
            static constexpr value_type id = number;
            using type = double;
        };

        template <>
        struct variant::deduce<const char *> {
            static constexpr value_type id = cstring;
            using type = const char *;
        };

        template <>
        struct variant::deduce<std::string> {
            static constexpr value_type id = string;
            using type = std::string;
        };

        template <typename T>
        inline constexpr variant::operator T() const {
            if (deduce<T>::id != type_) {
                if (std::is_constant_evaluated())
                    throw "implicit type conversion is not supported";
                else
                    throw vidre::exception() << throw_message("implicit type conversion is not supported");
            }

            return static_cast<T>(std::get<typename deduce<T>::type>(data_));
        }

        template <>
        inline constexpr variant::operator const char *() const {
            switch (type_) {
                case cstring:
                    return std::get<const char *>(data_);
                case string:
                    return std::get<std::string>(data_).c_str();
                default:
                    if (std::is_constant_evaluated())
                        throw "implicit type conversion is not supported";
                    else
                        throw vidre::exception() << throw_message("implicit type conversion is not supported");
            }
        }

        template <>
        inline variant::operator std::string() const {
            switch (type_) {
                case cstring:
                    return std::get<const char *>(data_);
                case string:
                    return std::get<std::string>(data_);
                default:
                    throw vidre::exception() << throw_message("implicit type conversion is not supported");
            }
        }

        template <typename T>
        inline constexpr variant &variant::operator=(const T &v) {
            if (deduce<T>::id != type_) {
                if (std::is_constant_evaluated())
                    throw "implicit type conversion is not supported";
                else
                    throw vidre::exception() << throw_message("implicit type conversion is not supported");
            }

            data_ = static_cast<typename deduce<T>::type>(v);

            return *this;
        }

        template <>
        inline constexpr variant &variant::operator=(const char *v) {
            switch (type_) {
                case cstring:
                    data_ = v;
                    break;
                case string:
                    data_ = std::string(v);
                    break;
                default:
                    if (std::is_constant_evaluated())
                        throw "implicit type conversion is not supported";
                    else
                        throw vidre::exception() << throw_message("implicit type conversion is not supported");
            }

            return *this;
        }

        template <>
        inline constexpr variant &variant::operator=(const std::string &v) {
            switch (type_) {
                case cstring:
                    type_ = string;
                    [[fallthrough]];
                case string:
                    data_ = v;
                    break;
                default:
                    if (std::is_constant_evaluated())
                        throw "implicit type conversion is not supported";
                    else
                        throw vidre::exception() << throw_message("implicit type conversion is not supported");
            }

            return *this;
        }

#endif

        /** Simple, small key-variant pair map.
         *
         * @todo this could, as variant above, also go into a "util" header?
         * */
        class VIDRE_CORE_VISIBILITY map : public std::vector<std::pair<std::string, variant>> {
        public:
            map() = default;

        public:
            /** Set key-variant pair.
             * */
            map &set(const std::string &key, variant value);

            /** Get key.
             * */
            variant get(const std::string &key);

            /** Check if key exists.
             * */
            bool has(const std::string &key);
        };

        /** Argument type definitions.
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
         * @see vidre::arg::parser
         * */
        struct argument {
            enum type : int {
                none,       ///< Option has no argument.
                optional,   ///< Option may have an argument (must not start with '-').
                required,   ///< An argument is required for the option.
                positional  ///< Argument is a positional parameter.
            };

            /** Type for callbacks.
             * Callbacks are immediately called during the parsing run, after possible
             * required (or optional) parameters are parsed.
             * */
            using callback = void (*)(parser &, const std::string &, variant);
        };

        /** Program argument description.
         *
         * A full argument description is declared by ([] denotes, that the setting
         * is optional):
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
         * For the __argument-type__, see @ref vidre::arg::argument::type, the default is @c none.
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
         * Alternatives for positional arguments:
         *
         *      {"<long-option>","<brief>",argument::postional,[<value-type>=variant::cstring]}
         *
         * is the same as
         *
         *      option::positional{"<long-option>","<brief>",[<value-type>=variant::cstring]}
         *
         * */
        struct VIDRE_CORE_VISIBILITY option {
            struct positional;

            template <size_t>
            class set;

            template <size_t N>
            set(const char *, const char *, const option (&)[N]) -> set<N>;

            consteval option() = default;
            consteval option(const char *long_, char short_, const char *brief_, argument::type type_, variant::value_type vtype_,
                             variant &&defval, argument::callback callback_) noexcept
                : long_option(long_)
                , short_option(short_)
                , brief(brief_)
                , type(type_)
                , value_type(vtype_)
                , default_value(defval)
                , callback(callback_) {}

            // argument::none
            consteval option(const char *long_, const char *brief_) noexcept : long_option(long_), brief(brief_) {}
            consteval option(const char *long_, char short_, const char *brief_) noexcept
                : long_option(long_), short_option(short_), brief(brief_) {}

            // callback
            consteval option(const char *long_, const char *brief_, argument::callback callback_) noexcept
                : long_option(long_), brief(brief_), callback(callback_) {}
            consteval option(const char *long_, char short_, const char *brief_, argument::callback callback_) noexcept
                : long_option(long_), short_option(short_), brief(brief_), callback(callback_) {}

            // other
            consteval option(const char *long_, const char *brief_, argument::type type_,
                             variant::value_type vtype_ = variant::cstring) noexcept
                : long_option(long_), brief(brief_), type(type_), value_type(vtype_) {}
            consteval option(const char *long_, char short_, const char *brief_, argument::type type_,
                             variant::value_type vtype_ = variant::cstring) noexcept
                : long_option(long_), short_option(short_), brief(brief_), type(type_), value_type(vtype_) {}

            consteval option(const char *long_, const char *brief_, argument::type type_, variant defval_) noexcept
                : long_option(long_), brief(brief_), type(type_), value_type(defval_.type()), default_value(defval_) {}
            consteval option(const char *long_, char short_, const char *brief_, argument::type type_, variant defval_) noexcept
                : long_option(long_)
                , short_option(short_)
                , brief(brief_)
                , type(type_)
                , value_type(defval_.type())
                , default_value(defval_) {}

            const char *long_option{};             ///< The "long" option, also used as key (required).
            char short_option{};                   ///< The "short" option (optional).
            const char *brief{};                   ///< Brief description of the option (required).
            argument::type type = argument::none;  ///< Option type.

            variant::value_type value_type = variant::cstring;  ///< Expected value type (if applicable).
            variant default_value{};                            ///< Default value (if applicable).
            argument::callback callback{};                      ///< Callback function.
        };

        /** Alternative construction of positional arguments.
         * @see vidre::arg::option
         * */
        struct VIDRE_CORE_VISIBILITY option::positional : public option {
        public:
            positional() = delete;

            consteval positional(const char *long_, const char *brief_, variant::value_type vtype_ = variant::cstring) noexcept
                : option(long_, brief_, argument::positional, vtype_) {}
        };

        class parser;

        /** FIXME
         * */
        template <size_t N>
        class option::set {
        public:
            /** Number of options.
             * */
            static constexpr size_t size = N;

        public:
            set() = delete;

            set(set &&) = default;
            set(const set &) = delete;

            /** Constructor.
             * @param brief         Short, brief program description.
             * @param description   Long program description.
             * @param options       Program options.
             */
            consteval set(const char *brief_, const char *description_, std::initializer_list<option> options_)
                : brief(brief_), description(description_), options() {
                if (options_.size() != N)
                    throw "size deduction failure";
                std::copy(options_.begin(), options_.end(), options);
            }

        protected:
            const char *brief{};        ///< Brief program description (see @ref examples/core/app.cc).
            const char *description{};  ///< Program usage text.
            option options[N]{};        ///< Options buffer (allowing constant evaluation).

        private:
            friend class parser;
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

    }  // namespace arg

}  // namespace vidre

#endif  // VIDRE_CORE_APPLICATION_ARGUMENTS_HH
