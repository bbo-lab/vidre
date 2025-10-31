#pragma once
#include "camera/camera.hpp"

#include <thread>

// #ifdef PYLON_WIN_BUILD
#include <pylon/PylonGUI.h>
// #endif
#include <pylon/PylonIncludes.h>
#include "pylon_buffer.hpp"

namespace vidre {
    /// @brief Camera Interface Implementation for Pylon API
    class PylonCamera : public ICamera {
    public:
        PylonCamera();
        ~PylonCamera();

        /// @brief creates Pylon::CInstantCamera
        virtual void Init() override;
        /// @brief creates Pylon::CInstantCamera from given serial number
        /// @param serial serial number of a camera
        virtual bool Init(std::string serial) override;
        /// @brief Preallocates buffer for camera
        virtual void Arm() override;
        virtual void Disarm() override;
        /// @brief Starts a new thread and executes run where frame grabbing happens
        virtual void Start() override;
        virtual void Stop() override;

        virtual bool IsArmed() override;
        virtual bool IsRunning() override;
        /// @brief Sets shared memory owned by vidre::Storage for Pylon::IBufferFactory which handles frame allocation
        /// @param sharedMemory shared memory instance
        /// @param ptrs vector of pointers to each of the shared memory block
        /// @param memory_name shared memory name (boost related)
        virtual void SetSharedMemory(boost::interprocess::managed_shared_memory* sharedMemory, std::vector<void *>* ptrs, std::string memory_name) override;
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
        virtual unsigned int GetFrameBytes() override;
        virtual unsigned int GetBufferSize() override;
        virtual double GetFrameRate() override;
        virtual std::string GetSerialNumber() override;
        virtual std::string GetVendorName() override;
        virtual void SetFrameWidth(unsigned int newFrameWidth) override;
        virtual void SetFrameHeight(unsigned int newFrameHeight) override;
        virtual void SetFrameOffsetX(unsigned int newFrameOffsetX) override;
        virtual void SetFrameOffsetY(unsigned int newFrameOffsetY) override;
        virtual void SetFrameRate(double newFrameRate) override;

        virtual void SetTriggerIn(unsigned int line) override;
        virtual void SetTriggerOut(unsigned int line) override;
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


        int64_t retrievedFramesNumber = 0;
        // --------------------------------
    };
}  // namespace vidre