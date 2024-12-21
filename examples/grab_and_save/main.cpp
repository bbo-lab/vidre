#include "vidre/storage.h"
#include "vidre/ring_buffer.h"
#include "vidre/runtime.h"

int main(int argc, char** argv)
{
    // later async/parallel staff


    // auto config = vidre::Config(); // create FIXED cam config
    // auto device = vidre::Device(config); // maybe not needed as config is enough
    
    // std::unique_ptr<vidre::RingBuffer> ring_buffer = vidre::RingBuffer(device);

    // while () or  for n frames
    // {
    //     vidre::IData data = vidre::Grab() or Grab(Put ptr from ring_buffer); // grab raw data ptr
    //     auto result = vidre::Save(data, device, config, ring_buffer); // save according to device, config, update ring_buffer
    //     if (!result)
    //     {
    //         return -1;
    //     }
    // }
    int frame_counter = 0;
    auto runtime = std::make_unique<vidre::RuntimeInstance>();
    while (runtime->IsGrabbing())
    {
        frame_counter++;
        runtime->Retrieve();
        vidre::Save(runtime->GetResultConfig(), runtime->GetBuffer(), runtime->GetBufferSize(), frame_counter);
        if (frame_counter == 2)
        {
            break;
        }
    }
    return 0;
}