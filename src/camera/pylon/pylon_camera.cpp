#include "vidre/pylon_camera.hpp"

namespace vidre 
{
    class CSampleImageEventHandler : public Pylon::CImageEventHandler {
    public:
        virtual void OnImageGrabbed(Pylon::CInstantCamera & /*camera*/, const Pylon::CGrabResultPtr &ptrGrabResult) {
            SharedFrameHeader *header = (SharedFrameHeader *)(ptrGrabResult->GetBuffer()) - 1;
            header->frame_id = ptrGrabResult->GetImageNumber();
            header->time_stamp = ptrGrabResult->GetTimeStamp();
        }
    };

    PylonCamera::PylonCamera() {
        // Reference counted by Pylon.
        running.store(false);
        armed.store(false);

        Pylon::PylonInitialize();
    }
    PylonCamera::~PylonCamera() {
        Stop();
        camera.Close();
        if (camera.IsPylonDeviceAttached())
            camera.DestroyDevice();

        Pylon::PylonTerminate();
    }

    void PylonCamera::run() {
        running.store(true);

        camera.StartGrabbing(Pylon::GrabStrategy_LatestImageOnly, Pylon::GrabLoop_ProvidedByUser);

        while (running.load()) {
            // Frame Grabbing
            Pylon::CGrabResultPtr currentPtrGrabResult;
            //camera.ExecuteSoftwareTrigger();
            if (!camera.RetrieveResult(500, currentPtrGrabResult)) continue;

            SharedFrameHeader* result = (SharedFrameHeader *)(currentPtrGrabResult->GetBuffer()) - 1;
            result->frame_id = currentPtrGrabResult->GetImageNumber();
            result->time_stamp = currentPtrGrabResult->GetTimeStamp();

            latestFrame.store(result);
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
        GenApi::INodeMap &nodemap = camera.GetNodeMap();
        Pylon::CIntegerParameter width(nodemap, "Width");
        return width.GetValue();
    }

    unsigned int PylonCamera::GetFrameHeight() {
        GenApi::INodeMap &nodemap = camera.GetNodeMap();
        Pylon::CIntegerParameter height(nodemap, "Height");
        return height.GetValue();
    }

    unsigned int PylonCamera::GetFrameOffsetX() {
        GenApi::INodeMap &nodemap = camera.GetNodeMap();
        Pylon::CIntegerParameter offsetX(nodemap, "OffsetX");
        return offsetX.GetValue();
    }

    void PylonCamera::LoadConfiguration(const char* filepath) {
        Pylon::CFeaturePersistence::Load(filepath, &GetNodeMap(), true);
    }
}
