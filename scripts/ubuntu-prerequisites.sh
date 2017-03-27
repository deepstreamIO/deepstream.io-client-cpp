#!/bin/sh

if [ "$(lsb_release -cs)" = "trusty" ]; then
    apt-add-repository "deb http://archive.ubuntu.com/ubuntu trusty-backports main restricted universe multiverse"
fi

apt update -yq

apt-get install eatmydata

eatmydata apt-get -yq --no-install-suggests --no-install-recommends install \
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

# Temporarily disabled 27/03/17
# if [ "$(lsb_release -cs)" = "trusty" ]; then
#     eatmydata apt-get -yq -t trusty-backports install swig3.0
#     eatmydata apt-get -yq -t trusty-backports install cmake
# else
#     eatmydata apt-get -yq install swig3.0
#     eatmydata apt-get -yq install cmake
# fi
