#!/bin/bash
#---------------------------------------------------------------------------------------------
#  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
#  See LICENSE.md in the repository root for full copyright notice.
#---------------------------------------------------------------------------------------------
# Wrapper script for vcpkg install, invoked from .mke build files.
# Customize IMODEL_VCPKG_ROOT for developer or CI environments.
#
# Usage: vcpkg_run_install.sh <manifest_dir> <install_root> <triplet> [--only-downloads] [--disable-binary-cache] [--disable-compiler-tracking]
#   manifest_dir: Directory containing vcpkg.json
#   install_root: Where vcpkg_installed/<triplet> output goes (e.g., $OutRoot/vcpkg)
#   triplet:      vcpkg triplet (e.g., arm64-osx, x64-linux)
#---------------------------------------------------------------------------------------------

set -e

if [ "$#" -lt 3 ]; then
    echo "Usage: $0 <manifest_dir> <install_root> <triplet> [--only-downloads] [--disable-binary-cache] [--disable-compiler-tracking]"
    exit 1
fi

MANIFEST_DIR="$1"
INSTALL_ROOT="$2"
TRIPLET="$3"
shift 3

ONLY_DOWNLOADS=0
DISABLE_BINARY_CACHE=0
DISABLE_COMPILER_TRACKING=0
while [ "$#" -gt 0 ]; do
    case "$1" in
        --only-downloads)
            ONLY_DOWNLOADS=1
            ;;
        --disable-binary-cache)
            DISABLE_BINARY_CACHE=1
            ;;
        --disable-compiler-tracking)
            DISABLE_COMPILER_TRACKING=1
            ;;
        *)
            echo "Unknown option: $1"
            echo "Usage: $0 <manifest_dir> <install_root> <triplet> [--only-downloads] [--disable-binary-cache] [--disable-compiler-tracking]"
            exit 1
            ;;
    esac
    shift
done

if [ -z "$MANIFEST_DIR" ] || [ -z "$INSTALL_ROOT" ] || [ -z "$TRIPLET" ]; then
    echo "Usage: $0 <manifest_dir> <install_root> <triplet> [--only-downloads] [--disable-binary-cache] [--disable-compiler-tracking]"
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

# Resolve both mutable cache directories to canonical, symlink-free paths and hand those to vcpkg.
# The locks below are keyed on the same canonical paths, so retargeting a symlink after this point
# cannot send vcpkg into a directory other than the one this run locked.
DOWNLOADS_ROOT="$(cd "$DOWNLOADS_ROOT" && pwd -P)"
X_VCPKG_REGISTRIES_CACHE="$(cd "$X_VCPKG_REGISTRIES_CACHE" && pwd -P)"
export X_VCPKG_REGISTRIES_CACHE

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

# vcpkg probes the target triplet's compiler only to hash it into the package ABI, which fails when
# the host cannot compile for that triplet (e.g. a source-only scan of a foreign triplet). Shadow
# the repo triplet with a generated one that opts out; the repo triplet still supplies every build
# setting, so nothing else about the invocation changes.
if [ "$DISABLE_COMPILER_TRACKING" -eq 1 ]; then
    GENERATED_TRIPLETS="$INSTALL_ROOT/generated-triplets"
    mkdir -p "$GENERATED_TRIPLETS"
    # include() of a relative path would resolve against the generated file's directory.
    INCLUDED_TRIPLET_FILE="$(cd "$(dirname "$OVERLAY_TRIPLET_FILE")" && pwd -P)/$(basename "$OVERLAY_TRIPLET_FILE")"
    {
        echo "include(\"$INCLUDED_TRIPLET_FILE\")"
        echo "set(VCPKG_DISABLE_COMPILER_TRACKING ON)"
    } > "$GENERATED_TRIPLETS/$TRIPLET.cmake"
    OVERLAY_ARGS=(--overlay-triplets="$GENERATED_TRIPLETS")
    echo "vcpkg: compiler tracking disabled; using generated triplet under $GENERATED_TRIPLETS"
