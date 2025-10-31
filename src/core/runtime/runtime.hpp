#pragma once

#include <thread>
#include <string>
#include <iostream>

#include "core/storage/storage.hpp"
#include "core/buffer/buffer.hpp"
#include "camera/camera.hpp"

namespace vidre {
    struct Config;

    class RuntimeInstance {
    public:
        RuntimeInstance() = default;
        RuntimeInstance(std::string configuration, std::string header_format = "ccv");
        ~RuntimeInstance() = default;
        
        void retrieveFrames();
        void run();
        void stop();

        void setCamera(std::unique_ptr<ICamera> camera);
        void createStorage(std::string output_file_name = "some.ccv", std::string header_format = "ccv", uint64_t bits_per_pixel = 8, uint8_t packed = 0);

        bool checkCamera();

        bool isRunning();

    private:
        int _currentFrame = 0;
        std::thread _thread;
        std::unique_ptr<ICamera> _camera;
        std::unique_ptr<Storage> _storage;
        std::unique_ptr<Buffer> _buffer;
        bool _saveTIFF = false;
        bool _presentFrame = false;
    };
}  // namespace vidre