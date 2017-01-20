#!/bin/bash

set -e


poco_url='https://pocoproject.org/releases/poco-1.7.7/poco-1.7.7.tar.gz'
poco_zip='poco-1.7.7.tar.gz'
poco_src='poco-1.7.7'

poco_tmp=$(mktemp --directory --tmpdir poco.XXXXXXXXXX)
pushd "$poco_tmp"
wget "$poco_url"
tar -zxf "$poco_zip"

mkdir build
cd build

cmake \
	-DCMAKE_BUILD_TYPE=Debug \
	-DCMAKE_DEBUG_POSTFIX='' \
	-DCMAKE_INSTALL_PREFIX=~ \
	-DENABLE_CRYPTO=OFF \
	-DENABLE_DATA=OFF \
	-DENABLE_JSON=OFF \
	-DENABLE_MONGODB=OFF \
	-DENABLE_NETSSL=OFF \
	-DENABLE_PAGECOMPILER=OFF \
	-DENABLE_PAGECOMPILER_FILE2PAGE=OFF \
	-DENABLE_UTIL=OFF \
	-DENABLE_XML=OFF \
	-DENABLE_ZIP=OFF \
	-- "../$poco_src"
make --jobs=2
make install
popd


# deepstream.io C++ client
my_src=$(pwd)
my_tmp=$(mktemp --directory)
cd "$my_tmp"

cmake \
	-DCMAKE_PREFIX_PATH=~ \
	-DBUILD_TESTING=ON \
	-- "$my_src"
make --jobs=2
make --jobs=2 test
