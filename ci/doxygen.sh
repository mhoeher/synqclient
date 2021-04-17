#!/bin/bash

set -e

SCRIPT_DIR="$(cd $(dirname "$0") && pwd)"
cd "$SCRIPT_DIR"
cd ..

doxygen
cd doc
make html
cd ..

if [ -n "$CI" ]; then
    mkdir public
    cp -r doc/_build/html/* public
fi
