#--------------------------------------------------------------------------------------
#
#     $Source: Android/CheckLogFilesForFailures.py $
#
#  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
#
#--------------------------------------------------------------------------------------
import os, sys, re

#-------------------------------------------------------------------------------------------
# bsimethod                                     Sam.Wilson      05/2016
#-------------------------------------------------------------------------------------------
def checkLogFileForFailures(logfilename):
    report = '\n' + logfilename 

    failureCount = 0

    foundSummary = False
    lineNo = 0

    with open(logfilename, 'r') as logfile:
        for line in logfile.readlines():
            lline = line.lower()
    
            if lineNo == 0:
                if lline.startswith('android junit tests were not run'):
                    report = report + '\n' + line
                    return 0,report

            lineNo = lineNo + 1

            if lline.startswith("failure in test"):
                failureCount = failureCount + 1
                report = report + line
                continue

            if lline.find('process crashed') != -1 or lline.find('instrumentation_result: shortmsg=native crash') != -1:
                failureCount = failureCount + 1
                report = report + line
                continue

            if lline.startswith('test results for beinstrumentationtestrunner'):
                foundSummary = True
                continue

            if foundSummary and lline.startswith('time:'):
                report = report + line

            if lline.startswith("ok ("):
                report = report + line

    # There is no point in printing the log file, as it does not contain any details of the failures.
    # The user must run "adb logcat" in order to see the native code logging output.
    # We cannot do that here.

    return failureCount, report
    
#-------------------------------------------------------------------------------------------
# bsimethod                                     Sam.Wilson      05/2016
#-------------------------------------------------------------------------------------------
if __name__ == '__main__':
    failureCount = 0
    checkedCount = 0
    report = ''
    failedProductCount = 0

    dir = sys.argv[1]
    breakonfailure = False
    if len(sys.argv) > 2 and int(sys.argv[2]) != 0:
        breakonfailure = True

    for root,dirs,files in os.walk (dir, topdown=True, onerror=None, followlinks=True):
        for file in files:
            if not file.endswith('.log'):
                continue;

            checkedCount = checkedCount + 1
            path = os.path.join(root, file)
            failures, reportThisLog = checkLogFileForFailures(path)
            report = report + reportThisLog
            if failures != 0:
                failureCount = failureCount + failures
                failedProductCount = failedProductCount + 1
     
    if checkedCount == 0:
        print 'no logfiles found in ' + dir
        exit(0)

    print '{0} test product logs found'.format(checkedCount)

    print report

    if failureCount == 0:
        print "All tests passed."

    if not breakonfailure:
        exit(0)

    exit (failureCount)

