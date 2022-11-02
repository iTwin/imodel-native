#---------------------------------------------------------------------------------------------
#  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
#  See LICENSE.md in the repository root for full copyright notice.
#---------------------------------------------------------------------------------------------
import sys
import os
from shutil import copytree, copyfile, rmtree

# Copies all pulled test runners into the test out folder and copies all pulled test files and all files created by
# the current software's runner into its folder, so that every pulled test runner runs against all available test files.

def mergeFolder(sourceFolder, targetFolder):
    for subdir in os.listdir(sourceFolder):
        sourceFullPath = os.path.join(sourceFolder, subdir)
        targetFullPath = os.path.join(targetFolder, subdir)
        if os.path.isdir(sourceFullPath):
            mergeFolder(sourceFullPath, targetFullPath)
        else:
            if not os.path.exists(targetFolder):
                os.makedirs(targetFolder)
            copyfile(sourceFullPath,targetFullPath)


def mergeFolders(sourceFolder1, sourceFolder2, targetFolder):
    if os.path.exists(sourceFolder1):
        copytree(sourceFolder1, targetFolder)
    if os.path.exists(sourceFolder2):
        mergeFolder(sourceFolder2, targetFolder)
    

#------------------------------------------------------------------------
# bsimethod
#------------------------------------------------------------------------
def main():
    if len(sys.argv) < 4:
        print ("Arg 1: Test runners nuget folder")
        print ("Arg 2: Sandbox folder where test runners are run from. The test runners are copied into this folder from the nuget folder.")
        print ("Arg 3: Central test files folder to which test files from all nugets were copied")
        print ("Arg 4: Created files folder (test files created by the current runner)")
        return

    testRunnersNugetPath = sys.argv[1]
    testRunnersSandboxFolder = sys.argv[2]
    testFilesPath = sys.argv[3]
    createdFilesPath = sys.argv[4]

    for subdir in os.listdir(testRunnersNugetPath):
        fullPath = os.path.join(testRunnersNugetPath, subdir)
        if os.path.isdir(fullPath):
            targetTestRunnerFolder = os.path.join(testRunnersSandboxFolder,subdir)
            if os.path.exists(targetTestRunnerFolder):
                rmtree(targetTestRunnerFolder)
            copytree(fullPath, targetTestRunnerFolder)
            mergeFolders(testFilesPath, createdFilesPath, os.path.join(targetTestRunnerFolder, "run", "TestFiles"))
            print ("Copied pulled test runner and pulled and created test files into sandbox folder (" + fullPath + " -> " + targetTestRunnerFolder + ")")


if __name__ == "__main__":
    main()