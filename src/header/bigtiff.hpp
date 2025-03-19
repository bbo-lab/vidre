#pragma once

#include "config.h"

#include "vidre/header.hpp"


namespace vidre {

    class CamCommandoHeader : public IHeader {
    public:
    private:
        bool byte_order;
        uint32_t version;
        uint32_t offset;
    }

}