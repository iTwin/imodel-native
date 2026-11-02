#---------------------------------------------------------------------------------------------
#  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
#  See LICENSE.md in the repository root for full copyright notice.
#---------------------------------------------------------------------------------------------
# Generate a Windows module-definition (.def) EXPORTS list for iTwinFreetype2.dll from FreeType's
# public headers.
#
# The vcpkg static Windows triplet builds FreeType without DLL_EXPORT, so the archive carries no
# __declspec(dllexport) markers.  When we re-link iTwinFreetype2.dll from that archive the export
# set must therefore come from us.  Every public FreeType API function is declared as
# `FT_EXPORT( <return type> ) <name>( ... )`, so we scan the delivered headers and export exactly
# those functions.  Deriving the list from the headers means a FreeType version bump needs no
# manual .def maintenance.
#
# Usage: makeFreetypeDef.py <freetype_include_dir> <output.def> <archive.lib> [stale_output ...]
#   <freetype_include_dir>  directory containing freetype.h, ftglyph.h, ... (vcpkg include/freetype)
#   <archive.lib>           linked archive whose content hash is embedded in the .def
#   <stale_output>          file (e.g. the previously linked iTwinFreetype2.dll) to delete when the
#                           exports or archive change, since bmake does not treat the .def as a
#                           prerequisite and would otherwise leave a DLL with the old exports in place
#---------------------------------------------------------------------------------------------
import sys
import os
import re
import glob

sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from make_def import write_def

# Headers whose FT_EXPORT functions are not compiled into the Windows library.
#   ftmac.h - Mac OS resource-fork font access; only built on Apple platforms.
EXCLUDED_HEADERS = {"ftmac.h"}


def main():
    if len(sys.argv) < 4:
        sys.stderr.write("Usage: makeFreetypeDef.py <freetype_include_dir> <output.def> <archive.lib> [stale_output ...]\n")
        return 1

    include_dir = sys.argv[1]
    out_def = sys.argv[2]
    archive_path = sys.argv[3]
    stale_outputs = sys.argv[4:]

    # Only the top-level public headers declare API functions; config/ only defines the macros.
    headers = sorted(h for h in glob.glob(os.path.join(include_dir, "*.h"))
                     if os.path.basename(h) not in EXCLUDED_HEADERS)
    if not headers:
        sys.stderr.write("error: no FreeType headers found in %s\n" % include_dir)
        return 1

    text = []
    for header in headers:
        with open(header, "r", encoding="utf-8", errors="replace") as f:
            text.append(f.read())
    src = "\n".join(text)

    # Strip comments and string literals first so the API documentation (which quotes
    # declarations) cannot be mistaken for a real exported symbol.
    src = re.sub(r"/\*.*?\*/", " ", src, flags=re.S)
    src = re.sub(r"//[^\n]*", " ", src)
    src = re.sub(r'"(?:\\.|[^"\\])*"', " ", src, flags=re.S)

    # FT_EXPORT( <type> ) <name>( ... ). The \b keeps FT_EXPORT_DEF / FT_EXPORT_VAR from matching.
    names = set(re.findall(r"\bFT_EXPORT\s*\(\s*[^)]*\)\s*(\w+)\s*\(", src))

    if not names:
        sys.stderr.write("error: no FT_EXPORT functions found under %s\n" % include_dir)
        return 1

    return write_def(names, archive_path, out_def, stale_outputs)


if __name__ == "__main__":
    sys.exit(main())
