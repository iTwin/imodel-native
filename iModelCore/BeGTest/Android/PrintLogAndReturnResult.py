#--------------------------------------------------------------------------------------
#
#     $Source: Android/PrintLogAndReturnResult.py $
#
#  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
#
#--------------------------------------------------------------------------------------
import os, sys

#-------------------------------------------------------------------------------------------
# bsimethod                                     Sam.Wilson      05/2016
#-------------------------------------------------------------------------------------------
if __name__ == '__main__':
    failureCount = 0
    with open(sys.argv[1], 'r') as logfile:
        for line in logfile.readlines():
            if line.lower().startswith("failure in test"):
                failureCount = failureCount + 1
                print line,
     
    if failureCount != 0:
        print "{0} tests failed".format(failureCount)
    else:
        print "All tests passed."

    exit (failureCount)

