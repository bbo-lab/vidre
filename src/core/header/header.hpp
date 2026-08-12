#pragma once
#include <string>

namespace vidre {
    struct CCVFrameTail;
    class DiskWriter;
    class IHeader {
    public:
        virtual ~IHeader() = default;
        virtual void initiateHeader(DiskWriter& out_file) = 0;
        virtual void updateHeader(DiskWriter& out_file, int frames) = 0;
        virtual void writeFrame(const void* buffer, DiskWriter& out_file) = 0;
        virtual void writeTail(DiskWriter& out_file, const CCVFrameTail& tail) = 0;
    };
}
