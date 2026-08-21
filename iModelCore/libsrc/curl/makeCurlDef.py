#---------------------------------------------------------------------------------------------
#  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
#  See LICENSE.md in the repository root for full copyright notice.
#---------------------------------------------------------------------------------------------
# Generate a Windows module-definition (.def) EXPORTS list for iTwinCurl.dll from curl's public
# headers.
#
# The vcpkg static Windows triplet compiles curl with CURL_STATICLIB defined, so the archive
# carries no __declspec(dllexport) markers.  When we re-link iTwinCurl.dll from that archive the
# export set must therefore come from us.  Every public curl API function is declared with the
# CURL_EXTERN marker, so we scan the delivered headers and export exactly those `curl_*`
# functions.  Deriving the list from the headers means a curl version bump needs no manual .def
# maintenance (unlike the hand-maintained OpenSSL .def).
#
# Usage: makeCurlDef.py <curl_include_dir> <output.def> [stale_output ...]
#   <curl_include_dir>  directory containing curl.h, easy.h, multi.h, ... (vcpkg include/curl)
#   <stale_output>      file (e.g. the previously linked iTwinCurl.dll) to delete when the export
#                       list changes, since bmake's DLL rule does not treat the .def as a
#                       prerequisite and would otherwise leave a DLL with the old exports in place
#---------------------------------------------------------------------------------------------
import sys
import os
import re
import glob


def main():
    if len(sys.argv) < 3:
        sys.stderr.write("Usage: makeCurlDef.py <curl_include_dir> <output.def> [stale_output ...]\n")
        return 1

    include_dir = sys.argv[1]
    out_def = sys.argv[2]
    stale_outputs = sys.argv[3:]

    headers = sorted(glob.glob(os.path.join(include_dir, "*.h")))
    if not headers:
        sys.stderr.write("error: no curl headers found in %s\n" % include_dir)
        return 1

    text = []
    for header in headers:
        with open(header, "r", encoding="utf-8", errors="replace") as f:
            text.append(f.read())
    src = "\n".join(text)

    # Strip comments and string literals first so their contents (e.g. a "Use curl_mime_init()"
    # deprecation message) cannot be mistaken for a real exported symbol.
    src = re.sub(r"/\*.*?\*/", " ", src, flags=re.S)
    src = re.sub(r"//[^\n]*", " ", src)
    src = re.sub(r'"(?:\\.|[^"\\])*"', " ", src, flags=re.S)

    # After each CURL_EXTERN, the exported function is the first lowercase `curl_*` identifier
    # immediately followed by `(`.  `[^;{]` keeps the match inside a single declaration so we do
    # not run past its terminating `;` into the next one.
    names = set(re.findall(r"CURL_EXTERN\b[^;{]*?\b(curl_[a-z0-9_]+)\s*\(", src, flags=re.S))

    if not names:
        sys.stderr.write("error: no CURL_EXTERN exports found under %s\n" % include_dir)
        return 1

    lines = ["EXPORTS"] + ["    %s" % name for name in sorted(names)]
    data = "".join("%s\r\n" % line for line in lines).encode("utf-8")

    # This runs on every build, so leave an unchanged .def alone rather than re-stamping its
    # timestamp and forcing a needless re-link of iTwinCurl.dll.
    if os.path.exists(out_def):
        with open(out_def, "rb") as f:
            if f.read() == data:
                return 0

    # The export list changed, so anything previously linked from the old .def is stale. Delete
    # those outputs *before* committing the new .def: on Windows the delete can fail while the
    # DLL is still loaded by another process, and leaving the old .def in place means the next
    # build again sees a changed export list and retries the deletion.
    for stale in stale_outputs:
        if os.path.exists(stale):
            try:
                os.remove(stale)
            except OSError as ex:
                sys.stderr.write("error: cannot delete stale output %s: %s\n" % (stale, ex))
                return 1

    with open(out_def, "wb") as f:
        f.write(data)

    return 0


if __name__ == "__main__":
    sys.exit(main())
