# Memory Structure
### Shared Image

| --Header-- | ----- Data ----- |


- Header: Per-frame variable data like frame number or time stamp.
- Data: The actual data. The layout is predetermined beforehand and fixed for a arm/disarm session.

The camera does not own the memory. Instead it just writes the frame header + data.

### Outside access idea
Add section to named shared memory where the devices can add their UUID. That way outside processes
can check for available devices and their respective result memory. Maybe even a way to describe the devices or at least determine their type. Then also this allows them to subscribe to the named semaphore.
Example:

# Image Grabbing
In the case of Pylon software, it can use shared memory handed by and outside objects. Pylon`s grabbing acts like a ring buffer already but it is kind of a black box. OnImageGrab events can be handled but there does not seem to be a way to change the behaviour right before Pylon wants to copy the device data to memory.

