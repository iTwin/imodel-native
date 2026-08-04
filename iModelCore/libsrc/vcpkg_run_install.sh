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

# --------------------------------------------------------------------------------------
# Persistent per-user vcpkg cache base.
#
# The downloads/tools tree, the registries git repo, and the binary "archives" cache all live
# under this base. It is placed under the user cache directory ($XDG_CACHE_HOME or ~/.cache)
# rather than under VCPKG_ROOT for two reasons:
#   * Writability: VCPKG_ROOT / IMODEL_VCPKG_ROOT may be a shared, read-only checkout that we
#     must not require to be writable.
#   * Persistence: the user cache is not under OutRoot, so a clean build does not wipe it. Tools,
#     source archives, and the shallow registry repo are downloaded/extracted once and reused
#     across clean builds instead of being re-extracted every time.
# If the user cache location cannot be created (e.g. a locked-down agent), fall back to a
# directory under INSTALL_ROOT so the build still works.
# --------------------------------------------------------------------------------------
VCPKG_CACHE_BASE="${XDG_CACHE_HOME:-$HOME/.cache}/Bentley/vcpkg"
if ! mkdir -p "$VCPKG_CACHE_BASE" 2>/dev/null; then
    echo "vcpkg: persistent cache base '$VCPKG_CACHE_BASE' unavailable; falling back to INSTALL_ROOT"
    VCPKG_CACHE_BASE="$INSTALL_ROOT/vcpkg-cache"
    mkdir -p "$VCPKG_CACHE_BASE"
fi

# Use a persistent local binary cache by default to avoid rebuilding heavy ports
# (for example, crashpad) across builds. Allow callers to override.
if [ -z "${VCPKG_DEFAULT_BINARY_CACHE:-}" ]; then
    export VCPKG_DEFAULT_BINARY_CACHE="$VCPKG_CACHE_BASE/archives"
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

# Per-platform downloads/tools tree and registries git repo. The platform key is the first two
# triplet tokens (e.g. arm64-android, x64-android), so parallel builds of different arches get
# separate trees and cannot race on tool extraction or registry git fetch/GC. Triplet variants
# of the same platform and configs (debug/release) intentionally share one persistent cache; the
# cross-process lock below is keyed on these directories, so same-platform runs are serialized
# and a VCPKG_DOWNLOADS / X_VCPKG_REGISTRIES_CACHE override that points two runs at one shared
# directory is serialized too. Honor those caller-provided escape hatches when set.
VCPKG_PLATFORM_KEY="$(echo "$TRIPLET" | cut -d- -f1,2)"
DOWNLOADS_ROOT="${VCPKG_DOWNLOADS:-$VCPKG_CACHE_BASE/downloads/$VCPKG_PLATFORM_KEY}"
mkdir -p "$DOWNLOADS_ROOT"

if [ -z "${X_VCPKG_REGISTRIES_CACHE:-}" ]; then
    export X_VCPKG_REGISTRIES_CACHE="$DOWNLOADS_ROOT/registries"
fi
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

# --------------------------------------------------------------------------------------
# Cross-process lock around the install.
#
# Two runs corrupt each other only when they share the mutable state vcpkg writes during an
# install: the downloads/tools tree ($DOWNLOADS_ROOT) and the registries git repo
# ($X_VCPKG_REGISTRIES_CACHE). vcpkg.PartFile.xml only serializes consumers within a single
# BentleyBuild graph, so two installs can still overlap (static + dynamic, two pipelines, or two
# users) and race on that state (see PR #1497: concurrent registry fetch/GC).
#
# Lock the actual mutable directories, not a triplet-derived name. Those paths can be redirected
# independently (VCPKG_DOWNLOADS / X_VCPKG_REGISTRIES_CACHE) and shared across arches or users, so
# a triplet+cache-base lock name can both miss real sharing (two arches pointed at one shared
# downloads tree get different lock files) and falsely serialize unrelated runs. The lock file
# lives inside each directory, so every run pointing at the same directory contends on the same
# inode regardless of triplet, user, or how the path was spelled. Acquire the canonical,
# de-duplicated directories in sorted order so runs with overlapping sets can never deadlock.
# Use an auto-releasing advisory lock so a crash, SIGKILL, or power loss cannot leave a stale lock
# behind that wedges every later install: flock(1) on Linux, Perl's flock() on stock macOS (which
# has no flock(1) but always ships Perl). Fail loudly if neither is available rather than fall back
# to an unrecoverable directory lock.
# --------------------------------------------------------------------------------------
# Build the install command once so every lock backend wraps the same invocation.
VCPKG_CMD=("$VCPKG_EXE" install
    --triplet "$TRIPLET"
    --downloads-root="$DOWNLOADS_ROOT"
    --x-install-root="$INSTALL_ROOT"
    --x-manifest-root="$MANIFEST_DIR"
    --x-buildtrees-root="$INSTALL_ROOT/buildtrees"
    --x-packages-root="$INSTALL_ROOT/packages"
    "${OVERLAY_ARGS[@]}")

