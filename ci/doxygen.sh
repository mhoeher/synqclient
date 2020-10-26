#!/bin/bash

set -e

SCRIPT_DIR="$(cd $(dirname "$0") && pwd)"
cd "$SCRIPT_DIR"
cd ..

if [ -n "$CI" ]; then
    dnf install -y doxygen
fi

doxygen

if [ -n "$CI" ]; then
    mkdir public
    cp -r doc/html/* public
fi
