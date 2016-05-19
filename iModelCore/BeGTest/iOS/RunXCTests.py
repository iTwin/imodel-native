#--------------------------------------------------------------------------------------
#
#     $Source: iOS/RunXCTests.py $
#
#  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
#
#--------------------------------------------------------------------------------------
import os, glob, sys, string, stat, re, subprocess

import sys
sys.path.append(os.path.join (os.getenv("SrcRoot"), "bsicommon", "build"))
from bentleybuild.utils import *

#-------------------------------------------------------------------------------------------
# bsimethod                                     Sam.Wilson              04/2016
#-------------------------------------------------------------------------------------------
def printProgress(procStdOutLine, status_len):
    msg = procStdOutLine.strip('\n')
    msg = msg.replace('\t', ' ')
    showRewritableLine(msg, INFO_LEVEL_Important)

#-------------------------------------------------------------------------------------------
# bsimethod                                     Sam.Wilson              04/2016
#-------------------------------------------------------------------------------------------
def RunTest(xcodeprojpath, deviceName, okToRetry, logfile):
    status_len = 132

    mustRetry = False
    failureCount = 0
    testCount = 0
    testSuiteCount = 0

    errpat = re.compile (r"error\:\s*\-\[(\w+)\s*(\w+).*failed")

    cmd = ["xcodebuild", "test", "-project", xcodeprojpath, "-scheme", "BeTestiOS", "-destination", "'platform=iOS,name=" + deviceName + "'"]
    print " ".join(cmd)
    proc = subprocess.Popen(" ".join(cmd), stdout=subprocess.PIPE, stderr=subprocess.STDOUT, shell = True)
    procStdOutLine = proc.stdout.readline ()
    while procStdOutLine:
        
        lline = procStdOutLine.lower()

        # We often get an error when copying resources to the device the first time we try to run a given set of tests
        if lline.startswith('cp') and -1 != lline.find('permission denied'):
            mustRetry = True
            break
        
        #if lline.find('error') != -1:
        #    print '\n' + procStdOutLine

        printProgress(procStdOutLine, status_len)

        logfile.write(procStdOutLine)

        if lline.startswith('test case'):
            testCount = testCount + 1
        else:
            if lline.startswith('test suite'):
                testSuiteCount = testSuiteCount + 1
            else:
                starterr = lline.find('error:')
                if -1 != starterr and -1 != lline.find('failed'):
                    err = errpat.match(procStdOutLine[starterr:])
                    if err != None:
                        print "\nFAILED " + err.group(1) + "." + err.group(2)
                        failureCount = failureCount + 1

        procStdOutLine = proc.stdout.readline ()

    retval = proc.wait()

    if mustRetry:
        if not okToRetry:
            print '\nERROR Failed to copy resources to device'
            return 1
        else:
            print '\nFailed to copy resources to device. Retrying...'
            testSuiteCount, testCount, failureCount = RunTest(walkRoot, dirName, deviceName, False)

    printProgress(' ', status_len)
    print '{0} suites, {1} tests, {2} failures'.format(testSuiteCount, testCount, failureCount)

    if failureCount != 0:
        print 'To debug the failed tests, use the following command line to open the test xcodeproj:'
        print '  open ' + xcodeprojpath
        print 'Then go to Test Navigator, filter on the test you want to run, and click its run arrow'

    # if any test fails to build or if any test does not succeed, the xcodebuild test command should return a non-zero status. 
    return retval

#-------------------------------------------------------------------------------------------
# bsimethod                                     Sam.Wilson              04/2016
#-------------------------------------------------------------------------------------------
def main():
    if (len (sys.argv) < 4):
        print "Syntax: ", sys.argv[0], " xcodeprojpath deviceName logfilename"
        exit(1)

    xcodeprojpath = sys.argv[1]
    deviceName = sys.argv[2]
    if deviceName.startswith("'"):
        deviceName = deviceName.substr(1, len(deviceName)-2)
    logfilename = sys.argv[3]

    with open (logfilename, 'w') as logfile:
        failureCount = RunTest(xcodeprojpath, deviceName, True, logfile)

    exit (failureCount)

if __name__ == '__main__':
    main()
