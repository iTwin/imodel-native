#----------------------------------------------------------------------
#
#  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
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
    if len(sys.argv) < 1:
        print "Arg 1: Test runners sandbox folder"
        return sys.exit(1)
        
    testRunnersSandboxFolder = sys.argv[1]
    if not os.path.exists(testRunnersSandboxFolder):
        return sys.exit(0)

    hasError = False
    for subdir in os.listdir(testRunnersSandboxFolder):
        fullPath = os.path.join(testRunnersSandboxFolder, subdir);
        if (os.path.isdir(fullPath)):
            exePath = os.path.join(fullPath, "iModelEvolutionTests.exe")
            if not os.path.exists(exePath):
                print >> sys.stderr, "Compatibility test runner '{0}' does not exist.".format(exePath)
                hasError = True
            try:
                print "Test runner '" + exePath + "' started..."
                subprocess.check_call(exePath)
                print "Test runner '" + exePath + "' succeeded."
            except subprocess.CalledProcessError as err:
                print >> sys.stderr, "Test runner '{0}' failed: {1}".format(subdir, err)
                hasError = True

    if hasError:
        return sys.exit(1)
        
if __name__ == "__main__":
    main()