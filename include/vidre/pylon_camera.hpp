#pragma once
#include "camera.hpp"

#include <thread>

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

        virtual bool IsArmed();
        virtual bool IsRunning();

        virtual SharedFrameHeader* RetrieveLatestFrame();

        virtual bool LoadConfiguration(const char* filepath);
        virtual bool SaveConfiguration(const char* filepath);

        GenApi::INodeMap &GetNodeMap();

        // For convenience.
        virtual unsigned int GetFrameWidth();
        virtual unsigned int GetFrameHeight();
        virtual unsigned int GetFrameOffsetX();
        virtual double GetFrameRate();
        virtual void SetFrameRate(double newFrameRate);
        
    private:
        void run();

        std::thread grabber;
        std::atomic<bool> running;

        std::atomic<bool> armed;

        std::atomic<SharedFrameHeader*> latestFrame;

        // -------- Pylon specific --------
        Pylon::CInstantCamera camera;
        Pylon::PylonAutoInitTerm pylonAutoInitTerm;
        ShrBufferFactory shrBufferFactory;
        // --------------------------------
    };
}  // namespace vidre