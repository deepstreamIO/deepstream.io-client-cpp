#!/bin/bash

: ${MODULE:=foo}

set -eux

swig -python -I${PWD}/include -c++ ${MODULE}.i
./fix-${MODULE}-swig.pl ${MODULE}_wrap.cxx
g++ -std=c++11 -fPIC -c -I${PWD}/include $(pkg-config --cflags python3) -I${PWD}/thirdparty/include ${MODULE}_wrap.cxx
g++ -std=c++11 -shared ${MODULE}_wrap.o -o _${MODULE}.so -L$PWD/lib -ldeepstream -Lthirdparty/lib -lPocoNetSSL -lPocoCrypto -lPocoNet -lPocoUtil -lPocoJSON -lPocoXML -lPocoFoundation $(pkg-config --libs python3) -ldeepstream
