#pragma once

namespace vidre {
    template <typename T>
    class IHeader {
    public:
        virtual void initiateHeader(const std::string& out_file) = 0;
        virtual void updateHeader(const std::string& out_file) = 0;
        virtual void writeFrame(const void* buffer, const std::string& out_file) = 0;
        virtual void writeTail(const std::string& out_file, const T& tail) = 0;
    };
}