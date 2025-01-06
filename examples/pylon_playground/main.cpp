#include <iostream>

#include <boost/asio.hpp>

#include "vidre/pylon_camera.hpp"
#include "vidre/shared_frame_header.hpp"

void presentationThread(vidre::ICamera* camera) {
    while (camera->IsRunning()) {
        vidre::SharedFrameHeader* result = camera->RetrieveLatestFrame();
        if (!result) continue;
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
    camera->Arm();
    camera->Start();

    // Start thread to query the latest frames. Stops when camera is stopped.
    std::thread presentation(presentationThread, camera.get());
    presentation.detach();

    std::cout << "Press Enter to stop.\n";
    while(std::cin.get() != '\n') {};

    camera->Stop();
    camera->Disarm();

    Pylon::WaitObject::Sleep(1000);
    // ---
    // Change some parameters
    // ---

    camera->Arm();
    camera->Start();

    // Start thread to query the latest frames. Stops when camera is stopped.
    presentation = std::thread(presentationThread, camera.get());
    presentation.detach();

    std::cout << "Press Enter to stop.\n";
    while(std::cin.get() != '\n') {};

    camera->Stop();
    camera->Disarm();


    return 0;
}