#include <memory>

#include "shared_image_header.hpp"

// NYI
//
// Might be better implemented as an interface in the future to allow for various memory management strategies.
class SharedImageMemoryController {
private:
    unsigned int num_frame_buffers;

public:
    SharedImageMemoryController();
    ~SharedImageMemoryController();

    void init();
    void preallocate();
    void deallocate();

    unsigned int getNumFrameBuffers();
    void *requestImageMemory(unsigned int frame_buffer_index);
};