#----------------------------------------------------------------------
#
#     $Source: Tests/DgnProject/Compatibility/CompatibilityRunner/SymlinkAssetDir.py $
#
#  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
#
#----------------------------------------------------------------------
import sys
import os
import subprocess

#------------------------------------------------------------------------
# bsimethod                         Kyle.Abramowitz             05/2018
#------------------------------------------------------------------------
def main():
    if len(sys.argv) < 4:
        print "Must give <path to CopyWithSymlinks.py> <Asset Dir Path> <Nuget Dir Path>"
        return
    
    symlinkMakerPath = sys.argv[1]
    assetDir = sys.argv[2]
    nugetDir = sys.argv[3]

    for filename in os.listdir(nugetDir):
        split = os.path.join(nugetDir, os.path.splitext(filename)[0])
        if os.path.isdir(split):
            subprocess.check_call("python " + symlinkMakerPath + " " + assetDir + " " +  os.path.join(split, "Assets"))
        else:
            print "not a dir: " + split

if __name__ == "__main__":
    main()