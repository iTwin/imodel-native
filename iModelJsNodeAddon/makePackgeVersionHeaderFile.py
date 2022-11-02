#---------------------------------------------------------------------------------------------
#  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
#  See LICENSE.md in the repository root for full copyright notice.
#---------------------------------------------------------------------------------------------
import os
import sys
import re

if __name__ == '__main__':
    if len(sys.argv) < 3:
        print ("Syntax: " + sys.argv[0] + " filename packageversionfile")
        exit(1)

    filename = sys.argv[1]
    packageVersionFileName = sys.argv[2]

    # The package semantic version number (n.m.p) is stored in a file. Inject this version number into all of the package files that we generate.
    packageVersion = "";
    with open(packageVersionFileName, 'r') as pvf:
        packageVersion = pvf.read()
    packageVersion = packageVersion.strip().lower();

    with open(filename, 'r+') as f:
        str = f.read()
        str = str.replace(r'${PACKAGE_VERSION}', packageVersion)
        f.seek(0)
        f.write(str)
        f.truncate()

    exit(0)
