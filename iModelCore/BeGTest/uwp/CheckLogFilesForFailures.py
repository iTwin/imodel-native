#---------------------------------------------------------------------------------------------
#  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
#  See COPYRIGHT.md in the repository root for full copyright notice.
#---------------------------------------------------------------------------------------------
import os, sys, re

summaryRe = re.compile('Total tests[:]\s*(\d+)[.]\s+Passed[:]\s*(\d+)[.]\s+Failed[:]\s*(\d+)')
failedTestRe = re.compile('Failed\s+(\w+)')

#-------------------------------------------------------------------------------------------
# bsimethod                                     Sam.Wilson      05/2016
#-------------------------------------------------------------------------------------------
def checkLogFileForFailures(logfilename):
    summarystr = logfilename + '\n'

    lineNo = 0
    previousLine = ''
    failedTestsList = ''
    advicestr = ''
    comma = ''
    foundSummary = False
    collectingErrorDetails = False

    with open(logfilename, 'r') as logfile:
        for line in logfile.readlines():

            if collectingErrorDetails:
                advicestr = advicestr + '\n' + line.rstrip()
                continue

            lline = line.lower()
    
            if lineNo == 0:
                if lline.startswith('cppunittests were not run'):
                    summarystr = summarystr + line
                    return '',summarystr

            lineNo = lineNo + 1

            if lline.find('test run failed.') != -1:
                summarystr = summarystr + line
                continue

            #Error: Installation of package 'C:\dgndb61-16q4\out\WinRTx64\Product\DgnClientFx-UwpTest\AppPackages\BeTestTest1\BeTestTest1_1.0.0.0_x64_Debug_Test\BeTestTest1_1.0.0.0_x64_Debug.appx' failed with Error: (0x5B4) Operation timed out. Unable to install Windows app package in 30 sec..
            #For more details look into Event Viewer under Applications and Services Logs -> Microsoft -> Windows -> AppXDeployment-Server -> Microsoft-Windows-AppXDeploymentServer/Operational.
            if lline.find('installation of package') != -1 and lline.find('failed') != -1:
                advicestr = advicestr + '\n' + line
                collectingErrorDetails = True
                continue

            if lline.find('the test execution process crashed while running the tests.') != -1:
                advicestr = advicestr + '\n' + line.rstrip() + '\nWhile attempting to run:'
                advicestr = advicestr + '\n' + previousLine
                summarystr = summarystr + line
                collectingErrorDetails = True
                continue

            failedTest = failedTestRe.match(line)
            if failedTest != None:
                failedTestsList = failedTestsList + comma + failedTest.group(1)
                comma = ', '
                continue

            if lline.startswith('total tests:'):
                stats = summaryRe.search(line)
                if stats.group(3) != '0':
                    anyFailures = True
                    advicestr = advicestr + '\n' + stats.group(3)  + ' tests failed'

                summarystr = summarystr + line
                foundSummary = True

            previousLine = line

    if lineNo == 0:
        summarystr = summarystr + '\n' + 'Empty test results log. Tests were not run?'
        return '',summarystr

    # Report all failures that we saw in the log
    if len(failedTestsList) != 0:
        advicestr = advicestr + "\nThe following tests failed:  " + failedTestsList

    if not foundSummary:
        advicestr = advicestr + "\nA hard error preempted or interrupted the tests"

    # There is no point in printing the log file, as it does not contain any details of the failures.
    # The user must look at the native code logging output.
    # We cannot do that here.

    return advicestr,summarystr
    
#-------------------------------------------------------------------------------------------
# bsimethod                                     Sam.Wilson      05/2016
#-------------------------------------------------------------------------------------------
if __name__ == '__main__':
    if len(sys.argv) < 2:
        print 'Syntax: ' + sys.argv[0] + ' logfilesdir [breakonfailureflag]'
        exit(1)

    dir = sys.argv[1]
    breakonfailure = False
    if len(sys.argv) > 2 and int(sys.argv[2]) != 0:
        breakonfailure = True

    advicestr = ''
    summarystr = ''
    checkedCount = 0
    for root,dirs,files in os.walk (dir, topdown=True, onerror=None, followlinks=True):
        for file in files:
            if not file.endswith('.log'):
                continue;

            checkedCount = checkedCount + 1
            path = os.path.join(root, file)
            adviceForThisLog,summarystrThisLog = checkLogFileForFailures(path)
            summarystr = summarystr + '\n' + summarystrThisLog
            if 0 != len(adviceForThisLog):
                advicestr = advicestr + '\n\n' + path + adviceForThisLog
     
    if checkedCount == 0:
        print 'no logfiles found in ' + dir
        exit(0)

    print '{0} test product logs found'.format(checkedCount)

    print summarystr

    if 0 == len(advicestr):
        print "All tests passed."
        exit (0)

    print '***************** TEST FAILURES ********************'
    print advicestr
    exit(breakonfailure)

