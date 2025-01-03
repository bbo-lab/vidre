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

    void PylonCamera::run() {
        while (running.load()) {
            // Frame Grabbing
            SharedFrameHeader* result = RetrieveLatestFrame();

            // Display Frame Image
#ifdef PYLON_WIN_BUILD
            Pylon::CPylonImage image;
            image.AttachUserBuffer(
                (void*)(result + 1), 
                GetFrameWidth() * GetFrameHeight(),
                Pylon::EPixelType::PixelType_Mono8, 
                GetFrameWidth(), 
                GetFrameHeight(),
                GetFrameOffsetX());
            std::cout << "ID: " << result->frame_id << "\t Address: " << result << '\n';
            Pylon::DisplayImage(1, image);
#endif

            // Save Frame Image
            // ...
        }
    }

    PylonCamera::PylonCamera() {
        // Reference counted by Pylon.
        Pylon::PylonInitialize();
    }
    PylonCamera::~PylonCamera() {
        Stop();
        camera.Close();
        if (camera.IsPylonDeviceAttached())
            camera.DestroyDevice();

        Pylon::PylonTerminate();
    }

    void PylonCamera::Init() {
        try {
            camera.Attach(Pylon::CTlFactory::GetInstance().CreateFirstDevice());
            camera.RegisterImageEventHandler(new CSampleImageEventHandler, Pylon::RegistrationMode_Append, Pylon::Cleanup_Delete);

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
    }
    void PylonCamera::Disarm() {
        // Free Shared Memory Buffers
    }
    void PylonCamera::Start() {
        if (!camera.CanWaitForFrameTriggerReady()) return;
            
        running.store(true);

        camera.StartGrabbing(Pylon::GrabStrategy_LatestImageOnly, Pylon::GrabLoop_ProvidedByUser);
        grabber = std::thread(&PylonCamera::run, this);
        grabber.detach();
    }
    void PylonCamera::Stop() {
        running.store(false);

        //grabber.join();
        camera.StopGrabbing();
    }

    SharedFrameHeader* PylonCamera::RetrieveLatestFrame() {
        Pylon::CGrabResultPtr ptrGrabResult;
        camera.RetrieveResult(1000, ptrGrabResult);
        return (SharedFrameHeader *)(ptrGrabResult->GetBuffer()) - 1;
    }

    bool PylonCamera::LoadConfiguration(char* filepath) {
        return false;
    }

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
}
