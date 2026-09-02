#pragma once

#include <config.h>

#include <boost/asio.hpp>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>
#include <thread>
#include <vector>

namespace vidre::test {

    struct CcvInfo {
        uint32_t headerSize = 0;
        uint32_t bytesPerPixel = 0;
        uint32_t bitsPerPixel = 0;
        uint32_t frameBytesOnDisk = 0;
        uint32_t width = 0;
        uint32_t height = 0;
        uint32_t frameCount = 0;
    };

    // Find a free TCP port by binding to :0 and closing. Small race window,
    // acceptable for tests.
    inline uint16_t findFreePort() {
        boost::asio::io_context io;
        boost::asio::ip::tcp::acceptor acceptor(io, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), 0));
        const uint16_t port = acceptor.local_endpoint().port();
        acceptor.close();
        return port;
    }

    // Unique name per test run (shared-memory names, temp dirs, serials).
    inline std::string uniqueId(const std::string& prefix) {
        static std::atomic<uint64_t> counter{0};
        const auto now = std::chrono::steady_clock::now().time_since_epoch().count();
        return prefix + "_" + std::to_string(now) + "_" + std::to_string(counter.fetch_add(1));
    }

    class TempDir {
    public:
        TempDir() {
            _path = std::filesystem::temp_directory_path() / uniqueId("vidre_test");
            std::filesystem::create_directories(_path);
        }
        ~TempDir() {
            std::error_code ec;
            std::filesystem::remove_all(_path, ec);
        }
        TempDir(const TempDir&) = delete;
        TempDir& operator=(const TempDir&) = delete;

        const std::filesystem::path& path() const {
            return _path;
        }

    private:
        std::filesystem::path _path;
    };

    inline bool writeFile(const std::filesystem::path& path, const std::string& content) {
        std::ofstream out(path);
        out << content;
        return out.good();
    }

    inline std::string yamlEscapePath(const std::filesystem::path& path) {
        std::string result = path.string();
        std::replace(result.begin(), result.end(), '\\', '/');
        return result;
    }

    inline std::vector<std::filesystem::path> findFiles(const std::filesystem::path& dir, const std::string& extension) {
        std::vector<std::filesystem::path> result;
        std::error_code ec;
        if (!std::filesystem::exists(dir, ec)) {
            return result;
        }
        for (const auto& entry : std::filesystem::directory_iterator(dir, ec)) {
            if (entry.is_regular_file(ec) && entry.path().extension() == extension) {
                result.push_back(entry.path());
            }
        }
        return result;
    }

    template <typename Predicate>
    bool pollUntil(Predicate predicate, std::chrono::milliseconds timeout) {
        const auto deadline = std::chrono::steady_clock::now() + timeout;
        while (std::chrono::steady_clock::now() < deadline) {
            if (predicate()) {
                return true;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }
        return predicate();
    }

    // Parse the CCV header (CamCommandoHeader::serializeHeader layout).
    inline bool readCcvInfo(const std::filesystem::path& path, CcvInfo& info) {
        std::ifstream file(path, std::ios::binary);
        if (!file) {
            return false;
        }
        auto readU8 = [&file]() {
            uint8_t value = 0;
            file.read(reinterpret_cast<char*>(&value), sizeof(value));
            return value;
        };
        auto readU32 = [&file]() {
            uint32_t value = 0;
            file.read(reinterpret_cast<char*>(&value), sizeof(value));
            return value;
        };
        auto readF64 = [&file]() {
            double value = 0.0;
            file.read(reinterpret_cast<char*>(&value), sizeof(value));
            return value;
        };
        auto readString = [&file](uint32_t length) {
            std::string value(length + 1, '\0');
            file.read(value.data(), static_cast<std::streamsize>(length + 1));
            return value;
        };

        info.headerSize = readU32();
        readString(readU32());  // cameraType
        readF64();              // headerVersion
        readString(readU32());  // imageType
        info.bytesPerPixel = readU32();
        info.bitsPerPixel = readU32();
        info.frameBytesOnDisk = readU32();
        info.width = readU32();
        info.height = readU32();
        readF64();  // frameRate
#if CCV_HEADER_VERSION >= 120
        readU8();  // packed
#endif
        info.frameCount = readU32();
        file.seekg(static_cast<std::streamoff>(info.headerSize) - static_cast<std::streamoff>(file.tellg()),
                   std::ios::cur);
        return file.good();
    }

    // Number of frames implied by the file size. On POSIX the header rewrite is
    // a no-op, so frameCount is only recoverable this way. On Windows the file
    // may be padded to sector size; floor division stays correct.
    inline uint64_t frameCountFromSize(uint64_t fileSize, const CcvInfo& info) {
        if (info.frameBytesOnDisk == 0 || fileSize < info.headerSize) {
            return 0;
        }
        return (fileSize - info.headerSize) / info.frameBytesOnDisk;
    }

    // Verify that the per-frame tails carry sequential frame indices 1..N.
    inline bool checkFrameSequence(const std::filesystem::path& path, const CcvInfo& info, uint64_t expectedFrames) {
        constexpr uint32_t kTailSize = 21;
        if (info.frameBytesOnDisk <= kTailSize) {
            return false;
        }
        std::ifstream file(path, std::ios::binary);
        if (!file) {
            return false;
        }
        file.seekg(info.headerSize);
        const uint32_t payloadSize = info.frameBytesOnDisk - kTailSize;
        for (uint64_t i = 0; i < expectedFrames; ++i) {
            std::vector<uint8_t> payload(payloadSize);
            file.read(reinterpret_cast<char*>(payload.data()), static_cast<std::streamsize>(payload.size()));
            uint32_t frameIndex = 0;
            file.read(reinterpret_cast<char*>(&frameIndex), sizeof(frameIndex));
            if (!file || frameIndex != static_cast<uint32_t>(i + 1)) {
                return false;
            }
            file.seekg(8 + 9, std::ios::cur);  // timeStamp + padding
        }
        return file.good();
    }

    // Number of finalized frames recorded for a CCV file: the header count on
    // Windows (rewritten at session close), the size-derived count on POSIX.
    inline uint64_t finalizedFrames(const std::filesystem::path& path, const CcvInfo& info) {
#ifdef _WIN32
        return info.frameCount;
#else
        return frameCountFromSize(std::filesystem::file_size(path), info);
#endif
    }

    // Poll until a CCV file matching nameHint (empty = any) has at least
    // minFrames finalized frames. The file is created at session begin, so
    // existence alone is not a completion signal.
    inline bool waitForCcvFile(const std::filesystem::path& dir, const std::string& nameHint, uint64_t minFrames,
                               std::chrono::milliseconds timeout) {
        return pollUntil(
            [&] {
                for (const auto& file : findFiles(dir, ".ccv")) {
                    if (nameHint.empty() || file.filename().string().find(nameHint) != std::string::npos) {
                        CcvInfo info;
                        if (readCcvInfo(file, info) && finalizedFrames(file, info) >= minFrames) {
                            return true;
                        }
                    }
                }
                return false;
            },
            timeout);
    }

}  // namespace vidre::test
