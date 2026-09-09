#pragma once
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <string>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
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
            bufferCapacity = 0;
            bufferOffset = 0;
            sectorSize = 4096;
            logicalSize = 0;
#else
            hFile = nullptr;
#endif
        }

        ~DiskWriter() {
            close();
        }

        DiskWriter(const DiskWriter&) = delete;
        DiskWriter& operator=(const DiskWriter&) = delete;

        bool open(const std::string& filename) {
            close();
            failed = false;
            fileName = filename;
#ifdef _WIN32
            DWORD sectorsPerCluster = 0, bytesPerSector = 0, freeClusters = 0, totalClusters = 0;
            std::string rootPath = "C:\\";
            if (filename.length() >= 3 && filename[1] == ':' && (filename[2] == '\\' || filename[2] == '/')) {
                rootPath = filename.substr(0, 3);
            }
            if (GetDiskFreeSpaceA(rootPath.c_str(), &sectorsPerCluster, &bytesPerSector, &freeClusters, &totalClusters) && bytesPerSector > 0) {
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

            if (hFile == INVALID_HANDLE_VALUE) {
                failed = true;
                return false;
            }

            bufferCapacity = 2 * 1024 * 1024; // 2 MB blocks
            alignedBuffer = static_cast<uint8_t*>(_aligned_malloc(bufferCapacity, sectorSize));
            bufferOffset = 0;
            logicalSize = 0;
            if (alignedBuffer == nullptr) {
                CloseHandle(hFile);
                hFile = INVALID_HANDLE_VALUE;
                failed = true;
                return false;
            }
            return true;
#else
            hFile = std::fopen(filename.c_str(), "w+b");
            if (hFile == nullptr) {
                failed = true;
                return false;
            }
            return true;
#endif
        }

        bool isOpen() const {
#ifdef _WIN32
            return hFile != INVALID_HANDLE_VALUE;
#else
            return hFile != nullptr;
#endif
        }

        bool hasError() const {
            return failed;
        }

        void close() {
#ifdef _WIN32
            if (hFile != INVALID_HANDLE_VALUE) {
                flushBuffer(); // Clear out remaining trailing bytes (padded to the sector size)
                CloseHandle(hFile);
                hFile = INVALID_HANDLE_VALUE;
                truncateToLogicalSize();
            }
            if (alignedBuffer) {
                _aligned_free(alignedBuffer);
                alignedBuffer = nullptr;
            }
#else
            if (hFile) {
                if (std::fclose(hFile) != 0) {
                    failed = true;
                }
                hFile = nullptr;
            }
#endif
        }

        void write(const void* data, size_t size) {
#ifdef _WIN32
            if (hFile == INVALID_HANDLE_VALUE) {
                failed = true;
                return;
            }
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
            logicalSize += size;
#else
            if (!hFile) {
                failed = true;
                return;
            }
            if (std::fwrite(data, 1, size, hFile) != size) {
                if (!failed) {
                    std::perror("DiskWriter: write failed");
                }
                failed = true;
            }
#endif
        }

        /// Rewrites the first header_size bytes of the file with an updated header.
        void finalizeHeader(const void* updated_header, size_t header_size) {
#ifdef _WIN32
            if (hFile == INVALID_HANDLE_VALUE) {
                failed = true;
                return;
            }
            if (header_size > sectorSize) {
                std::cerr << "Direct I/O Error: header (" << header_size << " bytes) exceeds one sector\n";
                failed = true;
                return;
            }
            flushBuffer();

            LARGE_INTEGER liZero = {0};
            SetFilePointerEx(hFile, liZero, nullptr, FILE_BEGIN);

            DWORD bytesRead = 0;
            if (!ReadFile(hFile, alignedBuffer, sectorSize, &bytesRead, nullptr)) {
                std::cerr << "Direct I/O Error: Header read failed. GLE: " << GetLastError() << "\n";
                failed = true;
                return;
            }

            std::memcpy(alignedBuffer, updated_header, header_size);

            SetFilePointerEx(hFile, liZero, nullptr, FILE_BEGIN);

            DWORD bytesWritten = 0;
            if (!WriteFile(hFile, alignedBuffer, sectorSize, &bytesWritten, nullptr) || bytesWritten != sectorSize) {
                std::cerr << "Direct I/O Error: Header rewrite failed. GLE: " << GetLastError() << "\n";
                failed = true;
            }

            // Leave the file pointer at the (sector-aligned) end so later writes append.
            LARGE_INTEGER liEnd = {0};
            SetFilePointerEx(hFile, liEnd, nullptr, FILE_END);
#else
            if (!hFile) {
                failed = true;
                return;
            }
            if (std::fflush(hFile) != 0 || std::fseek(hFile, 0, SEEK_SET) != 0) {
                failed = true;
                return;
            }
            if (std::fwrite(updated_header, 1, header_size, hFile) != header_size) {
                std::perror("DiskWriter: header rewrite failed");
                failed = true;
            }
            if (std::fflush(hFile) != 0 || std::fseek(hFile, 0, SEEK_END) != 0) {
                failed = true;
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
            if (!WriteFile(hFile, alignedBuffer, static_cast<DWORD>(writeSize), &bytesWritten, nullptr)
                || bytesWritten != static_cast<DWORD>(writeSize)) {
                std::cerr << "Direct I/O Error: Buffer flush failed. GLE: " << GetLastError() << "\n";
                failed = true;
            }

            bufferOffset = 0;
#endif
        }

#ifdef _WIN32
        void truncateToLogicalSize() {
            HANDLE h = CreateFileA(fileName.c_str(), GENERIC_WRITE, FILE_SHARE_READ, nullptr,
                                   OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
            if (h == INVALID_HANDLE_VALUE) {
                std::cerr << "DiskWriter: could not reopen file to trim padding. GLE: " << GetLastError() << "\n";
                failed = true;
                return;
            }
            LARGE_INTEGER size;
            size.QuadPart = static_cast<LONGLONG>(logicalSize);
            if (!SetFilePointerEx(h, size, nullptr, FILE_BEGIN) || !SetEndOfFile(h)) {
                std::cerr << "DiskWriter: could not trim padding. GLE: " << GetLastError() << "\n";
                failed = true;
            }
            CloseHandle(h);
        }
#endif

        std::string fileName;
        bool failed = false;
#ifdef _WIN32
        HANDLE hFile;
        uint8_t* alignedBuffer;
        size_t bufferCapacity;
        size_t bufferOffset;
        DWORD sectorSize;
        uint64_t logicalSize;
#else
        FILE* hFile;
#endif
    };
}
