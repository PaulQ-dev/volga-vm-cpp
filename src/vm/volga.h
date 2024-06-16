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
                _stack_start, _stack_end,
                _cout, _cin, _cnl, 
                _prog_count, _addr_buffer;
        ubyte_vm _proc_flags, _stack_ptr,
                _a_reg, _b_reg, _x_reg, _y_reg,
                _mem_buffer, _stack_buffer;
        memory _mem;
        void loadA(ubyte_vm value), loadA(addr_vm address), storeA(addr_vm address);
        void loadB(ubyte_vm value), loadB(addr_vm address), storeB(addr_vm address);
        void loadX(ubyte_vm value), loadX(addr_vm address), storeX(addr_vm address);
        void loadY(ubyte_vm value), loadY(addr_vm address), storeY(addr_vm address);
        void read(), readPC(), readAddr(), readAddrPC(), readProgCountPC(), write(), writeAddr();
        void push(), pull();
        ubyte_vm ror(ubyte_vm original), rol(ubyte_vm original);
        void bufferAddr(bool mem_buff_first), bufferProgC(bool mem_buff_first);
        void jsrPrep(bool is_direct);
    public:
        int run();
        bool loadRom(ubyte_vm rom[], int rom_len);
        bool loadRom(string rom_file);
        string helloWorld();
        volga();
        ~volga();
    };
}