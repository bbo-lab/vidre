#pragma once
#include <cstdint>

namespace vidre {
    struct SharedFrameHeader {
        uint64_t timeStamp; 
        uint64_t frameID;
    };
}  // namespace vidre