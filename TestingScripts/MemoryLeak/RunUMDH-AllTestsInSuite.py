#---------------------------------------------------------------------------------------------
#  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
#  See COPYRIGHT.md in the repository root for full copyright notice.
#---------------------------------------------------------------------------------------------
import os
import sys
import subprocess
import argparse
import shutil

logFailingTests = [];
scriptsDir = os.path.dirname(os.path.abspath(__file__))

def ParseTestList(exeName):
    path, file = os.path.split(exeName)
    listFile = os.path.join(path, "TestList.txt")
    os.chdir(path)
    
    if not os.path.exists(listFile):
        filename = open(listFile, 'w')
        filename.close()

    #obtaining list of all tests in suite, printing into testlist.txt
    cmd = file + " --gtest_list_tests > TestList.txt"
    os.system(cmd)
    filename = open(listFile, 'r')
    fileList = filename.readlines()
    
    #getting list into usable format in an array
    testNameList=[]
    mostRecentTestHeader=""
    selectTest = True
    for i in range(len(fileList)):
        if fileList[i][0] != " ":
            mostRecentTestHeader = fileList[i].strip()
            selectTest = True
        else:
            #special case for GeoCoord tests where --gtest_list_tests do not provide exact test names for many cases
            #select first test from parameterized test cases
            if (("# GetParam()" in fileList[i]) and (selectTest)):
                testName = fileList[i].split("  # GetParam()")[0].strip()
                testNameList.append(mostRecentTestHeader + testName)
                selectTest = False
            #deals with the normal case
            elif ("# GetParam()" not in fileList[i]):
                testNameList.append(mostRecentTestHeader + fileList[i].strip())
            
    filename.close()
    os.remove(listFile)

    return testNameList

def RunTest(exeName, gtestFilter, runPath, gflagsPath):
    exeFilePath, exeFileName = os.path.split(exeName)
    os.chdir(exeFilePath)

    #running test with specific gtest and umdh
    testCmd = exeFileName + " --gtest_filter=" + gtestFilter + " --umdh" + " --bsitools_path=" + gflagsPath
    result = subprocess.call(testCmd)
    
    #Saving log files in gflags folder temporarily because umdh.exe is in the same folder and is needed for comparison
    firstSnap = os.path.join(runPath, "FirstSnapshot.log")
    secondSnap = os.path.join(runPath, "SecondSnapshot.log")

    newFirstSnap = os.path.join(gflagsPath, "FirstSnapshot.log")
    newSecondSnap = os.path.join(gflagsPath, "SecondSnapshot.log")

    #special case for GeoCoord where some test names contain '/'
    if ("/" in gtestFilter):
        gtestFilter = gtestFilter.replace(r'/', '-')
    compLog = os.path.join(gflagsPath, gtestFilter + ".Comparison.log")

    if os.path.exists(newFirstSnap):
        os.remove(newFirstSnap)
    if os.path.exists(newSecondSnap):
        os.remove(newSecondSnap)
    if os.path.exists(compLog):
        os.remove(compLog)

    shutil.move(firstSnap, gflagsPath)
    shutil.move(secondSnap, gflagsPath)

    #creating log file
    os.chdir(gflagsPath)
    logCmd = "umdh FirstSnapshot.log SecondSnapshot.log -f:" + gtestFilter + ".Comparison.log -d"
    result = subprocess.call(logCmd)
    
    return compLog, newFirstSnap, newSecondSnap

def AnalyzeLog(logPath, totIncrease, testWithSuiteName):
    if not os.path.exists(logPath):
        print("ERROR: Log File not successfully created.")
        return

    filename = open(logPath,'r')
    fileLines = filename.readlines()
    
    #If the log shows an increase of at least 4000 bytes or at least 10 call stacks, the log file will be moved to the folder 
    #which will be published as an artifact in the pipeline as well as the two snapshot files used to create it
    #Otherwise, they will be removed
    lastLine = fileLines[-1]
    filename.close()
    splitLine = lastLine.split(" ")
    lastNum = int(splitLine[-1])

    if ((splitLine[1] == "increase") and (lastNum >= 4000)):
        totIncrease = True

    #checking number of call stacks
    numStacks = 0
    for line in fileLines:
        if (line[-1] == "allocations"):
            numStacks +=1

    if (numStacks >= 10):
        totIncrease = True

    if (totIncrease):
        logFailingTests.append(testWithSuiteName  + "     Call Stacks: " + str(numStacks) + "     Bytes: " + str(lastNum));

    os.chdir(scriptsDir)
    return totIncrease

