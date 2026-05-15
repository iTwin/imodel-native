#!/bin/bash
#---------------------------------------------------------------------------------------------
#  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
#  See LICENSE.md in the repository root for full copyright notice.
#---------------------------------------------------------------------------------------------
# Wrapper script for vcpkg install, invoked from .mke build files.
# Customize VCPKG_ROOT for developer or CI environments.
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

if [ -z "$MANIFEST_DIR" ] || [ -z "$INSTALL_ROOT" ] || [ -z "$TRIPLET" ]; then
    echo "Usage: $0 <manifest_dir> <install_root> <triplet>"
    exit 1
fi

# Locate vcpkg. Override with VCPKG_ROOT environment variable.
if [ -z "$VCPKG_ROOT" ]; then
    VCPKG_ROOT="$HOME/src/vcpkg"
fi

# Local binary caching is enabled by default, but can be disabled by setting VCPKG_BINARY_SOURCES to "clear".

# # Disable binary caching by default. Override with VCPKG_BINARY_SOURCES env var.
# if [ -z "$VCPKG_BINARY_SOURCES" ]; then
#     export VCPKG_BINARY_SOURCES="clear"
# fi

VCPKG_EXE="$VCPKG_ROOT/vcpkg"

if [ ! -x "$VCPKG_EXE" ]; then
    echo "Error: vcpkg not found at $VCPKG_EXE"
    echo "Set VCPKG_ROOT to the vcpkg installation directory."
    exit 1
fi

# Use custom overlay triplets from the manifest directory (if present) for build flags
OVERLAY_TRIPLETS="$MANIFEST_DIR/triplets"

echo "vcpkg: installing packages from $MANIFEST_DIR (triplet=$TRIPLET, install-root=$INSTALL_ROOT)"

OVERLAY_ARGS=()
if [ -d "$OVERLAY_TRIPLETS" ]; then
    OVERLAY_ARGS+=(--overlay-triplets="$OVERLAY_TRIPLETS")
fi

"$VCPKG_EXE" install \
    --triplet "$TRIPLET" \
    --x-install-root="$INSTALL_ROOT" \
    --x-manifest-root="$MANIFEST_DIR" \
    "${OVERLAY_ARGS[@]}"
