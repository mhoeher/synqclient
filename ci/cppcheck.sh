#!/bin/bash

set -e

SCRIPT_DIR="$(cd $(dirname "$0") && pwd)"
cd "$SCRIPT_DIR"
cd ..

if [ -n "$CI" ]; then
    dnf install -y cppcheck
fi

cppcheck  \
    --enable=warning,style,performance,portability,missingInclude \
    --error-exitcode=1 \
    --inline-suppr \
    --template='{file}:{line} {severity} "{id}": {message}' \
    --library=qt \
    --std=c++11 \
    --quiet \
    libsynqclient/ tests/
