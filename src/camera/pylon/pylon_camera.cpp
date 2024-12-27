#include "vidre/pylon_camera.hpp"

namespace vidre 
{
    class CSampleImageEventHandler : public Pylon::CImageEventHandler {
    public:
        virtual void OnImageGrabbed(Pylon::CInstantCamera & /*camera*/, const Pylon::CGrabResultPtr &ptrGrabResult) {
            std::cout << ptrGrabResult->GetImageNumber() << std::endl;
            SharedImageHeader *header = (SharedImageHeader *)(ptrGrabResult->GetBuffer()) - 1;
            header->frame_id = ptrGrabResult->GetImageNumber();
            header->time_stamp = ptrGrabResult->GetTimeStamp();

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
            // camera.RegisterConfiguration(new CSoftwareTriggerConfiguration, RegistrationMode_ReplaceAll, Cleanup_Delete);
            // camera.RegisterConfiguration(new CConfigurationEventPrinter, RegistrationMode_Append, Cleanup_Delete);
            // camera.RegisterImageEventHandler(new CImageEventPrinter, RegistrationMode_Append, Cleanup_Delete);
            camera.RegisterImageEventHandler(new CSampleImageEventHandler, RegistrationMode_Append, Cleanup_Delete);

            camera.MaxNumBuffer = 2;
        } catch (const GenericException &e) {
            // Error handling.
            std::cerr << "An exception occurred." << std::endl << e.GetDescription() << std::endl;
            throw;
        }
        camera.Open();
    }
    void PylonCamera::Arm() {
        // Arming of various devices
        shrBufferFactory.PreallocateBuffers(sizeof(SharedImageHeader), GetImageWidth() * GetImageHeight() * sizeof(double), 2);

        camera.SetBufferFactory(&shrBufferFactory, Pylon::Cleanup_None);
    }
    void PylonCamera::Disarm() {
        // Free Shared Memory Buffers
    }
    void PylonCamera::Start() {
        if (camera.CanWaitForFrameTriggerReady())
            camera.StartGrabbing(GrabStrategy_LatestImages, GrabLoop_ProvidedByInstantCamera);
    }
    void PylonCamera::Stop() {
        camera.StopGrabbing();
    }

    GenApi::INodeMap &PylonCamera::GetNodeMap() {
        return camera.GetNodeMap();
    }

    unsigned int PylonCamera::GetImageWidth() {
        GenApi::INodeMap &nodemap = camera.GetNodeMap();
        CIntegerParameter width(nodemap, "Width");
        return width.GetValue();
    }

    unsigned int PylonCamera::GetImageHeight() {
        GenApi::INodeMap &nodemap = camera.GetNodeMap();
        CIntegerParameter height(nodemap, "Height");
        return height.GetValue();
    }
}
