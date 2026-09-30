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

set -e -o pipefail

OUTPUT="$1"
shift

if [ -z "$OUTPUT" ] || [ $# -lt 2 ]; then
    echo "Usage: $0 <output.a> <input1> <input2> [input3 ...]"
    exit 1
fi

# Deliberately not named TMPDIR: that variable is exported in many environments and is honored
# by mktemp and other child processes, so shadowing it would redirect their scratch files into
# our staging tree. The trap removes the directory on every exit path, including errors.
STAGEDIR=$(mktemp -d)
trap 'rm -rf "$STAGEDIR"' EXIT

# Stage every input in its own subdirectory named after its position on the command line.
# Basenames are not unique (two inputs may both be named "same.o" or "libz.a"), so an index is
# the only collision-free choice.
INDEX=0
for LIB in "$@"; do
    INDEX=$((INDEX + 1))
    INPUTDIR="$STAGEDIR/input_$INDEX"
    mkdir -p "$INPUTDIR"
    case "$LIB" in
        *.o)
            # Raw object file: stage it directly.
            cp "$LIB" "$INPUTDIR/"
            ;;
        *)
            # Static archive: extract its members. Resolve to an absolute path first, since
            # "ar x" runs from inside the staging directory.
            case "$LIB" in
                /*) ARCHIVE="$LIB" ;;
                *)  ARCHIVE="$PWD/$LIB" ;;
            esac
            (cd "$INPUTDIR" && ar x "$ARCHIVE")
            ;;
    esac
done

rm -f "$OUTPUT"

OBJECT_LIST="$STAGEDIR/objects.list"
# sort -z keeps member order deterministic: find returns entries in readdir order, which varies
# between machines and filesystems and would otherwise make the archive byte-differ run to run.
find "$STAGEDIR" -type f -name '*.o' -print0 | LC_ALL=C sort -z > "$OBJECT_LIST"

OBJECTS=()
while IFS= read -r -d '' OBJ; do
    OBJECTS+=("$OBJ")
done < "$OBJECT_LIST"

if [ "${#OBJECTS[@]}" -eq 0 ]; then
    echo "Error: no object files were staged for archive creation"
    exit 1
fi

# Build the archive in one invocation: avoids xargs-driven ARG_MAX chunking, which can
# overwrite same-named members across multiple ar calls. If argv is too large, fail loudly.
ar rcs "$OUTPUT" "${OBJECTS[@]}"
