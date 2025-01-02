#include "vidre/pylon_camera.hpp"

namespace vidre 
{
    class CSampleImageEventHandler : public Pylon::CImageEventHandler {
    public:
        virtual void OnImageGrabbed(Pylon::CInstantCamera & /*camera*/, const Pylon::CGrabResultPtr &ptrGrabResult) {
            //std::cout << ptrGrabResult->GetImageNumber() << std::endl;
            std::cout << ptrGrabResult->GetBufferContext() << std::endl;
            SharedFrameHeader *header = (SharedFrameHeader *)(ptrGrabResult->GetBuffer()) - 1;
            header->frame_id = ptrGrabResult->GetImageNumber();
            header->time_stamp = ptrGrabResult->GetTimeStamp(); // Time Stamp relies on running time of camera

    #ifdef PYLON_WIN_BUILD
            // Display the image
            Pylon::DisplayImage(1, ptrGrabResult);
    #endif
        }
    };

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
    void PylonCamera::Arm() {
        camera.SetBufferFactory(&shrBufferFactory, Pylon::Cleanup_None);
    }
    void PylonCamera::Disarm() {
        // Free Shared Memory Buffers
    }
    void PylonCamera::Start() {
        if (camera.CanWaitForFrameTriggerReady())
            camera.StartGrabbing(Pylon::GrabStrategy_LatestImages, Pylon::GrabLoop_ProvidedByUser);
    }
    void PylonCamera::Stop() {
        camera.StopGrabbing();
    }

    GenApi::INodeMap &PylonCamera::GetNodeMap() {
        return camera.GetNodeMap();
    }

    SharedFrameHeader* PylonCamera::RetrieveLatestFrame() {
        Pylon::CGrabResultPtr ptrGrabResult;
        camera.RetrieveResult(INFINITE, ptrGrabResult);
        return (SharedFrameHeader *)(ptrGrabResult->GetBuffer()) - 1;
    }

    unsigned int PylonCamera::GetImageWidth() {
        GenApi::INodeMap &nodemap = camera.GetNodeMap();
        Pylon::CIntegerParameter width(nodemap, "Width");
        return width.GetValue();
    }

    unsigned int PylonCamera::GetImageHeight() {
        GenApi::INodeMap &nodemap = camera.GetNodeMap();
        Pylon::CIntegerParameter height(nodemap, "Height");
        return height.GetValue();
    }
}
