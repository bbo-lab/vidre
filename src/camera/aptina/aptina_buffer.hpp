#pragma once

#include <string>

#include <boost/interprocess/managed_shared_memory.hpp>

#include <core/runtime/shared_frame_header.hpp>
#include <iostream>
#include <new>

namespace vidre {
    class AptinaBuffer {
    public:
        AptinaBuffer() : m_lastBufferContext(-1) {}

        ~AptinaBuffer() {}  // todo

        void SetSharedMemory(boost::interprocess::managed_shared_memory *sharedMemory, std::vector<void *> *ptrs,
                             std::string memory_name) {
            shared_memory_ptrs = ptrs;
            memoryName = memory_name;
            shared_memory = sharedMemory;
        }

        void Reset() {
            m_lastBufferContext = -1;
            ptr_counter = 0;
            shared_memory_ptrs = nullptr;
            shared_memory = nullptr;
        }

        void AllocateBuffer(size_t bufferSize) {
            ++m_lastBufferContext;
            try {
            void *allocatedMemory = shared_memory->allocate(bufferSize + sizeof(vidre::SharedFrameHeader));
            new (allocatedMemory) vidre::SharedFrameHeader{};

            auto *buffer_data = static_cast<uint8_t*>(allocatedMemory) + sizeof(vidre::SharedFrameHeader);
            for (size_t i = 0; i < bufferSize; ++i) {
                buffer_data[i] = 0;
            }

            shared_memory_ptrs->at(m_lastBufferContext) = allocatedMemory;
            } catch
                (const boost::interprocess::interprocess_exception& ex) {
                    std::cerr << "Interprocess exception: " << ex.what() << std::endl;
            }
        }

        std::pair<uint8_t*, SharedFrameHeader*> CurrentBuffer(unsigned int buffer_num) {
            auto buffer_idx = ptr_counter % buffer_num;

            auto* buffer_ptr = shared_memory_ptrs->at(buffer_idx);

            ptr_counter++;

            return {static_cast<uint8_t*>(buffer_ptr) + sizeof(vidre::SharedFrameHeader),  static_cast<vidre::SharedFrameHeader*>(buffer_ptr)};
        }

    protected:
        std::string memoryName;
        std::vector<void *> *shared_memory_ptrs;
        boost::interprocess::managed_shared_memory *shared_memory;
        int64_t m_lastBufferContext;
        int64_t ptr_counter = 0;
    };
}  // namespace vidre