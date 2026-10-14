#---------------------------------------------------------------------------------------------
#  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
#  See LICENSE.md in the repository root for full copyright notice.
#---------------------------------------------------------------------------------------------
import sys
import os
from shutil import copytree, copyfile, rmtree

# Copies all pulled test runners into the test out folder and stages the test files each pulled runner should run against.
# By default (RUN_EVOLUTION_TESTS_OPTIMIZED=1) only the *created* files are staged into the pulled runners. The combination
# "pulled runner x pulled files" consists of two immutable, already published artefacts and was validated when they were published.
# If the optimizations are disabled (--optimized=0 or RUN_EVOLUTION_TESTS_OPTIMIZED=0), the full matrix (pulled + created files) is staged as before.

OPTIMIZED_ENV_VAR = "RUN_EVOLUTION_TESTS_OPTIMIZED"

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


def mergeFolders(sourceFolders, targetFolder):
    for sourceFolder in sourceFolders:
        if os.path.exists(sourceFolder):
            mergeFolder(sourceFolder, targetFolder)


def isOptimized(args):
    value = os.environ.get(OPTIMIZED_ENV_VAR, "1")
    for arg in args:
        if arg.startswith("--optimized="):
            value = arg[len("--optimized="):]
    return value.strip().lower() not in ("0", "false", "")

#------------------------------------------------------------------------
# bsimethod
#------------------------------------------------------------------------
def main():
    args = [a for a in sys.argv[1:] if not a.startswith("--")]
    if len(args) < 4:
        print ("Arg 1: Test runners nuget folder")
        print ("Arg 2: Sandbox folder where test runners are run from. The test runners are copied into this folder from the nuget folder.")
        print ("Arg 3: Central test files folder to which test files from all nugets were copied")
        print ("Arg 4: Created files folder (test files created by the current runner)")
        print ("Optional: --optimized=0|1 (or env {0}) master switch for the optimizations, defaults to 1. If 0, the pulled test files are staged as well (legacy full matrix).".format(OPTIMIZED_ENV_VAR))
        return sys.exit(1)

    testRunnersNugetPath = args[0]
    testRunnersSandboxFolder = args[1]
    testFilesPath = args[2]
    createdFilesPath = args[3]

    fullMatrix = not isOptimized(sys.argv[1:])
    testFileSources = [testFilesPath, createdFilesPath] if fullMatrix else [createdFilesPath]
    print ("Staging test files for pulled runners: {0}".format("full matrix (pulled + created files)" if fullMatrix else "created files only (set {0}=0 for the full matrix)".format(OPTIMIZED_ENV_VAR)))

    if not os.path.exists(createdFilesPath) or not os.listdir(createdFilesPath):
        print ("Created files folder '{0}' is empty. The current test runner did not produce any test files.".format(createdFilesPath), file=sys.stderr)
        return sys.exit(1)

    runnerFolders = [d for d in os.listdir(testRunnersNugetPath) if os.path.isdir(os.path.join(testRunnersNugetPath, d))] if os.path.exists(testRunnersNugetPath) else []
    if not runnerFolders:
        print("No pulled test runners found in '{0}'.".format(testRunnersNugetPath), file=sys.stderr)
        return sys.exit(1)

    for subdir in runnerFolders:
        fullPath = os.path.join(testRunnersNugetPath, subdir)
        targetTestRunnerFolder = os.path.join(testRunnersSandboxFolder, subdir)
        if os.path.exists(targetTestRunnerFolder):
            rmtree(targetTestRunnerFolder)
        copytree(fullPath, targetTestRunnerFolder)
        mergeFolders(testFileSources, os.path.join(targetTestRunnerFolder, "run", "TestFiles"))
        print ("Copied pulled test runner and staged test files into sandbox folder (" + fullPath + " -> " + targetTestRunnerFolder + ")")


if __name__ == "__main__":
    main()