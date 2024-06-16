#include "volga.h"
#include <iostream>
#include <bitset>
#include <stdio.h>

using namespace paulqpro::volga;

string volga::helloWorld() {
    return "Hello C++";
}

bool volga::loadRom(ubyte_vm rom[], int rom_len){// false - success
    _prog_count = _rom_start;
    int i = 0;
    while(_prog_count < 0xFFFF && i < rom_len){
        _mem.setMemory(_prog_count, rom[i]);
        _prog_count++;
        i++;
    }
    if (i <= rom_len - 1) return false;
    return true;
}
bool volga::loadRom(string rom_file){// false - success
    FILE* f = fopen(rom_file.c_str(), "r");
    int fsize = 0;
    while(true){
        int b = fgetc(f);
        if(b == EOF) break;
        fsize++;
    }
    fclose(f);
    f = fopen(rom_file.c_str(), "r");
    ubyte_vm rom[fsize];
    for(int i = 0; i < fsize; i++){
        rom[i] = fgetc(f);
    }
    fclose(f);
    return loadRom(rom,fsize);
}

void volga::loadA(ubyte_vm value){
    _a_reg = value;
}
void volga::loadA(addr_vm address){
    _addr_buffer = _prog_count;
    _prog_count = address;
    read();
    loadA(_mem_buffer);
}
void volga::storeA(addr_vm address){
    _addr_buffer = address;
    _mem_buffer = _a_reg;
    write();
}

void volga::loadB(ubyte_vm value){
    _b_reg = value;
}
void volga::loadB(addr_vm address){
    _addr_buffer = _prog_count;
    _prog_count = address;
    read();
    loadB(_mem_buffer);
}
void volga::storeB(addr_vm address){
    _addr_buffer = address;
    _mem_buffer = _b_reg;
    write();
}

void volga::loadX(ubyte_vm value){
    _x_reg = value;
}
void volga::loadX(addr_vm address){
    _addr_buffer = _prog_count;
    _prog_count = address;
    read();
    loadX(_mem_buffer);
}
void volga::storeX(addr_vm address){
    _addr_buffer = address;
    _mem_buffer = _x_reg;
    write();
}

void volga::loadY(ubyte_vm value){
    _y_reg = value;
}
void volga::loadY(addr_vm address){
    _addr_buffer = _prog_count;
    _prog_count = address;
    read();
    loadY(_mem_buffer);
}
void volga::storeY(addr_vm address){
    _addr_buffer = address;
    _mem_buffer = _y_reg;
    write();
}

void volga::read(){ //reads byte from memory, indexed by address buffer to memory buffer
    _mem_buffer = _mem.getMemory(_addr_buffer);
}
void volga::readPC(){ //reads byte from memory, indexed by program counter to memory buffer
    _mem_buffer = _mem.getMemory(_prog_count);
    _prog_count++;
}
void volga::readAddr(){ //reads two bytes from memory, indexed by address buffer to address buffer
    read();
    _stack_buffer = _mem_buffer;//just need a buffer :)
    read();
    _addr_buffer = (_stack_buffer << 8) + _mem_buffer;
}
void volga::readAddrPC(){ //reads twobytes from memory, indexed by program counter to address buffer
    readPC();
    _stack_buffer = _mem_buffer;
    readPC();
    _addr_buffer = (_stack_buffer << 8) + _mem_buffer;
}
void volga::readProgCountPC(){ //reads twobytes from memory, indexed by program counter to program counter
    readPC();
    _stack_buffer = _mem_buffer;
    readPC();
    _prog_count = (_stack_buffer << 8) + _mem_buffer;
}
void volga::write(){ //writes content of memory buffer to memory, indexed by address buffer
    if(_addr_buffer >= _rom_start && _addr_buffer <= _rom_end) _mem.setMemory(_addr_buffer, _mem_buffer);
}
void volga::writeAddr(){ //writes address (memory buffer - upper byte, stack buffer - lower byte), indexed by address buffer
    write();
    _addr_buffer++;
    _mem_buffer = _stack_buffer;
    write();
}

void volga::pull(){
    _stack_ptr = (_stack_ptr - 1 & 0xff);
    _stack_buffer = _mem.getMemory(_stack_start + _stack_ptr);
}
void volga::push(){
    _mem.setMemory(_stack_start + _stack_ptr, _stack_buffer);
    _stack_ptr = (_stack_ptr + 1) & 0xff;
}

