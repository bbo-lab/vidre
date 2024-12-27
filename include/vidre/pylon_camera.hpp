#pragma once
#include <string>
#include <memory>

#include <pylon/PylonIncludes.h>

#include "pylon_shr_mem_buffer_factory.hpp"
#include "shared_image_header.hpp"
#include "shared_image_memory_controller.hpp"

#ifdef PYLON_WIN_BUILD
#include <pylon/PylonGUI.h>
#endif

namespace vidre {
    class PylonCamera {
    public:
        PylonCamera();
        ~PylonCamera();

        void Init();
        void Arm();
        void Disarm();
        void Start();
        void Stop();

        // void setSharedImageMemoryController(SharedImageMemoryController shrImageMemoryController);

        GenApi::INodeMap &GetNodeMap();
        unsigned int GetImageWidth();
        unsigned int GetImageHeight();
        
    private:
        // -------- Pylon specific --------
        Pylon::CInstantCamera camera;
        ShrBufferFactory shrBufferFactory;
        // --------------------------------

        // SharedImageMemoryController shrImageMemoryController;

    };
}  // namespace vidre