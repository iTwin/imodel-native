#!/usr/bin/env python3
#---------------------------------------------------------------------------------------------
#  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
#  See LICENSE.md in the repository root for full copyright notice.
#---------------------------------------------------------------------------------------------
# report_vcpkg_cache.py - Print what kind of vcpkg binary caching is in effect for this build,
# derived from the VCPKG_BINARY_SOURCES environment variable. Informational only; it never changes
# the build. See docs/vcpkg-shared-binary-cache.md in imodel-native-internal for the shared cache.
#
# Classification:
#   AZURE BLOB  - an x-azcopy source is active (the shared cache), with mode READ-ONLY / WRITE-ONLY
#                 / READ/WRITE taken from that source's trailing access flag (default read/write).
#   LOCAL FILE  - the built-in vcpkg 'files' cache (either explicitly, or the vcpkg default when
#                 VCPKG_BINARY_SOURCES is unset).
#   NONE        - all sources cleared and none re-added (caching disabled).
#   UNKNOWN     - some other provider (nuget, http, ...) that this reporter doesn't classify.
#
# Usage: python report_vcpkg_cache.py
#---------------------------------------------------------------------------------------------

import os

_ACCESS_FLAGS = ("read", "write", "readwrite")
_MODE_PRETTY = {"read": "READ-ONLY", "write": "WRITE-ONLY", "readwrite": "READ/WRITE"}


def _mode_of(parts):
    # A source is "<name>,<arg>,...,[access]"; the access flag is optional and, when present, is the
    # last comma-separated token. vcpkg defaults an omitted flag to read/write.
    if len(parts) >= 2 and parts[-1].lower() in _ACCESS_FLAGS:
        return parts[-1].lower()
    return "readwrite"


def describe(raw):
    if raw is None or raw.strip() == "":
        return "vcpkg binary cache: LOCAL FILE (vcpkg default; VCPKG_BINARY_SOURCES not set)"

    # Sources are applied left to right; 'clear' removes everything configured so far (including the
    # default 'files' source), so track state as we go and let the last word win.
    azcopy = None
    files = False
    others = []
    for source in (s.strip() for s in raw.split(";")):
        if source == "":
            continue
        parts = [p.strip() for p in source.split(",")]
        name = parts[0].lower()
        if name == "clear":
            azcopy, files, others = None, False, []
        elif name == "x-azcopy":
            azcopy = parts
        elif name == "files":
            files = True
        else:
            others.append(name)

    if azcopy is not None:
        mode = _mode_of(azcopy)
        return "vcpkg binary cache: AZURE BLOB (x-azcopy), mode={} ({})".format(
            mode, _MODE_PRETTY.get(mode, "UNKNOWN"))
    if files:
        return "vcpkg binary cache: LOCAL FILE (files provider)"
    if others:
        return "vcpkg binary cache: UNKNOWN (providers: {})".format(", ".join(others))
    return "vcpkg binary cache: NONE (caching disabled)"


def main():
    raw = os.environ.get("VCPKG_BINARY_SOURCES")
    print(describe(raw))
    print("VCPKG_BINARY_SOURCES={}".format(raw if raw is not None else "<unset>"))


if __name__ == "__main__":
    main()
