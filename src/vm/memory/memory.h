#pragma once

#include <algorithm>
#include "../../misc.h"

namespace paulqpro::volga{
    class memory
    {
    private:
        byte_vm* _mem;
    public:
        byte_vm getMemory(addr_vm address);
        void setMemory(addr_vm address, byte_vm value);
        memory();
        ~memory();
    };
}