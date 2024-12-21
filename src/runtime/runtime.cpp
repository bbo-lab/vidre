#include "vidre/runtime.h"
namespace vidre
{
    RuntimeInstance::RuntimeInstance()
    {
        Pylon::PylonInitialize();
        camera.Attach(Pylon::CTlFactory::GetInstance().CreateFirstDevice());
        camera.StartGrabbing(Pylon::GrabStrategy_LatestImageOnly);
    }
    RuntimeInstance::~RuntimeInstance()
    {
        camera.StopGrabbing();
        camera.Close();
        Pylon::PylonTerminate();
    }
    bool RuntimeInstance::IsGrabbing()
    {
        return camera.IsGrabbing();
    }
    void RuntimeInstance::Retrieve()
    {
        camera.RetrieveResult( 5000, ptrGrabResult, Pylon::TimeoutHandling_ThrowException );
    }
    Config RuntimeInstance::GetResultConfig()
    {
        return {
        ptrGrabResult->GetPixelType(),
        ptrGrabResult->GetWidth(),
        ptrGrabResult->GetHeight(),
        ptrGrabResult->GetPaddingX()
        };
    }
    const void* RuntimeInstance::GetBuffer()
    {
        return ptrGrabResult->GetBuffer();
    }
    size_t RuntimeInstance::GetBufferSize()
    {
        return ptrGrabResult->GetBufferSize();
    }
}