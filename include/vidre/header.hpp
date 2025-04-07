#pragma once
#include <string>

namespace vidre {
    class IHeader {
    public:
        virtual void initiateHeader(FILE* out_file) {};
        virtual void updateHeader(FILE* out_file, int frames) {};
        virtual void writeFrame(const void* buffer, FILE* out_file) {};

        template <class Header, typename Tail>
        void writeTail(FILE* out_file, Tail tail) {
            static_cast<Header*>(this)->writeTail(out_file, tail);
        }
    };
}