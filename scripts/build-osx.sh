#!/bin/bash

if [ -z "$1" ]; then
  echo "valid arguments: [debug|coverage|release]"
  exit 1
fi

# change working directory to project root (location above build script)
cd "$(dirname "$0")/.."

# install dependencies
brew install openssl poco flex cmake

# optional:
brew install doxygen swig ccache boost python3 lcov

if [ "$1" = "debug" ]; then
  echo "## Running debug build ##"

  # out-of-source cmake development build
  if [ -e build/ ]; then rm -rf build; fi
  mkdir build && cd build
  cmake .. \
    -DOPENSSL_ROOT_DIR=/usr/local/opt/openssl/ \
    -DPoco_LIBRARIES='/usr/local/lib/libPocoFoundation.dylib;/usr/local/lib/libPocoCrypto.dylib;/usr/local/lib/libPocoNet.dylib;/usr/local/lib/libPocoNetSSL.dylib' \
    && make -j 2 || exit 1

  # run tests
  export PYTHONPATH=$PWD/lib:$PWD/python
  make test

elif [ "$1" = "coverage" ]; then
  echo "## Running code coverage ##"
  
  # run code coverage
  if [ -e build/ ]; then rm -rf build; fi
  mkdir build && cd build
  cmake .. -DBUILD_POCO=ON -DBUILD_COVERAGE=ON \
    && make -j 2 || exit 1

  ../scripts/lcov.sh

elif [ "$1" = "release" ]; then
  echo "## Optimized build for release ##"

  if [ -e release/ ]; then rm -rf release; fi
  mkdir release && cd release
  cmake .. -DBUILD_POCO=ON -DCMAKE_BUILD_TYPE=Release \
    && make -j 2
fi
