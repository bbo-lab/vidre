#pragma once
#include <pylon/PylonIncludes.h>
#include <pylon/ImagePersistence.h>
#include "storage.h"
#include "camera.hpp"
namespace vidre {
    struct Config;

    class RuntimeInstance {
    public:
        RuntimeInstance() = default;
        RuntimeInstance(std::string configuration);
        ~RuntimeInstance();
        
        void presentationThread();
        void run(int n_iter);

    private:
        std::unique_ptr<ICamera> camera;
        std::unique_ptr<Storage> storage;
    };
}  // namespace vidre