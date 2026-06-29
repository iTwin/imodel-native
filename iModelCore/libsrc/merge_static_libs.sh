#!/bin/bash
#---------------------------------------------------------------------------------------------
#  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
#  See LICENSE.md in the repository root for full copyright notice.
#---------------------------------------------------------------------------------------------
# Merge multiple static libraries into one using ar. Currently this is only used for compress/libzlib.a, which combines
# the original zlib static library with our minizip additions, and only on Linux, since on Windows and macOS this is a
# trivial operation that doesn't require this script (the .mke files can just list all the .a files and the linker will
# merge them).
# Usage: merge_static_libs.sh <output.a> <input1.a> <input2.a> [input3.a ...]
#---------------------------------------------------------------------------------------------

set -e

OUTPUT="$1"
shift

if [ -z "$OUTPUT" ] || [ $# -lt 2 ]; then
    echo "Usage: $0 <output.a> <input1.a> <input2.a> [input3.a ...]"
    exit 1
fi

TMPDIR=$(mktemp -d)
trap 'rm -rf "$TMPDIR"' EXIT

for LIB in "$@"; do
    # Extract each archive into a unique subdirectory to avoid name collisions
    LIBNAME=$(basename "$LIB" .a)
    mkdir -p "$TMPDIR/$LIBNAME"
    (cd "$TMPDIR/$LIBNAME" && ar x "$LIB")
done

rm -f "$OUTPUT"
ar rcs "$OUTPUT" "$TMPDIR"/*/*.o
