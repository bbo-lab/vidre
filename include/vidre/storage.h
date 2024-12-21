#include "config.h"
#include <pylon/PylonIncludes.h>
#include <pylon/ImagePersistence.h>

namespace vidre
{
    struct Config
    {
        Pylon::EPixelType sourcePixelType;
        uint32_t sourceWidth;
        uint32_t sourceHeight;
        size_t sourcePaddingX;
    };

    void Save(Config config, const void* data_ptr, size_t data_size, int frame);
}