#--------------------------------------------------------------------------------------
#
#     $Source: iOS/CheckLogFilesForFailures.py $
#
#  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
#
#--------------------------------------------------------------------------------------
import os, sys, re

#-------------------------------------------------------------------------------------------
# bsimethod                                     Sam.Wilson      05/2016
#-------------------------------------------------------------------------------------------
def checkLogFileForFailures(logfilename):
    report = '\n' + logfilename + '\n'

    lineNo = 0
    xctestproj = ''
    lastLine = ''
    failed = False
    foundCrash = False
    foundTerminator = False
    foundAddressSanitizerErrors = False
    failedTests = ''
    comma = ''
    errpat = re.compile (r"error\:\s*\-\[(\w+)\s*(\w+).*failed")
    with open(logfilename, 'r') as logfile:
        for line in logfile.readlines():

            if lineNo == 0:
               if line.lower().find('ios xctests were not run') != -1:
                   report = report + line
                   return 0,report

            lastLine = line

            lineNo = lineNo + 1
            if lineNo == 1:
                xctestproj = line
                continue

            lline = line.lower()

            if -1 != lline.find('summary: addresssanitizer'):
                foundAddressSanitizerErrors = True
		failed = True

            # We get ** test failed ** if the build or the prep or the tests failed
            if -1 != lline.find('** test failed **'):
                failed = True
                foundTerminator = True

            # xcodebuild prints ** test succeeded ** if all tests succeeded
            if -1 != lline.find('** test succeeded **'):
                foundTerminator = True

            err = errpat.search(line, re.IGNORECASE)
            if err != None:
                failedTests = failedTests + comma + err.group(1) + "." + err.group(2)
                comma = ', '
                failed = True
       
            crashmsg = 'unexpected exit or crash in '
            icrashmsg = lline.find(crashmsg)
	    if icrashmsg != -1:
                failed = True
                foundCrash = True
                itnamestart = icrashmsg + len(crashmsg)
                itnameend = lline.find(';')
                failedTests = failedTests + comma + line[itnamestart:itnameend] + '(*)'
                comma = ', '
                
     
    if foundTerminator  == False:
        failed = True

    if failed:
        if len(failedTests) != 0:
            report = report + "*** FAILURES ***\n"
            report = report + "To debug, open\n"
            report = report + "    " + xctestproj
            report = report + "and re-run the following tests:\n"
            report = report + failedTests + "\n"
            if foundCrash:
                report = report + "(*) crashed."
            if foundAddressSanitizerErrors:
                report = report + '*** ADDRESS SANITIZER ERRORS DETECTED ***\nSee log for details.'
        else:
            report = report + "*** BUILD FAILURE OR CRASH ***\n"
            report = report + "See " + logfilename + " for details\n"
    else:
        report = report + lastLine


    return failed, report

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

