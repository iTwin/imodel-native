#--------------------------------------------------------------------------------------
#
#     $Source: gtest/CheckResultsForFailures.py $
#
#  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
#
#--------------------------------------------------------------------------------------
import os, sys, re
from sets import Set

#-------------------------------------------------------------------------------------------
# bsimethod                                     Sam.Wilson      05/2016
#-------------------------------------------------------------------------------------------
if __name__ == '__main__':
    failedtests = Set()
    foundSummary = False
    lastTestSeen = ""
    summarypat = re.compile (r"\[==========\].*ran", re.I)
    runpat = re.compile (r"RUN\s*]\s*(\w+\.\w+)", re.I)
    # Failure patterns to match
    # [  FAILED  ] RleEditorTester.MaxRleRunSetPixels    
    # [  FAILED  ] FileFormatTests/ExportTester.ToBestiTiff/21, where GetParam() = L"D:\\ATP\\Dataset\\Images_Files\\_forATPs\\Images\\jpeg\\Mosaic_Jpeg_WF\\TC06L0\\15.jpg"
    errpat = re.compile (r"FAILED\s*]\s*(\w+\.\w+|\w+/\w+\.\w+/\d+)", re.I)
    with open(sys.argv[1], 'r') as logfile:
        for line in logfile.readlines():

            if summarypat.search(line) != None:
                 foundSummary = True
            else:
                run = runpat.search(line)
                if run != None:
                    lastTestSeen = run.group(1)
                else:
                    err = errpat.search(line)
                    if err != None:
                        failedtests.add(err.group(1))

    if not foundSummary:
        print ""
        print "*** TEST CRASHED ***"
        print ""
        print lastTestSeen
        print ""
        print "To run just the crashing test, run the following command in your debugger:"
        print "    " + sys.argv[2] + " --gtest_break_on_failure --gtest_filter=" + lastTestSeen
        print ""

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
        print "    " + sys.argv[2] + " --gtest_break_on_failure --gtest_filter=" + failedTestsStr
        print ""

    exit (len(failedtests))

