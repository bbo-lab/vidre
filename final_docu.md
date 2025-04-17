## Assumed Architecture
Due to the expected modular nature of the system, the elements are broken down into modules that can be linked to process data in a stream-like fashion. Each module is assumed to run independently, e.g. in its own thread (or potentially threads). It can be started and stopped and it continuously acquires data either from external sources (like camera devices) or internal sources (like frames from camera modules). It processes the data packets according to the module's description and makes it available to other modules or external sinks.

A module can be a producer or consumer of data packets or both. What that essentially necessitates is a buffer the producer part can write to and the consumer part can read from.

## Classes
### ICamera
An abstract interface for concrete camera SDK implementations by e.g. different vendors like Basler (PylonSDK). This allows for a unified access to camera features such as starting, stopping and parameter controls by other parts of the system.

The parameters are expected to be accessed and modified according to the Standard Feature Naming Convention (SFNC). It does not necessarily have to be directly implemented by the SDK
but the concrete wrapper should adapt and make sure that such parameter accesses are "translated" to the SDKs way handling of parameter access.

Since it is also kind of a producer of data it should eventually implement an abstraction of that, that all modules implement.

### PylonCamera
A concrete implementation of the ICamera interface for cameras using PylonSDK (such as Basler cameras). 

### Future concrete camera classes

## Expected Behaviours
### Camera Acquisition