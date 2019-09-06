#--------------------------------------------------------------------------------------
#
#     $Source: MemoryLeak/DetectMemLeak.py $
#
#  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
#
#--------------------------------------------------------------------------------------
import os
import sys
import shutil
import subprocess
import argparse
import csv

scriptsDir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
commonScriptsDir = os.path.join(scriptsDir, 'CommonTasks')
sys.path.append(commonScriptsDir)
import Components as cmp

#-------------------------------------------------------------------------------------------
# bsimethod                     Majd.Uddin    07/2017
#-------------------------------------------------------------------------------------------
def singleTestLeak(exeName, testName, logFile):
    cmdForOneTest = exeName + ' --gtest_filter=' + testName + ' --umdh'
    print cmdForOneTest
    result = subprocess.call(cmdForOneTest)

    if result != 0: #there was some error
        return True

    if not os.path.exists(logFile): #somehow log was not generated
        print "not exists " + logFile
        return True
    filename = open(logFile,'r')
    fileList = filename.readlines()
    num=0

    redFlag = False
    for num in range(len(fileList)):
        if 'TestBody' in fileList[num]:
            for backTr in reversed(fileList[0:num]):
                if 'BackTrace' in backTr:
                    print backTr
                    splitTr = backTr.split(" ")
                    for pos, item in enumerate(splitTr):
                        if item == "(":
                            print splitTr[pos-1]
                            if int(splitTr[pos-1])%5 != 1:
                                redFlag = True
                            break    
                    break
    filename.close()
    return redFlag
    
#-------------------------------------------------------------------------------------------
# bsimethod                     Majd.Uddin    07/2017
#-------------------------------------------------------------------------------------------
def printColored(string, colorCode, bold):
    attr = []
    attr.append(colorCode)
    if bold:
        attr.append('1')
    return '\x1b[%sm%s\x1b[0m' % (';'.join(attr), string)

#-------------------------------------------------------------------------------------------
# bsimethod                     Majd.Uddin    07/2017
#-------------------------------------------------------------------------------------------
def getTestsListFromLog(logFile):
    testList = []
    fileTestNames = open(logFile,'r')
    line_num = 0
    search_phrase = "[       OK ]"
    count = 0
    fileToOpNew=""

    for testName in fileTestNames.readlines():
        line_num += 1
        if testName.find(search_phrase) >= 0:
            testName = testName.split(" ")[-3]
            testList.append(testName)
    return testList

#-------------------------------------------------------------------------------------------
# bsimethod                     Majd.Uddin    07/2017
#-------------------------------------------------------------------------------------------
def printResults(testResults, failDir):
    print printColored('\n *** Memory Leak Test Results: \n', '36', True)
    failCount = passCount = 0
    for testName in testResults:
        if testResults[testName]:
            print printColored('[  FAILED  ] ', '31', True) + testName
            failCount = failCount + 1
        else:
            print printColored('[  PASSED  ] ', '32', True) + testName
            passCount = passCount + 1
    print printColored('\n *** Summary: ', '36', True)
    print printColored('     Total Tests: ' + str(failCount + passCount), '36', True)
    print printColored('     Failed Tests: ' + str(failCount), '31', True)
    print printColored('     Passed Tests: ' + str(passCount), '32', True)
    print printColored('\n *** Repoart and any failure log(s) are at: ' + failDir, '36', True)

#-------------------------------------------------------------------------------------------
# bsimethod                     Majd.Uddin    07/2017
#-------------------------------------------------------------------------------------------
def saveResultsAsCSV(testResults, failDir, exefileName):
    passFail = {'[True]': 'Failed', 'True' : 'Failed', '[]': 'Passed', 'False': 'Passed'}

    fN, fExt = os.path.splitext(exefileName)
    compName = fN[:fN.find('Test')]
    csvFile = open(os.path.join(failDir, 'MemLeakResults.csv'), 'wb')
    writer = csv.writer(csvFile)
    writer.writerow(('Component', 'TestName', 'Result'))
    for testName in testResults:
        writer.writerow((compName, testName, passFail[str(testResults[testName])]))
    csvFile.close()
def copytree(src, dst, symlinks=False, ignore=None):
    for item in os.listdir(src):
        s = os.path.join(src, item)
        d = os.path.join(dst, item)
        if os.path.isdir(s):
            shutil.copytree(s, d, symlinks, ignore)
        else:
            shutil.copy2(s, d)
#-------------------------------------------------------------------------------------------
# bsimethod                     Majd.Uddin    07/2017
#-------------------------------------------------------------------------------------------
def clearpreviouslogs(dir):
    for f in os.listdir(dir):
        fN, fExt = os.path.splitext(f)
        if fExt == '.log':
            os.remove(os.path.join(dir,f))
