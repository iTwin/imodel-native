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

if [ -z "$MANIFEST_DIR" ] || [ -z "$INSTALL_ROOT" ] || [ -z "$TRIPLET" ]; then
    echo "Usage: $0 <manifest_dir> <install_root> <triplet>"
    exit 1
fi

# For cross-compilation triplets (iOS, Android), vcpkg/CMake manages its own
# toolchain (Xcode SDK via xcrun for iOS, NDK for Android). Unset any compiler
# env vars that might have been set at the pipeline level so CMake's platform
# modules can discover the correct tools via xcrun.
if [[ "$TRIPLET" == *-ios ]] || [[ "$TRIPLET" == *-android ]]; then
    unset CC CXX AR RANLIB NM STRIP
fi

# BentleyBuild uses LLVM_DIR as the LLVM install root. If present, put that
# toolchain first so vcpkg ports and GN/Ninja builds use the same compiler.
# Skip for cross-compilation triplets handled above.
if [ -n "${LLVM_DIR:-}" ] && [[ "$TRIPLET" != *-ios ]] && [[ "$TRIPLET" != *-android ]]; then
    LLVM_BIN="$LLVM_DIR/bin"
    if [ ! -x "$LLVM_BIN/clang" ] || [ ! -x "$LLVM_BIN/clang++" ]; then
        echo "Error: LLVM_DIR is set, but clang/clang++ were not found under $LLVM_BIN"
        exit 1
    fi

    echo "Using LLVM toolchain from LLVM_DIR: $LLVM_DIR"
    export PATH="$LLVM_BIN:$PATH"
    export CC="$LLVM_BIN/clang"
    export CXX="$LLVM_BIN/clang++"

    if [ -x "$LLVM_BIN/llvm-ar" ]; then
        export AR="$LLVM_BIN/llvm-ar"
    fi
    if [ -x "$LLVM_BIN/llvm-ranlib" ]; then
        export RANLIB="$LLVM_BIN/llvm-ranlib"
    fi
    if [ -x "$LLVM_BIN/llvm-nm" ]; then
        export NM="$LLVM_BIN/llvm-nm"
    fi
    if [ -x "$LLVM_BIN/llvm-strip" ]; then
        export STRIP="$LLVM_BIN/llvm-strip"
    fi
fi

# Locate vcpkg. Override with IMODEL_VCPKG_ROOT environment variable.
# Use IMODEL_VCPKG_ROOT instead of VCPKG_ROOT to avoid conflicts with
# tooling that may set VCPKG_ROOT to an undesired location.
if [ -n "$IMODEL_VCPKG_ROOT" ]; then
    VCPKG_ROOT="$IMODEL_VCPKG_ROOT"
elif [ -z "$VCPKG_ROOT" ]; then
    VCPKG_ROOT="$SrcRoot/vcpkg"
fi

# Local binary caching is enabled by default, but can be disabled by setting VCPKG_BINARY_SOURCES to "clear".

# # Disable binary caching by default. Override with VCPKG_BINARY_SOURCES env var.
# if [ -z "$VCPKG_BINARY_SOURCES" ]; then
#     export VCPKG_BINARY_SOURCES="clear"
# fi

VCPKG_EXE="$VCPKG_ROOT/vcpkg"

if [ ! -x "$VCPKG_EXE" ]; then
    echo "Error: vcpkg not found at $VCPKG_EXE"
    echo "Set IMODEL_VCPKG_ROOT to the vcpkg installation directory."
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
    --x-buildtrees-root="$INSTALL_ROOT/buildtrees" \
    --x-packages-root="$INSTALL_ROOT/packages" \
    "${OVERLAY_ARGS[@]}"
