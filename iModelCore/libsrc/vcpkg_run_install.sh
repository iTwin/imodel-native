#!/bin/bash
#---------------------------------------------------------------------------------------------
#  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
#  See LICENSE.md in the repository root for full copyright notice.
#---------------------------------------------------------------------------------------------
# Wrapper script for vcpkg install, invoked from .mke build files.
# Customize IMODEL_VCPKG_ROOT for developer or CI environments.
#
# Usage: vcpkg_install.sh <manifest_dir> <install_root> <triplet>
#   manifest_dir: Directory containing vcpkg.json
#   install_root: Where vcpkg_installed/<triplet> output goes (e.g., $OutRoot/vcpkg)
#   triplet:      vcpkg triplet (e.g., arm64-osx, x64-linux)
#---------------------------------------------------------------------------------------------

set -e

MANIFEST_DIR="$1"
INSTALL_ROOT="$2"
TRIPLET="$3"

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

if [ -z "$MANIFEST_DIR" ] || [ -z "$INSTALL_ROOT" ] || [ -z "$TRIPLET" ]; then
    echo "Usage: $0 <manifest_dir> <install_root> <triplet>"
    exit 1
fi

# Locate vcpkg. Override with IMODEL_VCPKG_ROOT environment variable.
# Use IMODEL_VCPKG_ROOT instead of VCPKG_ROOT to avoid conflicts with
# tooling that may set VCPKG_ROOT to an undesired location.
if [ -n "$IMODEL_VCPKG_ROOT" ]; then
    VCPKG_ROOT="$IMODEL_VCPKG_ROOT"
elif [ -z "$VCPKG_ROOT" ]; then
    VCPKG_ROOT="$SCRIPT_DIR/vcpkg"
fi

# Use a persistent local binary cache by default to avoid rebuilding heavy ports
# (for example, crashpad) across builds. Allow callers to override.
if [ -z "${VCPKG_DEFAULT_BINARY_CACHE:-}" ]; then
    export VCPKG_DEFAULT_BINARY_CACHE="$VCPKG_ROOT/archives"
fi
mkdir -p "$VCPKG_DEFAULT_BINARY_CACHE"

if [ -z "${VCPKG_BINARY_SOURCES:-}" ]; then
    export VCPKG_BINARY_SOURCES="clear;files,$VCPKG_DEFAULT_BINARY_CACHE,readwrite"
fi

VCPKG_EXE="$VCPKG_ROOT/vcpkg"

if [ ! -x "$VCPKG_EXE" ]; then
    echo "Error: vcpkg not found at $VCPKG_EXE"
    echo "Set IMODEL_VCPKG_ROOT to the vcpkg installation directory."
    exit 1
fi

# Use custom overlay triplets from the manifest directory (if present) for build flags
OVERLAY_TRIPLETS="$MANIFEST_DIR/triplets"

echo "vcpkg: installing packages from $MANIFEST_DIR (triplet=$TRIPLET, install-root=$INSTALL_ROOT)"
echo "vcpkg: binary-cache=$VCPKG_DEFAULT_BINARY_CACHE"
echo "vcpkg: binary-sources=$VCPKG_BINARY_SOURCES"

OVERLAY_ARGS=()
if [ -d "$OVERLAY_TRIPLETS" ]; then
    OVERLAY_ARGS+=(--overlay-triplets="$OVERLAY_TRIPLETS")
fi

"$VCPKG_EXE" install \
    --triplet "$TRIPLET" \
    --x-install-root="$INSTALL_ROOT" \
    --x-manifest-root="$MANIFEST_DIR" \
    "${OVERLAY_ARGS[@]}"