#-------------------------------------------------------------------------------------------
# bsimethod                     ali.hasan    09/2019
#-------------------------------------------------------------------------------------------
def runTest(exeName):
    logDir = ""
    OutputDir = ""
    if exeName == None:
        print printColored('\n Script requires to get test Exe Name. Error.', '31', True)
        exit(-1)
    if not os.path.exists(exeName):
        print printColored('\n Test exe could not be found. ' + exeName, '31', True)
        exit(-1)

    exefilePath, exefileName = os.path.split(exeName)
    exefilePath1, exefileName1 = os.path.split(exefilePath)
    exefilePath2, exefileName2 = os.path.split(exefilePath1)
    logDir = os.path.join(exefilePath2, 'LogFiles')
    OutputDir = os.path.join(logDir, 'OutputDir',exefileName)
    if os.path.exists(OutputDir):
        shutil.rmtree(OutputDir)
    os.makedirs(OutputDir)
    print "Path " + OutputDir

    exefilePath, exefileName = os.path.split(exeName)
    runDir = os.path.join(exefilePath, 'run')
    testName = args.gtest_filter
    testResults = {}

    # Remove previous run and create folder for failure report
    failDir = os.path.join(runDir, 'MemLeakFailures')
    if os.path.exists(failDir):
        shutil.rmtree(failDir)
    os.makedirs(failDir)
        
    testlog = os.path.join(failDir, 'testlist.log')
    
    if testName == None:
        print '\n\n Let us first run the tests \n\n'
        print 'No gtest_filter specified. It will take some time to run tests and then detect Memory leaks.'
        cmdForTests = exeName + ' > ' + testlog
        result = os.system(cmdForTests)
        if result == 0 : # Test run successful
            testList = getTestsListFromLog(testlog)
            testCounter = 1
            for testName in testList:
                logFile = os.path.join(runDir, testName + '.log')
                testResults.setdefault(testName, [])
                print printColored( '\nPerformaing Memory Leak Detection for Test: ' + str(testCounter) + ' of ' + str(len(testList)), '36', True)
                failed = singleTestLeak(exeName, testName, logFile)           
                if failed:
                    print "In Failing...."
                    if os.path.exists(logFile):
                        shutil.copy2(logFile, failDir)
                    testResults[testName].append(failed)
                testCounter = testCounter + 1
        else:
            print printColored('\n Test execution failed for: ' + exeName, '31', True)
            exit(-1)
                        
    else:
        if '*' in testName or ';' in testName:
            print '\n\n Let us first run the tests \n\n'
            cmdForTests = exeName + ' --gtest_filter=' + testName  + ' > ' + testlog
            result = os.system(cmdForTests)
            if result == 0 : # Test run successful
                testList = getTestsListFromLog(testlog)
                testCounter = 1
                for testName in testList:
                    logFile = os.path.join(runDir, testName + '.log')
                    testResults.setdefault(testName, [])
                    print printColored( '\nPerformaing Memory Leak Detection for Test: ' + str(testCounter) + ' of ' + str(len(testList)), '36', True)
                    failed = singleTestLeak(exeName, testName, logFile)           
                    if failed:
                        if os.path.exists(logFile):
                            shutil.copy2(logFile, failDir)
                        testResults[testName].append(failed)
                    testCounter = testCounter + 1
            else:
                print printColored('\n Test execution failed for: ' + exeName, '31', True)
                exit(-1)

        else:
            logFile = os.path.join(runDir, testName + '.log')
            failed = singleTestLeak(exeName, testName, logFile)
            testResults.setdefault(testName, failed)
            if failed:
                if os.path.exists(logFile):
                    shutil.copy2(logFile, failDir)

#Clean up, print and save results
    clearpreviouslogs(runDir)
    printResults(testResults, failDir)
    saveResultsAsCSV (testResults, failDir, exefileName)
    copytree(failDir,OutputDir)

#-------------------------------------------------------------------------------------------
# bsimethod                     Majd.Uddin    07/2017
#-------------------------------------------------------------------------------------------
if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("--testExe", help = "The Full path of the Tests exe e.g. --testExe=C:\BSW\Bim0200\out\winx64\Product\Bentley-GTest\BentleyTests.exe", required = False)
    parser.add_argument("--gtest_filter", help = "The usual gtest_filter to run it selectively e.g. --gtest_filter=MyTest.Test1", nargs='?')


    ComponentList = ["BeSQLite","BeHttp","ECDb","ECObjects","ECPresentation","GeomLibs","Units","WSClient","ConstructionSchema","DgnClientFx","DgnDisplay","DgnDomains-Planning" ]
            
    args = parser.parse_args()
    exeName = args.testExe
    if exeName != None:
        runTest(exeName)
    elif exeName == None:
        comps1 = []
        comps1 = cmp.AllCompsProper()
        exes = ""
        for component in comps1:
            if component in ComponentList:
                print '***Printing components: ' + component
                exes = cmp.ExePathForComp(component)
                print "Exe to run: " + exes
                runTest(exes)