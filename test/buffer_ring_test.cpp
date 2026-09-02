#include <core/buffer/buffer.hpp>
#include <core/runtime/shared_frame_header.hpp>
#include <core/storage/storage.hpp>

#include "test_helpers.hpp"

#include <iostream>
#include <stdexcept>

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

    // The consumer has caught up: slot 3 is still empty. pop() must report nothing AND stay on
    // slot 3, so that frame 4 is found where the producer puts it (regression: the cursor used to
    // advance on empty slots and frame 4 was skipped as "dropped").
    CHECK(buffer.pop().first == nullptr);
    CHECK(buffer.pop().first == nullptr);
    CHECK(buffer.getSavedFramesNumber() == 3);
    setFrame(3, 4);
    writePayloadMarker(3, 0xCD);
    {
        auto [d, h] = buffer.pop();
        CHECK(d != nullptr && h != nullptr);
        CHECK(h != nullptr && h->frameID.load(std::memory_order_relaxed) == 4);
        CHECK(d != nullptr && static_cast<uint8_t*>(d)[0] == 0xCD);
    }
    CHECK(buffer.getSavedFramesNumber() == 4);
    CHECK(buffer.getDroppedFrames() == 0);

    // Gap: frame 4 consumed, next slot holds frame 7 -> 2 dropped (5 and 6).
    setFrame(4, 7);
    CHECK(buffer.pop().first != nullptr);
    CHECK(buffer.getSavedFramesNumber() == 5);
    CHECK(buffer.getDroppedFrames() == 2);

    // Stale frame id (already consumed) in the expected slot is ignored and the cursor stays put.
    setFrame(5, 6);
    CHECK(buffer.pop().first == nullptr);
    CHECK(buffer.getSavedFramesNumber() == 5);
    CHECK(buffer.getDroppedFrames() == 2);

    // Once the producer overwrites that slot with a newer frame it is consumed.
    setFrame(5, 8);
    CHECK(buffer.pop().first != nullptr);
    CHECK(buffer.getSavedFramesNumber() == 6);
    CHECK(buffer.getDroppedFrames() == 2);

    // Frames continue over the ring wrap-around.
    setFrame(6, 9);
    setFrame(7, 10);
    CHECK(buffer.pop().first != nullptr);
    CHECK(buffer.pop().first != nullptr);
    CHECK(buffer.getSavedFramesNumber() == 8);

    setFrame(0, 11);
    CHECK(buffer.pop().first != nullptr);
    CHECK(buffer.getSavedFramesNumber() == 9);
    CHECK(buffer.getDroppedFrames() == 2);

    // A slot whose pointer was never registered yields nothing (and does not advance).
    slots->at(1) = nullptr;
    CHECK(buffer.pop().first == nullptr);
    CHECK(buffer.getSavedFramesNumber() == 9);

    // reset() clears counters and slot pointers.
    buffer.reset();
    CHECK(buffer.getSavedFramesNumber() == 0);
    CHECK(buffer.getDroppedFrames() == 0);
    CHECK(buffer.pop().first == nullptr);

    // A zero-sized ring is rejected instead of dividing by zero later.
    {
        vidre::Config bad = config;
        bad.maxNumBuffer = 0;
        bad.serialNumber = vidre::test::uniqueId("buffer_ring_zero");
        bool threw = false;
        try {
            vidre::Buffer zero(bad);
        } catch (const std::invalid_argument&) {
            threw = true;
        }
        CHECK(threw);
    }

    if (failures != 0) {
        std::cerr << failures << " buffer_ring_test failure(s)\n";
        return 1;
    }
    std::cout << "buffer_ring_test: OK\n";
    return 0;
}
