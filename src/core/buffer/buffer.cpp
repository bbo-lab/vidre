#include <core/buffer/buffer.hpp>
#include <core/storage/storage.hpp>

#include <algorithm>
#include <stdexcept>

namespace vidre {
    namespace {
        constexpr size_t kPerSlotOverhead = 64;
        constexpr size_t kSegmentOverhead = 64 * 1024;
    }

    Buffer::Buffer(const Config& config)
    : _nextFrameToPop(0), _bufferSize(static_cast<int>(config.maxNumBuffer)), _lastConsumedFrameID(0),
      _segmentName(config.serialNumber)
    {
        if (config.maxNumBuffer == 0) {
            throw std::invalid_argument("Buffer: maxNumBuffer must be > 0 (camera '" + config.serialNumber + "')");
        }
        if (config.frameBytesBuffer == 0) {
            throw std::invalid_argument("Buffer: frameBytesBuffer must be > 0 (camera '" + config.serialNumber + "')");
        }

        boost::interprocess::shared_memory_object::remove(_segmentName.c_str());

        const size_t slotBytes = sizeof(SharedFrameHeader) + config.frameBytesBuffer + kPerSlotOverhead;
        const size_t capacity = slotBytes * static_cast<size_t>(_bufferSize) + kSegmentOverhead;

        _sharedMemory = std::make_unique<boost::interprocess::managed_shared_memory>(
            boost::interprocess::create_only, _segmentName.c_str(), capacity);

        _sharedMemoryPtrs = std::make_unique<std::vector<void *>>(static_cast<size_t>(_bufferSize), nullptr);
    }

    Buffer::~Buffer() {
        _sharedMemory.reset();
        if (!_segmentName.empty()) {
            boost::interprocess::shared_memory_object::remove(_segmentName.c_str());
        }
    }

    std::pair<void*, SharedFrameHeader*> Buffer::pop() {
        if (_bufferSize <= 0 || !_sharedMemoryPtrs) {
            return {nullptr, nullptr};
        }

        const auto buffer_idx = static_cast<size_t>(_nextFrameToPop % static_cast<uint64_t>(_bufferSize));

        void* buffer_ptr = (*_sharedMemoryPtrs)[buffer_idx];
        if (buffer_ptr == nullptr) {
            return {nullptr, nullptr};
        }

        auto* header = static_cast<SharedFrameHeader*>(buffer_ptr);

        const int64_t frameID = header->frameID.load(std::memory_order_acquire);
        if (frameID == 0 || frameID <= _lastConsumedFrameID) {
            return {nullptr, nullptr};
        }

        if (frameID > _lastConsumedFrameID + 1) {
            _droppedFrames.fetch_add(static_cast<int>(frameID - _lastConsumedFrameID - 1), std::memory_order_relaxed);
        }

        _lastConsumedFrameID = frameID;
        _consumedFrames.fetch_add(1, std::memory_order_relaxed);
        _nextFrameToPop++;

        return {static_cast<uint8_t*>(buffer_ptr) + sizeof(SharedFrameHeader), header};
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
