#include <core/header/ccv/ccv_format.hpp>
#include <core/storage/storage.hpp>
#include <core/storage/disk_writer.hpp>

#include <algorithm>
#include <fstream>
#include <iostream>

namespace vidre {

    inline int pack_bits_16_10(const uint8_t *data_in, size_t nbytes_in, uint8_t *data_out, size_t buffersize_out) {
        size_t  byte_offset_in, byte_offset_out, buffersize_out_in_samples, nsamples, remaining_samples;
        buffersize_out_in_samples = (size_t) floor(((double) buffersize_out) * 0.8);
        nsamples = nbytes_in / 2; //rounds down
        size_t n = std::min(nsamples, buffersize_out_in_samples);
        
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

    vidre::CamCommandoHeader::CamCommandoHeader(vidre::Config config) {
        cameraType = GreenbergString(config.cameraVendor);
        imageType = GreenbergString(config.cameraVendor);

        headerSize += (cameraType.length + 1) + (imageType.length + 1);
        
        bitsPerPixel = config.bitsPerPixel;
        bytesPerPixel = bitsPerPixel / 8; 

        width = config.width;
        height = config.height;

        frameBytesOnDisk = config.frameBytesActual; //* bytesPerPixel;
        frameBytesOnDisk += 21; // size of tail
        
        frameRate = config.frameRate;
        

#if CCV_HEADER_VERSION >= 120
        packed = config.packed;
        if (cameraType.data == "aptina" && packed) {
            aptinaBuffer = (uint64_t) ceil((bitsPerPixel / 8.0) * width * height);  
        }
#endif
        

        frameCount = 0;
    }

    std::vector<uint8_t> CamCommandoHeader::serializeHeader() {
        std::vector<uint8_t> buffer;
        buffer.reserve(headerSize); // Pre-allocate for performance

        // Helper lambdas to pack bytes into the vector
        auto writeField = [&buffer](const auto& value) {
            const uint8_t* ptr = reinterpret_cast<const uint8_t*>(&value);
            buffer.insert(buffer.end(), ptr, ptr + sizeof(value));
        };
        auto writeString = [&buffer](const std::string& value) {
            const uint8_t* ptr = reinterpret_cast<const uint8_t*>(value.c_str());
            buffer.insert(buffer.end(), ptr, ptr + value.length() + 1); // +1 for null terminator
        };

        writeField(headerSize);
        writeField(cameraType.length);
        writeString(cameraType.data);
        writeField(headerVersion);
        writeField(imageType.length);
        writeString(imageType.data);

        writeField(bytesPerPixel);
        writeField(bitsPerPixel);
        writeField(frameBytesOnDisk);

        writeField(width);
        writeField(height);
        writeField(frameRate);
#if CCV_HEADER_VERSION >= 120
        writeField(packed);
#endif
        writeField(frameCount);
#if CCV_HEADER_VERSION >= 120
        writeField(sensor.offset);
        writeField(sensor.size);
        writeField(sensor.clock);
        writeField(sensor.exposure);
        writeField(sensor.gain);
#endif
        return buffer;
    }

    void vidre::CamCommandoHeader::initiateHeader(DiskWriter& out_file) {
        std::vector<uint8_t> headerData = serializeHeader();
        out_file.write(headerData.data(), headerData.size());
    }

    void vidre::CamCommandoHeader::updateHeader(DiskWriter& out_file, int frames) {
        frameCount = frames;
        
        // Update with actual Aptina Buffer size ()
#if CCV_HEADER_VERSION >= 120
        if (cameraType.data == "aptina" && packed) {
            frameBytesOnDisk = aptinaBuffer + 21;
        }
#endif

        std::vector<uint8_t> headerData = serializeHeader();
        out_file.finalizeHeader(headerData.data(), headerData.size());
    }

    void vidre::CamCommandoHeader::writeFrame(const void* buffer, DiskWriter& out_file) {
#if CCV_HEADER_VERSION >= 120
        if (cameraType.data == "aptina" && packed) {
            if (packedBuffer.size() < aptinaBuffer) {
                packedBuffer.resize(aptinaBuffer);
            }
            pack_bits_16_10(static_cast<const uint8_t*>(buffer), frameBytesOnDisk - 21, packedBuffer.data(), aptinaBuffer);
            out_file.write(packedBuffer.data(), aptinaBuffer);
            return;
        }
#endif        
        out_file.write(buffer, frameBytesOnDisk - 21);
    }

    void vidre::CamCommandoHeader::writeTail(DiskWriter& out_file, const CCVFrameTail& tail) {
        out_file.write(&tail.frameIndex, sizeof(tail.frameIndex));
        out_file.write(&tail.timeStamp, sizeof(tail.timeStamp));
        out_file.write(tail.padding, sizeof(tail.padding));
    }
}