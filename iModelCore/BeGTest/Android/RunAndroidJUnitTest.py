#--------------------------------------------------------------------------------------
#
#     $Source: Android/RunAndroidJUnitTest.py $
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
def RunTest(adbCmd, logfile):
    status_len = 132

    failureCount = 0
    testCount = 0
    testSuiteCount = 0

    cmd = [adbCmd, "shell", "am", "instrument", "-w", "com.bentley.unittest/com.bentley.unittest.BeInstrumentationTestRunner"]
    print " ".join(cmd)
    proc = subprocess.Popen(" ".join(cmd), stdout=subprocess.PIPE, stderr=subprocess.STDOUT, shell = True)
    procStdOutLine = proc.stdout.readline ()
    while procStdOutLine:
        
        # We often get an error when copying resources to the device the first time we try to run a given set of tests
        if procStdOutLine.startswith('com.bentley.unittest'):
            printProgress(procStdOutLine, status_len)
            testCount = testCount + 1
        else:
            if procStdOutLine.startswith("Failure"):
                printProgress(' ', status_len)
                print procStdOutLine,
                failureCount = failureCount + 1
        
        logfile.write(procStdOutLine)

        procStdOutLine = proc.stdout.readline ()

    retval = proc.wait()

    printProgress(' ', status_len)
    print '{0} failures'.format(failureCount)

    # if any test fails to build or if any test does not succeed, the xcodebuild test command should return a non-zero status. 
    return retval + failureCount

#-------------------------------------------------------------------------------------------
# bsimethod                                     Sam.Wilson              04/2016
#-------------------------------------------------------------------------------------------
def main():
    if (len (sys.argv) < 2):
        print "Syntax: ", sys.argv[0], " logfilename"
        exit(1)

    adbCmd = os.path.join(os.path.join(os.environ["ANDROID_SDK_ROOT"], "platform-tools"), "adb");
    logfilename = sys.argv[1]

    with open (logfilename, 'w') as logfile:
        failureCount = RunTest(adbCmd, logfile)

    exit (failureCount)

if __name__ == '__main__':
    main()
