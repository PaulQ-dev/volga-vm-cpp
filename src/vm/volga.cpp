#include "volga.h"
#include <iostream>
#include <bitset>
#include <stdio.h>

using namespace paulqpro::volga;

string volga::helloWorld() {
    return "Hello C++";
}

bool volga::loadRom(byte_vm rom[], int rom_len){// false - success
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
    byte_vm rom[fsize];
    for(int i = 0; i < fsize; i++){
        rom[i] = fgetc(f);
    }
    fclose(f);
    return loadRom(rom,fsize);
}

void volga::loadA(byte_vm value){
    _accum = value;
}
void volga::loadA(addr_vm address){
    _addr_buffer = _prog_count;
    _prog_count = address;
    read();
    loadA(_mem_buffer);
}
void volga::storeA(addr_vm address){
    _addr_buffer = address;
    _mem_buffer = _accum;
    write();
}

void volga::loadX(byte_vm value){
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

void volga::loadY(byte_vm value){
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

void volga::read(){ //reads byte from memory, indexed by address buffer to opcode buffer
    _mem_buffer = _mem.getMemory(_addr_buffer);
}
void volga::readPC(){ //reads byte from memory, indexed by program counter to opcode buffer
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
    byte_vm addr_buffer = _mem_buffer;
    readPC();
    _addr_buffer = (addr_buffer << 8) + _mem_buffer;
}
void volga::write(){ //writes content of opcode buffer to memory, indexed by address buffer
    if(_addr_buffer >= _rom_start && _addr_buffer <= _rom_end) _mem.setMemory(_addr_buffer, _mem_buffer);
}

void volga::pull(){
    _stack_buffer = _mem.getMemory(_stack_start + _stack_ptr);
    _stack_ptr %= _stack_ptr - 1;
}
void volga::push(){
    _stack_ptr %= _stack_ptr + 1;
    _mem.setMemory(_stack_start + _stack_ptr, _stack_buffer);
}

byte_vm volga::ror(byte_vm original){
    _mem_buffer = _proc_flags;//just need a buffer :)
    _proc_flags = (_proc_flags & 0b11110111) | (0b00001000 * (original & 0b00000001));
    return (original >> 1) | (0b10000000 * ((_stack_buffer >> 3) & 0b00000001));
}
byte_vm volga::rol(byte_vm original){
    _mem_buffer = _proc_flags;//just need a buffer :)
    _proc_flags = (_proc_flags & 0b11110111) | (0b00001000 * ((original & 0b10000000) >> 7));
    return (original << 1) | ((_stack_buffer >> 3) & 0b00000001);
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
            return _buffer;}
        case 0x02: {// BRK A/HLT a
            return _accum;}
        case 0x0D: {// CMP
            int cmp = _accum - _buffer;
            _proc_flags= 0b00000000;
            if(cmp == 0)        _proc_flags = _proc_flags | 0b00100000;
            if(cmp < 0)         _proc_flags = _proc_flags | 0b00010000;
            if(abs(cmp) > 0xFF) _proc_flags = _proc_flags | 0b01000000;
            break;}
        case 0x10: {// BBS0
            _buffer = _buffer | 0b00000001;
            break;}
        case 0x11: {// BBC0
            _buffer = _buffer & 0b11111110;
            break;}
        case 0x12: {// BBS1
            _buffer = _buffer | 0b00000010;
            break;}
        case 0x13: {// BBC1
            _buffer = _buffer & 0b11111101;
            break;}
        case 0x14: {// BBS2
            _buffer = _buffer | 0b00000100;
            break;}
        case 0x15: {// BBC2
            _buffer = _buffer & 0b11111011;
            break;}
        case 0x16: {// BBS3
            _buffer = _buffer | 0b00001000;
            break;}
        case 0x17: {// BBC3
            _buffer = _buffer & 0b11110111;
            break;}
        case 0x18: {// BBS4
            _buffer = _buffer | 0b00010000;
            break;}
        case 0x19: {// BBC4
            _buffer = _buffer & 0b11101111;
            break;}
        case 0x1A: {// BBS5
            _buffer = _buffer | 0b00100000;
            break;}
        case 0x1B: {// BBC5
            _buffer = _buffer & 0b11011111;
            break;}
        case 0x1C: {// BBS6
            _buffer = _buffer | 0b01000000;
            break;}
        case 0x1D: {// BBC6
            _buffer = _buffer & 0b10111111;
            break;}
        case 0x1E: {// BBS7
            _buffer = _buffer | 0b10000000;
            break;}
        case 0x1F: {// BBC7
            _buffer = _buffer & 0b01111111;
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
            _accum = ror(_accum);
            break;}
        case 0x29: {// RLA
            _accum = rol(_accum);
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
            _buffer = ror(_buffer);
            break;}
        case 0x2F: {// RLB
            _buffer = rol(_buffer);
            break;}
        case 0x30: {// BEQ a
            readAddrPC();
            if(_accum == _buffer){
                readAddr();
                _prog_count = _addr_buffer;
            }
            break;}
        case 0x31: {// BEQ #
            readPC();
            if(_accum == _buffer){
                _prog_count += (signed char)_mem_buffer;
            }
            break;}
        case 0x32: {// BNQ a
            readAddrPC();
            if(_accum != _buffer){
                readAddr();
                _prog_count = _addr_buffer;
            }
            break;}
        case 0x33: {// BNQ #
            readPC();
            if(_accum != _buffer){
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
        case 0x42: {// BCC a
            readAddrPC();
            if(((_proc_flags & 0b10000000) >> 7) == 0){
                readAddr();
                _prog_count = _addr_buffer;
            }
            break;}
        case 0x43: {// BCC #
            readPC();
            if(((_proc_flags & 0b10000000) >> 7) == 0){
                _prog_count += (signed char)_mem_buffer;
            }
            break;}
        case 0x44: {// BOS a
            readAddrPC();
            if(((_proc_flags & 0b01000000) >> 6) == 1){
                readAddr();
                _prog_count = _addr_buffer;
            }
            break;}
        case 0x45: {// BOS #
            readPC();
            if(((_proc_flags & 0b01000000) >> 6) == 1){
                _prog_count += (signed char)_mem_buffer;
            }
            break;}
        case 0x46: {// BOC a
            readAddrPC();
            if(((_proc_flags & 0b01000000) >> 6) == 0){
                readAddr();
                _prog_count = _addr_buffer;
            }
            break;}
        case 0x47: {// BOC #
            readPC();
            if(((_proc_flags & 0b01000000) >> 6) == 0){
                _prog_count += (signed char)_mem_buffer;
            }
            break;}
        case 0x48: {// BZS a
            readAddrPC();
            if(((_proc_flags & 0b00100000) >> 5) == 1){
                readAddr();
                _prog_count = _addr_buffer;
            }
            break;}
        case 0x49: {// BZS #
            readPC();
            if(((_proc_flags & 0b00100000) >> 5) == 1){
                _prog_count += (signed char)_mem_buffer;
            }
            break;}
        case 0x4A: {// BZC a
            readAddrPC();
            if(((_proc_flags & 0b00100000) >> 5) == 0){
                readAddr();
                _prog_count = _addr_buffer;
            }
            break;}
        case 0x4B: {// BZC #
            readPC();
            if(((_proc_flags & 0b00100000) >> 5) == 0){
                _prog_count += (signed char)_mem_buffer;
            }
            break;}
        case 0x4C: {// BNS a
            readAddrPC();
            if(((_proc_flags & 0b00010000) >> 4) == 1){
                readAddr();
                _prog_count = _addr_buffer;
            }
            break;}
        case 0x4D: {// BNS #
            readPC();
            if(((_proc_flags & 0b00010000) >> 4) == 1){
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
/*NImp*/case 0x50: {// RTS
            break;}
/*NImp*/case 0x51: {// JSR ##
            break;}
/*NImp*/case 0x52: {// JSR B
            break;}
/*NImp*/case 0x53: {// JSR a,X
            break;}
/*NImp*/case 0x54: {// JSR X,a
            break;}
/*NImp*/case 0x55: {// JSR a,Y
            break;}
/*NImp*/case 0x56: {// JSR Y,a
            break;}
/*NImp*/case 0X57: {// JSR X,Y
            break;}
        case 0x59: {// JMP ##
            readAddrPC();
            _prog_count = _addr_buffer;
            break;}
        case 0x5A: {// JMP B
            _prog_count = _addr_buffer;
            break;}
        case 0x5B: {// JMP a,X
            readPC();
            _prog_count = (_mem_buffer << 8) + _x_reg;
            break;}
        case 0x5C: {// JMP X,a
            readPC();
            _prog_count = (_x_reg << 8) + _mem_buffer;
            break;}
        case 0x5D: {// JMP a,Y
            readPC();
            _prog_count = (_mem_buffer << 8) + _y_reg;
            break;}
        case 0x5E: {// JMP Y,a
            readPC();
            _prog_count = (_y_reg << 8) + _mem_buffer;
            break;}
        case 0X5F: {// JMP X,Y
            readPC();
            _prog_count = (_x_reg << 8) + _y_reg;
        case 0xA8: {// LDA #
            readPC();
            _accum = _mem_buffer;
            break;}
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
    _accum = 0x00;
    _x_reg = 0x00;
    _y_reg = 0x00;
    _buffer = 0x00;
    _mem_buffer = 0x00;
    _stack_buffer = 0x00;
    _stack_ptr = 0x00;
}
volga::~volga(){
    _mem.~memory();
}