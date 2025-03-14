#pragma once

namespace vidre {
    class IHeader {
    public:
        virtual void writeHeader() = 0;
        virtual void updateHeader() = 0;
        virtual void writeFrame() = 0;
        virtual void writeTail() = 0;
    };
}