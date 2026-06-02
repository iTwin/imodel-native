#!/usr/bin/env python3
"""Small git helper used by vcpkg.mke for cross-platform command execution."""

from __future__ import annotations

import argparse
import subprocess
import sys


def clone_vcpkg(dest: str) -> int:
    # Have the URL here so bmake does not rewrite slashes in command args.
    url = "https://github.com/microsoft/vcpkg.git"
    subprocess.check_call(["git", "clone", url, dest])
    return 0


def verify_commit(tag: str, expected_commit: str) -> int:
    head = subprocess.check_output(["git", "rev-parse", "HEAD"], text=True).strip()
    if head == expected_commit:
        return 0

    print(
        f"ERROR: vcpkg tag {tag} does not match expected commit {expected_commit}",
        file=sys.stderr,
    )
    return 1


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description="Git helper for vcpkg.mke")
    subparsers = parser.add_subparsers(dest="command", required=True)

    clone_parser = subparsers.add_parser("clone-vcpkg", help="Clone the vcpkg repository")
    clone_parser.add_argument("dest", help="Destination folder for clone")

    verify_parser = subparsers.add_parser(
        "verify-commit",
        help="Verify current HEAD matches expected commit",
    )
    verify_parser.add_argument("tag", help="Tag name used for error text")
    verify_parser.add_argument("expected_commit", help="Expected commit hash")

    return parser


def main() -> int:
    parser = build_parser()
    args = parser.parse_args()

    if args.command == "clone-vcpkg":
        return clone_vcpkg(args.dest)

    if args.command == "verify-commit":
        return verify_commit(args.tag, args.expected_commit)

    parser.error(f"Unknown command: {args.command}")
    return 2


if __name__ == "__main__":
    raise SystemExit(main())
