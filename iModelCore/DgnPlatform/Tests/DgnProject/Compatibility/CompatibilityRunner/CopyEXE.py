#----------------------------------------------------------------------
#
#     $Source: Tests/DgnProject/Compatibility/CompatibilityRunner/CopyEXE.py $
#
#  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
#
#----------------------------------------------------------------------
import sys
import os
from shutil import copyfile, rmtree, copytree

#------------------------------------------------------------------------
# bsimethod                         Kyle.Abramowitz             05/2018
#------------------------------------------------------------------------
def main():
    if len(sys.argv) < 3:
        print "Must give the <nuget path> <nuget dst path> <current dataset dir>"
        return
    
    nugetSrcPath = sys.argv[1]
    nugetDstPath = sys.argv[2]
    dataDir = sys.argv[3]

    for filename in os.listdir(nugetSrcPath):
        name = os.path.splitext(filename)[0]
        split = os.path.join(nugetSrcPath, name)
        if os.path.isdir(split):
            dst = os.path.join(nugetDstPath, name, "DgnCompatibility.exe")
            os.remove(dst)
            copyfile(os.path.join(split, "DgnCompatibility.exe"), dst)
            rmtree(os.path.join(nugetDstPath, name, "run", "SeedData"), ignore_errors=True)
            copytree(dataDir, os.path.join(nugetDstPath, name, "run", "SeedData"))
if __name__ == "__main__":
    main()