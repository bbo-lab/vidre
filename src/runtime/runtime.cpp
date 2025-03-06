#include "vidre/runtime.h"
#include "vidre/pylon_camera.hpp"
#include "vidre/shared_frame_header.hpp"
#include "vidre/storage.h"
#include <string>


#ifdef PYLON_WIN_BUILD
static void presentFrame(void *dataBuffer, const vidre::Config &config) {
    Pylon::CPylonImage image;
    image.AttachUserBuffer(dataBuffer, config.sourceWidth * config.sourceHeight, config.sourcePixelType, config.sourceWidth,
                           config.sourceHeight, config.sourcePaddingX);
    Pylon::DisplayImage(1, image);
}
#endif

namespace vidre
{
    RuntimeInstance::~RuntimeInstance()
    {
        // Unsure where this last (and to be closed) PylonInit comes from.
        Pylon::PylonTerminate();
    }

    RuntimeInstance::RuntimeInstance(std::string configuration)
    {
        _camera = std::make_unique<vidre::PylonCamera>();
        // Eventually add Aptina Camera support.
        // std::unique_ptr<vidre::ICamera> camera = std::make_unique<vidre::AptinaCamera>();
        // Eventually there might be a way to generalize all GenICam (or USB3Vision) cameras under one class using GenAPI.
        // std::unique_ptr<vidre::ICamera> camera = std::make_unique<vidre::USB3VisionCamera>();

        // Initialize and start the camera
        _camera->Init();
        // camera->SaveConfiguration(defaultConfig);
        if (!_camera->LoadConfiguration(configuration.c_str())) {
            std::cout << "Could not load configuration file \"" << configuration << "\"" << std::endl;
        }


        
        vidre::Config cameraConfig{};
        cameraConfig.sourcePixelType = Pylon::EPixelType::PixelType_Mono8;
        cameraConfig.sourcePaddingX = camera->GetFrameOffsetX();
        cameraConfig.sourceWidth = camera->GetFrameWidth();
        cameraConfig.sourceHeight = camera->GetFrameHeight();
        cameraConfig.sourceMaxNumBuffer = camera->GetMaxNumBuffer();
        cameraConfig.sourceFrameRate =  camera->GetFrameRate();
        
        storage = std::make_unique<vidre::Storage>(cameraConfig);

        camera->SetSharedMemory(storage->GetSharedMemory());
    }
    void RuntimeInstance::SetCamera(std::unique_ptr<ICamera> camera_)
    {
        if (camera != nullptr || camera_ == nullptr) 
        {
            return;
        }
        camera = std::move(camera_);
    }
    void RuntimeInstance::CreateStorage(std::string output_file_name)
    {
        if (storage != nullptr || camera == nullptr)
        {
            return;
        }
        
        vidre::Config cameraConfig{};
        cameraConfig.sourcePixelType = Pylon::EPixelType::PixelType_Mono8;
        cameraConfig.sourcePaddingX = camera->GetFrameOffsetX();
        cameraConfig.sourceWidth = camera->GetFrameWidth();
        cameraConfig.sourceHeight = camera->GetFrameHeight();
        cameraConfig.sourceMaxNumBuffer = camera->GetMaxNumBuffer();
        cameraConfig.sourceFrameRate =  camera->GetFrameRate();
        
        storage = std::make_unique<vidre::Storage>(cameraConfig, output_file_name);

        camera->SetSharedMemory(storage->GetSharedMemory());
    }

    void RuntimeInstance::presentationThread() {
        // Setting up relevant camera data for saving
        const vidre::SharedFrameHeader *result = nullptr, *previousResult = nullptr;
        const auto cameraConfig = storage->GetConfig();
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
    // #ifdef PYLON_WIN_BUILD
    //         presentFrame((void *)(result + 1), cameraConfig);
    // #endif
            std::cout << "Frame: " << result->frameID << "\tSkipped frames: " << result->numSkippedBefore << std::endl;  //'\n';

            // Save Image
            // storage->Save((const void *)(result + 1), cameraConfig.sourceWidth * cameraConfig.sourceHeight,
            //             result->frameID);

            previousResult = result;
            
            // update ccv_header
            storage->UpdateCCVHeader(result->frameID);

            // create_tail
            vidre::ccv_frame_tail tail;
            tail.frameIndex = result->frameID; // + result->numSkippedBefore? TODO: think about skipped logic
            // tail.skippedFrames = numSkippedBefore;
            tail.timeStamp = result->timeStamp;
            tail.timeStamp /= 1e9;
            storage->AppendCCVFrame((const void *)(result + 1), tail);
        }
    }

    void RuntimeInstance::run()
    {
        if (camera == nullptr || storage == nullptr)
        {
            return;
        }
        camera->Arm();
        camera->Start();
        thread = std::thread(&RuntimeInstance::presentationThread, this);
    }

    void RuntimeInstance::stop()
    {
        // todo: maybe clear storage
        if (camera == nullptr )
        {
            return;
        }
        camera->Stop();
        
        if (thread.joinable())
        {
            thread.join();
        }
        
        camera->Disarm();
    }

    bool RuntimeInstance::CheckCamera()
    {
        return camera != nullptr;
    }

}