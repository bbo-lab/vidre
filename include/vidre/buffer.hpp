#pragma once

#include <boost/interprocess/managed_shared_memory.hpp>

#include "config.h"


namespace vidre {
    struct Config;

    class Buffer {
    public:
        Buffer() = delete;
        Buffer(const Config& config);
        ~Buffer() {};

        std::vector<void*>* getSharedMemoryPtrs() {
            return _sharedMemoryPtrs.get();
        }

        boost::interprocess::managed_shared_memory* getSharedMemory() {
            return _sharedMemory.get();
        }
        
        int getSavedFramesNumber();
        
        void* pop() {return nullptr;}; // -> pop() without args _currentFrame here from runtime; sort of retrieveFrame
    private:
        int _currentFrameID;
        int _bufferSize;
        int _previousFrameID;
        std::unique_ptr<boost::interprocess::managed_shared_memory> _sharedMemory;
        std::unique_ptr<std::vector<void *>> _sharedMemoryPtrs;
    };
}  // namespace vidre