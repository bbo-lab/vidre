#pragma once

#include <string>

#include <pylon/PylonIncludes.h>
#include <boost/interprocess/managed_shared_memory.hpp>

#include "shared_frame_header.hpp"

class ShrBufferFactory : public Pylon::IBufferFactory {
public:
    ShrBufferFactory() : m_lastBufferContext(-1) {}

    virtual ~ShrBufferFactory() {}


    void SetSharedMemory(boost::interprocess::managed_shared_memory* storage_shared_memory) {
        shared_memory = storage_shared_memory;
    }

    // void PreallocateFrameBuffers(size_t capacity, const char *newMemoryName) {
    void PreallocateFrameBuffers() {
        // boost::interprocess::shared_memory_object::remove(newMemoryName);
        // shared_memory = {boost::interprocess::open_or_create, newMemoryName, capacity};
        m_lastBufferContext = -1;
    }

    virtual void AllocateBuffer(size_t bufferSize, void **pCreatedBuffer, intptr_t &bufferContext) {
        try {
            // Allocation should be delegated to a seperate object so that the data can be accessed by other resources.
            bufferContext = ++m_lastBufferContext;
            *pCreatedBuffer =
                (void *)((vidre::SharedFrameHeader *)(shared_memory->allocate(bufferSize + sizeof(vidre::SharedFrameHeader))) + 1);
            // Zero out buffer
            byte *bytePointer = (byte *)(*pCreatedBuffer);
            for (size_t i = 0; i < bufferSize; ++i) {
                bytePointer[i] = 0;
            }

            // std::cout << "Acquired buffer " << bufferContext << ", " << *pCreatedBuffer << " of size " << bufferSize <<
            // std::endl;
        } catch (const std::exception &) {
            if (*pCreatedBuffer != NULL) {
                shared_memory->deallocate(pCreatedBuffer);
            }
            throw;
        }
    }
    virtual void FreeBuffer(void *pCreatedBuffer, intptr_t bufferContext) {
        shared_memory->deallocate((vidre::SharedFrameHeader *)pCreatedBuffer - 1);
    }
    virtual void DestroyBufferFactory() {
        boost::interprocess::shared_memory_object::remove(memoryName);
        delete this;
    }

protected:
    const char *memoryName;
    boost::interprocess::managed_shared_memory* shared_memory;
    unsigned long m_lastBufferContext;
};