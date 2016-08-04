#--------------------------------------------------------------------------------------
#
#     $Source: gtest/CheckLogfilesForFailures.py $
#
#  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
#
#--------------------------------------------------------------------------------------
import os, sys, re

# Failure patterns to match
# [  FAILED  ] RleEditorTester.MaxRleRunSetPixels    
# [  FAILED  ] FileFormatTests/ExportTester.ToBestiTiff/21, where GetParam() = L"D:\\ATP\\Dataset\\Images_Files\\_forATPs\\Images\\jpeg\\Mosaic_Jpeg_WF\\TC06L0\\15.jpg"
failedpat = re.compile (r"FAILED\s*]\s*(\w+\.\w+|\w+/\w+\.\w+/\d+)", re.I)
summarypat = re.compile (r"\[==========\].*ran", re.I)

#-------------------------------------------------------------------------------------------
# bsimethod                                     Sam.Wilson      05/2016
#-------------------------------------------------------------------------------------------
def checkLogFileForFailures(logfilename):
    exename = ''
    foundSummary = False
    failedTestsList = ""
    comma = ''
    anyFailures = False
    lineNo = 0
    with open(logfilename, 'r') as logfile:
        for line in logfile.readlines():

            lineNo = lineNo + 1

            # Name of test runner exe is always the first line
            if lineNo == 1:
                exename = line.strip('\n')
                continue
            
            if not foundSummary:
                # Ignore everything until we hit the summary
                if summarypat.search(line) != None:
                    foundSummary = True
                continue

            else:
                if line.find('FAILED TESTS') != -1:
                    anyFailures = True

                # Get the list of failures
                failed = failedpat.search(line)
                if failed != None:
                    failedTestsList = failedTestsList + comma + failed.group(1)
                    continue

    if not anyFailures and foundSummary:
        return ''

    advicestr = '************ Failures from: ' + logfilename + ' ******************'

    # If we never got to the summary, that means that the last test to run was interrupted by a crash
    if not foundSummary:
        advicestr = advicestr + "\n"
        advicestr = advicestr + "\n*** CRASH ***"
        advicestr = advicestr + "\n"

    # Report all failures that we saw in the log
    if len(failedTestsList) != 0:
        advicestr = advicestr + "\n"
        advicestr = advicestr + "\nTo re-run failing tests, run the following command in your debugger:"
        advicestr = advicestr + "\n    " + exename + " --gtest_break_on_failure --gtest_filter=" + failedTestsList
        advicestr = advicestr + "\n"

    if anyFailures or not foundSummary:
        # When we detect failing or crashing tests, print the whole log. That will then go into bb's build log.
        # The user will want to scroll up to see complete details.
        with open(logfilename, 'r') as logfile:
            for line in logfile.readlines():
                print line,

    return advicestr

#-------------------------------------------------------------------------------------------
# bsimethod                                     Sam.Wilson      05/2016
#-------------------------------------------------------------------------------------------
if __name__ == '__main__':
    checkedCount = 0

    if len(sys.argv) < 2:
        print 'Syntax: ' + sys.argv[0] + ' logfilesdir [breakonfailureflag]'
        exit(1)

    dir = sys.argv[1]
    breakonfailure = False
    if len(sys.argv) > 2 and sys.argv[2] != 0:
        breakonfailure = True

    advicestr = ''
    for root,dirs,files in os.walk (dir, topdown=True, onerror=None, followlinks=True):
        for file in files:
            if not file.endswith('.log'):
                continue;

            checkedCount = checkedCount + 1
            path = os.path.join(root, file)
            adviceForThisLog = checkLogFileForFailures(path)
            if 0 != len(adviceForThisLog):
                advicestr = advicestr + '\n\n' + adviceForThisLog
     
    if checkedCount == 0:
        print 'no logfiles found in ' + dir
        exit(0)

    print '{0} test product logs found'.format(checkedCount)

    if 0 == len(advicestr):
        print "All tests passed."
        exit (0)

    print adviceForThisLog
    exit(breakonfailure)

