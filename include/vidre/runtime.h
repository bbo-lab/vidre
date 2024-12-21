#include <pylon/PylonIncludes.h>
#include <pylon/ImagePersistence.h>
#include "vidre/storage.h"
namespace vidre
{
    struct Config;

    class RuntimeInstance
    {
    public:
        RuntimeInstance();
        ~RuntimeInstance();
        bool IsGrabbing();
        void Retrieve();
        Config GetResultConfig();
        const void* GetBuffer();
        size_t GetBufferSize();
    private:
        Pylon::CGrabResultPtr ptrGrabResult;
        Pylon::CInstantCamera camera;
    };
}