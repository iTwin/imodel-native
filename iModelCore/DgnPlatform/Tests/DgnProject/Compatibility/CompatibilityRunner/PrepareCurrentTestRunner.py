#----------------------------------------------------------------------
#
#     $Source: Tests/DgnProject/Compatibility/CompatibilityRunner/PrepareCurrentTestRunner.py $
#
#  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
#
#----------------------------------------------------------------------
import sys
import os
from shutil import copytree, rmtree

# Copies the pulled test files into the current test runner's run folder, so that it runs its tests against
# the pulled test files as well

#------------------------------------------------------------------------
# bsimethod                         Krischan.Eberle           07/2018
#------------------------------------------------------------------------
def main():
    if len(sys.argv) < 2:
        print "Arg 1: Central test files folder to which test files from all nugets were copied"
        print "Arg 2: Current test runner's test files folder"
        return

    testFilesSeedPath = sys.argv[1]
    testRunnerTestFilesFolder = sys.argv[2]
    if os.path.exists(testRunnerTestFilesFolder):
        rmtree(testRunnerTestFilesFolder)
    copytree(testFilesSeedPath, testRunnerTestFilesFolder)
    print "Copied pulled test files into current runner's run folder (" + testFilesSeedPath + " -> " + testRunnerTestFilesFolder + ")"

if __name__ == "__main__":
    main()