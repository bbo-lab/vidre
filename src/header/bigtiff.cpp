#include "bigtiff.hpp"
#include "vidre/storage.hpp"

#include <fstream>
#include <iostream>

namespace vidre {
    vidre::BigTIFFHeader::BigTIFFHeader(
        vidre::Config config
    ) {
        auto bytesPerPixel = 1; // PixelType_Mono8; 
        auto bitsPerPixel = 8; // PixelType_Mono8;

        auto width = config.width;
        auto height = config.height;

        frameBytesOnDisk = width * height * bytesPerPixel;
    }

    void vidre::BigTIFFHeader::initiateHeader(FILE* out_file) {
        writeHeader(out_file);
    }

    void vidre::BigTIFFHeader::updateHeader(FILE* out_file, int frames) {
        frameCount = frames;
        rewind(out_file);
        writeHeader(out_file);
    }

    void vidre::BigTIFFHeader::writeFrame(const void* buffer, FILE* out_file) {
        fwrite(reinterpret_cast<const char*>(buffer), 1, frameBytesOnDisk, out_file);
    }

    template <typename T>
    void vidre::BigTIFFHeader::writeField(FILE* outFile, const T& value) {
        fwrite(reinterpret_cast<const char *>(&value), 1, sizeof(T), outFile);
    }

    template <>
    void vidre::BigTIFFHeader::writeField<std::string>(FILE* outFile, const std::string& value) {
        fwrite(value.c_str(), 1, value.length() + 1, outFile);
    }

    void vidre::BigTIFFHeader::writeHeader(FILE* outFile) {
        writeField(outFile, byteOrder);
        writeField(outFile, version);
        writeField(outFile, offsetSize);
        writeField(outFile, reservedZero);
        writeField(outFile, firstIFDOffset);
        writeField(outFile, magicNumber);
        writeField(outFile, TiffVersion);
        writeField(outFile, frameBytesOnDisk);
        // writeField(outFile, ROI);
        // writeField(outFile, IFD);
    }
}