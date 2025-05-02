/* VidRe
 * Copyright (C) 2024 Max-Planck-Institute f. Neurobiol. of Behavior — caesar, Bonn, Germany
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
#ifndef VIDRE_CORE_UTIL_UUID_HH
#define VIDRE_CORE_UTIL_UUID_HH
#pragma once

#include <vidre/core/config.hh>

#if VIDRE_HAVE_LIBUUID
#include <uuid/uuid.h>
#else
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/time_generator_v1.hpp>
#endif

#include <string>
#include <stdexcept>

namespace vidre {
    /**@defgroup api-core-util-uuid UUID
     * @ingroup api-core-util
     * @details
     * @fileinfo
     *
     * <b>Example</b>:
     * @include examples/core/uuid.cc
     *
     * @see
     * https://en.wikipedia.org/wiki/Universally_unique_identifier
     * */

    namespace detail {

        /** UUID abstraction (libuuid vs. Boost).
         * */
        namespace uuid {}

        ///@cond
#if VIDRE_HAVE_LIBUUID
        struct uuid_t {
            ::uuid_t data;
        };

        inline int compare(const uuid_t &lhs, const uuid_t &rhs) noexcept {
            return uuid_compare(lhs.data, rhs.data);
        }

        inline bool operator==(const uuid_t &lhs, const uuid_t &rhs) noexcept {
            return compare(lhs, rhs) == 0;
        }

        inline std::strong_ordering operator<=>(const uuid_t &lhs, const uuid_t &rhs) noexcept {
            auto value = compare(lhs, rhs);
            if (value == 0)
                return std::strong_ordering::equal;
            else if (value < 0)
                return std::strong_ordering::less;
            else
                return std::strong_ordering::greater;
        }

        namespace uuid {

            enum variant {
                NCS = UUID_VARIANT_NCS,
                DCE = UUID_VARIANT_DCE,
                Microsoft = UUID_VARIANT_MICROSOFT,
                Future = UUID_VARIANT_OTHER
            };

            inline variant get_variant(uuid_t u) noexcept {
                return static_cast<variant>(uuid_variant(u.data));
            }

            enum type {
                Invalid = -1,
                NIL = UUID_TYPE_DCE_NIL,
                Time = UUID_TYPE_DCE_TIME,
                Security = UUID_TYPE_DCE_SECURITY,
                MD5 = UUID_TYPE_DCE_MD5,
                Random = UUID_TYPE_DCE_RANDOM,
                SHA1 = UUID_TYPE_DCE_SHA1
            };

            inline type get_type(uuid_t u) noexcept {
                return static_cast<type>(uuid_type(u.data));
            }

            inline std::string tostring(uuid_t u, bool lowercase) {
                std::string result;
                result.resize(UUID_STR_LEN - 1);

                if (lowercase)
                    uuid_unparse_lower(u.data, result.data());
                else
                    uuid_unparse_upper(u.data, result.data());

                return result;
            }

            struct string_generator {
                string_generator() = default;

                uuid_t operator()(const std::string &s) const {
                    uuid_t result;
                    if (uuid_parse(s.c_str(), result.data) != 0)
                        throw std::runtime_error("not a UUID");
                    return result;
                }
            };

            struct random_generator {
                random_generator() = default;

                uuid_t operator()() const noexcept {
                    uuid_t result;
                    uuid_generate_random(result.data);
                    return result;
                }
            };

            struct time_generator {
                time_generator() = default;

                uuid_t operator()() const noexcept {
                    uuid_t result;
                    uuid_generate_time(result.data);
                    return result;
                }
            };

            struct time_safe_generator {
                time_safe_generator() = default;

                uuid_t operator()() const noexcept {
                    uuid_t result;
                    uuid_generate_time_safe(result.data);
                    return result;
                }
            };

            struct name_generator_md5 {
                uuid_t ns;

                name_generator_md5(uuid_t ns_) noexcept : ns(ns_) {}

                uuid_t operator()(const std::string &name) const noexcept {
                    uuid_t result;
                    uuid_generate_md5(result.data, ns.data, name.data(), name.size());
                    return result;
                }
            };

            struct name_generator_sha1 {
                uuid_t ns;

                name_generator_sha1(uuid_t ns_) noexcept : ns(ns_) {}

                uuid_t operator()(const std::string &name) const noexcept {
                    uuid_t result;
                    uuid_generate_sha1(result.data, ns.data, name.data(), name.size());
                    return result;
                }
            };

            struct generator {
                generator() = default;

                uuid_t operator()() const noexcept {
                    uuid_t result;
                    uuid_generate(result.data);
                    return result;
                }
            };

        }  // namespace uuid
#else
        using uuid_t = boost::uuids::uuid;

        namespace uuid {

            enum variant {
                NCS = boost::uuids::uuid::variant_type::variant_ncs,
                DCE = boost::uuids::uuid::variant_type::variant_rfc_4122,
                Microsoft = boost::uuids::uuid::variant_type::variant_microsoft,
                Future = boost::uuids::uuid::variant_type::variant_future
            };

            inline variant get_variant(uuid_t u) noexcept {
                return static_cast<variant>(u.variant());
            }

            enum type {
                Invalid = boost::uuids::uuid::version_type::version_unknown,
                NIL = 0,
                Time = boost::uuids::uuid::version_type::version_time_based,
                Security = boost::uuids::uuid::version_type::version_dce_security,
                MD5 = boost::uuids::uuid::version_type::version_name_based_md5,
                Random = boost::uuids::uuid::version_type::version_random_number_based,
                SHA1 = boost::uuids::uuid::version_type::version_name_based_sha1
            };

            inline type get_type(uuid_t u) noexcept {
                if (u.is_nil())
                    return type::NIL;
                else
                    return static_cast<type>(u.version());
            }

            inline std::string tostring(uuid_t u, bool lowercase) {
                std::string result;
                // result.resize(UUID_STR_LEN - 1);

                // if (lowercase)
                //     uuid_unparse_lower(u.data, result.data());
                // else
                //     uuid_unparse_upper(u.data, result.data());

                return result;
            }

            using boost::uuids::random_generator;
            using boost::uuids::string_generator;

            struct time_generator {
                time_generator() = default;

                uuid_t operator()() const noexcept {
                    boost::uuids::time_generator_v1 gen;  // FIXME v6/v7 ?
                    return gen();
                }
            };

            struct time_safe_generator {
                time_safe_generator() = default;

                uuid_t operator()() const noexcept;
            };

            using boost::uuids::name_generator_md5;
            using boost::uuids::name_generator_sha1;

            struct generator {
                generator() = default;

                uuid_t operator()() const noexcept;
            };

        }  // namespace uuid
#endif
        ///@endcond

    }  // namespace detail

    /**@addtogroup api-core-util-uuid
     * */
    ///@{

    /** UUIDs.
     *
     * This class wraps the [uuid](https://www.man7.org/linux/man-pages/man3/uuid.3.html) library if available,
     * otherwise uses [Boost](https://www.boost.org/).
     *
     * @see
     *      [RFC 4122](https://datatracker.ietf.org/doc/html/rfc4122)
     * */
    class VIDRE_CORE_VISIBILITY uuid {
    public:
        /** UUID variant.
         * @see
         *      [RFC 4122](https://datatracker.ietf.org/doc/html/rfc4122)
         * */
        enum class variant {
            NCS = detail::uuid::variant::NCS,  ///< Apollo NCS variant (backwards compatibility).
            DCE = detail::uuid::variant::DCE,  ///< DCE 1.1 UUIDs (see [RFC 4122](https://datatracker.ietf.org/doc/html/rfc4122)).
            Microsoft = detail::uuid::variant::Microsoft,  ///< Microsoft COM/DCOM variant (compatibility).
            Future = detail::uuid::variant::Future         ///< Reserved for future versions.
        };

        /** DCE variant type.
         * @see
         *      [RFC 4122](https://datatracker.ietf.org/doc/html/rfc4122)
         * */
        enum class type {
            Invalid = detail::uuid::type::Invalid,  ///< Invalid type.
            NIL = detail::uuid::type::NIL,          ///< "NIL" (refers to the ["Nil
                                            ///< UUID"](https://en.wikipedia.org/wiki/Universally_unique_identifier#Nil_UUID)).
            Time = detail::uuid::type::Time,          ///< Time-/MAC-based.
            Security = detail::uuid::type::Security,  ///< Security version, with embedded POSIX UIDs.
            MD5 = detail::uuid::type::MD5,            ///< Name-based with MD5 hashing.
            Random = detail::uuid::type::Random,      ///< Random or pseudo-random.
            SHA1 = detail::uuid::type::SHA1           ///< Name-based with SHA1 hashing.
        };

        /** Generate random or time-based UUID.
         *
         * This function generates a random UUID if a high-quality random
         * source is available, a time-/MAC-based UUID otherwise.
         *
         * @see
         *      uuid::makeTime() <br/>
         *      uuid::makeTimeSafe() <br/>
         *      uuid::makeRandom()
         * */
        static uuid make() noexcept {
            detail::uuid::generator gen;
            return gen();
        }

        /** Create a time- and MAC-based UUID.
         *
         * > This function forces the use of the alternative algorithm which uses
         * > the current time and the local ethernet MAC address (if available).
         * > This algorithm used to be the default one used to generate UUIDs,
         * > but because of the use of the ethernet MAC address, it can leak
         * > information about when and where the UUID was generated.
         * (see [uuid_generate_time man-page](https://www.man7.org/linux/man-pages/man3/uuid_generate.3.html))
         *
         * Note, however, that uniqueness of UUIDs is not guaranteed.
         *
         * @see
         *      uuid::makeTimeSafe()
         * */
        static uuid makeTime() noexcept {
            detail::uuid::time_generator gen;
            return gen();
        }

        /** Safely create a time- and MAC-based UUID.
         *
         * @throws std::runtime_error() if synchronization fails
         *
         * This function uses synchronization mechanisms to ensure uniqueness
         * of time-based UUIDs
         *
         * @see
         *      uuid::makeTime()
         * */
        static uuid makeTimeSafe() {
            detail::uuid::time_safe_generator gen;
            return gen();
        }

        /** Create a random or pseudo-random UUID.
         * */
        static uuid makeRandom() noexcept {
            detail::uuid::random_generator gen;
            return gen();
        }

        /** Create a name-based UUID with MD5 hashing.
         * */
        static uuid makeMD5(const uuid ns, const std::string &name) noexcept {
            detail::uuid::name_generator_md5 gen(ns.uuid_);
            return gen(name);
        }

        /** Create a name-based UUID with SHA-1 hashing.
         * */
        static uuid makeSHA1(const uuid ns, const std::string &name) noexcept {
            detail::uuid::name_generator_sha1 gen(ns.uuid_);
            return gen(name);
        }

    public:
        uuid() = default;
        uuid(const std::string &s) {
            detail::uuid::string_generator gen;
            uuid_ = gen(s);
        }
        uuid(const detail::uuid_t &u) noexcept : uuid_(u) {}

    protected:
        detail::uuid_t uuid_{};

    public:
        /** Implicit variant conversion.
         *
         * Usage:
         * @code
         * void fn(const uuid &uuid) {
         *     if (uuid == uuid::Variant::DCE) {
         *         // ...
         *     }
         * }
         * @endcode
         * */
        operator variant() const noexcept {
            return static_cast<variant>(detail::uuid::get_variant(uuid_));
        }

        /** Implicit DCE type conversion.
         * @see
         *     `uuid::operator Variant()`
         * */
        operator type() const noexcept {
            return static_cast<type>(detail::uuid::get_type(uuid_));
        }

        /** Convert to string.
         * @param lowercase Output with lower-case letters (default: false).
         * */
        std::string tostring(bool lowercase = false) const noexcept {
            return detail::uuid::tostring(uuid_, lowercase);
        }

        /** Coerce to string.
         * */
        operator std::string() const noexcept {
            return tostring();
        }

        /** Equality.
         * */
        bool operator==(const uuid &other) const noexcept {
            return uuid_ == other.uuid_;
        }
        bool operator==(const std::string &s) const noexcept {
            return std::string(*this) == s;
        }
    };

    ///@}
}  // namespace vidre

#endif  // VIDRE_CORE_UTIL_UUID_HH
