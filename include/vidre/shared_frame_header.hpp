#pragma once
#include <cstdint>

namespace vidre {
    struct SharedFrameHeader {
        int64_t frameID;
        uint64_t timeStamp; 
    };
}  // namespace vidre