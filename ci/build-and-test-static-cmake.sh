#!/bin/bash

set -e

SCRIPT_DIR="$(cd $(dirname "$0") && pwd)"
cd "$SCRIPT_DIR"
cd ..

mkdir -p build/fedora-static-cmake
cd build/fedora-static-cmake
cmake -DBUILD_SHARED_LIBS=OFF -GNinja ../../
cmake --build .
cmake --build . --target test
DESTDIR=$PWD/_ cmake --install .

ls libsynqclient | grep libsynqclient-qt5.a

cd _
mkdir -p build
cd build
cmake \
    -DCMAKE_INSTALL_PREFIX=$PWD/../usr/local \
    -GNinja \
    ../../../../tests/cmake-install-test
cmake --build .
