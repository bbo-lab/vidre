# Memory Structure
### Shared Image

| --Header-- | ----- Data ----- |


- Header: Per-frame variable data like frame number or time stamp.
- Data: The actual data. The layout is predetermined beforehand and fixed for a arm/disarm session.

The camera does not own the memory. Instead it just writes the frame header + data.