# Canonicalize the directories this install mutates, then identify their lock files by device and
# inode. De-dup is required: the two paths can name the same directory (including through aliases
# that differ only by case on macOS), and locking the same file twice from one process would
# self-deadlock. Sorting by filesystem identity instead of path spelling gives every process the
# same global acquire order, preventing AB/BA deadlock between runs whose directory sets overlap.
_canon_dirs=()
for _d in "$DOWNLOADS_ROOT" "$X_VCPKG_REGISTRIES_CACHE"; do
    _canon="$(cd "$_d" 2>/dev/null && pwd -P)" || _canon="$_d"
    _canon_dirs+=("$_canon")
done

LOCK_FILES=()
_lock_keys=()
for _d in "${_canon_dirs[@]}"; do
    _f="$_d/.vcpkg-install.lock"
    : >> "$_f"
    if _key="$(stat -Lc '%d:%i' "$_f" 2>/dev/null)"; then
        : # GNU stat (Linux)
    elif _key="$(stat -Lf '%d:%i' "$_f" 2>/dev/null)"; then
        : # BSD stat (macOS)
    else
        echo "Error: failed to identify vcpkg install lock $_f" >&2
        exit 1
    fi
    if [ "${_lock_keys[0]:-}" != "$_key" ]; then
        _lock_keys+=("$_key")
        LOCK_FILES+=("$_f")
    fi
done

# There are at most two lock files. Put them in a path-independent global order using their
# device/inode keys so aliases cannot cause different processes to acquire the same pair backward.
if [ "${#LOCK_FILES[@]}" -eq 2 ] &&
   [ "$(printf '%s\n%s\n' "${_lock_keys[0]}" "${_lock_keys[1]}" | LC_ALL=C sort | head -n 1)" = "${_lock_keys[1]}" ]; then
    _tmp="${LOCK_FILES[0]}"; LOCK_FILES[0]="${LOCK_FILES[1]}"; LOCK_FILES[1]="$_tmp"
fi

if command -v flock >/dev/null 2>&1; then
    # Hold each lock on its own fd for the lifetime of this process; the kernel drops them on exit.
    _fd=9
    for _f in "${LOCK_FILES[@]}"; do
        # Computed fd numbers need eval on bash 3.2 (stock macOS), which lacks the {var}> form.
        eval "exec $_fd>\"\$_f\""
        if ! flock "$_fd"; then
            echo "Error: failed to acquire vcpkg install lock $_f"
            exit 1
        fi
        _fd=$((_fd - 1))
    done
    "${VCPKG_CMD[@]}"
elif command -v perl >/dev/null 2>&1; then
    # Perl's flock() is advisory and released by the kernel when the holding process dies, so a
    # killed or crashed build never leaves a stale lock (unlike a directory lock). It locks every
    # file in the given sorted order, then EXECs the install in place -- after clearing
    # close-on-exec on the lock fds so the install inherits them -- making the install itself the
    # lock holder. That closes the gap where a separate wrapper could be killed while the install
    # kept mutating the caches: the lock is now held for exactly the install's lifetime, and the
    # install's exit status becomes this script's exit status.
    perl -e '
        use Fcntl qw(F_GETFD F_SETFD FD_CLOEXEC);
        my @locks;
        while (@ARGV && $ARGV[0] ne "--") { push @locks, shift @ARGV }
        shift @ARGV if @ARGV;
        my @held;
        for my $f (@locks) {
            open(my $fh, ">>", $f) or die "vcpkg lock: cannot open $f: $!\n";
            flock($fh, 2)          or die "vcpkg lock: cannot lock $f: $!\n";
            my $fl = fcntl($fh, F_GETFD, 0);
            defined($fl)                             or die "vcpkg lock: F_GETFD $f: $!\n";
            fcntl($fh, F_SETFD, $fl & ~FD_CLOEXEC)   or die "vcpkg lock: clear cloexec $f: $!\n";
            push @held, $fh;
        }
        exec { $ARGV[0] } @ARGV;
        die "vcpkg lock: cannot exec $ARGV[0]: $!\n";
    ' "${LOCK_FILES[@]}" -- "${VCPKG_CMD[@]}"
else
    echo "Error: no auto-releasing lock backend available (need 'flock' or 'perl')." >&2
    echo "Refusing to use an unrecoverable directory lock; install 'flock' or 'perl' and retry." >&2
    exit 1
fi
