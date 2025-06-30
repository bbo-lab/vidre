#pragma once

#include <string>

#include <boost/interprocess/managed_shared_memory.hpp>

#include "shared_frame_header.hpp"

class AptinaBuffer  {
public:
    AptinaBuffer() : m_lastBufferContext(-1) {}

    ~AptinaBuffer() {} // todo


    void SetSharedMemory(boost::interprocess::managed_shared_memory* sharedMemory, std::vector<void *>* ptrs, std::string memory_name) {
        shared_memory_ptrs = ptrs;
        memoryName = memory_name;
        shared_memory = sharedMemory;
    }

    void AllocateBuffer(size_t bufferSize)  {
        ++m_lastBufferContext;

        void *allocatedMemory = shared_memory->allocate(bufferSize + sizeof(vidre::SharedFrameHeader));
        auto *pCreatedBuffer =
            (void *)((vidre::SharedFrameHeader *)(allocatedMemory) + 1);
        
        uint8_t* bytePointer = (uint8_t *)(pCreatedBuffer);
        for (size_t i = 0; i < bufferSize; ++i) {
            bytePointer[i] = 0;
        }
        
        shared_memory_ptrs->at(m_lastBufferContext) = allocatedMemory;
    }

    void* CurrentBuffer(unsigned int buffer_num) {
        auto buffer_idx = ptr_counter % buffer_num;

        void* buffer_ptr = shared_memory_ptrs->at(buffer_idx);

        ptr_counter++;

        return buffer_ptr;
    }

protected:
    std::string memoryName;
    std::vector<void *>* shared_memory_ptrs;
    boost::interprocess::managed_shared_memory* shared_memory;
    int64_t m_lastBufferContext;
    int64_t ptr_counter = 0;
};