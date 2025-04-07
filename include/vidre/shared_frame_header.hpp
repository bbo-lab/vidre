#pragma once
#include <cstdint>

namespace vidre {
    struct SharedFrameHeader {
        int64_t frameID = 0;
        uint64_t timeStamp = 0; 
    };
}  // namespace vidre