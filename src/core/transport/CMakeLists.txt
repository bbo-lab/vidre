target_sources(vidre PRIVATE
    transport.cpp
    transport.hpp
    wire_protocol.cpp
    wire_protocol.hpp
    storage_server.cpp
    storage_server.hpp
    camera_backend.cpp
    camera_backend.hpp
    control_server.cpp
    control_server.hpp
    control_client.cpp
    control_client.hpp
    preview_server.cpp
    preview_server.hpp
    preview_client.cpp
    preview_client.hpp
)

target_link_libraries(vidre PUBLIC Boost::headers)

source_group(core\transport FILES transport.cpp transport.hpp frame_transport.proto)
