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
        print "Must give the <nuget path> <nuget dst path> <nuget dataset dir>"
        return
    
    nugetSrcPath = sys.argv[1]
    nugetDstPath = sys.argv[2]
    dataDir = sys.argv[3]

    # Copy out exectuable file to product dir. It can't be symlinked because it will execute where the actual
    # executable is and not in the product directory. We need to to be a sibling of the run dir we give it.
    for filename in os.listdir(nugetSrcPath):
        name = os.path.splitext(filename)[0]
        split = os.path.join(nugetSrcPath, name)
        if os.path.isdir(split):
            dst = os.path.join(nugetDstPath, name, "iModelSchemaEvolution.exe")
            os.remove(dst)
            copyfile(os.path.join(split, "iModelSchemaEvolution.exe"), dst)
            rmtree(os.path.join(nugetDstPath, name, "run", "NewData"), ignore_errors=True)
            if os.path.exists(dataDir):
                copytree(dataDir, os.path.join(nugetDstPath, name, "run", "NewData"))
if __name__ == "__main__":
    main()