#pragma once
#include <string>
#include <memory>

#include "shared_frame_header.hpp"
#include "shared_frame_buffer_controller.hpp"

namespace vidre {
    // Aptina camera does not seem to support GenICam cameras. Is adapting Aptina to 
    // GenICam interface a valid and reliable option?
    
    // Might be needed adding another inheritance layer between
    // ICamera and PylonCamera: GenICamera. That way mainly the 
    // aspects around camera parameters can be combined.

    class ICamera {
    public:
        //virtual ~ICamera() = default;

        // -------------------
        // Runtime controls
        // -------------------

        virtual void Init() = 0;
        virtual void Arm() = 0;
        virtual void Disarm() = 0;
        virtual void Start() = 0;
        virtual void Stop() = 0;

        virtual bool IsArmed() = 0;
        virtual bool IsRunning() = 0;

        virtual SharedFrameHeader* RetrieveLatestFrame() = 0;

        virtual bool LoadConfiguration(const char* filepath) = 0;
        virtual bool SaveConfiguration(const char* filepath) = 0;

        // Convenience functions for now. Using GenApi or own abstraction?
        virtual unsigned int GetFrameWidth() = 0;
        virtual unsigned int GetFrameHeight() = 0;
        virtual unsigned int GetFrameOffsetX() = 0;
        virtual double GetFrameRate() = 0;
        virtual void SetFrameRate(double newFrameRate) = 0;
    };
}  // namespace vidre