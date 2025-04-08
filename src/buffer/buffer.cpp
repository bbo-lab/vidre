#include "vidre/buffer.hpp"
#include "vidre/storage.hpp"
#include "vidre/shared_frame_header.hpp"

namespace vidre {
    Buffer::Buffer(const Config& config) 
    : _currentFrameID(0), _bufferSize(config.maxNumBuffer), _previousFrameID(0)
    {
        boost::interprocess::shared_memory_object::remove(_config.serialNumber.c_str());
        
        size_t capacity = (sizeof(SharedFrameHeader) + _config.width * _config.height) * 
                          (_config.maxNumBuffer + 1);

        _sharedMemory = std::make_unique<boost::interprocess::managed_shared_memory>(
            boost::interprocess::create_only, _config.serialNumber.c_str(), capacity);

        _sharedMemoryPtrs = std::make_unique<std::vector<void*>>(_config.maxNumBuffer, nullptr);
    }

    SharedFrameHeader* Buffer::pop() {
        auto buffer_idx = _currentFrameID % _bufferSize;
        void* buffer_ptr = _sharedMemoryPtrs->at(buffer_idx);
        
        if (buffer_ptr == nullptr) {
            return nullptr;
        }

        vidre::SharedFrameHeader* result = static_cast<vidre::SharedFrameHeader *>(buffer_ptr);

        if (result == nullptr || result->frameID == 0 || prev_frame_id >= result->frameID) {
            return nullptr;
        }
        
        if (result->frameID > _previousFrameID + 1) {
            std::cout << "WARNING: RETRIEVED FRAME BIGGER THAN BUFFER + 1\n";
        }
        
        _previousFrameID = result->frameID;

        _currentFrameID++;


        return result;
    }

    int Buffer::getSavedFramesNumber() {
        return _currentFrameID;
    }
        
}  // namespace vidre