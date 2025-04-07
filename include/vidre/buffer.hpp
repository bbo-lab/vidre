#pragma once

#include <boost/interprocess/managed_shared_memory.hpp>

#include "config.h"


namespace vidre {
    class Buffer {
    public:
        Buffer() {};
        ~Buffer() {};

        std::vector<void*>* getSharedMemoryPtrs() {
            return _sharedMemoryPtrs.get();
        }

        boost::interprocess::managed_shared_memory* getSharedMemory() {
            return _sharedMemory.get();
        }
        
        void* pop() {return nullptr;}; // -> pop() without args _currentFrame here from runtime; sort of retrieveFrame
    private:
        std::unique_ptr<boost::interprocess::managed_shared_memory> _sharedMemory;
        std::unique_ptr<std::vector<void *>> _sharedMemoryPtrs;
    };
}  // namespace vidre