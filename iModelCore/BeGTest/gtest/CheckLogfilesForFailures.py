#--------------------------------------------------------------------------------------
#
#     $Source: gtest/CheckLogfilesForFailures.py $
#
#  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
#
#--------------------------------------------------------------------------------------
import os, sys, re
from sets import Set

# Failure patterns to match
# [  FAILED  ] RleEditorTester.MaxRleRunSetPixels    
# [  FAILED  ] FileFormatTests/ExportTester.ToBestiTiff/21, where GetParam() = L"D:\\ATP\\Dataset\\Images_Files\\_forATPs\\Images\\jpeg\\Mosaic_Jpeg_WF\\TC06L0\\15.jpg"
errpat = re.compile (r"FAILED\s*]\s*(\w+\.\w+|\w+/\w+\.\w+/\d+)", re.I)
summarypat = re.compile (r"\[==========\].*ran", re.I)
runpat = re.compile (r"RUN\s*]\s*(\w+\.\w+)", re.I)

#-------------------------------------------------------------------------------------------
# bsimethod                                     Sam.Wilson      05/2016
#-------------------------------------------------------------------------------------------
def checkLogFileForFailures(logfilename):
    exename = ''
    failedtests = Set()
    foundSummary = False
    lastTestSeen = ""
    lineNo = 0
    with open(logfilename, 'r') as logfile:
        for line in logfile.readlines():

            lineNo = lineNo + 1

            # Name of test runner exe is always the first line
            if lineNo == 1:
                exename = line
                continue

            run = runpat.search(line)
            if run != None:
                lastTestSeen = run.group(1)
            else:
                err = errpat.search(line)
                if err != None:
                    failedtests.add(err.group(1))
                else:
                    if summarypat.search(line) != None:
                         foundSummary = True


    crashed = False
    if not foundSummary:
        print ""
        print "*** TEST CRASHED ***"
        print ""
        print lastTestSeen
        print ""
        print "To run just the crashing test, run the following command in your debugger:"
        print "    " + exename + " --gtest_break_on_failure --gtest_filter=" + lastTestSeen
        print ""
        crashed = True

    if len(failedtests) != 0:
        print ""
        print "*** TESTS FAILED ***"
        comma = ""
        failedTestsStr = ""
        for t in failedtests:
            print t
            failedTestsStr = failedTestsStr + comma + t
            comma = ","
        print "********************"
        print ""
        print "To re-run failing tests, run the following command in your debugger:"
        print "    " + exename + " --gtest_break_on_failure --gtest_filter=" + failedTestsStr
        print ""

    if (0 != len(failedtests)) or crashed:
        print 'See ' + logfilename + ' for failure details'

    return len(failedtests) + crashed

#-------------------------------------------------------------------------------------------
# bsimethod                                     Sam.Wilson      05/2016
#-------------------------------------------------------------------------------------------
if __name__ == '__main__':
    failureCount = 0
    checkedCount = 0
    failedProductCount = 0

    if len(sys.argv) < 2:
        print 'Syntax: ' + sys.argv[0] + ' logfilesdir [breakonfailureflag]'
        exit(1)

    dir = sys.argv[1]
    breakonfailure = False
    if len(sys.argv) > 2 and sys.argv[2] != 0:
        breakonfailure = True

    for root,dirs,files in os.walk (dir, topdown=True, onerror=None, followlinks=True):
        for file in files:
            if not file.endswith('.log'):
                continue;

            checkedCount = checkedCount + 1
            path = os.path.join(root, file)
            failures = checkLogFileForFailures(path)
            if failures != 0:
                failureCount = failureCount + failures
                failedProductCount = failedProductCount + 1
     
    if checkedCount == 0:
        print 'no logfiles found in ' + dir
        exit(0)

    print '{0} test product logs found'.format(checkedCount)

    if failureCount != 0:
        print "{0} tests in {1} products failed".format(failureCount, failedProductCount)
    else:
        print "All tests passed."

    if not breakonfailure:
        exit(0)

    exit (failureCount)

