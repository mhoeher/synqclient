#!/bin/bash

set -e

SCRIPT_DIR="$(cd $(dirname "$0") && pwd)"
cd "$SCRIPT_DIR"
cd ..

mkdir -p build/pri
cd build/pri
qmake-qt5 ../../ci/pri-test-data/
make -j4

