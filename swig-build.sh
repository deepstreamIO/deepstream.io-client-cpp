#!/bin/bash

: ${MODULE:=foo}

set -eux

swig -python -I${PWD}/include -c++ ${MODULE}.i
g++ -std=c++11 -fPIC -c -I${PWD}/include $(pkg-config --cflags python3) ${MODULE}_wrap.cxx
g++ -std=c++11 -shared ${MODULE}_wrap.o -o _${MODULE}.so -Llib -ldeepstream $(pkg-config --libs python3)
