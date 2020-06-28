#!/bin/bash

VNX_INTERFACE_DIR=${VNX_INTERFACE_DIR:-/usr/interface}

cd $(dirname "$0")

vnxcppcodegen --cleanup generated/ vnx.addons interface/ ${VNX_INTERFACE_DIR}/vnx/
