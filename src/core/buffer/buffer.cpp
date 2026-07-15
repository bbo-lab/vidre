#include <core/buffer/buffer.hpp>
#include <core/storage/storage.hpp>

#include <limits>
#include <iostream>

namespace vidre {
    Buffer::Buffer(const Config& config)
    : _nextFrameToPop(0), _bufferSize(config.maxNumBuffer), _lastConsumedFrameID(0)
    {
        boost::interprocess::shared_memory_object::remove(config.serialNumber.c_str());

        size_t capacity = (sizeof(SharedFrameHeader) + config.frameBytesBuffer) *
                          (_bufferSize + 1);

        _sharedMemory = std::make_unique<boost::interprocess::managed_shared_memory>(
            boost::interprocess::create_only, config.serialNumber.c_str(), capacity);

        _sharedMemoryPtrs = std::make_unique<std::vector<void *>>(_bufferSize, nullptr);

        _frameSemaphore = std::make_unique<std::counting_semaphore<>>(0);
    }

    std::pair<void*, SharedFrameHeader*> Buffer::pop() {
        auto buffer_idx = _nextFrameToPop % _bufferSize;
        _nextFrameToPop++;

        void* buffer_ptr = _sharedMemoryPtrs->at(buffer_idx);
        if (buffer_ptr == nullptr) {
            return {nullptr, nullptr};
        }

        auto* header = static_cast<SharedFrameHeader*>(buffer_ptr);

        int64_t frameID = header->frameID.load(std::memory_order_acquire);
        if (frameID == 0 || frameID <= _lastConsumedFrameID) {
            return {nullptr, nullptr};
        }

        if (frameID > _lastConsumedFrameID + 1) {
            int dropped = static_cast<int>(frameID - _lastConsumedFrameID - 1);
            _droppedFrames += dropped;
            std::cerr << " WARNING: dropped " << dropped
                      << " frame(s) before frame " << frameID << "\n";
        }

        _lastConsumedFrameID = frameID;
        _consumedFrames++;

        return {static_cast<uint8_t*>(buffer_ptr) + sizeof(SharedFrameHeader), header};
    }

    int Buffer::getSavedFramesNumber() {
        return _consumedFrames;
    }

    void Buffer::reset() {
        _nextFrameToPop = 0;
        _lastConsumedFrameID = 0;
        _consumedFrames = 0;
        _droppedFrames = 0;

        if (_sharedMemoryPtrs) {
            std::fill(_sharedMemoryPtrs->begin(), _sharedMemoryPtrs->end(), nullptr);
        }
    }

}  // namespace vidre