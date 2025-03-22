#pragma once
#include <cstdint>

namespace vidre {
    struct SharedFrameHeader {
        void* data;
        int64_t frameID;
        uint64_t timeStamp; 
    };
}  // namespace vidre