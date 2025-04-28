# Problem
Given the actual virtual and hardware topology some performance considerations have to be taken
into account.

# Glossary
- Node: A pc in the network
- Module: A processing unit that creates or modifies data

# System setup timeline
- Connecting hardware manually so that it equates the wanted hardware topology
- Start daemons on nodes that listen to setup commands over networks
- Using one node to load the system configuration (virtual topology)
- Loading hardware topology hints
- Combine both to identify an optimal setup
- Send setup commands to the nodes according to the combined configuration
- The nodes use those commands to set up their local virtual topology

# Idea
Assume each module has its own thread, processing input and providing output and suitable constructs to provide a "piping" behaviour. Taking into account the virtual topology (e.g. recognizing that there is just one camera and one storage) and hints about the hardware topology (e.g. there exists an externally triggered group of cameras) the loaded system configuration should be transformed into something optimal automatically.

In the latter case for example the simultaneous triggering of cameras causes a burst of frames from different cameras to be processed. If every camera has its own thread and is connected to one node, the context switching and cache misses might cause a far worse system performance. In that case it might be better to have just a pool of a few threads dividing the grabbing and annotation task between themselves.

We need some kind of unified wrapper.

Maybe there is a need to rework the camera. I am not quite sure if the event system from Pylon SDK is hardware or software based, i.e. are the events fired by the device and through usb or does the Pylon runtime listen to it. If it
is hardware based it would make more sense to use Pylon's OnEvent. If not the gain is probably negligible and its cleaner for us to do the listening.

Should the shared memory part be explicit? I.e. have a shared memory module.

Using other libraries for handling piping behaviour?
- Gstreamer: A big known library with big capabilities. Does include pylon, genicam, aptina, etc. sources. Might be too big?
- RaftLib: More basic and less dependencies. Needs implementation of our use-cases

Generally: Splitting configuration into software and hardware topology.
-> Software Topology: Revolves around the piping around the system, what modules to connect how
-> Hardware Topolgy: Hints towards the actual connections, nodes of modules, expected connection bottlenecks etc.
    Also this configuration file might include a sort of config of a domain name service so that e.g. a named device may be replaced and does not have its serial number replaced in the entire configurations
=> Combine both to create suitable relationships between modules. (e.g. choosing sequential over multi-threading)

# How to do Producer/Consumer
We have a sort of lock-free queue that is filled by the camera without regards to how consumers use it. 

What should take precedence? Camera frame grabs or frame storage. I.e. should frames be dropped by the camera thread or the following (e.g. Storage) threads.

https://github.com/rigtorp/MPMCQueue

https://moodycamel.com/blog/2014/a-fast-general-purpose-lock-free-queue-for-c++

https://github.com/tudinfse/FFQ

Weak valid check: At the start of handling a frame copy the frame id to local memory. During writing occasionally check


# Coding Style

# Modules
- Camera
    - Camera Group? (For sharing threads)
- Storage
- Presenter
- Transport
    - Source (also goes for Camera?)
    - Sink (also goes for Storage?)
- Processing
    - Transform
    - Splitter
- Logging
- Custom Modules?




# Handle Warnings
- Warnings about frame droppings
- Noted ccv files
- Check if failed trigger
- Change Interface to storage buffer


# Start-Up
There is a certain dependency graph. Through topological sorting we can determine a good start-up order. However cycles might be included.