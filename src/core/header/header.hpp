#pragma once
#include <string>

namespace vidre {
    class DiskWriter;
    class IHeader {
    public:
        virtual void initiateHeader(DiskWriter& out_file) {};
        virtual void updateHeader(DiskWriter& out_file, int frames) {};
        virtual void writeFrame(const void* buffer, DiskWriter& out_file) {};

        template <class Header, typename Tail>
        void writeTail(DiskWriter& out_file, Tail tail) {
            static_cast<Header*>(this)->writeTail(out_file, tail);
        }
    };
}