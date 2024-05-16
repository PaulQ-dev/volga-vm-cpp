#pragma once

#include <string>
#include "../misc.h"
#include "memory/memory.h"

using namespace std;

namespace paulqpro::volga{
    class volga
    {
    private:
        addr_vm _ram_start, _ram_end,
                _rom_start, _rom_end,
                _prog_count, _addr_buffer;
        byte_vm _proc_flags, _accum, _x_reg, _y_reg, _buffer;
        memory _mem;
        void loadA(byte_vm value), loadA(addr_vm address), storeA(addr_vm address);
        void loadX(byte_vm value), loadX(addr_vm address), storeX(addr_vm address);
        void loadY(byte_vm value), loadY(addr_vm address), storeY(addr_vm address);
        void read(), readAddr(), write();
    public:
        int run();
        bool loadRom(byte_vm rom[], int rom_len);
        bool loadRom(string rom_file);
        string helloWorld();
        volga();
        ~volga();
    };
}