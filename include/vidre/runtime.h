#pragma once
#include <pylon/PylonIncludes.h>
#include <pylon/ImagePersistence.h>
#include "storage.h"
#include "camera.hpp"
#include <thread>

namespace vidre {
    struct Config;

    class RuntimeInstance {
    public:
        RuntimeInstance() = default;
        RuntimeInstance(std::string configuration);
        ~RuntimeInstance();
        
        void presentationThread();
        void run();
        void stop();

        void SetCamera(std::unique_ptr<ICamera> camera_);
        void CreateStorage(std::string output_file_name = "some.ccv");

        bool CheckCamera();

        bool IsRunning()
        {
            return camera->IsRunning();
        }

    private:
        std::thread thread;
        std::unique_ptr<ICamera> camera;
        std::unique_ptr<Storage> storage;
    };
}  // namespace vidre