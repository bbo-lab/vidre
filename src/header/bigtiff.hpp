#pragma once

#include "config.h"

#include "vidre/header.hpp"



namespace vidre {

    class BigTIFFHeader : public IHeader {
    public:
        BigTIFFHeader() :
            byteOrder(0x4949),    // Default to little-endian
            version(43),          // BigTIFF version
            offsetSize(8),
            reserved(0),
            firstIFDOffset(16)
        {}
        virtual void initiateHeader(const std::string& out_file) override;
        virtual void updateHeader(const std::string& out_file) override;
        virtual void writeFrame(const void* buffer, const std::string& out_file) override;
        void writeTail(const std::string& out_file);

    private:
        uint16_t byteOrder; // "II" (0x4949) for little-endian, "MM" (0x4D4D) for big-endian
        uint16_t version;
        uint16_t offsetSize;
        uint16_t reserved_zero;
        uint64_t firstIFDOffset;
    }

}