#pragma once
#include <pylon/PylonIncludes.h>
#include <boost/interprocess/managed_shared_memory.hpp>

#include "shared_image_header.hpp"

using namespace Pylon;
using namespace std;
using namespace boost::interprocess;

class ShrBufferFactory : public IBufferFactory
{
public:
    ShrBufferFactory()
        : m_lastBufferContext(-1)
    {
    }

    virtual ~ShrBufferFactory()
    {
    }

    void PreallocateBuffers(size_t headerSize, size_t dataSize, size_t N)
    {
        // Eventually will be delegated to an own management object.
        shared_memory_object::remove("Pylon");
        shared_memory = managed_shared_memory(open_or_create, "Pylon", N * (headerSize + dataSize));
    }
    virtual void AllocateBuffer(size_t bufferSize, void **pCreatedBuffer, intptr_t &bufferContext)
    {
        try
        {
            // Allocation should be delegated to a seperate object so that the data can be accessed by other resources.
            *pCreatedBuffer = (void *)((SharedImageHeader *)(shared_memory.allocate(bufferSize + sizeof(SharedImageHeader))) + 1);

            bufferContext = ++m_lastBufferContext;

            cout << "Created buffer " << bufferContext << ", " << *pCreatedBuffer << " of size " << bufferSize << endl;
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
        shared_memory.deallocate((SharedImageHeader *)(pCreatedBuffer)-1);
        // shared_memory.deallocate(pCreatedBuffer);
        // cout << "Freed buffer " << bufferContext << ", " << pCreatedBuffer << endl;
    }
    virtual void DestroyBufferFactory()
    {
        delete this;
    }

protected:
    managed_shared_memory shared_memory;
    unsigned long m_lastBufferContext;
};