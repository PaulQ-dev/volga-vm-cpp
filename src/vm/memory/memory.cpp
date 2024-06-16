#include "./memory.h"

using namespace paulqpro::volga;

ubyte_vm memory::getMemory(addr_vm address){
    return _mem[address];
}

void memory::setMemory(addr_vm address, ubyte_vm value){
    _mem[address] = value;
}

memory::memory(){
    _mem = new ubyte_vm[0x10000];
}

memory::~memory(){
    //delete [] _mem;// - causes double free
}