ubyte_vm volga::ror(ubyte_vm original){
    _mem_buffer = _proc_flags;//just need a buffer :)
    _proc_flags = (_proc_flags & 0b11110111) | (0b00001000 * (original & 0b00000001));
    return (original >> 1) | (0b10000000 * ((_stack_buffer >> 3) & 0b00000001));
}
ubyte_vm volga::rol(ubyte_vm original){
    _mem_buffer = _proc_flags;//just need a buffer :)
    _proc_flags = (_proc_flags & 0b11110111) | (0b00001000 * ((original & 0b10000000) >> 7));
    return (original << 1) | ((_stack_buffer >> 3) & 0b00000001);
}

void volga::bufferAddr(bool mem_buff_first){ //Writes address buffer to memory and stack buffers. order of bytes is determined by bool parametr
    if(mem_buff_first){
        _stack_buffer = (ubyte_vm)_addr_buffer & 0xFF;
        _mem_buffer = (ubyte_vm)((_addr_buffer & 0xFF00) >> 8);
    }
    else{
        _mem_buffer = (ubyte_vm)_addr_buffer & 0xFF;
        _stack_buffer = (ubyte_vm)((_addr_buffer & 0xFF00) >> 8);
    }
}
void volga::bufferProgC(bool mem_buff_first){ //Writes program counter to memory and stack buffers. order of bytes is determined by bool parametr
    if(mem_buff_first){
        _stack_buffer = (ubyte_vm)_prog_count & 0xFF;
        _mem_buffer = (ubyte_vm)((_prog_count & 0xFF00) >> 8);
    }
    else{
        _mem_buffer = (ubyte_vm)_prog_count & 0xFF;
        _stack_buffer = (ubyte_vm)((_prog_count & 0xFF00) >> 8);
    }
}

void volga::jsrPrep(bool is_direct){
    if(is_direct){
        _prog_count+=2;
        bufferProgC(false);
        _prog_count-=2;
    } else {
        _prog_count++;
        bufferProgC(false);
        _prog_count--;
    }
    push();
    _stack_buffer = _mem_buffer;
    push();
    _stack_buffer = _proc_flags;
    push();
}

