#pragma once
#include <cstdint>
#include <limits>
#include <chrono>

namespace vidre {
    const auto StartTime = std::chrono::system_clock::now();

    struct SharedFrameHeader {
        int64_t frameID = 0;
        uint64_t timeStamp = 0;
        long computerTimeStamp = 0;  // ms
    };
}  // namespace vidre