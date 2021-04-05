#UNMAINTAINED  

[deepstream.io](http://deepstream.io/) C++ Client [![Build Status](https://travis-ci.org/deepstreamIO/deepstream.io-client-cpp.svg?branch=master)](https://travis-ci.org/deepstreamIO/deepstream.io-client-cpp) [![codecov](https://codecov.io/gh/deepstreamIO/deepstream.io-client-cpp/branch/master/graph/badge.svg)](https://codecov.io/gh/deepstreamIO/deepstream.io-client-cpp)
=======================

[Documentation](http://dsh.cloud/docs/client-cpp/Deepstream/)

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

# debug build
scripts/build-ubuntu.sh debug
```

Running an example client
-------------------------

Run an example client application against a local deepstream server

See examples folder for source.

```bash
build/bin/ds-example 'localhost:6020/deepstream'
```
