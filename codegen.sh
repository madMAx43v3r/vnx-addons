#!/bin/bash

VNX_INTERFACE_DIR=${VNX_INTERFACE_DIR:-/usr/interface}

VNX_INCLUDE=$1

if [ -z "$VNX_INCLUDE" ]; then
        VNX_INCLUDE=${VNX_INTERFACE_DIR}/vnx/
fi

cd $(dirname "$0")

vnxcppcodegen --cleanup generated/ vnx.addons interface/ modules/ $VNX_INCLUDE
