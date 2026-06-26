#!/usr/bin/env python3
#---------------------------------------------------------------------------------------------
#  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
#  See LICENSE.md in the repository root for full copyright notice.
#---------------------------------------------------------------------------------------------
# bootstrap_vcpkg_with_timeout.py - Run bootstrap-vcpkg.bat/.sh with a 5-minute timeout.
# Usage: python bootstrap_vcpkg_with_timeout.py <vcpkg_dir>
#   vcpkg_dir: path to the vcpkg checkout containing bootstrap-vcpkg.bat/.sh
#---------------------------------------------------------------------------------------------

import subprocess
import sys
import os
import time

TIMEOUT_SECONDS = 300  # 5 minutes
MAX_ATTEMPTS = 3       # Retry to tolerate transient network failures (e.g. curl connection reset).
RETRY_DELAY_SECONDS = 5
STAMP_FILENAME = ".bootstrap_success_hash"  # Records the vcpkg commit a successful bootstrap was run for.

def get_vcpkg_hash(vcpkg_dir):
    """Return the current vcpkg checkout commit hash, or None if it can't be determined."""
    try:
        result = subprocess.run(
            ["git", "rev-parse", "HEAD"],
            cwd=vcpkg_dir,
            stdout=subprocess.PIPE,
            stderr=subprocess.DEVNULL,
            universal_newlines=True,
            timeout=30)
    except (OSError, subprocess.SubprocessError):
        return None
    if result.returncode != 0:
        return None
    commit = result.stdout.strip()
    return commit or None

def get_vcpkg_executable(vcpkg_dir):
    name = "vcpkg.exe" if sys.platform == "win32" else "vcpkg"
    return os.path.join(vcpkg_dir, name)

def main():
    if len(sys.argv) < 2:
        print("Usage: {} <vcpkg_dir>".format(sys.argv[0]), file=sys.stderr)
        sys.exit(1)

    vcpkg_dir = sys.argv[1]

    if sys.platform == "win32":
        script = os.path.join(vcpkg_dir, "bootstrap-vcpkg.bat")
        cmd = [script]
    else:
        script = os.path.join(vcpkg_dir, "bootstrap-vcpkg.sh")
        cmd = [script]

    if not os.path.isfile(script):
        print("Error: bootstrap script not found: {}".format(script), file=sys.stderr)
        sys.exit(1)

    stamp_path = os.path.join(vcpkg_dir, STAMP_FILENAME)
    current_hash = get_vcpkg_hash(vcpkg_dir)

    # Skip bootstrap if we already succeeded for this exact vcpkg commit and the
    # vcpkg executable is still present.
    if current_hash and os.path.isfile(get_vcpkg_executable(vcpkg_dir)):
        try:
            with open(stamp_path) as stamp_file:
                recorded_hash = stamp_file.read().strip()
        except (OSError, IOError):
            recorded_hash = None
        if recorded_hash == current_hash:
            print("vcpkg already bootstrapped for commit {}; skipping.".format(current_hash))
            sys.exit(0)

    for attempt in range(1, MAX_ATTEMPTS + 1):
        print("Running {} with {}-second timeout (attempt {} of {})...".format(
            script, TIMEOUT_SECONDS, attempt, MAX_ATTEMPTS))
        try:
            result = subprocess.run(cmd, cwd=vcpkg_dir, timeout=TIMEOUT_SECONDS)
            if result.returncode == 0:
                if current_hash:
                    try:
                        with open(stamp_path, "w") as stamp_file:
                            stamp_file.write(current_hash)
                    except (OSError, IOError) as ex:
                        print("Warning: could not write bootstrap stamp file {}: {}".format(stamp_path, ex), file=sys.stderr)
                sys.exit(0)
            print("Error: bootstrap-vcpkg exited with code {}.".format(result.returncode), file=sys.stderr)
        except subprocess.TimeoutExpired:
            print("Error: bootstrap-vcpkg timed out after {} seconds.".format(TIMEOUT_SECONDS), file=sys.stderr)

        if attempt < MAX_ATTEMPTS:
            print("Retrying in {} seconds...".format(RETRY_DELAY_SECONDS), file=sys.stderr)
            time.sleep(RETRY_DELAY_SECONDS)

    print("Error: bootstrap-vcpkg failed after {} attempts.".format(MAX_ATTEMPTS), file=sys.stderr)
    sys.exit(1)

if __name__ == "__main__":
    main()
