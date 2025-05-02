/* VidRe
 * Copyright (C) 2024-2025 Max-Planck-Institute f. Neurobiol. of Behavior — caesar, Bonn, Germany
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
#ifndef VIDRE_FFMPEG_UTIL_DICTIONARY_HH
#define VIDRE_FFMPEG_UTIL_DICTIONARY_HH
#pragma once

#include <vidre/ffmpeg/config.hh>
#include <vidre/ffmpeg/util/exception.hh>

extern "C" {
#include <libavutil/dict.h>
}

namespace vidre {

    namespace ffmpeg {
        /**@addtogroup api-support-ffmpeg-util-data
         * */
        ///@{

        /** Dictionary.
         * @see [AVDictionary etal.](https://ffmpeg.org/doxygen/trunk/group__lavu__dict.html)
         * */
        class VIDRE_FFMPEG_VISIBILITY dictionary {
        public:
            /** Dictionary and method flags.
             * */
            enum class flags {
                normal = 0,                              ///< Default behavior.
                match_cast = AV_DICT_MATCH_CASE,         ///< Only get an entry with exact-case key match.
                ignore_suffix = AV_DICT_IGNORE_SUFFIX,   ///< Return first entry in a dictionary whose first part corresponds to the
                                                         ///< search key, ignoring the suffix of the found key string.
                no_key_dup = AV_DICT_DONT_STRDUP_KEY,    ///< Take ownership of a key that's been allocated with av_malloc() or
                                                         ///< another memory allocation function.
                no_value_dup = AV_DICT_DONT_STRDUP_VAL,  ///< Take ownership of a value that's been allocated with av_malloc() or
                                                         ///< another memory allocation function.
                no_overwrite = AV_DICT_DONT_OVERWRITE,   ///< Don't overwrite existing entries.
                append = AV_DICT_APPEND,                 ///< If the entry already exists, append to it.
                multi_key = AV_DICT_MULTIKEY             ///< Allow to store several equal keys in the dictionary.
            };

            /** Dictionary entry.
             * */
            using entry = AVDictionaryEntry;

            ///@cond
            /** Helper for construction with initializer_list.
             * */
            struct const_entry {
                const char *key;
                const char *value;
            };
            ///@endcond

        protected:
            AVDictionary *m_ = {};

        public:
            dictionary() noexcept = default;
            ~dictionary() {
                if (m_)
                    av_dict_free(&m_);
            }

            /** Constrution with elements.
             * Typical usage:
             * @code
             * ffmpeg::dict dict = {{"keya", "valuea"}, {"keyb", "valueb"}};
             * @endcode
             */
            dictionary(std::initializer_list<const_entry> list) {
                for (auto &e : list)
                    set(e.key, e.value);
            }

            // FIXME need a vidre::options type and a constructor for it.

        public:
            /** Dereference AVDictionary.
             * */
            AVDictionary *&operator&() noexcept {
                return m_;
            }
            AVDictionary *const &operator&() const noexcept {
                return m_;
            }

            /** Assign dictionary entry.
             * */
            void set(const char *key, const char *value, flags flags_ = flags::normal) {
                auto ecode = av_dict_set(&m_, key, value, static_cast<int>(flags_));
                if (ecode < 0) {
                    throw ffmpeg::exception() << ffmpeg::errc_info(static_cast<ffmpeg::errc>(AVERROR(ENOMEM)));
                }
            }

            // TODO
            // av_dict_get
            // av_dict_count
            // av_dict_iterate
            // av_dict_set_int
            // av_dict_copy
            // av_dict_parse_string
            // av_dict_get_string
        };

        ///@}
    }  // namespace ffmpeg

}  // namespace vidre

#endif  // VIDRE_FFMPEG_UTIL_DICTIONARY_HH
