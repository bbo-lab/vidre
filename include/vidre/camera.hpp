#pragma once
#include <string>
#include <memory>

#include "shared_frame_header.hpp"
#include "shared_frame_buffer_controller.hpp"

namespace vidre {
    class ICamera {
    public:
        //virtual ~ICamera() = default;

        virtual void Init() = 0;
        virtual void Arm() = 0;
        virtual void Disarm() = 0;
        virtual void Start() = 0;
        virtual void Stop() = 0;

        virtual bool IsRunning() = 0;

        virtual SharedFrameHeader* RetrieveLatestFrame() = 0;

        virtual void LoadConfiguration(const char* filepath) = 0;

        virtual unsigned int GetFrameWidth() = 0;
        virtual unsigned int GetFrameHeight() = 0;
        virtual unsigned int GetFrameOffsetX() = 0;
    };
}  // namespace vidre