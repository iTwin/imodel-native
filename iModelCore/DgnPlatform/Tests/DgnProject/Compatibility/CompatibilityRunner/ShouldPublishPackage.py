#----------------------------------------------------------------------
#
#     $Source: Tests/DgnProject/Compatibility/CompatibilityRunner/ShouldPublishPackage.py $
#
#  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
#
#----------------------------------------------------------------------
from __future__ import print_function
import sys
import os
import subprocess
from shutil import rmtree

#------------------------------------------------------------------------
# bsimethod                         Kyle.Abramowitz             05/2018
#------------------------------------------------------------------------
def main():
    currentDatasetPath = sys.argv[1]
    oldDatasetPath = sys.argv[2]
    currentVersions = []

    #Append bedb, dgndb, ecdb versions in that order to list
    for subdir in os.listdir(currentDatasetPath):
        currentVersions.append(os.listdir(os.path.join(currentDatasetPath, subdir))[0])

    # check to see if we already have this version
    i = 0
    for subdir in os.listdir(oldDatasetPath):
        dirs = os.listdir(os.path.join(oldDatasetPath, subdir))
        if currentVersions[i] in dirs:
            return  
    
    import sys

    print("No changes, publishing datasets not necessary", file=sys.stderr)


if __name__ == "__main__":
    main()
