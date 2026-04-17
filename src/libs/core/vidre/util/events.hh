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
#ifndef VIDRE_CORE_UTIL_EVENTS_HH
#define VIDRE_CORE_UTIL_EVENTS_HH
#pragma once

/**@defgroup api-core-util-events Event handling
 * @ingroup api-core-util
 * @details
 * @fileinfo
 * 
 * @{
 * @defgroup api-core-util-events-details Implementation details
 * @}
 * @todo see signalfd
 * */

#include <vidre/core/config.hh>
#include <vidre/exception.hh>
#include <vidre/util/status.hh>

#if defined(__linux__)
#include <poll.h>
#include <sys/eventfd.h>
#include <unistd.h>
#elif defined(__APPLE__)
#include <unistd.h>
#include <fcntl.h>
#elif defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

#include <optional>

#include <atomic>
#include <chrono>
#include <thread>

#include <cassert>

namespace vidre {

    namespace detail {

        /**@addtogroup api-core-util-events-details
         * */
        ///@{

#if defined(__linux__) or defined(DOXYGEN)
        using eventfd_t = int;

        inline eventfd_t eventfd_create(unsigned int initval = 0, int flags = 0) {
            return eventfd(initval, flags | EFD_CLOEXEC | EFD_NONBLOCK);
        }
        
        inline void eventfd_destroy(eventfd_t fd) {
            close(fd);
        }

        inline uint64_t eventfd_notify(eventfd_t fd, uint64_t count = 1) {
            ssize_t ret = write(fd, &count, sizeof(count));
            if (ret != sizeof(count)) {
                switch (errno) {
                    case EAGAIN:
                        return 0;
                    default:
                        throw vidre::exception::runtime() << error::system();
                }
            }
            return count;
        }

        inline std::optional<uint64_t> eventfd_wait(eventfd_t fd) {
            uint64_t count;

            ssize_t ret = read(fd, &count, sizeof(count));
            while ((ret != sizeof(count)) && (errno == EAGAIN)) {
                std::this_thread::yield();
                ret = read(fd, &count, sizeof(count));
            }

            if (ret != sizeof(count)) {
                switch (errno) {
                    case EAGAIN:
                        return std::nullopt;
                    default:
                        throw vidre::exception::runtime() << error::system();
                }
            }

            return count;
        }

        inline std::optional<uint64_t> eventfd_wait(eventfd_t fd, std::chrono::milliseconds timeout) {
            struct pollfd pfd = {fd, POLLIN, 0};
            int ret = poll(&pfd, 1, static_cast<int>(timeout.count()));
            if (ret == 0)
                return std::nullopt;
            else if (ret < 0)
                throw vidre::exception::runtime() << error::system();

            return eventfd_wait(fd);
        }

#elif defined(__APPLE__)
        using eventfd_t = int[2];

        // see https://chat.deepseek.com/a/chat/s/084df7a5-ea3f-4f74-8078-505668d8dd2d
        inline eventfd_t eventfd_create(unsigned int initval = 0, int flags = 0) {}

        inline void eventfd_destroy(eventfd_t fd) {}

        inline uint64_t eventfd_notify(eventfd_t fd, uint64_t count = 1) {}

        inline std::optional<uint64_t> eventfd_wait(eventfd_t fd) {}

        inline std::optional<uint64_t> eventfd_wait(eventfd_t fd, std::chrono::milliseconds timeout) {}
#elif defined(_WIN32)
        using eventfd_t = HANDLE;

        // see https://chat.deepseek.com/a/chat/s/084df7a5-ea3f-4f74-8078-505668d8dd2d
        inline eventfd_t eventfd_create(unsigned int initval = 0, int flags = 0) {}

        inline void eventfd_destroy(eventfd_t fd) {}

        inline uint64_t eventfd_notify(eventfd_t fd, uint64_t count = 1) {}

        inline std::optional<uint64_t> eventfd_wait(eventfd_t fd) {}

        inline std::optional<uint64_t> eventfd_wait(eventfd_t fd, std::chrono::milliseconds timeout) {}
#endif

        ///@}

    }  // namespace detail

    /**@addtogroup api-core-util-events
     * */
    ///@{

    /** Event signalling primitive.
     * */
    class VIDRE_CORE_VISIBILITY event {
    public:
        event() : fd_(detail::eventfd_create()) {}
        ~event() {
            if (fd_)
                detail::eventfd_destroy(fd_);
        }

    protected:
        detail::eventfd_t fd_{};

    public:
        /** Notify the event, i.e. wake up any waiters.
         * @param count The number of notifications to send.
         * @return The number of notifications actually sent.
         * @note On Linux, the eventfd counter is a 64-bit unsigned integer. If the counter would overflow, the call will block
         *       until it can be incremented without overflowing. In this implementation, we return 0 instead of blocking if the
         *       counter is full.
         * */
        uint64_t notify(uint64_t count = 1) {
            assert(count > 0);
            return detail::eventfd_notify(fd_, count);
        }

        /** Wait for the event to be notified.
         * @return The number of notifications received, or std::nullopt if the wait timed out.
         * */
        std::optional<uint64_t> wait() {
            return detail::eventfd_wait(fd_);
        }

        /** Wait for the event to be notified, with a timeout.
         * @param timeout The maximum time to wait for the event.
         * @return The number of notifications received, or std::nullopt if the wait timed out.
         * */
        std::optional<uint64_t> wait(std::chrono::milliseconds timeout) {
            return detail::eventfd_wait(fd_, timeout);
        }
    };

    ///@}

}  // namespace vidre

#endif  //  VIDRE_CORE_UTIL_EVENTS_HH