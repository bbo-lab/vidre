#include <core/buffer/buffer.hpp>
#include <core/runtime/shared_frame_header.hpp>
#include <core/storage/storage.hpp>

#include "test_helpers.hpp"

#include <iostream>

namespace {

int failures = 0;

#define CHECK(cond)                                                                            \
    do {                                                                                       \
        if (!(cond)) {                                                                         \
            std::cerr << "FAIL: " #cond " at " << __FILE__ << ":" << __LINE__ << "\n";         \
            ++failures;                                                                        \
        }                                                                                      \
    } while (0)

}  // namespace

int main() {
    constexpr unsigned int kWidth = 64;
    constexpr unsigned int kHeight = 48;
    constexpr unsigned int kBuffers = 8;

    vidre::Config config{};
    config.width = kWidth;
    config.height = kHeight;
    config.maxNumBuffer = kBuffers;
    config.paddingX = 0;
    config.frameRate = 120.0;
    config.frameBytesActual = kWidth * kHeight;
    config.frameBytesBuffer = kWidth * kHeight;
    config.serialNumber = vidre::test::uniqueId("buffer_ring_test");
    config.bitsPerPixel = 8;
    config.packed = false;
    config.cameraVendor = "dummy";

    vidre::Buffer buffer(config);
    CHECK(buffer.numSlots() == kBuffers);
    CHECK(buffer.getSavedFramesNumber() == 0);
    CHECK(buffer.getDroppedFrames() == 0);

    // Populate the ring the same way DummyCamera::allocateBuffers does.
    auto* memory = buffer.getSharedMemory();
    auto* slots = buffer.getSharedMemoryPtrs();
    CHECK(memory != nullptr && slots != nullptr);
    CHECK(slots->size() == kBuffers);
    for (unsigned int i = 0; i < kBuffers; ++i) {
        void* block = memory->allocate(sizeof(vidre::SharedFrameHeader) + config.frameBytesBuffer);
        new (block) vidre::SharedFrameHeader{};
        slots->at(i) = block;
    }

    auto setFrame = [slots](unsigned int slot, int64_t id) {
        auto* header = static_cast<vidre::SharedFrameHeader*>(slots->at(slot));
        header->frameID.store(id, std::memory_order_release);
    };
    auto writePayloadMarker = [slots](unsigned int slot, uint8_t marker) {
        auto* block = static_cast<uint8_t*>(slots->at(slot));
        block[sizeof(vidre::SharedFrameHeader)] = marker;
    };

    // Sequential frames 1..3 in slots 0..2.
    setFrame(0, 1);
    writePayloadMarker(0, 0xAB);
    auto [data, header] = buffer.pop();
    CHECK(data != nullptr && header != nullptr);
    CHECK(header->frameID.load(std::memory_order_relaxed) == 1);
    CHECK(static_cast<uint8_t*>(data)[0] == 0xAB);  // payload starts right after the header
    CHECK(buffer.getSavedFramesNumber() == 1);
    CHECK(buffer.getDroppedFrames() == 0);

    setFrame(1, 2);
    setFrame(2, 3);
    CHECK(buffer.pop().first != nullptr);
    CHECK(buffer.pop().first != nullptr);
    CHECK(buffer.getSavedFramesNumber() == 3);
    CHECK(buffer.getDroppedFrames() == 0);

    // Gap: frame 3 consumed, next slot holds frame 6 -> 2 dropped.
    setFrame(3, 6);
    CHECK(buffer.pop().first != nullptr);
    CHECK(buffer.getSavedFramesNumber() == 4);
    CHECK(buffer.getDroppedFrames() == 2);

    // Stale frame id (already consumed) is ignored.
    setFrame(4, 5);
    CHECK(buffer.pop().first == nullptr);
    CHECK(buffer.getSavedFramesNumber() == 4);
    CHECK(buffer.getDroppedFrames() == 2);

    // Empty slot (pointer never registered) yields nothing.
    CHECK(buffer.pop().first == nullptr);
    CHECK(buffer.getSavedFramesNumber() == 4);

    // Frames continue over the ring wrap-around.
    setFrame(6, 7);
    setFrame(7, 8);
    CHECK(buffer.pop().first != nullptr);
    CHECK(buffer.pop().first != nullptr);
    CHECK(buffer.getSavedFramesNumber() == 6);

    setFrame(0, 9);
    CHECK(buffer.pop().first != nullptr);
    CHECK(buffer.getSavedFramesNumber() == 7);

    // reset() clears counters and slot pointers.
    buffer.reset();
    CHECK(buffer.getSavedFramesNumber() == 0);
    CHECK(buffer.getDroppedFrames() == 0);
    CHECK(buffer.pop().first == nullptr);

    if (failures != 0) {
        std::cerr << failures << " buffer_ring_test failure(s)\n";
        return 1;
    }
    std::cout << "buffer_ring_test: OK\n";
    return 0;
}
