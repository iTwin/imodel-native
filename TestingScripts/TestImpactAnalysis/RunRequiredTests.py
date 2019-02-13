#--------------------------------------------------------------------------------------
#
#     $Source: TestImpactAnalysis/RunRequiredTests.py $
#
#  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
#
#--------------------------------------------------------------------------------------
import os, sys, shutil
import argparse
import re
import subprocess
import sqlite3
import time
import psutil
from GetSourceFiles import getFilesHg

#BentleyBuild script for color printing
srcRoot = os.getenv('SrcRoot')
if srcRoot is None:
    print 'Script should be called from shell with SrcRoot and OutRoot defined.'
    exit(-1)
#Common Scripts to be used by any task
scriptsDir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
commonScriptsDir = os.path.join(scriptsDir, 'CommonTasks')
sys.path.append(commonScriptsDir)
from TestResults import TestResults
import Components as cmp

#-------------------------------------------------------------------------------------------
# bsimethod                                     Jeff.Marker
#-------------------------------------------------------------------------------------------
def fixPath(path):
    if not path or len(path) == 0:
        return path
    
    return os.path.normcase(os.path.realpath(path))

#-------------------------------------------------------------------------------------------
# bsimethod                                     Majd.Uddin    09/2017
#-------------------------------------------------------------------------------------------
def getTests(sfName, dll):
    srcRoot = fixPath(os.path.join(os.getenv('SrcRoot'), 'imodel02')
    if not srcRoot or len(srcRoot) == 0:
        print "\n %SrcRoot% must be defined. Error."
        return None

    sfName = fixPath(sfName)

    sfPath = sfName[sfName.find(srcRoot)+len(srcRoot):]
    if not srcRoot in sfName:
        print "\n Source file path must reside within %SrcRoot%. Error.\n"
        return None

    sfPath = sfName[len(srcRoot)+1:]
    tiaMapDir = os.path.join(srcRoot, 'TestingScripts', 'TestImpactAnalysis', 'TIAMaps')
    
    comp = sfPath.split('\\')[0]
    if comp.lower() not in cmp.AllComps():
        print "\n Source file path is not in the Components that are tracked. \n"
        return None
        
    tests = []
    if dll == None:
        utils.showInfoMsg ("\n Run for all components.\n", utils.INFO_LEVEL_Important, utils.LIGHT_BLUE)
        for file in os.listdir(tiaMapDir):
            if file.startswith('TIAMap'):
                if comp.lower() in file.lower():
                    mapFile = open(os.path.join(tiaMapDir, file) ,'r')
                    lines = mapFile.readlines()
                    for i in range(0, len(lines)):
                        line = lines[i]
                        if line.strip().startswith('['):
                            fName = line.strip()[1:-1]
                            if fName.lower() in sfPath.lower():
                                tests.append(os.path.expandvars(lines[i+1]))

        return tests
    else:
        utils.showInfoMsg ("\nRun for component: " + dll + "\n", utils.INFO_LEVEL_Important, utils.LIGHT_BLUE)
        fileName =  os.path.join(tiaMapDir, 'TIAMap_' + dll + '_' + comp + '.txt')
        print "\n Map file: " + fileName + "\n"
        if os.path.exists(fileName):
            mapFile = open(fileName,'r')
            lines = mapFile.readlines()
            for i in range(0, len(lines)):
                line = lines[i]
                if sfPath.lower() in line.lower():
                    tests.append(os.path.expandvars(lines[i+1]))
        return tests

#-------------------------------------------------------------------------------------------
# bsimethod                                     Majd.Uddin    10/2017
#-------------------------------------------------------------------------------------------
def consolidateTests(testsCollect):
    testsToRun = {}
    for tests in testsCollect:
        testsParts = tests.split(" ")
        testsExe = testsParts[0]
        testsFilter = testsParts[1].strip()
        if testsExe not in testsToRun:
            testsToRun.setdefault(testsExe, [])
        testNames = testsFilter.split("=")[1].split(":")
        for name in testNames:
            if name not in testsToRun[testsExe]:
                testsToRun[testsExe].append(name)
    testsList = []
    for testExe in testsToRun:
        testStr = testExe + " --gtest_filter="
        for tests in testsToRun[testExe]:
            testStr = testStr + tests + ":"
        testsList.append(testStr[:-2])
    return testsList

#-------------------------------------------------------------------------------------------
# bsimethod                                     Majd.Uddin    10/2017
#-------------------------------------------------------------------------------------------
def executeTests(dll, testsToRun, logsPath):
    allPassedTests = []
    failedTests = []
    failingComp = None
    i = 0
    testLogs = os.path.join(logsPath, 'ExecutionLogs')
    if os.path.exists(testLogs):
        result = os.system('rmdir /S /Q "{}"'.format(testLogs))
        if result is not 0:
            print "\nTest logs dir could not be removed.\n"
            exit(-1)
    os.mkdir(testLogs)               
    comps = []
    testExeNames = []
    roundsToRun = 0
    testsNotExecuted = []
    for test in testsToRun:
        testExePath = test.split(' ')[0].strip()
        if not os.path.exists(testExePath):
            if testExePath not in testsNotExecuted:
                testsNotExecuted.append(testExePath)
            continue
        roundsToRun = roundsToRun + 1
        testExeName, ext = os.path.splitext(os.path.basename(testExePath))
        testExeNames.append(testExeName)
        comp = testExeName[:testExeName.find('Test')]

        ignoreTests = getIgnoredTests(comp)
        if len(ignoreTests) > 0:
            test = test + ':-'
            for ignTest in ignoreTests:
                test = test + ignTest + ':'
        print '*** Running tests for: ' + comp
        comps.append(comp)

        cmdFile = os.path.join(testLogs, comp + '_command.bat')
        resFile = os.path.join(testLogs, comp + '_result.log')
        finFile = os.path.join(testLogs, comp + '_finished.txt')
        if os.path.exists(cmdFile):
            os.remove(cmdFile)
        if os.path.exists(resFile):
            os.remove(resFile)
        if os.path.exists(finFile):
            os.remove(finFile)
        
        with open(cmdFile, 'w') as f:
            f.write(test + ' >' + resFile)
            f.write('\n')
            f.write('echo. 2>' + finFile)
            f.write('\n')
            f.write('exit')
        cmd = 'start ' + cmdFile
            
        result = subprocess.call(cmd, shell=True)
        if result != 0 : # Test run failed
            failingComp = comp
            failedTests.append(test)
            break
        else:
            i = i + 1
            continue
        
    compFinished = []
    keepRunning = True
    j = 0
    while len(compFinished) < len(comps) and keepRunning:
        for comp in comps:
            finFile = os.path.join(testLogs, comp + '_finished.txt')
            resFile = os.path.join(testLogs, comp + '_result.log')
            if os.path.exists(finFile):
                if comp not in compFinished:
                    compFinished.append(comp)
                    tr = TestResults(resFile)
                    failedTests = tr.getFailedTests()
                    allPassedTests.append(tr.getPassedTests())
                    if len(failedTests) > 0:
                        failingComp = comp
                        print 'Some test(s) failed for: ' + comp
                        keepRunning = False
                        break
                    print "\n" + comp + " tests finished successfully.\n"
            else:
                j = j + 1
                time.sleep(5)
                print '*** Waiting (' + str(j*5) +' seconds elapsed) for tests to finish: ' + comp
                continue
    # Kill any tests that are still running
    for pid in psutil.pids():
        if psutil.pid_exists(pid):
            p = psutil.Process(pid)
            for pName in testExeNames:
                if pName in p.name():
                    if psutil.pid_exists(pid):
                        p.terminate()
    if len(failedTests) > 0 and failingComp is not None:
        print "\n Rounds of Test execution finished as some tests failed.\n"
    else:
        print "\n" + str(roundsToRun) + " testing round(s) finished successfully.\n"
        if len(testsNotExecuted) > 0:
            print "\n Following tests couldn't be executed. Please build those parts first. \n"
            for testN in testsNotExecuted:
                print testN
    return failingComp, failedTests, allPassedTests

#-------------------------------------------------------------------------------------------
# bsimethod                                     Majd.Uddin    10/2017
#-------------------------------------------------------------------------------------------
def getIgnoredTests(comp):
    ignoreTests = []
    ignorelist = cmp.IgnorePathForComp(comp)
    if os.path.exists(ignorelist):
        file1 = open(ignorelist,'r')
        for line in file1.readlines():
            if line.strip().startswith("#") or not line.strip(): # It is a comment or empty line, let's move to next line
                continue
            else:
                testName = line.split('#')[0].strip() # Take out the comment part
                ignoreTests.append(testName)
        
    else:
        print "\nIgnored list could not be located for comp: " + comp + " \n"
        print ignorelist
    return ignoreTests
        

#-------------------------------------------------------------------------------------------
# bsimethod                                     Majd.Uddin    09/2017
#-------------------------------------------------------------------------------------------
def main():
    start = time.time()
    parser = argparse.ArgumentParser()
    parser.add_argument("--sfName", help = "Provide the source file or a text file having source files. If not provided, gets all source files from RepoName. e.g. --sfName=D:\src\Bim0200\src\Bentley\Bentley\WString.cpp or --sfName=D:\\files.txt")
    parser.add_argument("--comp", help = "The Component filter to run tests against. If not specified, runs test for all components. e.g. --comp=BeSQlite")
    parser.add_argument("--repoName", help = "Name of repository to get source files. Usually passed through Make file. e.g. --repoName=DgnClientFx")    

    args = parser.parse_args()
    sfName = args.sfName
    dll = args.comp
    repoName = args.repoName

    repos = []
    if repoName is None: # we need to run for all repositories that we care
        repos = cmp.AllComps()
    else:
        repos.append(repoName)

    logsPath = os.path.join(os.getenv('OutRoot'), 'winx64', 'LogFiles', 'RunRequiredTests')
    if not os.path.exists(logsPath):
        os.mkdir(logsPath)

    if sfName is None:
        utils.showInfoMsg("\n Getting source file list from git repository... \n", utils.INFO_LEVEL_Important, utils.LIGHT_BLUE)
        sfLog = os.path.join(logsPath, 'sfNames.txt')
        if os.path.exists(sfLog):
            os.remove(sfLog)
        for repo in repos:
            print 'Checking files for component: ' + repo
            os.chdir(cmp.RepoPathForComp(repo))
            sf = getFilesHg()
            if len(sf) > 0:
                with open(sfLog, 'a') as f:
                    for sfile in sf:
                        print sfile
                        f.write(sfile)
                        f.write('\n')
        sfName = sfLog

    if not os.path.exists(sfName):
        print "\n The Source File path or text file path is not valid: " + sfName + "\n"
        exit(-1)
    fName, fExt = os.path.splitext(sfName)
    if fExt == ".txt":
        testsCollect = []
        print "\nLet us retrieve file paths from text file"
        txtFile = open(sfName ,'r')
        lines = txtFile.readlines()
        for i in range(0, len(lines)):
            sfName1 = lines[i].strip()
            if len(sfName1) > 0:
                if '%SrcRoot%' in sfName1:
                    sfName1 = sfName1.replace('%SrcRoot%', fixPath(os.getenv('SrcRoot')))
                print "\nFinding tests for file: " + sfName1
                tests = getTests(sfName1, dll)
                if tests is None:
                    print "\nNo tests for given source file(s)"
                    exit(-1)
                for test in tests:
                    if test not in testsCollect:
                        testsCollect.append(test)
        testsToRun = consolidateTests(testsCollect)
                
    else: #it is a single file
        testsToRun = getTests(sfName, dll)
        if testsToRun is None:
            print "\nNo tests for given source file(s)"
            exit(-1)
        testsToRun = consolidateTests(testsToRun)
    if len(testsToRun) > 0 :
        print "\nLet us run tests. There are " + str(len(testsToRun)) + " round(s).\n"
        failingComp, failedTests, allPassedTests = executeTests(dll, testsToRun, logsPath)
        if len(failedTests) > 0 and failingComp is not None:
            utils.showErrorMsg("\nTest execution failed for following tests in comp: " + failingComp + " \n")
            for test in failedTests:
                print test
            print "\nTest execution failed for above tests in comp: " + failingComp + " \n"
            print "\nRerun tests using following batch file. \n" + os.path.join(logsPath, 'ExecutionLogs', failingComp+'_command.bat')
            exit(-1)
        else:
            totalCount = 0
            for tests in allPassedTests:
                totalCount = totalCount + len(tests)
            if totalCount == 0:
                print "\nNo tests could be executed. See log for details. \n"
                exit(-1)
            else:
                if len(testsToRun) == len(allPassedTests):
                    print "\n All Tests passed. " + str(len(allPassedTests)) + " rounds.\n"
                else:
                    print "\n Some Tests passed. " + str(len(allPassedTests)) + " round(s) out of "+ str(len(testsToRun)) + " round(s). See log for test rounds which were not executed.\n"
                print "\n Total Tests passed: " + str(totalCount) + "\n"
            
    else:
        print "\nNo tests are affected by changing this source file. \n" + str(len(testsToRun)) + " rounds.\n"
    diff = time.time() - start
    mins = int(diff / 60)
    secs = int(diff) % 60

    print "\nTotal time to execute tests: " + '{:0>2}:{:0>2}'.format(mins,secs) + " (MM:SS).\n"
    print 'Test execution logs are at: ' + os.path.join(logsPath, 'ExecutionLogs')


main()