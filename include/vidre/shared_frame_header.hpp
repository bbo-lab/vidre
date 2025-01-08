#pragma once
#include <cstdint>

namespace vidre {
struct SharedFrameHeader {
    // Type/Tag
    uint64_t timeStamp;  // Timestamp data type as suggested by Pylon.
    uint64_t frameID;
    int64_t numSkippedBefore;
    // - bool is_valid;   // Flag to determine if a header has become invalid. Might also be detectable by comparing frame_ids.
    // Length
    // - uint16_t width;
    // - uint16_t height;
    // - Pixel Type
    // Values
    // ...
};
}