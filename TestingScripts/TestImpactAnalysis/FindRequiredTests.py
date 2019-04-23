#--------------------------------------------------------------------------------------
#
#  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
#
#--------------------------------------------------------------------------------------
import os, sys, shutil
import argparse
import subprocess
import time

#Common Scripts to be used by any task
scriptsDir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
commonScriptsDir = os.path.join(scriptsDir, 'CommonTasks')
sys.path.append(commonScriptsDir)

from GitCommands import GitCommand
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
    srcRoot = fixPath(os.path.join(os.getenv('SrcRoot'), 'imodel02', 'iModelCore',))
    if not srcRoot or len(srcRoot) == 0:
        print ("\n %SrcRoot% must be defined. Error.")
        return None

    sfName = fixPath(sfName)

    sfPath = sfName[sfName.find(srcRoot)+len(srcRoot):]
    if not srcRoot in sfName:
        print "\n Source file path is outside tracked path: " +  srcRoot + ". Skipping.\n"
        return None

    sfPath = sfName[len(srcRoot)+1:]
    tiaMapDir = os.path.join(os.getenv('SrcRoot'), 'imodel02', 'TestingScripts', 'TestImpactAnalysis', 'TIAMaps')
    
    comp = sfPath.split('\\')[0]
    if comp.lower() not in cmp.AllComps():
        print ("\n Source file path is not in the Components that are tracked. \n")
        
    tests = []
    if dll == None:
        print  ("\n Run for all components.\n")
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
        print  "\nRun for component: " + dll + "\n"
        name_find = 'TIAMap_' + dll + '_' + comp
        for f in os.listdir(tiaMapDir):
            if f.lower().startswith(name_find.lower()):
                fileName =  os.path.join(tiaMapDir,  f)
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
def saveReqTests(dll, testsToRun, logsPath):
    comps = []
    for test in testsToRun:
        testExePath = test.split(' ')[0].strip()
        testExeName, ext = os.path.splitext(os.path.basename(testExePath))
        comp = cmp.CompForExe(testExeName)

        ignoreTests = getIgnoredTests(comp)
        if len(ignoreTests) > 0:
            test = test + ':-'
            for ignTest in ignoreTests:
                test = test + ignTest + ':'
        print '*** Finding tests for: ' + comp
        comps.append(comp)

        cmdFile = os.path.join(logsPath, testExeName + '_command.txt')
        if os.path.exists(cmdFile):
            os.remove(cmdFile)
        
        with open(cmdFile, 'w') as f:
            f.write(test.split(' ')[1])
    return True

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
        print ("\nIgnored list could not be located for comp: " + comp + " \n")
        print ignorelist
    return ignoreTests
        

#-------------------------------------------------------------------------------------------
# bsimethod                                     Majd.Uddin    09/2017
#-------------------------------------------------------------------------------------------
def main():
    start = time.time()
    parser = argparse.ArgumentParser()
    parser.add_argument("--gtestName", help = "Name of GTest Exe against which changes are required. Comes from Make file. e.g.: --gtestExe=UnitsTest", required=True)

    args = parser.parse_args()

    comp = cmp.CompForExe(args.gtestName)

    logsDir = os.path.join(os.getenv('OutRoot'), 'winx64', 'LogFiles')
    if not os.path.exists(logsDir):
        os.mkdir(logsDir)
    logsPath = os.path.join(logsDir, 'RunRequiredTests')
    if not os.path.exists(logsPath):
        os.mkdir(logsPath)

    comps_tracked = ['BeHttp', 'Bentley', 'BeSecurity', 'BeSQLite', 'DgnPlatform', 'ECDb', 'ECObjects', 'ECPresentation', 'GeomLibs', 'Units', 'Licensing', 'WSClient']
    if comp not in comps_tracked: # we don't have Maps for these components
        testExeName = cmp.ExeForComp(comp)
        nnFile = os.path.join(logsPath, testExeName + '_NotNeeded.txt')
        if os.path.exists(nnFile):
            os.remove(nnFile)
        with open(nnFile, 'w') as f:
            f.write('Component out of scope')        
        exit()
    dll = comp
    repoPath = cmp.RepoPathForComp(comp)

    repos = []
    repos.append(repoPath)

    print ("\n Getting source file list from git repository... \n")
    sfLog = os.path.join(logsPath, comp + '_sfNames.txt')
    if os.path.exists(sfLog):
        os.remove(sfLog)
    src_branch = 'origin/' + os.path.basename(os.getenv('SourceBranch'))
    tgt_branch = 'origin/' + os.path.basename(os.getenv('TargetBranch'))
    for repo in repos:
        print 'Checking files at path: ' + repo
        os.chdir(cmp.RepoPathForComp(repo))
        gc = GitCommand()
        sf = gc.files_branches(src_branch, tgt_branch)
        if len(sf) > 0:
            with open(sfLog, 'a') as f:
                for sfile in sf:
                    print sfile
                    f.write(sfile)
                    f.write('\n')
    sfName = sfLog

    if not os.path.exists(sfName):
        print ("\n The Source File path or text file path is not valid: " + sfName + "\n")
        print ("\n Perhaps, there is no source file that changed")
        exit()
    fName, fExt = os.path.splitext(sfName)

    testsCollect = []
    print ("\nLet us retrieve file paths from text file")
    txtFile = open(sfName ,'r')
    lines = txtFile.readlines()
    for i in range(0, len(lines)):
        sfName1 = lines[i].strip()
        if len(sfName1) > 0:
            if '%SrcRoot%' in sfName1:
                sfName1 = sfName1.replace('%SrcRoot%', fixPath(os.getenv('SrcRoot')))
            print ("\nFinding tests for file: " + sfName1)
            tests = getTests(sfName1, dll)
            if tests is None:
                print ("\nNo tests for given source file(s)")
            else:
                for test in tests:
                    if test not in testsCollect:
                        testsCollect.append(test)
    testsToRun = consolidateTests(testsCollect)
                
    if len(testsToRun) > 0 :
        print  ("\nLet us save test details. \n")
        saved = saveReqTests(dll, testsToRun, logsPath)
        if not saved:
            print ("\n Required Tests could not be determined for: " + comp + " \n")
        else:
            print  ("\n Required Tests saved. \n")
            
    else:
        print  ("\nNo tests are affected by current change. \n")

    print 'Logs are at: ' + logsPath


if __name__ == '__main__':    
    main()
