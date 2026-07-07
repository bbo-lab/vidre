target_sources(vidre PRIVATE
    transport.cpp
    transport.hpp
)

source_group(core\transport FILES transport.cpp transport.hpp frame_transport.proto)