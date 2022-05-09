#!/bin/bash

if test \( $# -ne 1 \);
then
    echo "Usage: build_ubuntu.sh config"
    echo ""
    echo "config:"
    echo "  debug   -   build the debug configuration"
    echo "  release -   build the release configuration"
    echo ""
    exit 1
fi

if test \( \( -n "$1" \) -a \( "$1" = "debug" \) \);then 
    MAKE_ARG_CONFIG="config=debug"
elif test \( \( -n "$1" \) -a \( "$1" = "release" \) \);then
    MAKE_ARG_CONFIG="config=release"
else
    echo "The config \"$1\" is NOT supported!"
    echo ""
    echo "Configs:"
    echo "  debug   -   build the debug configuration"
    echo "  release -   build the release configuration"
    echo ""
    exit 1
fi

MY_DIR="$(cd "$(dirname "$0")" 1>/dev/null 2>/dev/null && pwd)"  

cd "${MY_DIR}/bgfx"
"${MY_DIR}/bx/tools/bin/linux/genie" --gcc=linux-gcc gmake

cd "${MY_DIR}/bgfx/.build/projects/gmake-linux"
make "${MAKE_ARG_CONFIG}" example-xx-arealights