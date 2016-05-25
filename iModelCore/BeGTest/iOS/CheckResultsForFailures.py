#--------------------------------------------------------------------------------------
#
#     $Source: iOS/CheckResultsForFailures.py $
#
#  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
#
#--------------------------------------------------------------------------------------
import os, sys, re

#-------------------------------------------------------------------------------------------
# bsimethod                                     Sam.Wilson      05/2016
#-------------------------------------------------------------------------------------------
if __name__ == '__main__':
    failed = False
    failedTests = ''
    comma = ''
    errpat = re.compile (r"error\:\s*\-\[(\w+)\s*(\w+).*failed")
    with open(sys.argv[1], 'r') as logfile:
        for line in logfile.readlines():

            lline = line.lower()

            # We get ** test failed ** if the build or the prep or the tests failed
            if -1 != lline.find('** test failed **'):
                print line,
                failed = True

            err = errpat.search(line, re.IGNORECASE)
            if err != None:
                failedTests = failedTests + comma + err.group(1) + "." + err.group(2)
                comma = ', '
                failed = True
     
    if failed:
        if len(failedTests) != 0:
            print "*** FAILURES ***"    
            print "To debug, open"
            print "    " + sys.argv[2]
            print "and re-run the following tests:"
            print failedTests
        else:
            print "See XCTest.log for details"
    else:
        print "All tests passed."

    exit (failed)

