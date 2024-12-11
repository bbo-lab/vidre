#pragma once
#include <cstdint>

struct SharedImageHeader
{
    uint64_t time_stamp; // Timestamp data type as suggested by Pylon.
    uint64_t frame_id;
    // bool is_valid;     // Flag to determine if a header has become invalid. Might also be detectable by comparing frame_ids.
};