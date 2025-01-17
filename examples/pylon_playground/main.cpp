#include <iostream>

#include "vidre/pylon_camera.hpp"
#include "vidre/shared_frame_header.hpp"
#include "vidre/storage.h"

#ifdef PYLON_WIN_BUILD
static void presentFrame(void *dataBuffer, const vidre::Config &config) {
    Pylon::CPylonImage image;
    image.AttachUserBuffer(dataBuffer, config.sourceWidth * config.sourceHeight, config.sourcePixelType, config.sourceWidth,
                           config.sourceHeight, config.sourcePaddingX);
    Pylon::DisplayImage(1, image);
}
#endif

void presentationThread(vidre::ICamera *camera) {
    // Setting up relevant camera data for saving
    vidre::Config cameraConfig{};
    cameraConfig.sourcePixelType = Pylon::EPixelType::PixelType_Mono8;
    cameraConfig.sourcePaddingX = camera->GetFrameOffsetX();
    cameraConfig.sourceWidth = camera->GetFrameWidth();
    cameraConfig.sourceHeight = camera->GetFrameHeight();

    const vidre::SharedFrameHeader *result = nullptr, *previousResult = nullptr;

    // Wait for Pylon's CInstantCamera to start running.
    while (!camera->IsRunning()) {
        std::this_thread::yield();
    }

    // As long as the camera is acquiring frames, (display the new frame) and save it to disk.
    while (camera->IsRunning()) {
        // Retrieve frame
        result = camera->RetrieveLatestFrame();  // Unsafe: Pointer to const is not actually constant
        if (result == previousResult) {
            std::this_thread::yield();
            continue;
        }

        // Present frame
#ifdef PYLON_WIN_BUILD
        presentFrame((void *)(result + 1), cameraConfig);
#endif
        std::cout << "Frame: " << result->frameID << "\tSkipped frames: " << result->numSkippedBefore << std::endl;  //'\n';

        // Save Image
        vidre::Save(cameraConfig, (const void *)(result + 1), cameraConfig.sourceWidth * cameraConfig.sourceHeight,
                    result->frameID);

        previousResult = result;
    }
}

int main() {
    std::thread presentation;
    const char defaultConfigFilepath[] = "C:\\Projects\\vidre\\defaultConfig.config";
    std::unique_ptr<vidre::ICamera> camera = std::make_unique<vidre::PylonCamera>();  // Or some factory class.
    // Eventually add Aptina Camera support.
    // std::unique_ptr<vidre::ICamera> camera = std::make_unique<vidre::AptinaCamera>();
    // Eventually there might be a way to generalize all GenICam (or USB3Vision) cameras under one class using GenAPI.
    // std::unique_ptr<vidre::ICamera> camera = std::make_unique<vidre::USB3VisionCamera>();

    // Initialize and start the camera
    camera->Init();
    // camera->SaveConfiguration(defaultConfig);
    if (!camera->LoadConfiguration(defaultConfigFilepath)) {
        std::cout << "Could not load configuration file \"" << defaultConfigFilepath << "\"" << std::endl;
    }
    // camera->SetFrameRate(10.0);

    for (int i = 0; i < 1; ++i) {
        camera->Arm();
        camera->Start();

        // Start thread to query the latest frames. Stops when camera is stopped.
        std::thread presentation(presentationThread, camera.get());

        // Pylon::WaitObject::Sleep(2000);

        std::cout << "Press Enter to stop.\n";
        while (std::cin.get() != '\n') {
        }

        camera->Stop();
        presentation.join();
        camera->Disarm();

        // Simulate change of parameters inbetween.
        camera->SetFrameRate(10.0);
        Pylon::WaitObject::Sleep(500);
    }

    // Unsure where this last (and to be closed) PylonInit comes from.
    Pylon::PylonTerminate();
    return 0;
}