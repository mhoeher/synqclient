#!/bin/bash

set -e

SCRIPT_DIR="$(cd $(dirname "$0") && pwd)"
cd "$SCRIPT_DIR"
cd ..

if [ -n "$CI" ]; then
    dnf install -y gcc gcc-c++ cmake ninja-build qt5-qtbase-devel
fi

mkdir -p build/fedora-cmake
cd build/fedora-cmake
cmake -DBUILD_SHARED_LIBS=ON -GNinja ../../
cmake --build .
cmake --build . --target test

ls libsynqclient | grep libsynqclient.so
