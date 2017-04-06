#!/bin/bash
brew install openssl poco flex

# optional:
brew install doxygen swig ccache boost python3 lcov

mkdir build && cd build
cmake .. \
  -DOPENSSL_ROOT_DIR=/usr/local/opt/openssl/ \
  -DPoco_LIBRARIES='/usr/local/opt/poco/lib/libPocoFoundation.dylib;/usr/local/opt/poco/lib/libPocoCrypto.dylib;/usr/local/opt/poco/lib/libPocoNet.dylib;/usr/local/opt/poco/lib/libPocoNetSSL.dylib'
make

#run tests
export PYTHONPATH=$PWD/lib:$PWD/python
make test

#run coverage
../scripts/lcov.sh
