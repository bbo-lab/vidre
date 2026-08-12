#pragma once
#include <atomic>
#include <cstdint>
#include <limits>
#include <chrono>

namespace vidre {
    inline const auto StartTime = std::chrono::system_clock::now();

    struct SharedFrameHeader {
        std::atomic<int64_t> frameID{0};
        uint64_t timeStamp = 0;
        int64_t computerTimeStamp = 0;
    };
}  // namespace vidre