def ArchiveLogs(compLog, firstSnap, secondSnap, resultsFolder):
    #Moving logs into results file
    if not os.path.exists(resultsFolder):
        os.mkdir(resultsFolder)
    
    compPath = os.path.join(resultsFolder, "Comparison.log")
    firstPath = os.path.join(resultsFolder, "FirstSnapshot.log")
    secondPath = os.path.join(resultsFolder, "SecondSnapshot.log")

    if os.path.exists(compPath):
        os.remove(compPath)
    if os.path.exists(firstPath):
        os.remove(firstPath)
    if os.path.exists(secondPath):
        os.remove(secondPath)
        
    shutil.move(compLog, os.path.join(resultsFolder, "Comparison.log"))
    shutil.move(firstSnap, os.path.join(resultsFolder, "FirstSnapshot.log"))
    shutil.move(secondSnap, os.path.join(resultsFolder, "SecondSnapshot.log"))

if __name__ == '__main__':
    #creating arguments
    parser = argparse.ArgumentParser()
    parser.add_argument("--test_exe", help = "The Full path of the Test exe e.g. --testExe=C:\BSW\Bim0200\out\winx64\Product\Bentley-GTest\BentleyTests.exe")
    parser.add_argument("--gflags_container", help = "The full path to the folder containing gflags. By default: C:/Program Files (x86)/Windows Kits/10/Debuggers/x64/ NOTE:If you are specifying a path which contains spaces, you must wrap the path in quotation marks. e.g. --umdhContainer=\"C:/Path with/Spaces\"")
    parser.add_argument("--results_container", help = "The full path to the folder in which results for failed test suites will be placed")
    args = parser.parse_args()

    exeName = args.test_exe
    exeFilePath, exeFileName = os.path.split(exeName)
    runPath = os.path.join(exeFilePath, 'run')
    gflagsContainerPath = args.gflags_container
    testResultsPath = args.results_container

    if not os.path.exists(testResultsPath):
            os.makedirs(testResultsPath)

    #Temporarily changing test exe name to keep gflags from affecting things if the script fails
    fileName = exeFileName.split(".exe")[0] #original file name without extension
    suiteName = exeFileName.split("Test")[0] #suite name
    fileNameAppended = exeFileName.replace(fileName, suiteName + "TestAPPENDED", 1)
    newExeName=os.path.join(exeFilePath, fileNameAppended)
    os.rename(exeName, newExeName)

    testNameList = ParseTestList(newExeName)
    
    #setting gflags
    os.chdir(gflagsContainerPath)
    gflagsCmd = "gflags /i " + fileNameAppended + " +ust"
    print(gflagsCmd)
    result = os.system(gflagsCmd)
    os.chdir(scriptsDir)

    #set symbol path 
    symbolName = exeFileName.replace(".exe", "_exe.pdb", 1)
    symbolCmd = "set _NT_SYMBOL_PATH=" + symbolName
    print(symbolCmd)
    result = os.system(symbolCmd)

    redFlag = False
    #running test on each individual gtest filter
    #if any of these gtests show an increase, red flag will be raised, and logs will be moved to results file
    for gtestFilter in testNameList:
        logPath, firstSnap, secondSnap = RunTest(newExeName, gtestFilter, runPath, gflagsContainerPath)
        totIncrease = AnalyzeLog(logPath, False, gtestFilter)
        if totIncrease:
            redFlag = True
            if ("/" in gtestFilter):
                gtestFilter = gtestFilter.replace(r'/', '-')
            ArchiveLogs(logPath, firstSnap, secondSnap, os.path.join(testResultsPath, gtestFilter))

        #cleaning up original files for subsequent runs of the script
        if os.path.exists(logPath):
            os.remove(logPath)
        if os.path.exists(firstSnap):
            os.remove(firstSnap)
        if os.path.exists(secondSnap):
            os.remove(secondSnap)

    #clearing gflags
    os.chdir(gflagsContainerPath)
    gflagsCmd = gflagsCmd = "gflags /i " + fileNameAppended + " -ust"
    print(gflagsCmd)
    result = os.system(gflagsCmd)

    os.rename(newExeName, exeName)

    if redFlag:
        print("\n*** POTENTIAL LEAK DETECTED ***\n")
        print("Following tests are causing the potential leak due bytes >= 4000 or Number of Stacks >= 10:");
        for item in logFailingTests:
            print("-> " + item)
        print("\nSee attached UMDH logs for more details")
        sys.exit(1)

    else:
        print("\n*** NO LEAK DETECTED ***")
        #remove the logs directory if no leak is detected
        os.rmdir(testResultsPath)
        sys.exit(0)