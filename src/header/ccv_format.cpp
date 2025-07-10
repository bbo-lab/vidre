#include "ccv_format.hpp"
#include "vidre/storage.hpp"

#include <fstream>
#include <iostream>

namespace vidre {

    inline int pack_bits_16_10(const uint8_t *data_in, size_t nbytes_in, uint8_t *data_out, size_t buffersize_out) {
        size_t  byte_offset_in, byte_offset_out, buffersize_out_in_samples, nsamples, remaining_samples;
        buffersize_out_in_samples = (size_t) floor(((double) buffersize_out) * 0.8);
        nsamples = nbytes_in / 2; //rounds down
        size_t n = min(nsamples, buffersize_out_in_samples);
        
        size_t nblocks = n / 4; //rounds down
        
        byte_offset_out = 0;
        byte_offset_in = 0;
        for (size_t jj = 0; jj < nblocks; jj++) {
            data_out[byte_offset_out    ] = data_in[byte_offset_in];
            data_out[byte_offset_out + 1] = data_in[byte_offset_in + 1] | (data_in[byte_offset_in + 2] << 2);
            data_out[byte_offset_out + 2] = (data_in[byte_offset_in + 2] >> 6) | (data_in[byte_offset_in + 3] << 2) | (data_in[byte_offset_in + 4] << 4);
            data_out[byte_offset_out + 3] = (data_in[byte_offset_in + 4] >> 4) | (data_in[byte_offset_in + 5] << 4) | (data_in[byte_offset_in + 6] << 6);
            data_out[byte_offset_out + 4] = (data_in[byte_offset_in + 6] >> 2) | (data_in[byte_offset_in + 7] << 6);
            byte_offset_out += 5;
            byte_offset_in += 8;
        }
        remaining_samples = n - 4 * nblocks;

        int nb_written = byte_offset_out;
        if (remaining_samples > 0) {
            data_out[byte_offset_out] = data_in[byte_offset_in];
            data_out[byte_offset_out + 1] = data_in[byte_offset_in + 1];
            nb_written += 2;
            if (remaining_samples > 1) {
                data_out[byte_offset_out + 1] |= (data_in[byte_offset_in + 2] << 2);
                data_out[byte_offset_out + 2] = (data_in[byte_offset_in + 2] >> 6) | (data_in[byte_offset_in + 3] << 2);
                nb_written++;
                if (remaining_samples > 2) {
                    data_out[byte_offset_out + 2] |= (data_in[byte_offset_in + 4] << 4);
                    data_out[byte_offset_out + 3] = (data_in[byte_offset_in + 4] >> 4) | (data_in[byte_offset_in + 5] << 4);
                    nb_written++;
                }
            }
        }

        return nb_written;
    }

    vidre::CamCommandoHeader::CamCommandoHeader() {
        headerSize += (cameraType.length + 1) + (imageType.length + 1);
        
        bytesPerPixel = 1; // PixelType_Mono8; 
        bitsPerPixel = 8; // PixelType_Mono8;
    }
    vidre::CamCommandoHeader::CamCommandoHeader(
        vidre::Config config,
        std::string camera_type, 
        std::string image_type
    ) {
        cameraType = GreenbergString(camera_type);
        imageType = GreenbergString(image_type);

        headerSize += (cameraType.length + 1) + (imageType.length + 1);
        
        bytesPerPixel = 1; // PixelType_Mono8; 
        bitsPerPixel = 8; // PixelType_Mono8;

        width = config.width;
        height = config.height;

        frameBytesOnDisk = config.frameBytes * bytesPerPixel;
        frameBytesOnDisk += 21; // size of tail
        std::cout << "WILL BE SAVED +21 : " << frameBytesOnDisk << std::endl;
        
        frameRate = config.frameRate;
        pb = (uint32_t) ceil(1.25 * width * height);  

#if CCV_HEADER_VERSION >= 120
        packed = 0;
#endif
        frameCount = 0;
    }

    void vidre::CamCommandoHeader::initiateHeader(FILE* out_file) {
        std::cout << "Width " << width << std::endl;
        std::cout << "height " << height << std::endl;
        std::cout << "frameBytesOnDisk " << frameBytesOnDisk << std::endl;
    #if CCV_HEADER_VERSION >= 120
        std::cout << "IS packed " << packed << std::endl;
    #endif
        writeHeader(out_file);
    }

    void vidre::CamCommandoHeader::updateHeader(FILE* out_file, int frames) {
        frameCount = frames;
        frameBytesOnDisk = pb + 21;
        rewind(out_file);
        writeHeader(out_file);
    }

    void vidre::CamCommandoHeader::writeFrame(const void* buffer, FILE* out_file) {
        uint8_t* packed = new uint8_t[frameBytesOnDisk - 21];

        pack_bits_16_10(static_cast<const uint8_t*>(buffer), frameBytesOnDisk - 21, packed, pb);
        fwrite(reinterpret_cast<const char*>(packed), 1, pb, out_file);
        delete[] packed;

        // file.write(static_cast<const char*>(buffer), frameBytesOnDisk - 21);
    }

    void vidre::CamCommandoHeader::writeTail(FILE* out_file, const CCVFrameTail& tail) {
        writeField(out_file, tail.frameIndex);
        writeField(out_file, tail.timeStamp);
        writeField(out_file, tail.padding);
    }

    template <typename T>
    void vidre::CamCommandoHeader::writeField(FILE* outFile, const T& value) {
        fwrite(reinterpret_cast<const char *>(&value), 1, sizeof(T), outFile);
        // outFile.write(reinterpret_cast<const char*>(&value), sizeof(T));
    }

    template <>
    void vidre::CamCommandoHeader::writeField<std::string>(FILE* outFile, const std::string& value) {
        fwrite(value.c_str(), 1, value.length() + 1, outFile);
        // outFile.write(value.c_str(), value.length() + 1);
    }

    void vidre::CamCommandoHeader::writeHeader(FILE* outFile) {
        writeField(outFile, headerSize);
        writeField(outFile, cameraType.length);
        writeField<std::string>(outFile, cameraType.data);
        writeField(outFile, headerVersion);
        writeField(outFile, imageType.length);
        writeField<std::string>(outFile, imageType.data);

        writeField(outFile, bytesPerPixel);
        writeField(outFile, bitsPerPixel);
        writeField(outFile, frameBytesOnDisk);

        writeField(outFile, width);
        writeField(outFile, height);
        writeField(outFile, frameRate);
#if CCV_HEADER_VERSION >= 120
        writeField(outFile, packed);
#endif
        writeField(outFile, frameCount);
#if CCV_HEADER_VERSION >= 120
        writeField(outFile, sensor.offset);
        writeField(outFile, sensor.size);
        writeField(outFile, sensor.clock);
        writeField(outFile, sensor.exposure);
        writeField(outFile, sensor.gain);
#endif
    }
}