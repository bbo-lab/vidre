#pragma once

#include <boost/interprocess/managed_shared_memory.hpp>

#include "core/runtime/shared_frame_header.hpp"
#include "config.h"

#include <memory>
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
        
        int getSavedFramesNumber();
        
        std::pair<void*, SharedFrameHeader*> pop();
    private:
        int _nextFrameToPop;
        int _bufferSize;
        int _previouslyPopedFrame;
        std::unique_ptr<boost::interprocess::managed_shared_memory> _sharedMemory;
        std::unique_ptr<std::vector<void *>> _sharedMemoryPtrs;
    };
}  // namespace vidre