#pragma once

#include <string>
#include "../misc.h"
#include "memory/memory.h"

using namespace std;

namespace paulqpro::volga{
    class volga
    {
    private:
        addr_vm _ram_start, _rom_start, _prog_count;
        byte_vm _proc_flags, _accum, _x_reg, _y_reg;
        memory _mem;
    public:
        bool loadRom(byte_vm rom[], int rom_len);
        bool loadRom(string rom_file);
        string helloWorld();
        volga();
        ~volga();
    };
}