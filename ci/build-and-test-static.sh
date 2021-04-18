#!/bin/bash

set -e

SCRIPT_DIR="$(cd $(dirname "$0") && pwd)"
cd "$SCRIPT_DIR"
cd ..

mkdir -p build/fedora-static
cd build/fedora-static
qmake-qt5 CONFIG+=staticlib ../..
make -j4
make check

ls libsynqclient | grep libsynqclient.a
