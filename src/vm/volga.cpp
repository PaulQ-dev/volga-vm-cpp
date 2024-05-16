#include "volga.h"
#include <iostream>
#include <stdio.h>

using namespace paulqpro::volga;

string volga::helloWorld() {
    return "Hello C++";
}

/**
 ** false - success
 **/
bool volga::loadRom(byte_vm rom[], int rom_len){
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
/**
 ** false - success
 **/
bool volga::loadRom(string rom_file){
    FILE* f = fopen(rom_file.c_str(), "r");
    int fsize = 0;
    while(true){
        byte_vm b;
        fread(&b,1,1,f);
    }
}

volga::volga() {
    _mem = memory();
    _ram_start = 0x0000;
    _rom_start = 0x9000;
    _prog_count = 0x0000;
    _proc_flags = 0b00000000;
    _accum = 0x00;
    _x_reg = 0x00;
    _y_reg = 0x00;
}
volga::~volga(){
    _mem.~memory();
}