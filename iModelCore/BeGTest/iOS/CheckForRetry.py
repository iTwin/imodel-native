#--------------------------------------------------------------------------------------
#
#     $Source: iOS/CheckForRetry.py $
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
    with open(sys.argv[1], 'r') as logfile:
        for line in logfile.readlines():
            lline = line.lower()
            if lline.startswith('cp') and -1 != lline.find('permission denied'):
                print "retry"
                exit(0)
    print "not found"
    exit (0)

