#pragma once
#include <pylon/PylonIncludes.h>
#include <pylon/ImagePersistence.h>
#include "storage.hpp"
#include "camera.hpp"
#include <thread>

namespace vidre {
    struct Config;

    class RuntimeInstance {
    public:
        RuntimeInstance() = default;
        RuntimeInstance(std::string configuration, std::string header_format = "ccv");
        ~RuntimeInstance();
        
        void retrieveFrames();
        void run();
        void stop();

        void setCamera(std::unique_ptr<ICamera> camera);
        void createStorage(std::string output_file_name = "some.ccv", std::string header_format = "ccv");

        bool checkCamera();

        bool isRunning()
        {
            return _camera->IsRunning();
        }

    private:
        std::thread _thread;
        std::unique_ptr<ICamera> _camera;
        std::unique_ptr<Storage> _storage;
        bool _saveTIFF = false;
        bool _presentFrame = false;
    };
}  // namespace vidre