fi

# Use custom overlay ports from the manifest directory (if present), mirroring
# vcpkg_run_install.ps1. This makes Linux/macOS/Android build from the local
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
# Lock the actual mutable directories, not a triplet-derived name: those paths can be redirected
# independently (VCPKG_DOWNLOADS / X_VCPKG_REGISTRIES_CACHE) and shared across arches or users, so
# a triplet+cache-base lock name can both miss real sharing (two arches pointed at one shared
# downloads tree get different locks) and falsely serialize unrelated runs.
# The lock objects themselves are regular files in a stable namespace ($VCPKG_CACHE_BASE/locks),
# outside the directories they guard, named from the canonical path of the guarded directory.
# vcpkg is given that same canonical path, so the lock domain and the install domain cannot be
# split by a cache cleanup, a symlink retarget, or a directory rename: whatever happens to the
# original alias, both the lock and vcpkg refer to the resolved directory.
# The lock files are opened read-write because Linux emulates flock on NFS with POSIX locks, which
# require a writable descriptor and cannot be taken on a directory at all -- an enterprise
# $HOME/.cache or a cache override on NFS would otherwise fail to lock.
# Acquire the de-duplicated locks in sorted order so runs with overlapping sets can never deadlock.
# Use an auto-releasing advisory lock so a crash, SIGKILL, or power loss cannot leave a stale lock
# behind that wedges every later install: flock(1) on Linux, Perl's flock() on stock macOS (which
# has no flock(1) but always ships Perl). Fail loudly if neither is available rather than fall back
# to an unrecoverable lock.
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

if [ "$ONLY_DOWNLOADS" -eq 1 ]; then
    VCPKG_CMD+=(--only-downloads)
    echo "vcpkg: source download mode enabled; extracted sources will remain under $INSTALL_ROOT/buildtrees"
fi

if [ "$DISABLE_BINARY_CACHE" -eq 1 ]; then
    VCPKG_CMD+=(--binarysource=clear)
    echo "vcpkg: binary cache disabled for this invocation"
fi

# Print "device:inode" for an open descriptor. GNU stat follows /dev/fd to fstat the open file
# (Linux); BSD stat describes the /dev/fd entry itself, so fall back to Perl's fstat (macOS).
_fd_identity() {
    stat -Lc '%d:%i' "/dev/fd/$1" 2>/dev/null && return 0
    command -v perl >/dev/null 2>&1 || return 1
    perl -e 'open(my $fh, "<&=$ARGV[0]") or exit 1; my @st = stat($fh) or exit 1; print "$st[0]:$st[1]";' "$1"
}

# Print "device:inode" for a path.
_path_identity() {
    stat -Lc '%d:%i' "$1" 2>/dev/null || stat -Lf '%d:%i' "$1" 2>/dev/null
}

VCPKG_LOCK_DIR="$VCPKG_CACHE_BASE/locks"
mkdir -p "$VCPKG_LOCK_DIR"

# De-dup because the two cache paths can name the same directory, and locking one file twice from
# a single process would self-deadlock. Sorting the canonical paths gives every process the same
# global acquire order, preventing AB/BA deadlock between runs whose directory sets overlap.
_lock_paths=()
for _d in "$DOWNLOADS_ROOT" "$X_VCPKG_REGISTRIES_CACHE"; do
    if [ "${_lock_paths[0]:-}" != "$_d" ]; then
        _lock_paths+=("$_d")
    fi
done
if [ "${#_lock_paths[@]}" -eq 2 ] &&
   [ "$(printf '%s\n%s\n' "${_lock_paths[0]}" "${_lock_paths[1]}" | LC_ALL=C sort | head -n 1)" = "${_lock_paths[1]}" ]; then
    _tmp="${_lock_paths[0]}"; _lock_paths[0]="${_lock_paths[1]}"; _lock_paths[1]="$_tmp"
