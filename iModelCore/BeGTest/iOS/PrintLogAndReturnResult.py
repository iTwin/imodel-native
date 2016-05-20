#--------------------------------------------------------------------------------------
#
#     $Source: iOS/PrintLogAndReturnResult.py $
#
#  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
#
#--------------------------------------------------------------------------------------
import os, sys, re

#-------------------------------------------------------------------------------------------
# bsimethod                                     Sam.Wilson      05/2016
#-------------------------------------------------------------------------------------------
if __name__ == '__main__':
    failureCount = 0
    errpat = re.compile (r"error\:\s*\-\[(\w+)\s*(\w+).*failed")
    with open(sys.argv[1], 'r') as logfile:
        for line in logfile.readlines():

            lline = line.lower()

            # We get ** test failed ** if the build or the prep or the tests failed
            if -1 != lline.find('** test failed **'):
                failureCount = failureCount + 1

            starterr = lline.find('error:')
            if -1 != starterr and -1 != lline.find('failed'):
                err = errpat.match(procStdOutLine[starterr:])
                if err != None:
                    print "\nFAILED " + err.group(1) + "." + err.group(2)
                    failureCount = failureCount + 1
     
    if failureCount != 0:
        print "{0} tests failed".format(failureCount)
    else:
        print "All tests passed."

    exit (failureCount)

