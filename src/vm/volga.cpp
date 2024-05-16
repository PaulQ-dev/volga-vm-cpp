#include "volga.h"
#include <iostream>
#include <stdio.h>

using namespace paulqpro::volga;

string volga::helloWorld() {
    return "Hello C++";
}

// false - success
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
// false - success
bool volga::loadRom(string rom_file){
    FILE* f = fopen(rom_file.c_str(), "r");
    int fsize = 0;
    while(true){
        cout<<fsize<<endl;
        int b = fgetc(f);
        if(b == EOF) break;
        fsize++;
    }
    byte_vm rom[fsize];
    for(int i = 0; i < fsize; i++){
        rom[i] = fgetc(f);
    }
    return loadRom(rom,fsize);
}

void volga::loadA(byte_vm value){
    _accum = value;
}
void volga::loadA(addr_vm address){
    _addr_buffer = _prog_count;
    _prog_count = address;
    read();
    loadA(_buffer);
}
void volga::storeA(addr_vm address){
    _addr_buffer = address;
    _buffer = _accum;
    write();
}

void volga::loadX(byte_vm value){
    _x_reg = value;
}
void volga::loadX(addr_vm address){
    _addr_buffer = _prog_count;
    _prog_count = address;
    read();
    loadX(_buffer);
}
void volga::storeX(addr_vm address){
    _addr_buffer = address;
    _buffer = _x_reg;
    write();
}

void volga::loadY(byte_vm value){
    _y_reg = value;
}
void volga::loadY(addr_vm address){
    _addr_buffer = _prog_count;
    _prog_count = address;
    read();
    loadY(_buffer);
}
void volga::storeY(addr_vm address){
    _addr_buffer = address;
    _buffer = _y_reg;
    write();
}

void volga::read(){
    _buffer = _mem.getMemory(_addr_buffer);
}
void volga::readAddr(){
    read();
    _addr_buffer = _buffer;
    read();
    _addr_buffer = (_addr_buffer * 256) + _buffer;
}
void volga::write(){
    _mem.setMemory(_addr_buffer, _buffer);
}

int volga::run(){
    _prog_count = _rom_start;
    
    return 0;
}

volga::volga() {
    _mem = memory();
    _ram_start = 0x0000;
    _ram_end = 0x7FFF;
    _rom_start = 0x9000;
    _rom_end = 0xFFFF;
    _prog_count = 0x0000;
    _proc_flags = 0b00000000;
    _accum = 0x00;
    _x_reg = 0x00;
    _y_reg = 0x00;
    _buffer = 0x00;
}
volga::~volga(){
    _mem.~memory();
}