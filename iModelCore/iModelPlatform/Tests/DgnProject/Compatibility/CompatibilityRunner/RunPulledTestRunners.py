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
def getIgnoreList(currentTestRunnerKey):
    scriptDir = os.path.dirname(os.path.realpath(__file__))
    ignoreListPath = os.path.join(scriptDir, "ignore_list.txt")
    fileData = open(ignoreListPath, "r")
    rawList = fileData.read().split('\n')
    fileData.close()
    rawList = [x for x in rawList if not x.startswith('##')]
    rawList = list(filter(None, rawList))

    ignoreList = []
    allVersions = False

    for item in rawList:
        if item.count(".") <= 1:
            print("Test format in the ignore list is not correct")
            print("Please follow the correct format:")
            print("-> TestRunnerName.version.TestFixtureName.TestName")
            print("-> Example: testRunnerNuget_bim0200_x64.2019.2.22.1.CompatibilityTestFixture.ECSqlColumnInfoForAliases")
            print("->    Note: Use `*` in place of TestRunnerName.version for applying it on all old test runners")
            print("To skip a test for all test runners of only a specific stream, use following format:")
            print("-> TestRunnerName.*.TestFixtureName.TestName")
            print("-> Example: testRunnerNuget_bim0200_x64.*.CompatibilityTestFixture.ECSqlColumnInfoForAliases")
            sys.exit(1)

        testRunnerInfo = item.split(".", 2)
        currentRunnerName = currentTestRunnerKey.split(".", 1)[0]
        testRunnerName = testRunnerInfo[0]

        if testRunnerInfo[1] == "*":
            allVersions = True
        else:
            allVersions = False

        cleanedItem = item.rsplit('.', 2)
        testRunnerKey = cleanedItem[0]

        # All versions of a specific stream runners
        if testRunnerName.lower() == currentRunnerName.lower() and allVersions:
            cleanedItem = cleanedItem[1] + "." + cleanedItem[2] 
            ignoreList.append(cleanedItem)

        # Specific key runner or all types of runners
        if (testRunnerKey.lower() == currentTestRunnerKey.lower() or testRunnerKey.lower() == "*") and not allVersions:
            cleanedItem = cleanedItem[1] + "." + cleanedItem[2] 
            ignoreList.append(cleanedItem)
            
    return ignoreList

#------------------------------------------------------------------------
# bsimethod
#------------------------------------------------------------------------
def createGTestFilter(exeDir, currentTestRunner):
    exePath = os.path.join(exeDir, "iModelEvolutionTests.exe")
    sys.stdout.flush();
    output = subprocess.Popen([exePath, "--gtest_list_tests"], stdout=subprocess.PIPE).communicate()[0].decode()

    subStr = "BEGTEST_LOGGING_CONFIG in environment."
    if subStr in output:
        print("Cleanup is required in logs...")
        output = output.split(subStr)[1]
        
    output = output.split('\n')
    output = [i.strip() for i in output]
    testsList = filter(lambda x: x != "", output)
    testsList = list(testsList)

    fixture = ''
    gtestCommand = ''
    ignoreList = getIgnoreList(currentTestRunner);
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
                gtestCommandArg = createGTestFilter(fullPath, subdir)
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
