#!/bin/bash

set -e

SCRIPT_DIR="$(cd $(dirname "$0") && pwd)"
cd "$SCRIPT_DIR"
cd ..

if [ -n "$CI" ]; then
    dnf install -y doxygen python3-pip
    pip install sphinx sphinx_rtd_theme breathe
fi

doxygen
cd doc
make html

if [ -n "$CI" ]; then
    mkdir public
    cp -r doc/_build/html/* public
fi
