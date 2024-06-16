cmake_minimum_required(VERSION 3.5)
project(VM)

set(BUILD_SHARED_LIBS 1)

add_executable(volga src/main.cpp src/vm/volga.cpp src/vm/volga.h src/vm/memory/memory.cpp src/vm/memory/memory.h src/misc.h)