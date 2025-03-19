# Problem
Given the actual virtual and hardware topology some performance considerations have to be taken
into account.

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

Should the shared memory part be explicit.

Using other libraries for handling piping behaviour?
- Gstreamer: A big known library with big capabilities. Does include pylon, genicam, aptina, etc. sources. Might be too big?
- RaftLib: More basic and less dependencies. Needs implementation of our use-cases

Generally: Splitting configuration into software and hardware topology.
-> Software Topology: Revolves around the piping around the system, what modules to connect how
-> Hardware Topolgy: Hints towards the actual connections, nodes of modules etc.