int volga::run(){
    _prog_count = _rom_start;
    bool running = true;
    while (running)
    {
        readPC();
        switch (_mem_buffer)
        {
        case 0x00: {// BRK/HTL
            return 0;}
        case 0x01: {// BRK a/HLT a
            readAddrPC();
            read();
            return _b_reg;}
        case 0x02: {// BRK A/HLT a
            return _a_reg;}
        case 0x0F: {// CMP
            int cmp = _a_reg - _b_reg;
            _proc_flags= 0b00000000;
            if(cmp == 0)        _proc_flags = _proc_flags | 0b00100000;
            if(cmp < 0)         _proc_flags = _proc_flags | 0b00010000;
            if(abs(cmp) > 0xFF) _proc_flags = _proc_flags | 0b01000000;
            break;}
        case 0x10: {// SAB0
            _a_reg = _a_reg | 0b00000001;
            break;}
        case 0x11: {// CAB0
            _a_reg = _a_reg & 0b11111110;
            break;}
        case 0x12: {// SAB1
            _a_reg = _a_reg | 0b00000010;
            break;}
        case 0x13: {// CAB1
            _a_reg = _a_reg & 0b11111101;
            break;}
        case 0x14: {// SAB2
            _a_reg = _a_reg | 0b00000100;
            break;}
        case 0x15: {// CAB2
            _a_reg = _a_reg & 0b11111011;
            break;}
        case 0x16: {// SAB3
            _a_reg = _a_reg | 0b00001000;
            break;}
        case 0x17: {// CAB3
            _a_reg = _a_reg & 0b11110111;
            break;}
        case 0x18: {// SAB4
            _a_reg = _a_reg | 0b00010000;
            break;}
        case 0x19: {// CAB4
            _a_reg = _a_reg & 0b11101111;
            break;}
        case 0x1A: {// SAB5
            _a_reg = _a_reg | 0b00100000;
            break;}
        case 0x1B: {// CAB5
            _a_reg = _a_reg & 0b11011111;
            break;}
        case 0x1C: {// SAB6
            _a_reg = _a_reg | 0b01000000;
            break;}
        case 0x1D: {// CAB6
            _a_reg = _a_reg & 0b10111111;
            break;}
        case 0x1E: {// SAB7
            _a_reg = _a_reg | 0b10000000;
            break;}
        case 0x1F: {// CAB7
            _a_reg = _a_reg & 0b01111111;
            break;}
        case 0x20: {// SCF
            _proc_flags = _proc_flags | 0b10000000;
            break;}
        case 0x21: {// CCF
            _proc_flags = _proc_flags & 0b01111111;
            break;}
        case 0x22: {// SOF
            _proc_flags = _proc_flags | 0b01000000;
            break;}
        case 0x23: {// COF
            _proc_flags = _proc_flags & 0b10111111;
            break;}
        case 0x24: {// SZF
            _proc_flags = _proc_flags | 0b00100000;
            break;}
        case 0x25: {// CZF
            _proc_flags = _proc_flags & 0b11011111;
            break;}
        case 0x26: {// SNF
            _proc_flags = _proc_flags | 0b00010000;
            break;}
        case 0x27: {// CNF
            _proc_flags = _proc_flags & 0b11101111;
            break;}
        case 0x28: {// RRA
            _a_reg = ror(_a_reg);
            break;}
        case 0x29: {// RLA
            _a_reg = rol(_a_reg);
            break;}
        case 0x2A: {// RRX
            _x_reg = ror(_x_reg);
            break;}
        case 0x2B: {// RLX
            _x_reg = rol(_x_reg);
            break;}
        case 0x2C: {// RRY
            _y_reg = ror(_y_reg);
            break;}
        case 0x2D: {// RLY
            _y_reg = rol(_y_reg);
            break;}
        case 0x2E: {// RRB
            _b_reg = ror(_b_reg);
            break;}
        case 0x2F: {// RLB
            _b_reg = rol(_b_reg);
            break;}
        case 0x30: {// BEQ a
            readAddrPC();
            if(_a_reg == _b_reg){
                readAddr();
                _prog_count = _addr_buffer;
            }
            break;}
        case 0x31: {// BEQ #
            readPC();
            if(_a_reg == _b_reg){
                _prog_count += (signed char)_mem_buffer;
            }
            break;}
        case 0x32: {// BNQ a
            readAddrPC();
            if(_a_reg != _b_reg){
                readAddr();
                _prog_count = _addr_buffer;
            }
            break;}
        case 0x33: {// BNQ #
            readPC();
            if(_a_reg != _b_reg){
                _prog_count += (signed char)_mem_buffer;
            }
            break;}
        case 0x40: {// BCS a
            readAddrPC();
            if(((_proc_flags & 0b10000000) >> 7) == 1){
                readAddr();
                _prog_count = _addr_buffer;
            }
            break;}
        case 0x41: {// BCS #
            readPC();
            if(((_proc_flags & 0b10000000) >> 7) == 1){
                _prog_count += (signed char)_mem_buffer;
            }
            break;}     
        case 0x42: {// BOS a
            readAddrPC();
            if(((_proc_flags & 0b01000000) >> 6) == 1){
                readAddr();
                _prog_count = _addr_buffer;
            }
            break;}
        case 0x43: {// BOS #
            readPC();
            if(((_proc_flags & 0b01000000) >> 6) == 1){
                _prog_count += (signed char)_mem_buffer;
            }
            break;}
        case 0x44: {// BZS a
            readAddrPC();
            if(((_proc_flags & 0b00100000) >> 5) == 1){
                readAddr();
                _prog_count = _addr_buffer;
            }
            break;}
        case 0x45: {// BZS #
            readPC();
            if(((_proc_flags & 0b00100000) >> 5) == 1){
                _prog_count += (signed char)_mem_buffer;
            }
            break;}
        case 0x46: {// BNS a
            readAddrPC();
            if(((_proc_flags & 0b00010000) >> 4) == 1){
                readAddr();
                _prog_count = _addr_buffer;
            }
            break;}
        case 0x47: {// BNS #
            readPC();
            if(((_proc_flags & 0b00010000) >> 4) == 1){
                _prog_count += (signed char)_mem_buffer;
            }
            break;}
        case 0x48: {// BCC a
            readAddrPC();
            if(((_proc_flags & 0b10000000) >> 7) == 0){
                readAddr();
                _prog_count = _addr_buffer;
            }
            break;}
        case 0x49: {// BCC #
            readPC();
            if(((_proc_flags & 0b10000000) >> 7) == 0){
                _prog_count += (signed char)_mem_buffer;
            }
            break;}
        case 0x4A: {// BOC a
            readAddrPC();
            if(((_proc_flags & 0b01000000) >> 6) == 0){
                readAddr();
                _prog_count = _addr_buffer;
            }
            break;}
        case 0x4B: {// BOC #
            readPC();
            if(((_proc_flags & 0b01000000) >> 6) == 0){
                _prog_count += (signed char)_mem_buffer;
            }
            break;}
        case 0x4C: {// BZC a
            readAddrPC();
            if(((_proc_flags & 0b00100000) >> 5) == 0){
                readAddr();
                _prog_count = _addr_buffer;
            }
            break;}
        case 0x4D: {// BZC #
            readPC();
            if(((_proc_flags & 0b00100000) >> 5) == 0){
                _prog_count += (signed char)_mem_buffer;
            }
            break;}
        case 0x4E: {// BNC a
            readAddrPC();
            if(((_proc_flags & 0b00010000) >> 4) == 0){
                readAddr();
                _prog_count = _addr_buffer;
            }
            break;}
        case 0x4F: {// BNC #
            readPC();
            if(((_proc_flags & 0b00010000) >> 4) == 0){
                _prog_count += (signed char)_mem_buffer;
            }
            break;}
        case 0x50: {// BBS0 a
            readAddrPC();
            if((_a_reg & 0b00000001) == 1){
                readAddr();
                _prog_count = _addr_buffer;
            }
            break;}
        case 0x51: {// BBS0 #
            readPC();
            if((_a_reg & 0b00000001) == 1){
                _prog_count += (signed char)_mem_buffer;
            }
            break;}
        case 0x52: {// BBS1 a
            readAddrPC();
            if(((_a_reg & 0b00000010) >> 1) == 1){
                readAddr();
                _prog_count = _addr_buffer;
            }
            break;}
        case 0x53: {// BBS1 #
            readPC();
            if(((_a_reg & 0b00000010) >> 1) == 1){
                _prog_count += (signed char)_mem_buffer;
            }
            break;}
        case 0x54: {// BBS2 a
            readAddrPC();
            if(((_a_reg & 0b00000100) >> 2) == 1){
                readAddr();
                _prog_count = _addr_buffer;
            }
            break;}
        case 0x55: {// BBS2 #
            readPC();
            if(((_a_reg & 0b00000100) >> 2) == 1){
                _prog_count += (signed char)_mem_buffer;
            }
            break;}
        case 0x56: {// BBS3 a
            readAddrPC();
            if(((_a_reg & 0b00001000) >> 3) == 1){
                readAddr();
                _prog_count = _addr_buffer;
            }
            break;}
        case 0x57: {// BBS3 #
            readPC();
            if(((_a_reg & 0b00001000) >> 3) == 1){
                _prog_count += (signed char)_mem_buffer;
            }
            break;}
        case 0x58: {// BBS4 a
            readAddrPC();
            if(((_a_reg & 0b00010000) >> 4) == 1){
                readAddr();
                _prog_count = _addr_buffer;
            }
            break;}
        case 0x59: {// BBS4 #
            readPC();
            if(((_a_reg & 0b00010000) >> 4) == 1){
                _prog_count += (signed char)_mem_buffer;
            }
            break;}
        case 0x5A: {// BBS5 a
            readAddrPC();
            if(((_a_reg & 0b00100000) >> 5) == 1){
                readAddr();
                _prog_count = _addr_buffer;
            }
            break;}
        case 0x5B: {// BBS5 #
            readPC();
            if(((_a_reg & 0b00100000) >> 5) == 1){
                _prog_count += (signed char)_mem_buffer;
            }
            break;}
        case 0x5C: {// BBS6 a
            readAddrPC();
            if(((_a_reg & 0b01000000) >> 6) == 1){
                readAddr();
                _prog_count = _addr_buffer;
            }
            break;}
        case 0x5D: {// BBS6 #
            readPC();
            if(((_a_reg & 0b01000000) >> 6) == 1){
                _prog_count += (signed char)_mem_buffer;
            }
            break;}
        case 0x5E: {// BBS7 a
            readAddrPC();
            if(((_a_reg & 0b10000000) >> 7) == 1){
                readAddr();
                _prog_count = _addr_buffer;
            }
            break;}
        case 0x5F: {// BBS7 #
            readPC();
            if(((_a_reg & 0b10000000) >> 7) == 1){
                _prog_count += (signed char)_mem_buffer;
            }
            break;}
        case 0x60: {// BBC0 a
            readAddrPC();
            if((_a_reg & 0b00000001) == 0){
                readAddr();
                _prog_count = _addr_buffer;
            }
            break;}
        case 0x61: {// BBC0 #
            readPC();
            if((_a_reg & 0b00000001) == 0){
                _prog_count += (signed char)_mem_buffer;
            }
            break;}
        case 0x62: {// BBC1 a
            readAddrPC();
            if(((_a_reg & 0b00000010) >> 1) == 0){
                readAddr();
                _prog_count = _addr_buffer;
            }
            break;}
        case 0x63: {// BBC1 #
            readPC();
            if(((_a_reg & 0b00000010) >> 1) == 0){
                _prog_count += (signed char)_mem_buffer;
            }
            break;}
        case 0x64: {// BBC2 a
            readAddrPC();
            if(((_a_reg & 0b00000100) >> 2) == 0){
                readAddr();
                _prog_count = _addr_buffer;
            }
            break;}
        case 0x65: {// BBC2 #
            readPC();
            if(((_a_reg & 0b00000100) >> 2) == 0){
                _prog_count += (signed char)_mem_buffer;
            }
            break;}
        case 0x66: {// BBC3 a
            readAddrPC();
            if(((_a_reg & 0b00001000) >> 3) == 0){
                readAddr();
                _prog_count = _addr_buffer;
            }
            break;}
        case 0x67: {// BBC3 #
            readPC();
            if(((_a_reg & 0b00001000) >> 3) == 0){
                _prog_count += (signed char)_mem_buffer;
            }
            break;}
        case 0x68: {// BBC4 a
            readAddrPC();
            if(((_a_reg & 0b00010000) >> 4) == 0){
                readAddr();
                _prog_count = _addr_buffer;
            }
            break;}
        case 0x69: {// BBC4 #
            readPC();
            if(((_a_reg & 0b00010000) >> 4) == 0){
                _prog_count += (signed char)_mem_buffer;
            }
            break;}
        case 0x6A: {// BBC5 a
            readAddrPC();
            if(((_a_reg & 0b00100000) >> 5) == 0){
                readAddr();
                _prog_count = _addr_buffer;
            }
            break;}
        case 0x6B: {// BBC5 #
            readPC();
            if(((_a_reg & 0b00100000) >> 5) == 0){
                _prog_count += (signed char)_mem_buffer;
            }
            break;}
        case 0x6C: {// BBC6 a
            readAddrPC();
            if(((_a_reg & 0b01000000) >> 6) == 0){
                readAddr();
                _prog_count = _addr_buffer;
            }
            break;}
        case 0x6D: {// BBC6 #
            readPC();
            if(((_a_reg & 0b01000000) >> 6) == 0){
                _prog_count += (signed char)_mem_buffer;
            }
            break;}
        case 0x6E: {// BBC7 a
            readAddrPC();
            if(((_a_reg & 0b10000000) >> 7) == 0){
                readAddr();
                _prog_count = _addr_buffer;
            }
            break;}
        case 0x6F: {// BBC7 #
            readPC();
            if(((_a_reg & 0b10000000) >> 7) == 0){
                _prog_count += (signed char)_mem_buffer;
            }
            break;}
        case 0x70: {// RTS
            pull();
            _proc_flags = _stack_buffer;
            pull();
            _mem_buffer = _stack_buffer;
            pull();
            _prog_count = ((addr_vm)_stack_buffer << 8) + _mem_buffer;
            break;}
        case 0x71: {// JSR a
            jsrPrep(true);
            readProgCountPC();
            break;}
        case 0x72: {// JSR B
            jsrPrep(false);
            _prog_count = _addr_buffer;
            break;}
        case 0x73: {// JSR a,X
            jsrPrep(false);
            readPC();
            _prog_count = ((addr_vm)_mem_buffer << 8) + _x_reg;
            break;}
        case 0x74: {// JSR X,a
            jsrPrep(false);
            readPC();
            _prog_count = ((addr_vm)_x_reg << 8) + _mem_buffer;
            break;}
        case 0x75: {// JSR a,Y
            jsrPrep(false);
            readPC();
            _prog_count = ((addr_vm)_mem_buffer << 8) + _y_reg;
            break;}
        case 0x76: {// JSR Y,a
            jsrPrep(false);
            readPC();
            _prog_count = ((addr_vm)_y_reg << 8) + _mem_buffer;
            break;}
        case 0x77: {// JSR X,Y
            jsrPrep(false);
            readPC();
            _prog_count = ((addr_vm)_x_reg << 8) + _y_reg;
            break;}
        case 0x79: {// JMP a
            readProgCountPC();
            break;}
        case 0x7A: {// JMP B
            _prog_count = _addr_buffer;
            break;}
        case 0x7B: {// JMP a,X
            readPC();
            _prog_count = ((addr_vm)_mem_buffer << 8) + _x_reg;
            break;}
        case 0x7C: {// JMP X,a
            readPC();
            _prog_count = ((addr_vm)_x_reg << 8) + _mem_buffer;
            break;}
        case 0x7D: {// JMP a,Y
            readPC();
            _prog_count = ((addr_vm)_mem_buffer << 8) + _y_reg;
            break;}
        case 0x7E: {// JMP Y,a
            readPC();
            _prog_count = ((addr_vm)_y_reg << 8) + _mem_buffer;
            break;}
        case 0x7F: {// JMP X,Y
            _prog_count = ((addr_vm)_x_reg << 8) + _y_reg;
            break;}
        case 0x80: // ADD #
            readPC();
            _proc_flags = _proc_flags & 0b00001111;
            if(_a_reg + _mem_buffer > 255) _proc_flags = _proc_flags | 0b10000000;
            _a_reg += _mem_buffer;
            if(_a_reg == 0) _proc_flags = _proc_flags |= 0b00100000;
            break;
        case 0x81: // ADD a
            readAddrPC();
            readAddr();
            _proc_flags = _proc_flags & 0b00001111;
            if(_a_reg + _mem_buffer > 255) _proc_flags = _proc_flags | 0b10000000;
            _a_reg += _mem_buffer;
            if(_a_reg == 0) _proc_flags = _proc_flags | 0b00100000;
            break;
        case 0x82L: // ADC #
            break;
        case 0xA8: {// LDA #
            readPC();
            _a_reg = _mem_buffer;
            break;}
        // case 0xE1: {// SAB a
        //     bufferAddr(true);
        //     readAddrPC();
        //     writeAddr();
        //     break;}
        // case 0xE2: {// SAB a,X
        //     bufferAddr(true);
        //     readAddrPC();
        //     _prog_count--;
        //     _addr_buffer = (_addr_buffer & 0xFF00) + _x_reg;
        //     writeAddr();
        //     break;}
        // case 0xE3: {// SAB X,a
        //     bufferAddr(true);
        //     readAddrPC();
        //     _prog_count--;
        //     _addr_buffer = ((_addr_buffer & 0xFF00) >> 8) + ((addr_vm)_x_reg << 8);
        //     writeAddr();
        //     break;}
        // case 0xE4: {// SAB a,Y
        //     bufferAddr(true);
        //     readAddrPC();
        //     _prog_count--;
        //     _addr_buffer = (_addr_buffer & 0xFF00) + _y_reg;
        //     writeAddr();
        //     break;}
        // case 0xE5: {// SAB Y,a
        //     bufferAddr(true);
        //     readAddrPC();
        //     _prog_count--;
        //     _addr_buffer = ((_addr_buffer & 0xFF00) >> 8) + ((addr_vm)_y_reg << 8);
        //     writeAddr();
        //     break;}
        // case 0xE6: {// SAB X,Y
        //     bufferAddr(true);
        //     _addr_buffer = (_x_reg << 8) + _y_reg;
        //     writeAddr();
        //     break;}
        // case 0xE8: // LAB ##
        case 0xff: //for debug
            cout<<bitset<8>(_a_reg)<<endl;
            break;
        default: {// NOP
            break;}
        }
    }
    return 0;
}

volga::volga() {
    _mem = memory();
    _ram_start = 0x0000;
    _ram_end = 0x7FFF;
    _stack_start = 0x8000;
    _stack_end = 0x80FF;
    _cout = 0x8100;
    _cin = 0x8101;
    _cnl = 0x8102;
    _rom_start = 0x9000;
    _rom_end = 0xFFFF;
    _prog_count = 0x0000;
    _proc_flags = 0b00000000; //COZNR000
    _a_reg = 0x00;
    _x_reg = 0x00;
    _y_reg = 0x00;
    _b_reg = 0x00;
    _mem_buffer = 0x00;
    _stack_buffer = 0x00;
    _stack_ptr = 0x00;
}
volga::~volga(){
    _mem.~memory();
}
