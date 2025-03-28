#pragma once
#include <string>

namespace vidre {
    class IHeader {
    public:
        virtual void initiateHeader(const std::string& out_file) {};
        virtual void updateHeader(const std::string& out_file, int frames) {};
        virtual void writeFrame(const void* buffer, const std::string& out_file) {};

        template <class Header, typename Tail>
        void writeTail(const std::string& out_file, Tail tail) {
            static_cast<Header*>(this)->writeTail(out_file, tail);
        }
    };
}