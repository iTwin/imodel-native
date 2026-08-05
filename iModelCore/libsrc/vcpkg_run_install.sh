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
    VCPKG_ROOT="${SrcRoot}vcpkg"
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

# Use a per-install-root downloads directory so parallel builds for different
# triplets (e.g. arm64-android and x64-android) each get their own
# downloads/tools tree and cannot race on tool extraction (e.g. MSYS2 on Windows,
# or any vcpkg_find_acquire_program downloads on macOS/Linux).
# The binary cache (VCPKG_DEFAULT_BINARY_CACHE) remains shared.
DOWNLOADS_ROOT="$INSTALL_ROOT/downloads"
mkdir -p "$DOWNLOADS_ROOT"

# Isolate the git registries cache per install-root to prevent a race condition
# where parallel vcpkg processes (building different arches) collide on the shared
# global cache at ~/.cache/vcpkg/registries. Concurrent git fetch/GC operations on
# that bare repo cause transient "port does not exist" failures.
export X_VCPKG_REGISTRIES_CACHE="$INSTALL_ROOT/registries"
mkdir -p "$X_VCPKG_REGISTRIES_CACHE"

echo "vcpkg: installing packages from $MANIFEST_DIR (triplet=$TRIPLET, install-root=$INSTALL_ROOT)"
echo "vcpkg: downloads=$DOWNLOADS_ROOT"
echo "vcpkg: registries-cache=$X_VCPKG_REGISTRIES_CACHE"
echo "vcpkg: binary-cache=$VCPKG_DEFAULT_BINARY_CACHE"
echo "vcpkg: binary-sources=$VCPKG_BINARY_SOURCES"

# Require a repo-provided overlay triplet for the requested triplet. Every supported build must
# use one of our custom triplet files: they carry CACHE_BUST markers and build flags that feed
# vcpkg's ABI hash, so falling back to vcpkg's built-in triplets would silently produce binaries
# with a different ABI and defeat the cache-busting scheme. Error out instead of using a default.
OVERLAY_TRIPLET_FILE="$OVERLAY_TRIPLETS/$TRIPLET.cmake"
if [ ! -f "$OVERLAY_TRIPLET_FILE" ]; then
    echo "Error: no custom overlay triplet '$TRIPLET' found at $OVERLAY_TRIPLET_FILE"
    echo "This build requires a repo-provided triplet; vcpkg's built-in triplets must not be used."
    exit 1
fi

OVERLAY_ARGS=(--overlay-triplets="$OVERLAY_TRIPLETS")

# Use custom overlay ports from the manifest directory (if present), mirroring
# vcpkg_run_install.bat. This makes Linux/macOS/Android build from the local
# crashpad fork (ports/crashpad) instead of the upstream registry port, so
# platform fixes not yet upstream are picked up on every platform.
OVERLAY_PORTS="$MANIFEST_DIR/ports"
if [ -d "$OVERLAY_PORTS" ]; then
    OVERLAY_ARGS+=(--overlay-ports="$OVERLAY_PORTS")
fi

# vcpkg scrubs the environment for port builds. Forward CRASHPAD_USE_LLD (opt-in
# to link the crashpad handler with lld; needed on hosts whose GNU ld mis-links
# it, e.g. binutils 2.46) so the crashpad portfile can see it. Note: toggling the
# variable does not change the package ABI hash, so a previously cached crashpad
# binary may be restored; clear it from $VCPKG_DEFAULT_BINARY_CACHE to force a
# relink.
if [ -n "${CRASHPAD_USE_LLD:-}" ]; then
    export VCPKG_KEEP_ENV_VARS="CRASHPAD_USE_LLD${VCPKG_KEEP_ENV_VARS:+;$VCPKG_KEEP_ENV_VARS}"
fi

"$VCPKG_EXE" install \
    --triplet "$TRIPLET" \
    --downloads-root="$DOWNLOADS_ROOT" \
    --x-install-root="$INSTALL_ROOT" \
    --x-manifest-root="$MANIFEST_DIR" \
    --x-buildtrees-root="$INSTALL_ROOT/buildtrees" \
    --x-packages-root="$INSTALL_ROOT/packages" \
    "${OVERLAY_ARGS[@]}"
