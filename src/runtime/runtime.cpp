#include "vidre/runtime.hpp"
#include "vidre/pylon_camera.hpp"
#include "vidre/shared_frame_header.hpp"
#include "vidre/storage.hpp"
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
    RuntimeInstance::~RuntimeInstance() {
        // Unsure where this last (and to be closed) PylonInit comes from.
        Pylon::PylonTerminate();
    }

    RuntimeInstance::RuntimeInstance(std::string configuration) {
        // switch (configuration.cameraVendor)  {
        //     // cameravendor is enum
        //     case CameraVendor::Pylon:
        //         _camera = std::make_unique<vidre::PylonCamera>();
        //     case CameraVendor::Aptina:
        //         _camera = std::make_unique<vidre::AptinaCamera>();
        //     default:
        //         _camera = nullptr;
        // }
        _camera = std::make_unique<vidre::PylonCamera>();


        // Initialize and start the camera
        _camera->Init();

        if (!_camera->LoadConfiguration(configuration.c_str())) {
            std::cout << "Could not load configuration file \"" << configuration << "\" \n";
        }


        
        vidre::Config cameraConfig{};
        cameraConfig.sourcePixelType = Pylon::EPixelType::PixelType_Mono8;
        cameraConfig.sourcePaddingX = _camera->GetFrameOffsetX();
        cameraConfig.sourceWidth = _camera->GetFrameWidth();
        cameraConfig.sourceHeight = _camera->GetFrameHeight();
        cameraConfig.sourceMaxNumBuffer = _camera->GetMaxNumBuffer();
        cameraConfig.sourceFrameRate =  _camera->GetFrameRate();
        
        storage = std::make_unique<vidre::Storage>(cameraConfig);

        _camera->SetSharedMemory(storage->GetSharedMemory());
    }

    void RuntimeInstance::setCamera(std::unique_ptr<ICamera> camera) {
        if (_camera != nullptr) {
            std::cout << "Camera is set already \n";
            return;
        }

        if (camera == nullptr) {
            std::cout << "Recieved nullptr camera \n";
            return;
        }

        _camera = std::move(camera);
    }

    void RuntimeInstance::createStorage(std::string outputFileName) {
        if (_storage != nullptr) {
            std::cout << "Storage is created already \n";
            return;
        }

        if (_camera == nullptr) {
            std::cout << "Camera is not set \n";
            return;
        }
        
        vidre::Config cameraConfig{};
        cameraConfig.sourcePixelType = Pylon::EPixelType::PixelType_Mono8;
        cameraConfig.sourcePaddingX = _camera->GetFrameOffsetX();
        cameraConfig.sourceWidth = _camera->GetFrameWidth();
        cameraConfig.sourceHeight = _camera->GetFrameHeight();
        cameraConfig.sourceMaxNumBuffer = _camera->GetMaxNumBuffer();
        cameraConfig.sourceFrameRate =  _camera->GetFrameRate();
        
        _storage = std::make_unique<vidre::Storage>(cameraConfig, outputFileName);

        _camera->SetSharedMemory(_storage->getSharedMemory());
    }

    void RuntimeInstance::presentationThread() {
        // Setting up relevant camera data for saving
        const vidre::SharedFrameHeader *result = nullptr, *previousResult = nullptr;
        
        const auto cameraConfig = _storage->getConfig();

        // Wait for Pylon's CInstantCamera to start running.
        while (!_camera->IsRunning()) {
            std::this_thread::yield();
        }

        // As long as the camera is acquiring frames, (display the new frame) and save it to disk.
        while (_camera->IsRunning()) {
            // Retrieve frame
            result = _camera->RetrieveLatestFrame();  // Unsafe: Pointer to const is not actually constant
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
            _storage->updateCCVHeader(result->frameID);

            // create_tail
            vidre::ccv_frame_tail tail;
            tail.frameIndex = result->frameID; // + result->numSkippedBefore? TODO: think about skipped logic
            // tail.skippedFrames = numSkippedBefore;
            tail.timeStamp = result->timeStamp;
            tail.timeStamp /= 1e9;
            _storage->appendCCVFrame((const void *)(result + 1), tail);
        }
    }

    void RuntimeInstance::run() {
        if (_camera == nullptr) {
            return;
        }

        if (_storage == nullptr) {
            return;
        }

        _camera->Arm();
        _camera->Start();
        
        _thread = std::thread(&RuntimeInstance::presentationThread, this);
    }

    void RuntimeInstance::stop() {
        // todo: maybe clear storage
        if (camera == nullptr) {
            return;
        }

        _camera->Stop();
        
        if (_thread.joinable()) {
            _thread.join();
        }
        
        _camera->Disarm();
    }

    bool RuntimeInstance::checkCamera() {
        return camera != nullptr;
    }

}