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
#ifndef VIDRE_CORE_IO_PACKET_BASE_HH
#define VIDRE_CORE_IO_PACKET_BASE_HH
#pragma once

/**@addtogroup api-core-io-packet-base
 * @brief Fundamental types for defining and handling packets.
 * @details
 * @fileinfo
 */

#include <vidre/core/config.hh>
#include <vidre/cpp/shared.hh>
#include <vidre/util/uuid.hh>
#include <vidre/util/chrono.hh>
#include <vidre/io/types.hh>
#include <vidre/settings/json.hh>

#include <bit>
#include <shared_mutex>
#include <queue>

namespace vidre {

    namespace io {

        // forward declarations
        class manifest;

        namespace detail {

            /**@addtogroup api-core-io-details
             * */
            ///@{

            /** Require IEEE 754 floating point types.
             * The internal packet system only works at this moment, if the standard
             * floating point data types conform to the IEEE 754 standard.
             *
             * If, though, raw file storage has a system description in the header,
             * this requirement could be relaxed in the future.
             * */
            static_assert(std::numeric_limits<float>::is_iec559);
            static_assert(sizeof(float) == sizeof(uint32_t));
            static_assert(sizeof(double) == sizeof(uint64_t));

            ///@}

        }  // namespace detail

        /**@addtogroup api-core-io-packet-base
         * */
        ///@{

        /** VidRe's magic number.
         * The magic number is expected to be written as byte sequence, therefore,
         * also giving information about the endianness of a transmission.
         *
         * @see `io::proto`
         * */
        static constexpr uint64_t magic = 0x4F424265'52646956;  ///< File magic ("VidReBBO").

        /** Default byte alignment.
         * */
        inline constexpr size_t alignment = 8;

        /** Version layout.
         * */
        struct __vidre_packed__(8) VIDRE_CORE_VISIBILITY version_info {
            uint32_t major{};
            uint32_t minor{};

            auto operator<=>(const version_info &) const = default;
        };

        /** Current version.
         * */
        static constexpr version_info version = {1, 0};

        /** Protocol identification header.
         *
         * This header is used to identify the protocol version and endianness
         * of a file or a transmission. It serves as initial bootstrap mechanism
         * for subsequent parsing and interpretation of the file or transmission.
         *
         * In that, it provides the package magic for file storage, and, as well,
         * protocol identification when sent via network or pipes.
         *
         * */
        struct __vidre_packed__(8) VIDRE_CORE_VISIBILITY proto {
            uint64_t magic = vidre::io::magic;          ///< File or transmission magic number.
            version_info version = vidre::io::version;  ///< File or transmission version.

            /** Create proto object from a memory view.
             * */
            static proto make(memory::view view) {
                proto p;
                if (view.size() < sizeof(proto))
                    throw exception::runtime() << "view too small for proto header";
                memory::copy(view, p);

                if (!p.valid())
                    throw exception::runtime() << "invalid proto header";

                if (!p.compatible())
                    throw exception::runtime() << "incompatible proto version";

                if (p.endian() != std::endian::native) {
                    p.version.major = std::byteswap(p.version.major);
                    p.version.minor = std::byteswap(p.version.minor);
                }

                return p;
            }

            /** Check if valid.
             * */
            bool valid() const noexcept {
                return (magic == vidre::io::magic) || (magic == std::byteswap(vidre::io::magic));
            }

            /** Check if version is compatible.
             * */
            bool compatible() const noexcept {
                return (version.major == vidre::io::version.major) && (version.minor <= vidre::io::version.minor);
            }

            /** Detect the endianness from the magic number.
             * */
            std::endian endian() const noexcept {
                if (magic == vidre::io::magic)
                    return std::endian::native;
                else
                    return (std::endian::native == std::endian::little)  //
                               ? std::endian::big
                               : std::endian::little;
            }
        };

        /** Major/minor packet code concept.
         * The concept ensures that a type can be used as a major or minor packet code.
         * @see code_cast<T>
         * */
        template <typename T>
        concept packet_code = std::integral<std::remove_cvref_t<T>> ||              //
                              (std::is_enum_v<std::remove_cvref_t<T>> &&            //
                               std::same_as<                                        //
                                   std::underlying_type_t<std::remove_cvref_t<T>>,  //
                                   uint8_t>);

