#include "vidre/buffer.hpp"
#include "vidre/storage.hpp"

#include <limits>

namespace vidre {
    Buffer::Buffer(const Config& config) 
    : _nextFrameToPop(0), _bufferSize(config.maxNumBuffer), _previouslyPopedFrame(0)
    {
        boost::interprocess::shared_memory_object::remove(config.serialNumber.c_str());

        size_t capacity = (sizeof(SharedFrameHeader) + config.frameBytesBuffer) * 
                          (_bufferSize + 1);

        _sharedMemory = std::make_unique<boost::interprocess::managed_shared_memory>(
            boost::interprocess::create_only, config.serialNumber.c_str(), capacity);

        _sharedMemoryPtrs = std::make_unique<std::vector<void *>>(_bufferSize, nullptr);
    }

    std::pair<void*, SharedFrameHeader*> Buffer::pop() {
        auto buffer_idx = _nextFrameToPop % _bufferSize;

        void* buffer_ptr = _sharedMemoryPtrs->at(buffer_idx); 
        if (buffer_ptr == nullptr) {
            return {nullptr, nullptr};
        }

        vidre::SharedFrameHeader* result = static_cast<vidre::SharedFrameHeader *>(buffer_ptr);
        if (result == nullptr) {
            return {nullptr, nullptr};
        }
        if (result == nullptr || result->frameID == 0 || _nextFrameToPop >= result->frameID) {
            return {nullptr, nullptr};
        }
        
        if (result->frameID > _previouslyPopedFrame + 1) {
            std::cout << " WARNING: RETRIEVED FRAME BIGGER THAN BUFFER + 1\n";
        }
        
        _previouslyPopedFrame = result->frameID;

        _nextFrameToPop++;

        return {static_cast<uint8_t*>(buffer_ptr) + sizeof(vidre::SharedFrameHeader) , result};
    }

    int Buffer::getSavedFramesNumber() {
        return _nextFrameToPop;
    }
        
}  // namespace vidre