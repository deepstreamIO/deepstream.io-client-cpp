#!/bin/bash

set -eu

poco_url='https://pocoproject.org/releases/poco-1.7.7/poco-1.7.7.tar.gz'
poco_zip='poco-1.7.7.tar.gz'
poco_src='poco-1.7.7'

if [ ! -f $poco_zip ]; then
    wget "$poco_url"
fi

tar -zxf $poco_zip

cd $poco_src

cmake						\
    -DCMAKE_BUILD_TYPE=Debug			\
    -DCMAKE_DEBUG_POSTFIX=''			\
    -DCMAKE_INSTALL_PREFIX="${1:-${poco_src}}"	\
    -DENABLE_CRYPTO=OFF				\
    -DENABLE_DATA=OFF				\
    -DENABLE_JSON=OFF				\
    -DENABLE_MONGODB=OFF			\
    -DENABLE_NETSSL=OFF				\
    -DENABLE_PAGECOMPILER=OFF			\
    -DENABLE_PAGECOMPILER_FILE2PAGE=OFF		\
    -DENABLE_UTIL=OFF				\
    -DENABLE_XML=OFF				\
    -DENABLE_ZIP=OFF

make --jobs=$(nproc)
make install