        /** Cast a packet code to its underlying uint8_t representation.
         * @tparam T The type of the packet code.
         * @param value The packet code value to cast.
         * @return The underlying uint8_t representation of the packet code.
         * */
        template <packet_code T>
        constexpr uint8_t code_cast(T value) {
            if constexpr (not std::is_enum_v<std::remove_cvref_t<T>>) {
                if (!std::in_range<uint8_t>(value))
                    throw exception::logic() << "packet code exceeds uint8_t range";
            }

            return static_cast<uint8_t>(value);
        }

        /** Stream packets.
         *
         * This reflects the basic type of an IO packet ([TLV
         * encoded](https://en.wikipedia.org/wiki/Type%E2%80%93length%E2%80%93value)),
         * used for dispatching to the appropriate handler. The actual content
         * of the packet is determined by the major and minor types, and may be
         * further specified by the stream ID (e.g., for multiplexed streams).
         *
         * Example:
         * @snippet examples/core/packetio.cc simple-packet
         * */
        class VIDRE_CORE_VISIBILITY packet {
        public:
            /** The "nil" packet.
             * This nil packet represents an empty or uninitialized packet, but is
             * also used instead of nullptr during shared<packet> operations.
             * */
            static shared<packet> nil;

        public:
            /** Major type of a packet.
             * */
            enum class major : uint8_t {
                control = 0x00,   ///< Control packets, not intended for storage (see FIXME).
                log = 0x01,       ///< Log message (see `log::packet`).
                generic = 0x02,   ///< Generic packets (see FIXME).
                                  //
                                  // Configuration data (JSON-like):
                value = 0x08,     ///< Plain data (see `io::packet::value`).
                sequence = 0x09,  ///< Sequence of values (see `io::packet::sequence`).
                object = 0x0A,    ///< Structured data (see `io::packet::object`).
                manifest = 0x0F,  ///< Manifest data (see `io::manifest`).
                                  //
                                  // Device data:
                video = 0x10,     ///< Video data (see @ref FIXME).
                audio = 0x20,     ///< Audio data (see @ref FIXME).
                sensor = 0x30,    ///< Sensor data, e.g. IMU, temperature (see @ref FIXME).
                                  //
                                  // === user types
                user = 0xA0,      ///< User-defined packet types (range 0xA0-0xFE, see @ref FIXME).
                                  //
                invalid = 0xFF,   ///< Invalid packet type (reserved for internal use).
            };

            /** Minor type of a packet.
             * FIXME
             * @note For every major type, the minor type '0x0000' is reserved for
             *       the group (one prominent exception is the log packet, which does
             *       not follow this scheme—[«Ausnahmen bestätigen die
             * Regel»](https://en.wikipedia.org/wiki/Exception_that_proves_the_rule)).
             * */
            enum class minor : uint8_t {
                nil = 0x00,  ///< The "nil".
            };

            /** Packet ID.
             * */
            struct __vidre_packed__(2) VIDRE_CORE_VISIBILITY id {
                uint8_t major{};  ///< Major packet type.
                uint8_t minor{};  ///< Sub-type.

                constexpr id() = default;
                template <packet_code Major, packet_code Minor>
                constexpr id(Major major_, Minor minor_) : major(code_cast(major_)), minor(code_cast(minor_)) {}

                constexpr bool operator==(const id &other) const noexcept {
                    return major == other.major && minor == other.minor;
                }

                /** Compare the packet ID with a major packet code.
                 * @tparam T The major packet code to compare with.
                 * @param lhs The packet ID.
                 * @param rhs The major packet code.
                 * @return True if the major packet code of the ID matches the given code, false otherwise.
                 * */
                template <packet_code T>
                friend constexpr bool operator==(const id &lhs, T rhs) noexcept {
                    return lhs.major == static_cast<uint16_t>(rhs);
                }
            };

