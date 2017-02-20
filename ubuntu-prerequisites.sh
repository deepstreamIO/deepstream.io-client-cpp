#!/bin/sh

apt update -yq

apt-get -yq --no-install-suggests --no-install-recommends install \
	libboost-dev \
	libboost-test-dev \
	flex \
	valgrind \
	lcov \
	build-essential \
	gcc \
	cmake \
	wget \
	curl
