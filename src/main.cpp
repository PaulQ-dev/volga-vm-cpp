#include <iostream>
#include "vm/volga.h"

using namespace std;
using namespace paulqpro::volga;

int main(){
    volga vm = volga();
    byte_vm rom[0x20000] = {1};
    cout << vm.loadRom(rom,sizeof(rom)) << endl;
    return 0;
}