            /** The packet header.
             * */
            struct __vidre_packed__(8) VIDRE_CORE_VISIBILITY header {
                id type{};          ///< Packet ID.
                uint32_t stream{};  ///< Stream IDfactory (e.g., for multiplexed streams).
                uint64_t length{};  ///< Size of the packet payload (excluding the header).
            };

            static_assert(sizeof(header) == 16);

            /** Create a shared pointer to a new packet.
             * This method simplifies the creation of shared pointers to packets,
             * forwarding the provided arguments to the constructor of the packet type.
             * @tparam T The type of the packet to create.
             * @tparam Args The types of the arguments to forward to the packet constructor.
             * @param args The arguments to forward to the packet constructor.
             * @return A shared pointer to the newly created packet.
             * */
            template <typename T, typename... Args>
            static auto make(Args &&...args) {
                return make_shared<T>(std::forward<Args>(args)...);
            }

            /** Create a shared pointer to a static packet.
             * This method creates a shared pointer to a packet that is
             * statically allocated, meaning it does not manage the
             * lifetime of the packet. The provided deleter does nothing,
             * ensuring that the static packet is not deleted when the
             * shared pointer goes out of scope.
             * @tparam T The type of the packet.
             * @param obj The static packet object.
             * @return A shared pointer to the static packet.
             * */
            template <typename T>
            static auto make_static(T &obj) {
                return shared<T>(&obj, foreign<T>{});
            }

            static shared<packet> decode(memory::view view, std::endian from = std::endian::native);

            template <typename Packet = io::packet>
            requires std::derived_from<Packet, io::packet>
            static shared<Packet> decode_as(memory::view view, std::endian from = std::endian::native) {
                return std::static_pointer_cast<Packet>(decode(view, from));
            }

        public:
            packet() = default;
            packet(const packet &) = delete;
            packet &operator=(const packet &) = delete;
            packet(packet &&) = default;
            packet &operator=(packet &&) = default;

            packet(id type_, uint32_t stream = 0, uint64_t length = 0) noexcept : hdr{type_, stream, length} {}
            template <packet_code Major, packet_code Minor>
            packet(Major major, Minor minor, uint32_t stream = 0, uint64_t length = 0) noexcept
                : hdr{{major, minor}, stream, length} {}

            packet(const header &hdr_) noexcept : hdr{hdr_} {}
            virtual ~packet() = default;

        protected:
            header hdr{};  ///< Packet header.

        public:
            /** Get size of serialized packet data (aligned).
             * @return The size of the packet data, aligned to the required byte boundary.
             * */
            constexpr size_t size() const noexcept {
                return sizeof(header) + ((hdr.length + alignment - 1) & ~(alignment - 1));
            }

            /** Get the packet header.
             * @return The packet header.
             * */
            constexpr const header &getheader() const noexcept {
                return hdr;
            }

            /** Get packet ID.
             * @return The ID of the packet.
             * */
            constexpr id getid() const noexcept {
                return hdr.type;
            }

            /** Get packet major.
             * @return The major code of the packet.
             * */
            constexpr major getmajor() const noexcept {
                return static_cast<major>(hdr.type.major);
            }

            /** Finalize packet for delivery.
             *
             * This method is intended to be overridden by derived packet types
             * that need to update their length based on their content or other
             * factors. The default implementation does nothing, because the
             * `header` is not part of the packet length.
             *
             * @note This method is automatically called before writing the packet
             *       to a sink in the base class implementation of `emit()`.
             * */
            virtual void finalize() noexcept {}

            /** Write the packet to a sink.
             * @param f The sink to write to.
             * @return The result of the write operation, including any error status and the number of bytes written.
             *
             * Inheriting classes must override this method to provide the specific
             * behavior for writing their packet data to the sink, for example:
             * @code
             * io::result log::packet::emit(io::sink &f) override {
             *   return io::packet::emit(f)
             *          + f.put(timestamp)
             *          + f.put({message.size(), reinterpret_cast<const uint8_t *>(message.data())})
             *          + pad(f);
             * }
             * @endcode
             *
             * */
            virtual result emit(io::sink &f);

            /** Helper for padding.
             * @note This method is to be called when the packet payload possibly
             *       does not align to the required byte boundary (see `alignment`)
             *       as packet writing finalization step.
             * */
            result pad(io::sink &f) const;

