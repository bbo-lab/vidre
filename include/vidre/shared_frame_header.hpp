#pragma once
#include <cstdint>
#include <limits>
#include <chrono>

namespace vidre {
    static auto StartTime = std::chrono::system_clock::now();

    struct SharedFrameHeader {
        int64_t frameID = std::numeric_limits<int64_t>::max();
        uint64_t timeStamp = 0; 
        long computerTimeStamp = 0; // ms
    };
}  // namespace vidre