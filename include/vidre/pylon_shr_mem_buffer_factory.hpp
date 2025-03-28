#pragma once

#include <string>

#include <pylon/PylonIncludes.h>
#include <boost/interprocess/managed_shared_memory.hpp>

#include "shared_frame_header.hpp"

class ShrBufferFactory : public Pylon::IBufferFactory {
public:
    ShrBufferFactory() : m_lastBufferContext(-1) {}

    virtual ~ShrBufferFactory() {}


    void SetSharedMemory(std::vector<void *>* ptrs, std::string memory_name) {
        shared_memory_ptrs = ptrs;
        memoryName = memory_name;
    }

    void PreallocateFrameBuffers() {
        m_lastBufferContext = -1;
    }

    void AllocateBuffer(size_t bufferSize, void **pCreatedBuffer, intptr_t &bufferContext) override {
        try {
            std::cout <<"H\n";
            boost::interprocess::managed_shared_memory shared_memory(boost::interprocess::open_only, memoryName.c_str());
            // Allocation should be delegated to a seperate object so that the data can be accessed by other resources.
            bufferContext = ++m_lastBufferContext;

            void *allocatedMemory = shared_memory.allocate(bufferSize + sizeof(vidre::SharedFrameHeader));
            *pCreatedBuffer =
                (void *)((vidre::SharedFrameHeader *)(allocatedMemory) + 1);
            // Zero out buffer
            byte *bytePointer = (byte *)(*pCreatedBuffer);
            for (size_t i = 0; i < bufferSize; ++i) {
                bytePointer[i] = 0;
            }
            std::cout <<"H\n";
            shared_memory_ptrs->push_back(
                shared_memory.construct<void *>(std::to_string(m_lastBufferContext).c_str())(allocatedMemory)
            );
        } catch (const std::exception &) {
            if (*pCreatedBuffer != NULL) {
                boost::interprocess::managed_shared_memory shared_memory(boost::interprocess::open_only, memoryName.c_str());
                shared_memory.deallocate(pCreatedBuffer);
            }
            throw;
        }
    }
    
    void FreeBuffer(void *pCreatedBuffer, intptr_t bufferContext) override {
            boost::interprocess::managed_shared_memory shared_memory(boost::interprocess::open_only, memoryName.c_str());
            shared_memory.deallocate((vidre::SharedFrameHeader *)pCreatedBuffer - 1);
    }

    // todo: fix this
    void DestroyBufferFactory() override {
        boost::interprocess::shared_memory_object::remove(memoryName.c_str());
        delete this;
    }

protected:
    std::string memoryName;
    std::vector<void *>* shared_memory_ptrs;
    unsigned long m_lastBufferContext;
};