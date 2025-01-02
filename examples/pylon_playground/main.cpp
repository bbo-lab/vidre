#include <iostream>

#include "vidre/pylon_camera.hpp"

int main() {
    std::unique_ptr<vidre::ICamera> camera = std::make_unique<vidre::PylonCamera>(); // Or some factory class.
    // Eventually add Aptina Camera support.
    // std::unique_ptr<vidre::ICamera> camera = std::make_unique<vidre::AptinaCamera>();

    // Eventually there might be a way to generalize all GenICam (or USB3Vision) cameras under one class using GenAPI.
    // std::unique_ptr<vidre::ICamera> camera = std::make_unique<vidre::USB3VisionCamera>();

    camera->Init();
    camera->Arm();

    camera->Start();
    for (int i=0; i<=200; ++i) {
        // Frame Grabbing
        SharedFrameHeader* result = camera->RetrieveLatestFrame();

        // Display Frame Image
#ifdef PYLON_WIN_BUILD
            // Display the image
            Pylon::CPylonImage image;
            image.AttachUserBuffer(
                (void*)(result + 1), 
                camera->GetFrameWidth() * camera->GetFrameHeight(),
                Pylon::EPixelType::PixelType_Mono8, 
                camera->GetFrameWidth(), 
                camera->GetFrameHeight(),
                camera->GetFrameOffsetX());
            Pylon::DisplayImage(1, image);
#endif

        // Save Frame Image
        // ...
    }
    camera->Stop();
    camera->Disarm();

    return 0;
}