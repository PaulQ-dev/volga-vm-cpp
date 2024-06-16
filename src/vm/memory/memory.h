#pragma once

#include <algorithm>
#include "../../misc.h"

namespace paulqpro::volga{
    class memory
    {
    private:
        ubyte_vm* _mem;
    public:
        ubyte_vm getMemory(addr_vm address);
        void setMemory(addr_vm address, ubyte_vm value);
        memory();
        ~memory();
    };
}