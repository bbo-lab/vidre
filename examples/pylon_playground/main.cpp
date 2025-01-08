#include <iostream>

#include "vidre/pylon_camera.hpp"
#include "vidre/shared_frame_header.hpp"

void presentationThread(vidre::ICamera* camera) {
    while (!camera->IsRunning()) {}
    vidre::SharedFrameHeader* previousResult = nullptr;
    while (camera->IsRunning()) {
        vidre::SharedFrameHeader* result = camera->RetrieveLatestFrame();
        if (result == previousResult) continue;
#ifdef PYLON_WIN_BUILD
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
        //std::cout << "ID: " << result->frame_id << "\t Address: " << result << '\n';
        //std::cout << "Frame: " << result->frameID << "\tSkipped images: " << result->numSkippedBefore << std::endl;//'\n';
        // Save Image

        previousResult = result;
    }
}

int main() {
    std::unique_ptr<vidre::ICamera> camera = std::make_unique<vidre::PylonCamera>(); // Or some factory class.
    // Eventually add Aptina Camera support.
    // std::unique_ptr<vidre::ICamera> camera = std::make_unique<vidre::AptinaCamera>();

    // Eventually there might be a way to generalize all GenICam (or USB3Vision) cameras under one class using GenAPI.
    // std::unique_ptr<vidre::ICamera> camera = std::make_unique<vidre::USB3VisionCamera>();

    std::thread presentation;
    const char* defaultConfig = "C:\\Projects\\vidre\\defaultConfig.config";

    // Initialize and start the camera
    camera->Init();
    //camera->SaveConfiguration(defaultConfig);
    if (!camera->LoadConfiguration(defaultConfig)) {
        std::cout << "Could not load configuration file \"" << defaultConfig << "\"" << std::endl;
    }

    for (int i=0; i<2; ++i) {
        camera->Arm();
        camera->Start();

        // Start thread to query the latest frames. Stops when camera is stopped.
        std::thread presentation(presentationThread, camera.get());

        std::cout << "Press Enter to stop.\n";
        while(std::cin.get() != '\n') {};

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