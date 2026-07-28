#!/bin/bash
#---------------------------------------------------------------------------------------------
#  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
#  See LICENSE.md in the repository root for full copyright notice.
#---------------------------------------------------------------------------------------------
# Merge multiple static libraries (and/or raw object files) into one using ar. Used for
# compress/libzlib.a (zlib + our minizip additions) and pugixml/libiTwinPugixml.a (our
# BePugiXml.o + the vcpkg pugixml archive), and only on Linux, since on Windows and macOS this
# is a trivial operation that doesn't require this script (the .mke files can just list all the
# .a/.o files and the linker will merge them).
# Usage: merge_static_libs.sh <output.a> <input1> <input2> [input3 ...]
#   Each input may be a static archive (.a) or a raw object file (.o). This script only runs
#   on Linux/Android, where object files always use the .o extension, so .obj is not accepted.
#---------------------------------------------------------------------------------------------

set -e

OUTPUT="$1"
shift

if [ -z "$OUTPUT" ] || [ $# -lt 2 ]; then
    echo "Usage: $0 <output.a> <input1> <input2> [input3 ...]"
    exit 1
fi

TMPDIR=$(mktemp -d)
trap 'rm -rf "$TMPDIR"' EXIT

for LIB in "$@"; do
    case "$LIB" in
        *.o)
            # Raw object file: stage it directly (into a unique subdirectory to avoid collisions).
            OBJDIR="$TMPDIR/obj_$(basename "$LIB")"
            mkdir -p "$OBJDIR"
            cp "$LIB" "$OBJDIR/"
            ;;
        *)
            # Extract each archive into a unique subdirectory to avoid name collisions
            LIBNAME=$(basename "$LIB" .a)
            mkdir -p "$TMPDIR/$LIBNAME"
            (cd "$TMPDIR/$LIBNAME" && ar x "$LIB")
            ;;
    esac
done

rm -f "$OUTPUT"
find "$TMPDIR" -type f -name '*.o' -print0 | xargs -0 ar rcs "$OUTPUT"
