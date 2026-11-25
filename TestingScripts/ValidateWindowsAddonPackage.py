# Copyright (c) Bentley Systems, Incorporated. All rights reserved.
# See LICENSE.md in the repository root for full copyright notice.

"""Check the PE tables and packaged DLLs of a privately linked Windows addon."""

import argparse
from pathlib import Path
import re
import subprocess


def validate(package, pe_tables, expected_exports, external_napi=False):
    if not re.search(r"^Format: COFF-x86-64$", pe_tables, re.MULTILINE):
        return ["imodeljs.node is not an x64 PE module"]

    tables = {"Import": [], "DelayImport": [], "Export": []}
    for kind, body in re.findall(
        r"^(Import|DelayImport|Export) \{\n(.*?)^\}", pe_tables, re.MULTILINE | re.DOTALL
    ):
        name = re.search(r"^  Name: (.*)$", body, re.MULTILINE)
        if not name or not name[1].strip():
            return [f"Unnamed {kind} in imodeljs.node"]
        tables[kind].append(name[1].strip())

    def is_itwin_dll(name):
        return re.fullmatch(r"itwin.*\.dll", name, re.IGNORECASE) is not None

    errors = []
    for kind, label, expected in (
        ("Import", "normal", ["itwinnapi.dll"] if external_napi else []),
        ("DelayImport", "delay", [] if external_napi else ["itwinnapi.dll"]),
    ):
        imports = [name for name in tables[kind] if is_itwin_dll(name)]
        if [name.lower() for name in imports] != expected:
            errors.append(f"Expected {', '.join(expected) or 'no'} iTwin {label} imports; "
                          f"found: {', '.join(imports) or 'none'}")
    if sorted(tables["Export"]) != sorted(expected_exports):
        errors.append(f"Expected exports {sorted(expected_exports)}; found {tables['Export']}")
    packaged_dlls = sorted(
        path.relative_to(package) for path in package.rglob("*")
        if is_itwin_dll(path.name) and path.is_file()
    )
    expected_dlls = ["itwinnapi.dll"] if external_napi else []
    if [str(path).lower() for path in packaged_dlls] != expected_dlls:
        errors.append(f"Expected packaged iTwin DLLs: {', '.join(expected_dlls) or 'none'}; "
                      f"found: {', '.join(map(str, packaged_dlls)) or 'none'}")
    return errors


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("package", type=Path, help="directory containing the final imodeljs.node")
    parser.add_argument("--llvm-readobj", default="llvm-readobj", help="path to llvm-readobj")
    parser.add_argument("--external-napi", action="store_true",
                        help="expect iTwinNapi.dll as a normal import and sibling file")
    parser.add_argument("--export", action="append", required=True, dest="exports",
                        help="expected exported symbol; repeat for each export")
    args = parser.parse_args()
    addon = args.package.resolve() / "imodeljs.node"
    if not addon.is_file():
        parser.exit(1, f"Addon not found: {addon}\n")
    try:
        pe_tables = subprocess.check_output(
            [args.llvm_readobj, "--coff-imports", "--coff-exports", str(addon)], text=True
        )
    except (OSError, subprocess.CalledProcessError) as error:
        parser.exit(1, f"Could not inspect {addon}: {error}\n")

    errors = validate(args.package, pe_tables, args.exports, args.external_napi)
    if errors:
        parser.exit(1, "\n".join(errors) + "\n")
    print("Windows addon PE and package checks passed.")


if __name__ == "__main__":
    main()
