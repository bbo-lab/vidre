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
        RuntimeInstance(std::string configuration);
        ~RuntimeInstance();
        
        void retrieveFrames();
        void run();
        void stop();

        void setCamera(std::unique_ptr<ICamera> camera);
        void createStorage(std::string outputFileName = "some.ccv");

        bool checkCamera();

        bool isRunning()
        {
            return camera->IsRunning();
        }

    private:
        std::thread _thread;
        std::unique_ptr<ICamera> _camera;
        std::unique_ptr<Storage> _storage;
        bool _saveTIFF = false;
        bool _presentFrame = false;
    };
}  // namespace vidre