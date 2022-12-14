#---------------------------------------------------------------------------------------------
#  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
#  See LICENSE.md in the repository root for full copyright notice.
#---------------------------------------------------------------------------------------------
import os, sys, re

# Failure patterns to match
# [  FAILED  ] RleEditorTester.MaxRleRunSetPixels
# [  FAILED  ] FileFormatTests/ExportTester.ToBestiTiff/21, where GetParam() = L"D:\\ATP\\Dataset\\Images_Files\\_forATPs\\Images\\jpeg\\Mosaic_Jpeg_WF\\TC06L0\\15.jpg"
# [  FAILED  ] Base64UtilitiesTests: SetUpTestSuite or TearDownTestSuite
failedpat = re.compile (r"FAILED\s*]\s*(\w+\.\w+|\w+/\w+\.\w+/\d+|\w+(?=:))", re.I)
summarypat = re.compile (r"\[==========\].*ran", re.I)
runpat = re.compile (r"RUN\s*]\s*(\w+\.\w+)", re.I)

filedir=os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.append(filedir)
sys.dont_write_bytecode = True

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def printLogFile(logFilename, hasErrors=False):
    if not hasErrors:
        print("ERROR_PARSE_SUSPEND")

    print("BEGIN_TEST_LOG " + logFilename)

    with open(logFilename, 'r') as logfile:
        for line in logfile.readlines():
            sys.stdout.write(line)

    print("END_TEST_LOG " + logFilename)

    if not hasErrors:
        print("ERROR_PARSE_RESUME")

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def checkLogFileForFailures(logfilename):
    global exename
    exename = ''
    foundSummary = False
    global failedTestsList
    failedTestsList = ""
    lastTestRun = ""
    colon = ''
    anyFailures = False
    lineNo = 0
    summarystr = logfilename + '\n'
    lines=''
    global failedtestlist_temp
    failedtestlist_temp=[]

    with open(logfilename, 'r') as logfile:
        lines=logfile.readlines()
        for line in lines:

            lineNo = lineNo + 1

            # Name of test runner exe is always the first line
            if lineNo == 1:
                exename = line.strip('\n')
                continue

            if not foundSummary:
                # We don't look for much in the log before the summary.
                if summarypat.search(line) != None:
                    summarystr = summarystr + line
                    foundSummary = True
                else:
                    # We do keep track of the last test run, in case we need to report a crash or early exit
                    run = runpat.search(line)
                    if run != None:
                        lastTestRun = run.group(1)

                continue

            else:
                # The summary has the results, either the count of tests run or a list of failing tests.
                summarystr = summarystr + line

                if line.find('FAILED TEST') != -1:
                    anyFailures = True

                failed = failedpat.search(line)
                if failed != None:
                    failedTestSelector = failed.group(1)
                    if '.' not in failedTestSelector:
                      failedTestSelector += '.*'
                    failedTestsList = failedTestsList + colon + failedTestSelector
                    colon = ':'
                    failedtestlist_temp.append(failedTestSelector)
                    continue

    #advicestr = '************ Failures from: ' + logfilename + ' ******************'
    advicestr = ''

    # If we never got to the summary, that means that the last test to run was interrupted by a crash
    if not foundSummary:
        advicestr = advicestr + "\n"
        advicestr = advicestr + "\n*** CRASH ***"
        advicestr = advicestr + "\n"
        advicestr = advicestr + "\nERROR: Test crashed in " + exename + " - " + lastTestRun
        advicestr = advicestr + "\n"
        advicestr = advicestr + "\nTo run just the crashing test, run the following command in your debugger:"
        advicestr = advicestr + "\n    " + exename + " --gtest_break_on_failure --gtest_filter=" + lastTestRun
        advicestr = advicestr + "\n"

    # Report all failures that we saw in the log
    if len(failedTestsList) != 0:
        advicestr = advicestr + "\n"
        advicestr = advicestr + "\nERROR: Test(s) failed in " + exename + " - " + failedTestsList
        advicestr = advicestr + "\n"
        advicestr = advicestr + "\nTo re-run failing tests, run the following command in your debugger:"
        advicestr = advicestr + "\n    " + exename + " --gtest_break_on_failure --gtest_filter=" + failedTestsList
        advicestr = advicestr + "\n"

    if anyFailures or not foundSummary:
        # When we detect failing or crashing tests, print the whole log. That will then go into bb's build log.
        # The user will want to scroll up to see complete details.
        print ('************ ' + logfilename + ' ******************')
        printLogFile(logfilename, hasErrors=True)
        print ('*********************************************************************************')

    return advicestr,summarystr,failedTestsList

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
if __name__ == '__main__':
    checkedCount = 0

    if len(sys.argv) < 2:
        print ('Syntax: ' + sys.argv[0] + ' logfilesdir [breakonfailureflag]')
        exit(1)

    dir = sys.argv[1]
    breakonfailure = False

    if len(sys.argv) > 2 and int(sys.argv[2]) != 0:
        breakonfailure = True

    advicestr = ''
    summarystr = ''
    exename = ''
    failedTestsList=''
    temp=''
    for root,dirs,files in os.walk (dir, topdown=True, onerror=None, followlinks=True):
        for file in files:
            if not file.endswith('.log'):
                continue

            checkedCount = checkedCount + 1
            path = os.path.join(root, file)
            adviceForThisLog,summarystrThisLog,failedTestsList = checkLogFileForFailures(path)
            summarystr = summarystr + '\n\n' + summarystrThisLog
            if 0 != len(adviceForThisLog):
                advicestr = advicestr + '\n\n' + adviceForThisLog
            if 0 != len(failedTestsList):
                if temp=='':
                    temp=failedTestsList
                else:
                    temp = temp +":"+failedTestsList

    if checkedCount == 0:
        print ('no logfiles found in ' + dir)
        exit(0)

    print ('{0} test product logs found'.format(checkedCount))

    print (summarystr)

    if 0 == len(advicestr):
        print ("All tests passed.")
        exit (0)

    failedTestsList=temp.split(":")

    print (advicestr)
    exit(breakonfailure)
