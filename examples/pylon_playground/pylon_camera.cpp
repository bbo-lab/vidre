#include "pylon_camera.hpp"

using namespace Pylon;

class CSampleImageEventHandler : public CImageEventHandler
{
public:
    virtual void OnImageGrabbed(CInstantCamera & /*camera*/, const CGrabResultPtr &ptrGrabResult)
    {
        std::cout << "CSampleImageEventHandler::OnImageGrabbed called." << std::endl;
        SharedImageHeader *header = (SharedImageHeader *)(ptrGrabResult->GetBuffer()) - 1;
        header->frame_id = ptrGrabResult->GetImageNumber();
        header->time_stamp = ptrGrabResult->GetTimeStamp();

#ifdef PYLON_WIN_BUILD
        // Display the image
        Pylon::DisplayImage(1, ptrGrabResult);
#endif
    }
};

PylonCamera::PylonCamera()
{
    // Reference counted by Pylon.
    Pylon::PylonInitialize();
}
PylonCamera::~PylonCamera()
{
    stop();
    camera.Close();
    if (camera.IsPylonDeviceAttached())
        camera.DestroyDevice();

    Pylon::PylonTerminate();
}

void PylonCamera::init()
{
    try
    {
        camera.Attach(CTlFactory::GetInstance().CreateFirstDevice());
        // camera.RegisterConfiguration(new CSoftwareTriggerConfiguration, RegistrationMode_ReplaceAll, Cleanup_Delete);
        camera.RegisterConfiguration(new CConfigurationEventPrinter, RegistrationMode_Append, Cleanup_Delete);
        camera.RegisterImageEventHandler(new CImageEventPrinter, RegistrationMode_Append, Cleanup_Delete);
        camera.RegisterImageEventHandler(new CSampleImageEventHandler, RegistrationMode_Append, Cleanup_Delete);

        camera.MaxNumBuffer = 2;
    }
    catch (const GenericException &e)
    {
        // Error handling.
        std::cerr << "An exception occurred." << std::endl
                  << e.GetDescription() << std::endl;
        throw;
    }
    camera.Open();
}
void PylonCamera::arm()
{
    shrBufferFactory.PreallocateBuffers(sizeof(SharedImageHeader), getImageWidth() * getImageHeight() * sizeof(double), 1);

    camera.SetBufferFactory(&shrBufferFactory, Pylon::Cleanup_None);
}
void PylonCamera::disarm()
{
    // Free Shared Memory Buffers
}
void PylonCamera::start()
{
    if (camera.CanWaitForFrameTriggerReady())
        camera.StartGrabbing(GrabStrategy_LatestImages, GrabLoop_ProvidedByInstantCamera);

    // if (camera.WaitForFrameTriggerReady(1000, TimeoutHandling_ThrowException))
    //     camera.ExecuteSoftwareTrigger();
    // if (camera.WaitForFrameTriggerReady(1000, TimeoutHandling_ThrowException))
    //     camera.ExecuteSoftwareTrigger();
    // if (camera.WaitForFrameTriggerReady(1000, TimeoutHandling_ThrowException))
    //     camera.ExecuteSoftwareTrigger();
}
void PylonCamera::stop()
{
    camera.StopGrabbing();
}

unsigned int PylonCamera::getImageWidth()
{
    GenApi::INodeMap &nodemap = camera.GetNodeMap();
    CIntegerParameter width(nodemap, "Width");
    return width.GetValue();
}

unsigned int PylonCamera::getImageHeight()
{
    GenApi::INodeMap &nodemap = camera.GetNodeMap();
    CIntegerParameter height(nodemap, "Height");
    return height.GetValue();
}