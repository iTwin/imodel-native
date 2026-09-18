#---------------------------------------------------------------------------------------------
#  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
#  See LICENSE.md in the repository root for full copyright notice.
#---------------------------------------------------------------------------------------------
from __future__ import print_function
import sys
import os
import subprocess
import time
from concurrent.futures import ThreadPoolExecutor, as_completed

# Runs every pulled (older) test runner found in the sandbox folder against the test files staged in its own run/TestFiles folder.
TESTRUNNER_EXE = "iModelEvolutionTests.exe"
JOBS_ENV_VAR = "IMODELEVOLUTION_RUNNER_JOBS"

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
    exePath = os.path.join(exeDir, TESTRUNNER_EXE)
    output = subprocess.Popen([exePath, "--gtest_list_tests"], stdout=subprocess.PIPE, cwd=exeDir).communicate()[0].decode()

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
def getJobCount(args):
    """Number of pulled test runners to execute concurrently."""
    for arg in args:
        if arg.startswith("--jobs="):
            return max(1, int(arg[len("--jobs="):]))
    envValue = os.environ.get(JOBS_ENV_VAR, "").strip()
    if envValue:
        return max(1, int(envValue))
    return max(1, (os.cpu_count() or 2) // 2)

#------------------------------------------------------------------------
# bsimethod
#------------------------------------------------------------------------
def runTestRunner(sandboxFolder, runnerKey):
    """Runs a single pulled test runner. Its output goes to a log file in its sandbox so that
    concurrently running runners do not interleave. Returns (runnerKey, succeeded, logPath, elapsedSeconds)."""
    exePath = os.path.join(sandboxFolder, TESTRUNNER_EXE)
    logPath = os.path.join(sandboxFolder, "iModelEvolutionTests.log")
    start = time.time()
    if not os.path.exists(exePath):
        with open(logPath, "w") as log:
            log.write("Compatibility test runner '{0}' does not exist.\n".format(exePath))
        return (runnerKey, False, logPath, 0.0)

    gtestFilter = "--gtest_filter=" + createGTestFilter(sandboxFolder, runnerKey)
    with open(logPath, "w") as log:
        log.write("Test runner: {0}\n{1}\n\n".format(exePath, gtestFilter))
        log.flush()
        returnCode = subprocess.call([exePath, gtestFilter], stdout=log, stderr=subprocess.STDOUT, cwd=sandboxFolder)
        log.write("\nExit code: {0}\n".format(returnCode))

    return (runnerKey, returnCode == 0, logPath, time.time() - start)

#------------------------------------------------------------------------
# bsimethod
#------------------------------------------------------------------------
def printLog(logPath):
    try:
        with open(logPath, "r", errors="replace") as log:
            sys.stdout.write(log.read())
    except IOError as err:
        print ("Could not read log '{0}': {1}".format(logPath, err), file=sys.stderr)
    sys.stdout.flush()

#------------------------------------------------------------------------
# bsimethod
#------------------------------------------------------------------------
def main():
    args = [a for a in sys.argv[1:] if not a.startswith("--")]
    if len(args) < 1:
        print ("Arg 1: Test runners sandbox folder")
        print ("Optional: --jobs=N (or env {0}=N) number of runners to execute in parallel".format(JOBS_ENV_VAR))
        return sys.exit(1)

    testRunnersSandboxFolder = args[0]
    if not os.path.exists(testRunnersSandboxFolder):
        print ("Test runners sandbox folder '{0}' does not exist. No pulled test runners were prepared.".format(testRunnersSandboxFolder), file=sys.stderr)
        return sys.exit(1)

    runners = sorted(d for d in os.listdir(testRunnersSandboxFolder) if os.path.isdir(os.path.join(testRunnersSandboxFolder, d)))
    if not runners:
        print ("No pulled test runners found in '{0}'.".format(testRunnersSandboxFolder), file=sys.stderr)
        return sys.exit(1)

    jobs = min(getJobCount(sys.argv[1:]), len(runners))
    print ("Executing {0} pulled test runner(s) with {1} parallel job(s)...".format(len(runners), jobs))
    for runner in runners:
        print ("  " + runner)
    sys.stdout.flush()

    results = []
    with ThreadPoolExecutor(max_workers=jobs) as executor:
        futures = {executor.submit(runTestRunner, os.path.join(testRunnersSandboxFolder, runner), runner): runner for runner in runners}
        for future in as_completed(futures):
            runnerKey, succeeded, logPath, elapsed = future.result()
            results.append((runnerKey, succeeded, elapsed))
            print ("\n" + "=" * 100)
            print ("Test runner '{0}' {1} after {2:.0f}s. Log: {3}".format(runnerKey, "succeeded" if succeeded else "FAILED", elapsed, logPath))
            print ("=" * 100)
            printLog(logPath)

    print ("\nSummary:")
    for runnerKey, succeeded, elapsed in sorted(results):
        print ("  {0:<9} {1:>7.0f}s  {2}".format("OK" if succeeded else "FAILED", elapsed, runnerKey))

    failed = [r for r in results if not r[1]]
    if failed:
        print ("{0} of {1} pulled test runner(s) failed.".format(len(failed), len(results)), file=sys.stderr)
        return sys.exit(1)

if __name__ == "__main__":
    main()
