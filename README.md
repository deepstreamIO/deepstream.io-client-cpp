deepstream.io-client-cpp [![Build Status](https://travis-ci.org/deepstreamIO/deepstream.io-client-cpp.svg?branch=master)](https://travis-ci.org/deepstreamIO/deepstream.io-client-cpp) [![codecov](https://codecov.io/gh/deepstreamIO/deepstream.io-client-cpp/branch/master/graph/badge.svg)](https://codecov.io/gh/deepstreamIO/deepstream.io-client-cpp)


=======================

The C++ client for [deepstream.io](http://deepstream.io/)

## Documentation Coming Soon!

Installation
------------

# OSX (requires homebrew)

```bash
# install dependencies and run debug build
./scripts/osx-build.sh debug
```

# Ubuntu (tested on Trusty + Xenial)
```bash
# install dependencies
sudo scripts/ubuntu-prerequisites.sh

# run debug build
scripts/build-ubuntu.sh debug
```

Running an example client
-------------------------

Run an example client application against a local deepstream server

See examples folder for source.

```bash
build/bin/ds-example 'localhost:6020/deepstream'
```
