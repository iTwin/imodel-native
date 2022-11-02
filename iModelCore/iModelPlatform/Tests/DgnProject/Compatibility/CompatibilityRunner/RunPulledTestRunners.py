#---------------------------------------------------------------------------------------------
#  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
#  See LICENSE.md in the repository root for full copyright notice.
#---------------------------------------------------------------------------------------------
from __future__ import print_function
import sys
import os
import subprocess
from shutil import rmtree

#------------------------------------------------------------------------
# bsimethod
#------------------------------------------------------------------------
def getIgnoreList():
    scriptDir = os.path.dirname(os.path.realpath(__file__))
    ignoreListPath = os.path.join(scriptDir, "ignore_list.txt")
    fileData = open(ignoreListPath, "r")
    ignoreList = fileData.read().split('\n')
    ignoreList = [x for x in ignoreList if not x.startswith('##')]
    fileData.close()
    return ignoreList

#------------------------------------------------------------------------
# bsimethod
#------------------------------------------------------------------------
def createGTestFilter(exeDir):
    exePath = os.path.join(exeDir, "iModelEvolutionTests.exe")
    sys.stdout.flush();
    output = subprocess.Popen([exePath, "--gtest_list_tests"], stdout=subprocess.PIPE).communicate()[0].decode()
    output = output.split("BEGTEST_LOGGING_CONFIG in environment.")[1].split('\n')
    output = [i.strip() for i in output]
    testsList = filter(lambda x: x != "", output)
    testsList = list(testsList)

    fixture = ''
    gtestCommand = ''
    ignoreList = getIgnoreList();
    for item in testsList:
        if item.endswith('.'):
            fixture = item
        else:
            subCommand = fixture + item 
            if subCommand not in ignoreList:
                gtestCommand = gtestCommand + subCommand + ':'

    gtestCommand = gtestCommand[:-1]
    return gtestCommand

#------------------------------------------------------------------------
# bsimethod
#------------------------------------------------------------------------
def main():
    if len(sys.argv) < 1:
        print ("Arg 1: Test runners sandbox folder")
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
                print ("Compatibility test runner '{0}' does not exist.".format(exePath), file=sys.stderr)
                hasError = True
            try:
                gtestCommandArg = createGTestFilter(fullPath)
                gtestFilter = "--gtest_filter=" + gtestCommandArg;
                print ("Test runner '" + exePath + "' started...")
                print(gtestFilter)
                subprocess.check_call([exePath, gtestFilter])
                print ("Test runner '" + exePath + "' succeeded.")
            except subprocess.CalledProcessError as err:
                print ("Test runner '{0}' failed: {1}".format(subdir, err), file=sys.stderr)
                hasError = True

    if hasError:
        return sys.exit(1)
        
if __name__ == "__main__":
    main()
