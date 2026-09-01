#!/usr/bin/env python3
"""
Compare LegacyChangesetRegression artifact directories from two builds
(e.g. main vs. branch).

Each artifact directory is the "LegacyChangesetRegression" output folder produced by
the tests, laid out as:

    <root>/
        <imodelId>/
            replay_0000.txt ... replay_NNNN.txt
            resume_ref_tip_NNNN.txt
            resume_tip_NNNN.txt
            briefcase_*.bim

Usage:
    python compare_regression_artifacts.py <baseline_root> <candidate_root> [--verbose]

Exit code 0 = identical, 1 = differences found, 2 = usage/IO error.
"""

import argparse
import difflib
import sys
from pathlib import Path

SECTION_PREFIX = "== "


def split_sections(text: str) -> dict:
    """Split artifact text into {section_name: [lines]}."""
    sections = {}
    current = None
    for line in text.splitlines():
        if line.startswith(SECTION_PREFIX) and line.rstrip().endswith(" =="):
            current = line.strip().strip("= ").strip()
            sections[current] = []
        elif current is not None:
            sections[current].append(line)
    return sections


def collect_artifacts(root: Path) -> dict:
    """Return {relative_posix_path: absolute_path} for all artifact .txt files."""
    return {
        p.relative_to(root).as_posix(): p
        for p in sorted(root.rglob("*.txt"))
    }


def compare_file(rel: str, baseline: Path, candidate: Path, verbose: bool) -> list:
    """Compare one artifact file pair. Returns a list of difference descriptions."""
    base_text = baseline.read_text(encoding="utf-8", errors="replace")
    cand_text = candidate.read_text(encoding="utf-8", errors="replace")
    if base_text == cand_text:
        return []

    diffs = []
    base_sections = split_sections(base_text)
    cand_sections = split_sections(cand_text)

    for name in sorted(set(base_sections) | set(cand_sections)):
        b = base_sections.get(name)
        c = cand_sections.get(name)
        if b == c:
            continue
        if b is None:
            diffs.append(f"  section [{name}]: missing in baseline")
            continue
        if c is None:
            diffs.append(f"  section [{name}]: missing in candidate")
            continue

        diff_lines = list(difflib.unified_diff(b, c, lineterm="",
                                               fromfile=f"baseline/{rel}[{name}]",
                                               tofile=f"candidate/{rel}[{name}]"))
        # count real +/- lines (skip headers)
        changed = sum(1 for l in diff_lines if l[:1] in "+-" and not l.startswith(("+++", "---")))
        diffs.append(f"  section [{name}]: {changed} changed line(s)")
        if verbose:
            diffs.extend("    " + l for l in diff_lines)
        else:
            # show first few changed lines even in terse mode
            shown = [l for l in diff_lines if l[:1] in "+-" and not l.startswith(("+++", "---"))][:6]
            diffs.extend("    " + l for l in shown)
            if changed > 6:
                diffs.append(f"    ... ({changed - 6} more; rerun with --verbose)")
    return diffs


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("baseline", type=Path, help="artifact root from the baseline build (e.g. main)")
    ap.add_argument("candidate", type=Path, help="artifact root from the candidate build (e.g. branch)")
    ap.add_argument("--verbose", action="store_true", help="print full unified diffs of mismatching sections")
    args = ap.parse_args()

    if not args.baseline.is_dir() or not args.candidate.is_dir():
        print("error: both arguments must be existing directories", file=sys.stderr)
        return 2

    base_files = collect_artifacts(args.baseline)
    cand_files = collect_artifacts(args.candidate)

    only_base = sorted(set(base_files) - set(cand_files))
    only_cand = sorted(set(cand_files) - set(base_files))
    common = sorted(set(base_files) & set(cand_files))

    problems = 0

    for rel in only_base:
        print(f"MISSING in candidate: {rel}")
        problems += 1
    for rel in only_cand:
        print(f"MISSING in baseline:  {rel}")
        problems += 1

    identical = 0
    for rel in common:
        diffs = compare_file(rel, base_files[rel], cand_files[rel], args.verbose)
        if not diffs:
            identical += 1
            continue
        problems += 1
        print(f"DIFFERS: {rel}")
        for line in diffs:
            print(line)

    print()
    print(f"Compared {len(common)} common artifact file(s): "
          f"{identical} identical, {len(common) - identical} differing, "
          f"{len(only_base) + len(only_cand)} unmatched.")

    if problems:
        print("RESULT: DIFFERENCES FOUND")
        return 1
    print("RESULT: OK - artifact trees are identical")
    return 0


if __name__ == "__main__":
    sys.exit(main())
