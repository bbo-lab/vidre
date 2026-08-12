#pragma once
#include <string>
#include <iostream>
#include <cstring>

#ifdef _WIN32
#include <windows.h>
#include <malloc.h>
#endif

namespace vidre {
    class DiskWriter {
    public:
        DiskWriter() {
#ifdef _WIN32
            hFile = INVALID_HANDLE_VALUE;
            alignedBuffer = nullptr;
#else
            hFile = nullptr;
#endif
        }

        ~DiskWriter() {
            close();
        }

        bool open(const std::string& filename) {
#ifdef _WIN32
            DWORD sectorsPerCluster = 0, bytesPerSector = 0, freeClusters = 0, totalClusters = 0;
            std::string rootPath = "C:\\";
            if (filename.length() >= 3 && filename[1] == ':' && filename[2] == '\\') {
                rootPath = filename.substr(0, 3);
            }
            if (GetDiskFreeSpaceA(rootPath.c_str(), &sectorsPerCluster, &bytesPerSector, &freeClusters, &totalClusters)) {
                sectorSize = bytesPerSector;
            } else {
                sectorSize = 4096; // Standard fallback
            }

            hFile = CreateFileA(
                filename.c_str(),
                GENERIC_READ | GENERIC_WRITE, // Read access needed to update header later
                FILE_SHARE_READ,
                nullptr,
                CREATE_ALWAYS,
                FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN | FILE_FLAG_NO_BUFFERING,
                nullptr);

            if (hFile == INVALID_HANDLE_VALUE) return false;

            bufferCapacity = 2 * 1024 * 1024; // 2 MB blocks
            alignedBuffer = static_cast<uint8_t*>(_aligned_malloc(bufferCapacity, sectorSize));
            bufferOffset = 0;
            return alignedBuffer != nullptr;
#else
            hFile = fopen(filename.c_str(), "wb");
            return hFile != nullptr;
#endif
        }

        void close() {
#ifdef _WIN32
            if (hFile != INVALID_HANDLE_VALUE) {
                flushBuffer(); // Clear out remaining trailing bytes
                CloseHandle(hFile);
                hFile = INVALID_HANDLE_VALUE;
            }
            if (alignedBuffer) {
                _aligned_free(alignedBuffer);
                alignedBuffer = nullptr;
            }
#else
            if (hFile) {
                fclose(hFile);
                hFile = nullptr;
            }
#endif
        }

        void write(const void* data, size_t size) {
#ifdef _WIN32
            if (hFile == INVALID_HANDLE_VALUE) return;
            const uint8_t* src = static_cast<const uint8_t*>(data);
            size_t remaining = size;

            while (remaining > 0) {
                size_t spaceLeft = bufferCapacity - bufferOffset;
                size_t toCopy = (remaining < spaceLeft) ? remaining : spaceLeft;

                std::memcpy(alignedBuffer + bufferOffset, src, toCopy);
                bufferOffset += toCopy;
                src += toCopy;
                remaining -= toCopy;

                if (bufferOffset == bufferCapacity) {
                    flushBuffer();
                }
            }
#else
            if (!hFile) return;
            std::fwrite(data, 1, size, hFile);
#endif
        }

        void finalizeHeader(const void* updated_header, size_t header_size) {
#ifdef _WIN32
            flushBuffer();

            LARGE_INTEGER liZero = {0};
            SetFilePointerEx(hFile, liZero, nullptr, FILE_BEGIN);

            DWORD bytesRead = 0;
            if (!ReadFile(hFile, alignedBuffer, sectorSize, &bytesRead, nullptr)) {
                std::cerr << "Direct I/O Error: Header read failed. GLE: " << GetLastError() << "\n";
                return;
            }

            std::memcpy(alignedBuffer, updated_header, header_size);

            SetFilePointerEx(hFile, liZero, nullptr, FILE_BEGIN);

            DWORD bytesWritten = 0;
            if (!WriteFile(hFile, alignedBuffer, sectorSize, &bytesWritten, nullptr)) {
                std::cerr << "Direct I/O Error: Header rewrite failed. GLE: " << GetLastError() << "\n";
            }
#endif
        }

    private:
        void flushBuffer() {
#ifdef _WIN32
            if (bufferOffset == 0 || hFile == INVALID_HANDLE_VALUE) return;

            size_t writeSize = bufferOffset;
            size_t remainder = writeSize % sectorSize;
            
            if (remainder != 0) {
                size_t padding = sectorSize - remainder;
                std::memset(alignedBuffer + bufferOffset, 0, padding);
                writeSize += padding;
            }

            DWORD bytesWritten = 0;
            if (!WriteFile(hFile, alignedBuffer, static_cast<DWORD>(writeSize), &bytesWritten, nullptr)) {
                std::cerr << "Direct I/O Error: Buffer flush failed. GLE: " << GetLastError() << "\n";
            }

            bufferOffset = 0;
#endif
        }

#ifdef _WIN32
        HANDLE hFile;
        uint8_t* alignedBuffer;
        size_t bufferCapacity;
        size_t bufferOffset;
        DWORD sectorSize;
#else
        FILE* hFile;
#endif
    };
}