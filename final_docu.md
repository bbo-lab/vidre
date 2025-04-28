## Assumed Architecture
Due to the expected modular nature of the system, the elements are broken down into modules that can be linked to process data in a stream-like fashion. Each module is assumed to run independently, e.g. in its own thread (or potentially threads). Each can be started and stopped and it continuously acquires data either from external sources (like camera devices) or internal sources (like frames coming from camera modules). It processes the data packets according to the module's description and makes it available to other modules or external sinks.

A module can be a producer or consumer of data packets or both. What that essentially necessitates is a buffer the producer part of one module can write to and the consumer part of another module can read from.

The main packet of information is image frame data with an additional header beforehand describing important meta data like the frame id or timestamp. Those are passed through the "pipeline" of processing modules.

## Classes
### ICamera
An abstract interface for concrete camera SDK implementations by e.g. different vendors like Basler (PylonSDK). This allows for a unified access to camera features such as starting, stopping and parameter controls by other parts of the system.

The parameters are expected to be accessed and modified according to the Standard Feature Naming Convention (SFNC). It does not necessarily have to be directly implemented by the SDK
but the concrete wrapper should adapt and make sure that such parameter accesses are "translated" to the SDKs way handling of parameter access.

Since it is also kind of a producer of data it should eventually implement an abstraction of that, that all modules implement.

### PylonCamera
A concrete implementation of the ICamera interface for cameras using PylonSDK (such as Basler cameras). 

The PylonSDK already offers a way to save and load configuration files for the cameras. The file path just needs to be passed on.

### DummyCamera
A sort of default implementation of ICamera for testing purposes. Especially useful when wanting to test out configuration loading behaviour but real cameras arent available on the PC. 

It is important to mention that this class in no way has meaningful data and behaves rather undefined when accessing parameters and/or accessing frames.

### Future concrete camera classes
To the outside they should behave similar to how PylonCamera behaves, especially according to the SFNC. PylonCamera is essentially the "example" implementation of an ICamera class.

### ConfigLoader
Not a class per se but contains a function that should be able to translate a configuration file into a working system. 

Preferably it should be able to load one file representing the desired software configuration and one file representing the actual hardware aspects and connections. That way multiple configurations can be created that rely on the same hardware setup. The hardware configuration gives the concrete devices unique names that can be used in the software configuration. Replacing a camera then just requires a change of the device address or serial number or similar.

## Expected Behaviours
### Loading Configuration Files
A configuration file is a YAML file that describes the "software topology", i.e. what the used modules are, how they themselves are configured and most importantly connected. Since the configurations are different for cameras vendors or more generally different module types, the classes should have a similar load configuration function that constructs an object of that type.
### Camera Acquisition
The relevant triggering setups are software and hardware triggered. Software triggered means that the camera frame grab is signalled by the computer sending a software signal over USB (or similar). Hardware triggered means that the camera frame start is triggered by a hardware line. It in turn can be sent a signal by an external device or a master camera.
### Memory
The camera modules should not own the memory they write frames to. It should be owned by an outside object that can also handle access to from other modules. Similar to how a lot of cameras and Pylon behaves, the frames are constantly grabbed in a circular buffer fashion. A certain amount of space for frames is available and oldest frames are replaced by new ones, without particular consideration for other modules' access. This should ensure that the grabbing of frames is as fast as possible.

