#pragma once

#include <boost/interprocess/managed_shared_memory.hpp>

#include <core/runtime/shared_frame_header.hpp>
#include <config.h>

#include <atomic>
#include <memory>
#include <utility>
#include <vector>

namespace vidre {
    struct Config;

    class Buffer {
    public:
        Buffer() = default;
        Buffer(const Config& config);
        ~Buffer() = default;

        std::vector<void*>* getSharedMemoryPtrs() {
            return _sharedMemoryPtrs.get();
        }

        boost::interprocess::managed_shared_memory* getSharedMemory() {
            return _sharedMemory.get();
        }

        int getSavedFramesNumber() {
            return _consumedFrames.load(std::memory_order_relaxed);
        }

        int getDroppedFrames() {
            return _droppedFrames.load(std::memory_order_relaxed);
        }

        int numSlots() {
            return _bufferSize;
        }

        void reset();

        std::pair<void*, SharedFrameHeader*> pop();
    private:
        int _nextFrameToPop = 0;
        int _bufferSize = 0;
        int64_t _lastConsumedFrameID = 0;
        std::atomic<int> _consumedFrames{0};
        std::atomic<int> _droppedFrames{0};
        std::unique_ptr<boost::interprocess::managed_shared_memory> _sharedMemory;
        std::unique_ptr<std::vector<void *>> _sharedMemoryPtrs;
    };
}  // namespace vidre
