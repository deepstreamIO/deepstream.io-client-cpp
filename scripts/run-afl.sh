#!/bin/bash

set -e


num_jobs=$(getconf _NPROCESSORS_ONLN)


poco_url='https://pocoproject.org/releases/poco-1.7.7/poco-1.7.7.tar.gz'
poco_zip='poco-1.7.7.tar.gz'
poco_src='poco-1.7.7'

poco_tmp=$(mktemp -d)
pushd "$poco_tmp"
wget "$poco_url"
tar -zxf "$poco_zip"

mkdir build
cd build

cmake \
	-DCMAKE_BUILD_TYPE=Debug \
	-DCMAKE_CXX_FLAGS='-std=c++11' \
	-DCMAKE_DEBUG_POSTFIX='' \
	-DCMAKE_INSTALL_PREFIX="$poco_tmp" \
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
make --jobs=$num_jobs
make install
popd


# deepstream.io C++ client
my_src=$(pwd)
my_tmp=$(mktemp -d)
pushd "$my_tmp"

export CC=afl-clang
export CXX=afl-clang++
export AFL_HARDEN=1

cmake \
	-DCMAKE_PREFIX_PATH="$poco_tmp" \
	-DBUILD_TESTING=ON \
	-DCMAKE_BUILD_TYPE=Debug \
	-DCMAKE_C_FLAGS='-O1' \
	-DCMAKE_CXX_FLAGS='-O1' \
	-- "$my_src"
make --jobs=$num_jobs
make --jobs=$num_jobs test


# set up input
mkdir input-dir

for i in {1..1}
do
	./src/make-fuzzer-input $i >"input-dir/$i.txt"
done

exit 0

afl-fuzz \
	-i input-dir \
	-o sync-dir \
	-M fuzzer-master \
	-- ./src/fuzz-me

afl-fuzz \
	-i input-dir \
	-o sync-dir \
	-S fuzzer-slave1 \
	-- ./src/fuzz-me
