#pragma once

#include "config.h"

#include "vidre/header.hpp"



namespace vidre {
    struct Config;

    class BigTIFFHeader : public IHeader {
    public:
        BigTIFFHeader() :
            byteOrder(0x4949),    // Default to little-endian
            version(43),          // BigTIFF version
            offsetSize(8),
            reservedZero(0),
            firstIFDOffset(16)
        {}
        BigTIFFHeader(vidre::Config config);
        virtual void initiateHeader(const std::string& out_file) override;
        virtual void updateHeader(const std::string& out_file) override;
        virtual void writeFrame(const void* buffer, const std::string& out_file) override;

    private:
        void writeHeader(FILE* outFile);
        template <typename T> void writeField(FILE* outFile, const T& value);
        // tiff
        uint16_t byteOrder; // "II" (0x4949) for little-endian, "MM" (0x4D4D) for big-endian
        uint16_t version;
        uint16_t offsetSize;
        uint16_t reservedZero;
        uint64_t firstIFDOffset;
        
        // scanimage
        uint32_t magicNumber = 117637889;
        uint32_t TiffVersion = 3;
        uint32_t frameBytesOnDisk;
        uint32_t ROI; // ?
        uint32_t IFD; // ?
        // further comes frame ?

        // impl specific
        uint64_t frameCount = 0;
    }

}