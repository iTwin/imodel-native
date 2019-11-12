#---------------------------------------------------------------------------------------------
#  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
#  See COPYRIGHT.md in the repository root for full copyright notice.
#---------------------------------------------------------------------------------------------
import sys

# This goal of this script is to make the generated MM file smaller and easier to work with in Xcode.

if __name__ == '__main__':
    fileName = sys.argv[1]

    with open(fileName, 'r') as file:
        lines = file.readlines()

    wasPreviousLineBlank = False
    wasPreviousLineExtern = False

    with open(fileName, 'w') as file:
        for line in lines:
            isCurrentLineBlank = (0 == len(line.strip()))
            isCurrentLineExtern = ("extern" in line)

            if (isCurrentLineBlank and wasPreviousLineBlank) or (isCurrentLineBlank and wasPreviousLineExtern):
                wasPreviousLineBlank = isCurrentLineBlank
                wasPreviousLineExtern = isCurrentLineExtern
                continue
            
            wasPreviousLineBlank = isCurrentLineBlank
            wasPreviousLineExtern = isCurrentLineExtern

            file.write(line)
