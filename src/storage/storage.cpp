#include "vidre/storage.h"
namespace vidre
{
    void Save(Config config, const void* data_ptr, size_t data_size, int frame)
    {
        Pylon::CPylonImage pylonImage;

        Pylon::CImageFormatConverter formatConverter;
        formatConverter.OutputPixelFormat = Pylon::PixelType_BGR8packed;
        formatConverter.Convert(pylonImage,
                              data_ptr,
                              data_size,
                              config.sourcePixelType,
                              config.sourceWidth,
                              config.sourceHeight,
                              config.sourcePaddingX, 
                                Pylon::EImageOrientation::ImageOrientation_TopDown
                              );

        std::string file_name = std::to_string(frame) + ".png";
        Pylon::CImagePersistence::Save(Pylon::ImageFileFormat_Tiff, file_name.c_str(), pylonImage);
    }
}