#--------------------------------------------------------------------------------------
#
#     $Source: gtest/CheckResultsForFailures.py $
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
    failedtests = ''
    comma = ''
    lookingforsummary = True
    # patterns to match
    # [  FAILED  ] RleEditorTester.MaxRleRunSetPixels    
    # [  FAILED  ] FileFormatTests/ExportTester.ToBestiTiff/21, where GetParam() = L"D:\\ATP\\Dataset\\Images_Files\\_forATPs\\Images\\jpeg\\Mosaic_Jpeg_WF\\TC06L0\\15.jpg"
    errpat = re.compile (r"FAILED\s*]\s*(\w+\.\w+|\w+/\w+\.\w+/\d+)", re.I)
    summarypat = re.compile(r"\[==========\]\s*(\d+)\s+tests?\s+from\s+(\d+)\s+test\s+cases?\s+ran\.\s*\((\d+)\s+ms\s+total\)")
    with open(sys.argv[1], 'r') as logfile:
        for line in logfile.readlines():

            if lookingforsummary:
                if summarypat.match(line) != None:
                    lookingforsummary = False
                continue

            err = errpat.search(line)
            if err != None:
                failedtests = failedtests + comma + err.group(1)
                comma = ","
                failureCount = failureCount + 1

    if len(failedtests) != 0:
        print ""
        print "*** TESTS FAILED ***"
        for t in failedtests.split(","):
            print t
        print "********************"
        print ""
        print "To re-run failing tests, run the following command in your debugger:"
        print "    " + sys.argv[2] + " --gtest_break_on_failure --gtest_filter=" + failedtests
        print ""

    exit (failureCount)

