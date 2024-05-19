#include <iostream>
#include "vm/volga.h"

using namespace std;
using namespace paulqpro::volga;

int main(){
    volga vm = volga();
    vm.loadRom("rom.bin");
    return vm.run();
}