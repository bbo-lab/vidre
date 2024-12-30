#pragma once
#include <pylon/PylonIncludes.h>
#include <boost/interprocess/managed_shared_memory.hpp>

#include "shared_frame_header.hpp"

class ShrBufferFactory : public Pylon::IBufferFactory
{
public:
    ShrBufferFactory()
        : m_lastBufferContext(-1)
    {
    }

    virtual ~ShrBufferFactory()
    {
    }

    virtual void AllocateBuffer(size_t bufferSize, void **pCreatedBuffer, intptr_t &bufferContext)
    {
        try
        {
            // Allocation should be delegated to a seperate object so that the data can be accessed by other resources.
            *pCreatedBuffer = (void *)((SharedFrameHeader *)(shared_memory.allocate(bufferSize + sizeof(SharedFrameHeader))) + 1);

            bufferContext = ++m_lastBufferContext;

            std::cout << "Created buffer " << bufferContext << ", " << *pCreatedBuffer << " of size " << bufferSize << std::endl;
        }
        catch (const std::exception &)
        {
            if (*pCreatedBuffer != NULL)
            {
                shared_memory.deallocate(pCreatedBuffer);
            }
            throw;
        }
    }

    virtual void FreeBuffer(void *pCreatedBuffer, intptr_t bufferContext)
    {
        shared_memory.deallocate((SharedFrameHeader *)(pCreatedBuffer)-1);
        // shared_memory.deallocate(pCreatedBuffer);
        // cout << "Freed buffer " << bufferContext << ", " << pCreatedBuffer << endl;
    }
    virtual void DestroyBufferFactory()
    {
        delete this;
    }

protected:
    boost::interprocess::managed_shared_memory shared_memory;
    unsigned long m_lastBufferContext;
};