fi

LOCK_DIRS=()
LOCK_FILES=()
LOCK_FDS=()
_lock_keys=()
_fd=9
for _d in "${_lock_paths[@]}"; do
    # Derive the lock name from a checksum of the canonical path: filesystem-safe and bounded in
    # length however deep the cache lives. A checksum collision can only over-serialize.
    _lock_file="$VCPKG_LOCK_DIR/$(printf '%s' "$_d" | cksum | tr -cd '0-9').lock"
    # <> is O_RDWR|O_CREAT, which both creates the lock file and keeps flock usable over NFS.
    # Computed fd numbers need eval on bash 3.2 (stock macOS), which lacks the {var}> form.
    eval "exec $_fd<>\"\$_lock_file\""
    # Keeping this descriptor open avoids a stat/reopen race; record its identity so the backends
    # can detect a lock file that was replaced between the open and the successful lock.
    if ! _key="$(_fd_identity "$_fd")"; then
        echo "Error: failed to identify vcpkg install lock file $_lock_file" >&2
        exit 1
    fi
    LOCK_DIRS+=("$_d")
    LOCK_FILES+=("$_lock_file")
    LOCK_FDS+=("$_fd")
    _lock_keys+=("$_key")
    _fd=$((_fd - 1))
done

if command -v flock >/dev/null 2>&1; then
    # Lock the descriptors opened above; the kernel drops them on exit. vcpkg inherits them, so a
    # killed wrapper still leaves the locks held for as long as the install is mutating the caches.
    for _i in "${!LOCK_FDS[@]}"; do
        if ! flock "${LOCK_FDS[$_i]}"; then
            echo "Error: failed to acquire vcpkg install lock ${LOCK_FILES[$_i]} for ${LOCK_DIRS[$_i]}" >&2
            exit 1
        fi
        if [ "$(_path_identity "${LOCK_FILES[$_i]}")" != "${_lock_keys[$_i]}" ]; then
            echo "Error: vcpkg install lock file was replaced while acquiring it: ${LOCK_FILES[$_i]}" >&2
            exit 1
        fi
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
    PERL_LOCK_ARGS=()
    for _i in "${!LOCK_FDS[@]}"; do
        PERL_LOCK_ARGS+=("${LOCK_FDS[$_i]}" "${LOCK_FILES[$_i]}" "${_lock_keys[$_i]}")
    done
    perl -e '
        use Fcntl qw(F_GETFD F_SETFD FD_CLOEXEC);
        my @locks;
        while (@ARGV && $ARGV[0] ne "--") {
            push @locks, [ splice(@ARGV, 0, 3) ];
        }
        shift @ARGV if @ARGV;
        my @held;
        for my $lock (@locks) {
            my ($fd, $file, $key) = @$lock;
            open(my $fh, "+<&=$fd") or die "vcpkg lock: cannot use fd $fd for $file: $!\n";
            flock($fh, 2)           or die "vcpkg lock: cannot lock $file: $!\n";
            my @st = stat($file);
            @st && "$st[0]:$st[1]" eq $key
                or die "vcpkg lock: lock file was replaced while acquiring it: $file\n";
            my $fl = fcntl($fh, F_GETFD, 0);
            defined($fl)                             or die "vcpkg lock: F_GETFD $file: $!\n";
            fcntl($fh, F_SETFD, $fl & ~FD_CLOEXEC)   or die "vcpkg lock: clear cloexec $file: $!\n";
            push @held, $fh;
        }
        exec { $ARGV[0] } @ARGV;
        die "vcpkg lock: cannot exec $ARGV[0]: $!\n";
    ' "${PERL_LOCK_ARGS[@]}" -- "${VCPKG_CMD[@]}"
else
    echo "Error: no auto-releasing lock backend available (need 'flock' or 'perl')." >&2
    echo "Refusing to use an unrecoverable lock-file lock; install 'flock' or 'perl' and retry." >&2
    exit 1
fi
