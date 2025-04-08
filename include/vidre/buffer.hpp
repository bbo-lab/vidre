#pragma once

#include <boost/interprocess/managed_shared_memory.hpp>

#include "vidre/shared_frame_header.hpp"
#include "config.h"

#include <memory>
#include <vector>

namespace vidre {
    struct Config;

    class Buffer {
    public:
        Buffer() = default;
        Buffer(const Config& config);
        ~Buffer() {};

        std::vector<void*>* getSharedMemoryPtrs() {
            return _sharedMemoryPtrs.get();
        }

        boost::interprocess::managed_shared_memory* getSharedMemory() {
            return _sharedMemory.get();
        }
        
        int getSavedFramesNumber();
        
        SharedFrameHeader* pop();
    private:
        int _currentFrameID;
        int _bufferSize;
        int _previousFrameID;
        std::unique_ptr<boost::interprocess::managed_shared_memory> _sharedMemory;
        std::unique_ptr<std::vector<void *>> _sharedMemoryPtrs;
    };
}  // namespace vidre