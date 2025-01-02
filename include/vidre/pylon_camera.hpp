#pragma once
#include "camera.hpp"

//#ifdef PYLON_WIN_BUILD
#include <pylon/PylonGUI.h>
//#endif
#include <pylon/PylonIncludes.h>
#include "pylon_shr_mem_buffer_factory.hpp"

namespace vidre {
    // Currently using explicit Pylon Camera. Can it be replace by the more generic GenICam or USB3Vision stuff?

    class PylonCamera : public ICamera {
    public:
        PylonCamera();
        ~PylonCamera();

        virtual void Init();
        virtual void Arm();
        virtual void Disarm();
        virtual void Start();
        virtual void Stop();

        virtual SharedFrameHeader* RetrieveLatestFrame();

        GenApi::INodeMap &GetNodeMap();
        unsigned int GetFrameWidth();
        unsigned int GetFrameHeight();
        unsigned int GetFrameOffsetX();
        
    private:
        // -------- Pylon specific --------
        Pylon::CInstantCamera camera;
        ShrBufferFactory shrBufferFactory;
        std::atomic<SharedFrameHeader*> latestFrame;
        // --------------------------------
    };
}  // namespace vidre