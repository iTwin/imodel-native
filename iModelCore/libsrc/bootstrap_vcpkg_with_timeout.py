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

TIMEOUT_SECONDS = 300  # 5 minutes

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

    print("Running {} with {}-second timeout...".format(script, TIMEOUT_SECONDS))
    try:
        result = subprocess.run(cmd, cwd=vcpkg_dir, timeout=TIMEOUT_SECONDS)
        sys.exit(result.returncode)
    except subprocess.TimeoutExpired:
        print("Error: bootstrap-vcpkg timed out after {} seconds".format(TIMEOUT_SECONDS), file=sys.stderr)
        sys.exit(1)

if __name__ == "__main__":
    main()
