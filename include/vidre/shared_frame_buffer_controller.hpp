#include <memory>
#include <atomic>

#include "shared_frame_header.hpp"

// Just one for now. In the future other plugins should have the ability to create their own SharedFrameBufferController.
// That way e.g. the following pipeline can be realised:
//
//  Camera -> Standard SFBC -> GPU Processing -> GPU SFBC
//               -> Storage                         -> Storage
//
// Also Frame could also mean just a snapshot in time. Maybe this can be reused to provide e.g. audio data or event data.
//
// Managing named shared memory for different plugins in general is also important. A sort of "name broker" should be utilised.
// Maybe using UUIDs for that even though some elements might not be physical devices.

namespace vidre {
    // This class is not realy able to sit on SharedMemory. Explicitly make it more static and keep just the relevant
    // data on SharedMemory.
    class SharedFrameBufferController {
    private:
        unsigned short maxNumFrames;
        std::atomic<SharedFrameHeader *> latestFrame;  // The pointer has to be replaced by boost::interprocess::offset_ptr

        SharedFrameBufferController() : maxNumFrames(0), latestFrame(0) {}

    public:
        ~SharedFrameBufferController();

        static SharedFrameBufferController *CreateOnSharedMemory();

        void Init();
        void Preallocate();
        void Deallocate();

        unsigned short GetMaxNumFrames();
        SharedFrameHeader *GetLatestFrame();
    };
}  // namespace vidre