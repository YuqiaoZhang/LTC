#!/bin/bash

if test \( $# -ne 1 \);
then
    echo "Usage: run_ubuntu.sh config"
    echo ""
    echo "config:"
    echo "  debug   -   run the debug configuration"
    echo "  release -   run the release configuration"
    echo ""
    exit 1
fi

MY_DIR="$(cd "$(dirname "$0")" 1>/dev/null 2>/dev/null && pwd)"  

cd "${MY_DIR}/bgfx/examples/runtime"
"${MY_DIR}/bgfx/scripts/example-xx-arealightsDebug"