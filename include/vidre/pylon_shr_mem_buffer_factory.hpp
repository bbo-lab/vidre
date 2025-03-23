#pragma once

#include <string>

#include <pylon/PylonIncludes.h>
#include <boost/interprocess/managed_shared_memory.hpp>

#include "shared_frame_header.hpp"

class ShrBufferFactory : public Pylon::IBufferFactory {
public:
    ShrBufferFactory() : m_lastBufferContext(-1) {}

    virtual ~ShrBufferFactory() {}


    void SetSharedMemory(boost::interprocess::managed_shared_memory* storage_shared_memory, std::string memory_name) {
        shared_memory = storage_shared_memory;
        memoryName = memory_name;
    }

    void PreallocateFrameBuffers() {
        m_lastBufferContext = -1;
    }

    void AllocateBuffer(size_t bufferSize, void **pCreatedBuffer, intptr_t &bufferContext) override {
        try {
            // Allocation should be delegated to a seperate object so that the data can be accessed by other resources.
            bufferContext = ++m_lastBufferContext;

            void *allocatedMemory = shared_memory->allocate(bufferSize + sizeof(vidre::SharedFrameHeader));
            *pCreatedBuffer =
                (void *)((vidre::SharedFrameHeader *)(allocatedMemory) + 1);
            // Zero out buffer
            byte *bytePointer = (byte *)(*pCreatedBuffer);
            for (size_t i = 0; i < bufferSize; ++i) {
                bytePointer[i] = 0;
            }
            shared_memory->find_or_construct<void *>(std::to_string(m_lastBufferContext).c_str())(allocatedMemory);
        } catch (const std::exception &) {
            if (*pCreatedBuffer != NULL) {
                shared_memory->deallocate(pCreatedBuffer);
            }
            throw;
        }
    }
    
    void FreeBuffer(void *pCreatedBuffer, intptr_t bufferContext) override {
        shared_memory->deallocate((vidre::SharedFrameHeader *)pCreatedBuffer - 1);
    }

    // todo: fix this
    void DestroyBufferFactory() override {
        boost::interprocess::shared_memory_object::remove(memoryName.c_str());
        delete this;
    }

protected:
    std::string memoryName;
    boost::interprocess::managed_shared_memory* shared_memory;
    unsigned long m_lastBufferContext;
};