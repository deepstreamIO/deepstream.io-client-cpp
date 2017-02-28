#!/bin/sh

if [ "$(lsb_release -cs)" = "trusty" ]; then
    apt-add-repository "deb http://archive.ubuntu.com/ubuntu trusty-backports main restricted universe multiverse"
fi

apt update -yq

apt-get -yq --no-install-suggests --no-install-recommends install \
	build-essential \
	flex \
	gcc \
	lcov \
	libboost-dev \
	libboost-test-dev \
	libssl-dev \
	python3 \
        python3-dev \
	valgrind \
	wget

if [ "$(lsb_release -cs)" = "trusty" ]; then
    apt-get -yq -t trusty-backports install swig3.0
    apt-get -yq -t trusty-backports install cmake
else
    apt-get -yq install swig3.0
    apt-get -yq install cmake
fi
