#include <iostream>

#include <boost/asio.hpp>

#include "vidre/pylon_camera.hpp"
#include "vidre/shared_frame_header.hpp"

void presentationThread(vidre::ICamera* camera) {
    while (!camera->IsRunning()) {}
    while (camera->IsRunning()) {
        vidre::SharedFrameHeader* result = camera->RetrieveLatestFrame();
        if (result == nullptr) continue;
#ifdef PYLON_WIN_BUILD
        Pylon::CPylonImage image;
        image.AttachUserBuffer(
            (void*)(result + 1), 
            camera->GetFrameWidth() * camera->GetFrameHeight(),
            Pylon::EPixelType::PixelType_Mono8, 
            camera->GetFrameWidth(), 
            camera->GetFrameHeight(),
            camera->GetFrameOffsetX());
        //std::cout << "ID: " << result->frame_id << "\t Address: " << result << '\n';
        Pylon::DisplayImage(1, image);
#endif
        // Save Image
    }
}

int main() {
    std::unique_ptr<vidre::ICamera> camera = std::make_unique<vidre::PylonCamera>(); // Or some factory class.
    // Eventually add Aptina Camera support.
    // std::unique_ptr<vidre::ICamera> camera = std::make_unique<vidre::AptinaCamera>();

    // Eventually there might be a way to generalize all GenICam (or USB3Vision) cameras under one class using GenAPI.
    // std::unique_ptr<vidre::ICamera> camera = std::make_unique<vidre::USB3VisionCamera>();

    // Initialize and start the camera
    camera->Init();
    std::thread presentation;

    for (int i=0; i<3; ++i) {
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
        Pylon::WaitObject::Sleep(1000);
    }

    // Dont know where this last PylonInit comes from.
    Pylon::PylonTerminate();
    return 0;
}