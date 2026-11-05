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
# Usage: makeCurlDef.py <curl_include_dir> <output.def> <archive.lib> [stale_output ...]
#   <curl_include_dir>  directory containing curl.h, easy.h, multi.h, ... (vcpkg include/curl)
#   <archive.lib>       linked archive whose content hash is embedded in the .def
#   <stale_output>      file (e.g. the previously linked iTwinCurl.dll) to delete when the export
#                       list or archive changes, since bmake's DLL rule does not treat the .def as a
#                       prerequisite and would otherwise leave a DLL with the old exports in place
#---------------------------------------------------------------------------------------------
import sys
import os
import re
import glob

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from make_def import write_def


def main():
    if len(sys.argv) < 4:
        sys.stderr.write("Usage: makeCurlDef.py <curl_include_dir> <output.def> <archive.lib> [stale_output ...]\n")
        return 1

    include_dir = sys.argv[1]
    out_def = sys.argv[2]
    archive_path = sys.argv[3]
    stale_outputs = sys.argv[4:]

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

    return write_def(names, archive_path, out_def, stale_outputs)


if __name__ == "__main__":
    sys.exit(main())
