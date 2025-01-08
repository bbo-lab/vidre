#include "vidre/pylon_camera.hpp"

namespace vidre 
{
    class CSampleImageEventHandler : public Pylon::CImageEventHandler {
    public:
        virtual void OnImageGrabbed(Pylon::CInstantCamera & /*camera*/, const Pylon::CGrabResultPtr &ptrGrabResult) {
            SharedFrameHeader *header = (SharedFrameHeader *)(ptrGrabResult->GetBuffer()) - 1;
            header->frameID = ptrGrabResult->GetImageNumber();
            header->timeStamp = ptrGrabResult->GetTimeStamp();
        }
    };

    PylonCamera::PylonCamera() : 
        running(false),
        armed(false)
    {}
    PylonCamera::~PylonCamera() {
        Stop();
        camera.Close();
        if (camera.IsPylonDeviceAttached())
            camera.DestroyDevice();
        
        //Pylon::PylonTerminate();
    }

    void PylonCamera::run() {
        running.store(true);

        camera.StartGrabbing(Pylon::GrabStrategy_LatestImageOnly, Pylon::GrabLoop_ProvidedByUser);

        // Might not be necessary because of the wait of RetrieveResult.
        void* previousBuffer = nullptr;

        while (running.load()) {
            // Frame Grabbing
            Pylon::CGrabResultPtr currentPtrGrabResult;
            //camera.ExecuteSoftwareTrigger();
            if (!camera.RetrieveResult(100, currentPtrGrabResult, Pylon::TimeoutHandling_Return)) {
                std::this_thread::yield();
                continue;
            }
            //if (currentPtrGrabResult->GetBuffer() == previousBuffer) continue;

            SharedFrameHeader* result = (SharedFrameHeader *)(currentPtrGrabResult->GetBuffer()) - 1;
            result->frameID = currentPtrGrabResult->GetImageNumber();
            result->timeStamp = currentPtrGrabResult->GetTimeStamp();
            result->numSkippedBefore = currentPtrGrabResult->GetNumberOfSkippedImages(); // ! Does not account for camera internal skipped images.

            latestFrame.store(result);
            previousBuffer = currentPtrGrabResult->GetBuffer();

            // Simulate skipping images
            // Pylon::WaitObject::Sleep(500);
        }
        camera.StopGrabbing();
    }

    void PylonCamera::Init() {
        try {
            camera.Attach(Pylon::CTlFactory::GetInstance().CreateFirstDevice());
            //camera.RegisterImageEventHandler(new CSampleImageEventHandler, Pylon::RegistrationMode_Append, Pylon::Cleanup_Delete);
            //camera.RegisterConfiguration(new Pylon::CSoftwareTriggerConfiguration, Pylon::ERegistrationMode::RegistrationMode_Append, Pylon::ECleanup::Cleanup_Delete);

            camera.MaxNumBuffer = 5;
        } catch (const Pylon::GenericException &e) {
            // Error handling.
            std::cerr << "An exception occurred." << std::endl << e.GetDescription() << std::endl;
            throw;
        }
        camera.Open();
    }
    /// Executing Arm() "fixes" the camera configuration in place.
    void PylonCamera::Arm() {
        shrBufferFactory.PreallocateFrameBuffers(
            (sizeof(SharedFrameHeader) + GetFrameWidth() * GetFrameHeight() /* * PixelSize*/) * (camera.MaxNumBuffer.GetValue() + 1)
            , "vidre");

        camera.SetBufferFactory(&shrBufferFactory, Pylon::Cleanup_None);
        camera.Close();
        armed.store(true);
    }
    void PylonCamera::Disarm() {
        // Free Shared Memory Buffers
        armed.store(false);
        camera.Open();
    }
    void PylonCamera::Start() {
        if (!armed.load()) return;
        latestFrame.store(nullptr);

        grabber = std::thread(&PylonCamera::run, this);
    }
    void PylonCamera::Stop() {
        running.store(false);
        grabber.join();
    }

    SharedFrameHeader* PylonCamera::RetrieveLatestFrame() {
        return latestFrame.load();
    }
    bool PylonCamera::IsArmed() {
        return armed.load();
    }
    bool PylonCamera::IsRunning() {
        return running.load();
    }




    /// --------------------------
    /// Camera parameter access
    /// --------------------------

    GenApi::INodeMap &PylonCamera::GetNodeMap() {
        return camera.GetNodeMap();
    }

    unsigned int PylonCamera::GetFrameWidth() {
        GenApi::INodeMap &nodemap = GetNodeMap();
        Pylon::CIntegerParameter width(nodemap, "Width");

        return width.GetValue();
    }

    unsigned int PylonCamera::GetFrameHeight() {
        GenApi::INodeMap &nodemap = GetNodeMap();
        Pylon::CIntegerParameter height(nodemap, "Height");

        return height.GetValue();
    }

    unsigned int PylonCamera::GetFrameOffsetX() {
        GenApi::INodeMap &nodemap = GetNodeMap();
        Pylon::CIntegerParameter offsetX(nodemap, "OffsetX");

        return offsetX.GetValue();
    }

    double PylonCamera::GetFrameRate() {
        GenApi::INodeMap &nodemap = GetNodeMap();
        Pylon::CFloatParameter frameRate(nodemap, "AcquisitionFrameRate");

        return frameRate.GetValue();
    }

    void PylonCamera::SetFrameRate(double newFrameRate) {
        GenApi::INodeMap &nodemap = GetNodeMap();
        Pylon::CBooleanParameter frameRateEnable(nodemap, "AcquisitionFrameRateEnable");
        Pylon::CFloatParameter frameRate(nodemap, "AcquisitionFrameRate");

        if (frameRateEnable.TrySetValue(true)) {
            if (!frameRate.TrySetValue(newFrameRate))
                std::cout << "Could not set frame rate!" << std::endl;
        }
    }

    bool PylonCamera::LoadConfiguration(const char* filepath) {
        try {
            Pylon::CFeaturePersistence::Load(filepath, &GetNodeMap(), true);
        }
        catch (const Pylon::GenericException &e) {
            std::cerr << "An exception occurred." << std::endl << e.GetDescription() << std::endl;
            return false;
        }
        return true;
    }

    bool PylonCamera::SaveConfiguration(const char* filepath) {
        try {
            Pylon::CFeaturePersistence::Save(filepath, &GetNodeMap());
        }
        catch (const Pylon::GenericException &e) {
            std::cerr << "An exception occurred." << std::endl << e.GetDescription() << std::endl;
            return false;
        }
        return true;
    }
}
