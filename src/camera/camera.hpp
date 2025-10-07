#pragma once
#include <string>
#include <memory>

#include <boost/interprocess/managed_shared_memory.hpp>
#include "core/storage/storage.hpp"



#ifdef _WIN32
    #define VIDRE_CAMERA_API __declspec(dllexport)
#else
    #define VIDRE_CAMERA_API __attribute__((visibility("default")))
#endif

namespace vidre {
    // Aptina camera does not seem to support GenICam cameras. Is adapting Aptina to
    // GenICam interface a valid and reliable option?

    // Might be needed adding another inheritance layer between
    // ICamera and PylonCamera: GenICamera. That way mainly the
    // aspects around camera parameters can be combined.

    class ICamera {
    public:
        // virtual ~ICamera() = default;

        // -------------------
        // Runtime controls
        // -------------------

        virtual void Init() = 0;
        virtual bool Init(std::string serial) = 0;
        virtual void Arm() = 0;
        virtual void Disarm() = 0;
        virtual void Start() = 0;
        virtual void Stop() = 0;

        virtual bool IsArmed() = 0;
        virtual bool IsRunning() = 0;

        virtual void SetSharedMemory(boost::interprocess::managed_shared_memory* sharedMemory, std::vector<void *>* ptrs, std::string memory_name) = 0;
        virtual const SharedFrameHeader *RetrieveLatestFrame() = 0;

        // Idea: Custom functions for different filetypes.
        // Idea: Seperate object for configurations.
        virtual bool SaveConfiguration(std::string filepath) = 0;
        virtual bool LoadConfiguration(std::string filepath) = 0;

        // Convenience functions for now. Using GenApi or own abstraction?
        virtual unsigned int GetFrameWidth() = 0;
        virtual unsigned int GetFrameHeight() = 0;
        virtual unsigned int GetFrameOffsetX() = 0;
        virtual unsigned int GetFrameOffsetY() = 0;
        virtual unsigned int GetMaxNumBuffer() = 0;
        virtual unsigned int GetFrameBytes() = 0;
        virtual unsigned int GetBufferSize() = 0;
        virtual double GetFrameRate() = 0;
        virtual std::string GetSerialNumber() = 0;
        virtual std::string GetVendorName() = 0;
        virtual void SetFrameWidth(unsigned int newFrameWidth) = 0;
        virtual void SetFrameHeight(unsigned int newFrameHeight) = 0;
        virtual void SetFrameOffsetX(unsigned int newFrameOffsetX) = 0;
        virtual void SetFrameOffsetY(unsigned int newFrameOffsetY) = 0;
        virtual void SetFrameRate(double newFrameRate) = 0;

        virtual void SetTriggerIn(unsigned int line) = 0;
        virtual void SetTriggerOut(unsigned int line) = 0;
    };
}  // namespace vidre