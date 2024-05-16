#include "./memory.h"

using namespace paulqpro::volga;

byte_vm memory::getMemory(addr_vm address){
    return _mem[address];
}

void memory::setMemory(addr_vm address, byte_vm value){
    _mem[address] = value;
}

memory::memory(){
    _mem = new byte_vm[0x10000];
}

memory::~memory(){
    
}