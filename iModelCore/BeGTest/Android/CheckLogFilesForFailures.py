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
def checkLogFileForFailures(file):
    failureCount = 0
    with open(file, 'r') as logfile:
        for line in logfile.readlines():
            lline = line.lower()
            if lline.startswith("failure in test") or lline.startswith('instrumentation_result:'):
                failureCount = failureCount + 1
                print line,
            else:
                if lline.find('process crashed') != -1:
                    failureCount = failureCount + 1
                    print line,
                else:
                    if lline.startswith("ok ("):
                        print line

    return failureCount
    
#-------------------------------------------------------------------------------------------
# bsimethod                                     Sam.Wilson      05/2016
#-------------------------------------------------------------------------------------------
if __name__ == '__main__':
    failureCount = 0
    checkedCount = 0
    failedProductCount = 0

    dir = sys.argv[1]
    breakonfailure = False
    if len(sys.argv) > 2 and sys.argv[2] != 0:
        breakonfailure = True

    for root,dirs,files in os.walk (dir, topdown=True, onerror=None, followlinks=True):
        for file in files:
            if not file.endswith('.log'):
                continue;

            print root

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

