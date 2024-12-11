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

// Include files used by samples.
#include "include/ConfigurationEventPrinter.h"
#include "include/ImageEventPrinter.h"

class PylonCamera {
private:
    // -------- Pylon specific --------
    Pylon::CInstantCamera camera;
    ShrBufferFactory shrBufferFactory;
    // --------------------------------

    // SharedImageMemoryController shrImageMemoryController;

    void *getSharedMemorySegment();

public:
    PylonCamera();
    ~PylonCamera();

    void init();
    void arm();
    void disarm();
    void start();
    void stop();

    // void setSharedImageMemoryController(SharedImageMemoryController shrImageMemoryController);

    unsigned int getImageWidth();
    unsigned int getImageHeight();
};