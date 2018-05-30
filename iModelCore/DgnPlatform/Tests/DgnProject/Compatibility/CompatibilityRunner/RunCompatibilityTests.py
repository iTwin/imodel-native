#----------------------------------------------------------------------
#
#     $Source: Tests/DgnProject/Compatibility/CompatibilityRunner/RunCompatibilityTests.py $
#
#  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
#
#----------------------------------------------------------------------
import sys
import os
import subprocess
from shutil import rmtree

#------------------------------------------------------------------------
# bsimethod                         Kyle.Abramowitz             05/2018
#------------------------------------------------------------------------
def main():
    if len(sys.argv) < 2:
        print "Must give the test root"
        return
    
    testRoot = sys.argv[1]

    print "TestRoot: " + testRoot + "\n"
    # All these packages should be of the form DgnCompatiblityNugetxx.xx.xx.xx
    for subdir in os.listdir(testRoot):
        fullPath = os.path.join(testRoot, subdir);
        if (os.path.isdir(fullPath) and subdir != "Assets"):
            print "calling " + os.path.join(fullPath, "iModelSchemaEvolution.exe")
            subprocess.check_call(os.path.join(fullPath, "iModelSchemaEvolution.exe"))
        else:
            print "not a dir: " + fullPath


if __name__ == "__main__":
    main()