            /** "Absorb" the packet from memory.
             * @param view The memory view containing the packet data.
             * */
            virtual void absorb(memory::view view, std::endian endian = std::endian::native);

            /** Convert to JSON (if applicable).
             * This method has to be overridden by value-type packets (i.e., packet::value,
             * packet::sequence, and packet::object) and packets, that are used in the
             * `io::manifest` to provide a JSON representation.
             * */
            virtual settings::json to_json() const {
                return {};
            }

        public:
            class queue;

            class value;
            class sequence;
            class object;

            template <typename T>
            struct registry;

        private:
            friend struct factory;
        };

        /** Queue for packets.
         *
         * This class provides a thread-safe queue for managing packets. It ensures
         * that multiple threads can safely add and remove packets without causing data
         * races or inconsistencies.
         *
         * In addition, it provides mechanisms to control the maximum depth of the queue,
         * allowing for both fixed and dynamic queue sizes.
         *
         * @see setdepth()
         * */
        class VIDRE_CORE_VISIBILITY packet::queue {
        public:
            /** Dynamic queue size indicator.
             * */
            constexpr static size_type dynamic = size_type(-1);

        public:
            queue(size_type d = dynamic) noexcept : depth_(d + (d == 0) * dynamic) {}

            queue(const queue &) = delete;
            queue &operator=(const queue &) = delete;
            queue(queue &&) = delete;
            queue &operator=(queue &&) = delete;

        protected:
            std::queue<shared<packet>> queue_{};  ///< The underlying queue storing the packets.
            mutable std::shared_mutex mutex_{};   ///< Mutex to protect access to the queue.
            size_type depth_{dynamic};            ///< Maximum size of the queue, dynamic if not fixed.

        public:
            /** Maximum depth of the queue.
             * @see setdepth()
             * */
            size_type depth() const noexcept {
                return depth_;
            }

            /** Set maximum depth of the queue.
             *
             * Devices, such as cameras, do not typically reallocate space for data packets
             * once allocated during device setup. This means, that packets may be overwritten
             * when the device produces new packets. This queue must, therefore, never hold
             * "dirty" (ie. overwritten) packets.
             *
             * @note The depth of the queue must be set before usage.
             *
             * @param d The new maximum depth (0 is understood as equivalent to the `dynamic` indicator).
             * @see push()
             * */
            void setdepth(size_type d) noexcept {
                depth_ = (d == 0) ? dynamic : d;
            }

            /** Number of queued packets.
             * */
            size_type size() const noexcept {
                std::shared_lock lock(mutex_);
                return queue_.size();
            }

            /** Check if the queue is empty.
             * */
            bool empty() const noexcept {
                std::shared_lock lock(mutex_);
                return queue_.empty();
            }

            /** Access the front packet in the queue.
             * @returns The packet at the front of the queue, or nullptr if the queue is empty.
             * */
            shared<packet> front() const noexcept {
                std::shared_lock lock(mutex_);
                if (!queue_.empty())
                    return queue_.front();
                return nullptr;
            }

            /** Push a packet into the queue.
             * @param p The packet to be added to the queue.
             * @returns `true` if the packet was successfully added to the queue,
             *          `false` if a packet had to be dropped (see `setdepth()`).
             * */
            bool push(shared<packet> p) {
                std::unique_lock lock(mutex_);
                if (queue_.size() < depth_) [[likely]]
                    queue_.push(std::move(p));
                else {
                    if (depth_ == 1) {
                        queue_.front() = std::move(p);
                    }
                    else {
                        queue_.pop();
                        queue_.push(std::move(p));
                    }
                    return false;
                }

                return true;
            }

            /** Pop the front packet from the queue.
             * */
            shared<packet> pop() {
                std::unique_lock lock(mutex_);

                if (queue_.empty())
                    return nullptr;

                auto p = std::move(queue_.front());
                queue_.pop();
                return p;
            }
        };

        ///@}

    }  // namespace io

}  // namespace vidre

#endif  // VIDRE_CORE_IO_PACKET_BASE_HH
