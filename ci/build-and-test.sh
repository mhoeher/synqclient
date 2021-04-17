#!/bin/bash

set -e

SCRIPT_DIR="$(cd $(dirname "$0") && pwd)"
cd "$SCRIPT_DIR"
cd ..

mkdir -p build/fedora
cd build/fedora
qmake-qt5 ../..
make -j4
LD_LIBRARY_PATH=$PWD/libsynqclient:$LD_LIBRARY_PATH make check

ls libsynqclient | grep libsynqclient.so
