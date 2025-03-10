#pragma once
#include "camera.hpp"

#include <thread>

// #ifdef PYLON_WIN_BUILD
#include <pylon/PylonGUI.h>
// #endif
#include <pylon/PylonIncludes.h>
#include "pylon_shr_mem_buffer_factory.hpp"

namespace vidre {
    // Currently using explicit Pylon Camera. Can it be replace by the more generic GenICam or USB3Vision stuff?

    class PylonCamera : public ICamera {
    public:
        PylonCamera();
        ~PylonCamera();

        virtual void Init() override;
        virtual bool Init(std::string serial) override;
        virtual void Arm() override;
        virtual void Disarm() override;
        virtual void Start() override;
        virtual void Stop() override;

        virtual bool IsArmed() override;
        virtual bool IsRunning() override;

        virtual void SetSharedMemory(boost::interprocess::managed_shared_memory* shared_memory) override;
        virtual const SharedFrameHeader *RetrieveLatestFrame() override;

        virtual bool SaveConfiguration(std::string filepath) override;
        virtual bool LoadConfiguration(std::string filepath) override;

        GenApi::INodeMap &GetGenAPINodeMap();

        // For convenience.
        virtual unsigned int GetFrameWidth() override;
        virtual unsigned int GetFrameHeight() override;
        virtual unsigned int GetFrameOffsetX() override;
        virtual unsigned int GetFrameOffsetY() override;
        virtual unsigned int GetMaxNumBuffer() override;
        virtual double GetFrameRate() override;
        virtual std::string GetSerialNumber() override;
        virtual void SetFrameWidth(unsigned int newFrameWidth) override;
        virtual void SetFrameHeight(unsigned int newFrameHeight) override;
        virtual void SetFrameOffsetX(unsigned int newFrameOffsetX) override;
        virtual void SetFrameOffsetY(unsigned int newFrameOffsetY) override;
        virtual void SetFrameRate(double newFrameRate) override;

    private:
        void run();

        std::thread grabber;
        std::atomic<bool> running;

        std::atomic<bool> armed;

        std::atomic<SharedFrameHeader *> latestFrame;

        // -------- Pylon specific --------
        Pylon::CInstantCamera camera;
        Pylon::PylonAutoInitTerm pylonAutoInitTerm;
        ShrBufferFactory shrBufferFactory;
        // --------------------------------
    };
}  // namespace vidre