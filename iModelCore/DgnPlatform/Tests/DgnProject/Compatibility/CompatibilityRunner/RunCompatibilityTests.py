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
        return 1
        
    hasError = False
    testRoot = sys.argv[1]

    for subdir in os.listdir(testRoot):
        fullPath = os.path.join(testRoot, subdir);
        if (os.path.isdir(fullPath) and subdir != "Assets"):
            try:
                subprocess.check_call(os.path.join(fullPath, "iModelSchemaEvolution.exe"))
                print "Compatibility test runner for '" + subdir + "' succeeded."
            except subprocess.CalledProcessError as err:
                print >> sys.stderr, "Compatibility test runner for '{0}' failed: {1}".format(subdir, err)
                hasError = True

    if hasError:
        return 1
        
if __name__ == "__main__":
    main()