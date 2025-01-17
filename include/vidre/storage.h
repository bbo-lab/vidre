#pragma once
#include "config.h"
#include <pylon/PylonIncludes.h>
#include <pylon/ImagePersistence.h>

namespace vidre {
    struct Config {
        Pylon::EPixelType sourcePixelType;
        uint32_t sourceWidth;
        uint32_t sourceHeight;
        size_t sourcePaddingX;
    };

    class Storage {
    public:
        Storage() {};
        Storage(const Config& config);
        ~Storage() {};

        void SetConfig(const Config& config);
        Config GetConfig();
        void Save(const void *data_ptr, size_t data_size, int frame);
    private:
        Config config_;
    };
}  // namespace vidre