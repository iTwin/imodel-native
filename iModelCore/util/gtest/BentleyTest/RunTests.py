#---------------------------------------------------------------------------------------------
#  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
#  See LICENSE.md in the repository root for full copyright notice.
#---------------------------------------------------------------------------------------------
import os, sys
import subprocess
from _winreg import *

#-----------------------------------------------------------------------------#
# bsimethod
#-------+---------+---------+---------+---------+---------+---------+---------#
def forceFileFlushAndClose (f):
    f.flush ()
    try:
        os.fsync(f.fileno())
    except:
        pass
    f.close()

#-----------------------------------------------------------------------------#
# bsiclass
#-------+---------+---------+---------+---------+---------+---------+---------#
class GtestStdoutResultParser ():

    #-------------------------------------------------------------------------#
    # bsimethod
    #---+---------+---------+---------+---------+---------+---------+---------#
    def __init__ (self, filename, fileWithAllTests, isx64):
        self.m_filename = filename
        self.m_listOfFailingTests = []
        self.m_listOfFailingTestNames = []
        self.m_listOfHaltingTests = []
        self.m_stackLines = []
        self.m_summaryList = []
        self.m_isTargetx64 = isx64
        self.m_finishedTests = 0
        self.m_numberOfTestsThatShouldRun = 0
        self.m_listOfAllTestNamesFilename = fileWithAllTests

        self.m_listOfAllTests = []
        self.m_listOfUnfinishedTests = []
        self.m_listOfFinishedTests = []

    #-------------------------------------------------------------------------#
    # bsimethod
    #---+---------+---------+---------+---------+---------+---------+---------#
    def PrepareListOfAllTests (self):
        file = open (self.m_listOfAllTestNamesFilename, "r")
        testGroup = ""
        testName = ""
        startReading = False
        for line in file.read ().split ("\n"):
            if line.find ("__START_TESTS__") == 0:
                startReading = True
                continue
            if line.find ("__END_TESTS__") == 0:
                break
            if startReading:
                if line.find ("  ") == -1:                  # testGroups have no leading white space
                    testGroup = line.strip()
                else:
                    testName = line.strip()
                    fullName = testGroup + testName
                    self.m_listOfAllTests.append (fullName)
        file.close()

    #-------------------------------------------------------------------------#
    # bsimethod
    #---+---------+---------+---------+---------+---------+---------+---------#
    def PrepareListOfTestsNotFinished (self):
        map = {}
        # make a map of all the tests and flag the finished ones
        for testname in self.m_listOfAllTests:
            map[testname] = 0
        for testname in self.m_listOfFinishedTests:
            if map.keys().count (testname) == 0:
                Log ("Error: PrepareListOfTestsNotFinished found a test that ran that wasn't considered a test previously..." + testname +"\n")
            else:
                map[testname] += 1
        # look at the unflagged ones
        for key in map.keys():
            if map[key] == 0:
                self.m_listOfUnfinishedTests.append (key)
        self.m_listOfUnfinishedTests.sort ()

    #-------------------------------------------------------------------------#
    # bsimethod
    #---+---------+---------+---------+---------+---------+---------+---------#
    def AddFinishedTest (self, testName):
        self.m_finishedTests += 1
        self.m_listOfFinishedTests.append (testName.split()[0])

    #-------------------------------------------------------------------------#
    # bsimethod
    #---+---------+---------+---------+---------+---------+---------+---------#
    def ParseFile (self):
        file = open (self.m_filename, "r")
        buffer = file.read ()
        file.close ()

        lineBuffer = []
        fillBuffer = False
        stopLookingForTotalCount = False
        for line in buffer.split ("\n"):
            if not stopLookingForTotalCount and line.find ("[==========]") != -1:
                stopLookingForTotalCount = True
                tokens = line.split (" ")
                self.m_numberOfTestsThatShouldRun = tokens[2]

            pos = line.find ("[  FAILED  ]")
            if fillBuffer and  pos != -1:
                if len(lineBuffer) > 0:
                    self.m_listOfFailingTests.append (lineBuffer)
                fillBuffer = False
                failingTestName = line[pos + len("[  FAILED  ]"):len(line)].strip().split()[0]
                self.AddFinishedTest (failingTestName)
                self.m_listOfFailingTestNames.append (failingTestName);
                lineBuffer.append (line)
            if line.find ("[ STACK RESULT ]") != -1:
                self.m_stackLines.append (line)
            pos = line.find ("[       OK ]")
            if pos != -1:
                fillBuffer = False
                self.AddFinishedTest (line[pos + len("[       OK ]"):len(line)].strip())
                lineBuffer.append (line)

            if fillBuffer:
                lineBuffer.append ("     " + line)

            if line.find ("[ RUN      ]") != -1:
                fillBuffer = True
                lineBuffer = []
                lineBuffer.append (line)
        self.m_listOfFinishedTests.sort ()
        # Something happened where the test did not finish.
        if fillBuffer:
            txt = lineBuffer[0]
            pos = txt.find ("[ RUN      ]")
            crashingTestName = txt[pos + len("[ RUN      ]"):len(txt)].strip().split()[0]
            group = [crashingTestName, []]
            for line in lineBuffer:
                group[1].append (line)
            self.m_listOfHaltingTests.append (group)

        if self.m_listOfAllTestNamesFilename != "":
            self.PrepareListOfAllTests ()
            self.PrepareListOfTestsNotFinished ()

    #-------------------------------------------------------------------------#
    # bsimethod
    #---+---------+---------+---------+---------+---------+---------+---------#
    def PrintCommandLineToRun (self, num, arrayOfTestNames, command, wasCrash):
        result = "\nTo debug just the failing tests: \n"
        if wasCrash:
            result = "\nTo debug the crashing test: \n"

        platformArg = "--x86"
        if self.m_isTargetx64:
            platformArg = "--x64"

        numFailing = num
        gtestFilter = ""
        sep = ""
        count = 0
        MAX = 10
        for failingTest in arrayOfTestNames :
            if count >= MAX:
                break
            gtestFilter += sep + failingTest
            sep = ":"
            count += 1

        if (numFailing > MAX):
            result += "WARNING::: Truncating command line to the first " + str(MAX) + " failing tests out of " + str (self.GetFailCount()) + " failing tests.\n"
        result += "    %srcroot%util\\gtest\\BentleyTest\\runtests.py " + command + " " + platformArg + " --vs10 --gtest_filter=" + gtestFilter + " --gtest_break_on_failure\n"
        if not wasCrash:
            result += "\nTo debug all tests: \n"
            result += "    %srcroot%util\\gtest\\BentleyTest\\runtests.py " + command + " " + platformArg + " --vs10 --gtest_break_on_failure\n"
        return result

    #-------------------------------------------------------------------------#
    # bsimethod
    #---+---------+---------+---------+---------+---------+---------+---------#
    def TruncatedErrorsToString (self, testType):
        result = ""

        if len (self.m_listOfFailingTestNames) > 0 or len (self.m_listOfHaltingTests) > 0:
            result += "{0}:\n".format (TestGroupNameMap[testType])
        for item in self.m_listOfFailingTestNames:
            result += " Error (Failed Test): {0}\n".format (item)

        for item in self.m_listOfHaltingTests:
            result += " Error (Crashed Test): {0}\n".format (item[0])

        return result

    #-------------------------------------------------------------------------#
    # bsimethod
    #---+---------+---------+---------+---------+---------+---------+---------#
    def TruncatedResultsToString (self, testResultsFilename, testType):
        result = ""
        seperator = "-"*80+"\n"
        starSep  = "*"*80+"\n"
        result += seperator
        result += seperator
        result += "Analyzing " + testResultsFilename  + " for test results.\n"
        result += seperator
        result += seperator

        areFailingTests = len (self.m_listOfFailingTests) > 0
        if areFailingTests:
            result += starSep
            result += "***** {0} Tests had Failing Tests \n".format (TestGroupNameMap[testType])
            result += starSep
            result += seperator

        for failblock in self.m_listOfFailingTests:
            for line in failblock:
                result += line + "\n"
            result += seperator

        areCrashingTests = len (self.m_listOfHaltingTests) > 0
        if areCrashingTests:
            result += starSep
            result += "***** {0} Tests had a Crashing Test \n".format (TestGroupNameMap[testType])
            result += starSep
            result += seperator

        if not areCrashingTests and not areFailingTests:
            result += starSep
            result += "**** All {0} Tests Passed\n".format (TestGroupNameMap[testType])
            result += starSep
            result += seperator

        for failblock in self.m_listOfHaltingTests:
            for line in failblock[1]:
                result += line + "\n"
            result += seperator

        result += "Out of " + str (self.m_numberOfTestsThatShouldRun) + " tests that should have run:\n"
        result += "    " + str (self.m_finishedTests) + " finished\n"

        result += "    " + str (self.GetFailCount ()) + " failed\n"
        if len (self.m_listOfHaltingTests) > 0:
            result += "1 crashed\n"
            result += self.PrintCommandLineToRun (len(self.m_listOfHaltingTests), [self.m_listOfHaltingTests[0][0]], TestCommandName[testType], True)

        if self.GetFailCount () > 0:
            result += self.PrintCommandLineToRun (len(self.m_listOfFailingTestNames), self.m_listOfFailingTestNames, TestCommandName[testType], False)

        return result

    #-------------------------------------------------------------------------#
    # bsimethod
    #---+---------+---------+---------+---------+---------+---------+---------#
    def UnfinishedTestsToString (self, testResultsFilename):
        result = ""
        seperator = "-"*80 + "\n"

        result += seperator
        result += seperator
        result += "Analyzing " + testResultsFilename  + " for tests that did not run.\n"
        result += seperator
        result += seperator

        if len (self.m_listOfUnfinishedTests) > 0:
            result += "\n\nTests that were not executed"
            if len (self.m_listOfHaltingTests) > 0:
                result += " or finished"
            else:
                result += " (filtered out)"
            result += ".\n"
            for testname in self.m_listOfUnfinishedTests:
                result += "  " + testname + "\n"
        return result

    #-------------------------------------------------------------------------#
    # bsimethod
    #---+---------+---------+---------+---------+---------+---------+---------#
    def StackPeakToString (self, testResultsFilename):
        result = ""
        count = 20
        seperator = "-"*80 + "\n"


        subset = []
        if len (self.m_stackLines) > 0:
            for i in range(0, count): # listing is in ascending, change to descending.
                j = len(self.m_stackLines) - 1 - i
                if j >= 0 and j < len(self.m_stackLines):
                    subset.append (self.m_stackLines[j] + "\n")

        if (len (subset) > 0):
            count = min (len(subset), count)
            result += "\n"
            result += seperator
            result += "Listing " + str(count) + " tests with peak stack usage. Check " + testResultsFilename + " for full list.\n"
            for l in subset:
                result += l

        return result

    #-------------------------------------------------------------------------#
    # bsimethod
    #---+---------+---------+---------+---------+---------+---------+---------#
    def ListToFile (self, filename, list):
        f = open (filename, "w")
        for testname in list:
            f.write (testname + "\n")
        forceFileFlushAndClose (f)

    #-------------------------------------------------------------------------#
    # bsimethod
    #---+---------+---------+---------+---------+---------+---------+---------#
    def FinishedTestListToFile (self, filename):
        self.ListToFile (filename, self.m_listOfFinishedTests)

    #-------------------------------------------------------------------------#
    # bsimethod
    #---+---------+---------+---------+---------+---------+---------+---------#
    def UnfinishedTestListToFile (self, filename):
        self.ListToFile (filename, self.m_listOfUnfinishedTests)

    #-------------------------------------------------------------------------#
    # bsimethod
    #---+---------+---------+---------+---------+---------+---------+---------#
    def GetErrorCount (self):
        return len (self.m_listOfFailingTests) + len (self.m_listOfHaltingTests)

    #-------------------------------------------------------------------------#
    # bsimethod
    #---+---------+---------+---------+---------+---------+---------+---------#
    def GetFailCount (self):
        return len (self.m_listOfFailingTests)

g_logFile = None
#-----------------------------------------------------------------------------#
# Helper to print error messages.
# bsimethod
#-------+---------+---------+---------+---------+---------+---------+---------#
def Log (msg):
    print msg
    global g_logFile
    if None != g_logFile:
        g_logFile.write (msg + "\n")

#-----------------------------------------------------------------------------#
# bsimethod
#-------+---------+---------+---------+---------+---------+---------+---------#
def Exit (result):
    Log ("\nRunTests.py exited with: " + str(result) + "\n")
    closeLogFileIfNecessary()
    exit (result)

#-----------------------------------------------------------------------------#
# print the error and return 0 (no errors) This is used for when called
# from start /WAIT runtests.py
# bsimethod
#-------+---------+---------+---------+---------+---------+---------+---------#
def FakeCleanExit (result):
    Log ("\nRunTests.py exited with: " + str(result) + "\n")
    closeLogFileIfNecessary()
    exit (0)

#-----------------------------------------------------------------------------#
# bsimethod
#-------+---------+---------+---------+---------+---------+---------+---------#
def closeLogFileIfNecessary ():
    global g_logFile
    if None != g_logFile:
        forceFileFlushAndClose (g_logFile)

#-----------------------------------------------------------------------------#
# bsimethod
#-------+---------+---------+---------+---------+---------+---------+---------#
def PrintError (msg):
    Log ("ERROR: " + msg)

DEBUGGER_NONE = 0
DEBUGGER_VS8 = 1
DEBUGGER_VS9 = 2
DEBUGGER_VS10 = 3
DEBUGGER_VS11 = 4

TEST_None = 0
TEST_Published = 1
TEST_NonPublished = 2
TEST_Scenario = 3
TEST_DgnView = 4
TEST_All = 5
TEST_Scenario_Host = 6
TEST_Regression = 7
TEST_Performance = 8
TEST_BentleyPublished = 9
TEST_NonPub_Scenario = 10

TestGroups = {}
TestGroupNameMap = {TEST_Published:         "Published",
                    TEST_NonPublished:      "NonPublished",
                    TEST_Scenario:          "Scenario",
                    TEST_DgnView:           "DgnView",
                    TEST_Regression:        "Regression",
                    TEST_Performance:       "Performance",
                    TEST_BentleyPublished:  "BentleyPublished",
                    TEST_NonPub_Scenario:   "NonPublished_Scenario" }

TestCommandName = {TEST_Published:          "",
                    TEST_NonPublished:      "--run_non_published",
                    TEST_Scenario:          "--run_scenario",
                    TEST_DgnView:           "--run_dgnview",
                    TEST_Regression:        "--run_regression",
                    TEST_Performance:       "--run_performance",
                    TEST_BentleyPublished:  "--run_bentley_published",
                    TEST_NonPub_Scenario:   "--run_nonpub_scenario" }

TestHostMap  = {TEST_Published:             "DgnPlatformTestHost.dll",
                    TEST_NonPublished:      "DgnPlatformTestHost.dll",
                    TEST_Scenario:          "DgnPlatformTestHost.dll",
                    TEST_DgnView:           "DgnViewTestHost.dll",
                    TEST_Scenario_Host:     "DgnPlatformHostCertif.dll",
                    TEST_Regression:        "DgnPlatformTestHost.dll",
                    TEST_Performance:       "DgnPlatformTestHost.dll",
                    TEST_BentleyPublished:  "NoHost",
                    TEST_NonPub_Scenario:   "DgnPlatformTestHost.dll" }

TestRunnerMap  = {TEST_Published:           "TestRunnerPagalloc.exe",
                    TEST_NonPublished:      "TestRunnerPagalloc.exe",
                    TEST_Scenario:          "TestRunnerPagalloc.exe",
                    TEST_DgnView:           "TestRunnerPagalloc.exe",
                    TEST_Scenario_Host:     "TestRunnerPagalloc.exe",
                    TEST_Regression:        "TestRunnerPagalloc.exe",
                    TEST_Performance:       "TestRunnerPagalloc.exe",
                    TEST_BentleyPublished:  "TestRunner.exe",
                    TEST_NonPub_Scenario:   "TestRunnerPagalloc.exe" }

TestDllMap = {TEST_Published:               "DgnPlatformTest_Published.dll",
                    TEST_NonPublished:      "DgnPlatformTest_NonPublished.dll",
                    TEST_Scenario:          "DgnPlatformTest_Scenario.dll",
                    TEST_DgnView:           "DgnViewTest_SmokeTest.dll",
                    TEST_Scenario_Host:     "DgnPlatformTest_Scenario.dll",
                    TEST_Regression:        "DgnPlatformTest_Regression.dll",
                    TEST_Performance:       "DgnPlatformTest_Performance.dll",
                    TEST_BentleyPublished:  "BentleyLibTest_Published.dll",
                    TEST_NonPub_Scenario:   "DgnPlatformTest_NonPublished_Scenario.dll"}

# Define which Product directory will the dll be in.
TestProductMap   = {TEST_Published:         "DgnPlatformTest",
                    TEST_NonPublished:      "DgnPlatformTest",
                    TEST_Scenario:          "DgnPlatformTest",
                    TEST_DgnView:           "DgnViewTest",
                    TEST_Scenario_Host:     "DgnPlatformTestCert",
                    TEST_Regression:        "DgnPlatformTest",
                    TEST_Performance:       "DgnPlatformTest",
                    TEST_BentleyPublished:  "BentleyLibTests",
                    TEST_NonPub_Scenario:   "DgnPlatformTest" }

#-----------------------------------------------------------------------------#
# bsiclass
#-------+---------+---------+---------+---------+---------+---------+---------#
class DgnPlatformTestRunner:

    #-------------------------------------------------------------------------#
    # bsimethod
    #---+---------+---------+---------+---------+---------+---------+---------#
    def __init__ (self, args):
        self.m_isTargetx64 = True

        self.m_isRunningCoverage = False
        self.m_isMakingCoverageReport = False
        self.m_isRunningFromBentleyBuild = False
        self.m_dumpLeakingTests = False
        self.m_testRunningType  = TEST_Published
        self.m_isRunningDevTests = False # run published and non published tests.
        self.m_hasDumpedErrorSummaryHeader = False
        self.m_isRunningFirebugTests = False # run published, nonpublished, performance, regression
        self.m_isRunningAllDevTests = False  # same as firebug. But does not print warning about which part to run.
        self.m_shouldPrintNonFinishingTests = False
        self.m_ignoreLeakFile = ""
        self.m_debugger = DEBUGGER_NONE
        self.m_waitToSum = True
        self.m_filterFile = ""
        self.m_doxygenXMLDir = ""
        self.m_isRunningIndividually = False
        self.m_outputDir = ""
        self.m_codewatch = ""
        self.m_logFile = ""
        self.ParseArgs (args)
        self.VerifyArgs ()

    #-------------------------------------------------------------------------#
    # bsimethod
    #---+---------+---------+---------+---------+---------+---------+---------#
    def ParseArgs (self, args):
        remainingArgs = []
        for i in range (1, len (args)):
            arg = args[i]
            upper = arg.upper ()

            if "--X64" == upper:
                self.m_isTargetx64 = True
                continue

            if "--X86" == upper:
                self.m_isTargetx64 = False
                continue

            if "--HELP" == upper:
                self.PrintHelp ()
                # Do not continue, this should get to DgnPlatformTest.exe

            if "--RUN_DGNVIEW" == upper:
                self.m_testRunningType = TEST_DgnView
                continue

            if "--RUN_NON_PUBLISHED" == upper:
                self.m_testRunningType = TEST_NonPublished
                continue

            if "--RUN_SCENARIO" == upper:
                self.m_testRunningType = TEST_Scenario
                continue

            if "--RUN_NONPUB_SCENARIO" == upper:
                self.m_testRunningType = TEST_NonPub_Scenario
                continue

            if "--RUN_SCENARIO_HOST" == upper:
                self.m_testRunningType = TEST_Scenario_Host
                continue

            if "--RUN_REGRESSION" == upper:
                self.m_testRunningType = TEST_Regression
                continue

            if "--RUN_PERFORMANCE" == upper:
                self.m_testRunningType = TEST_Performance
                continue

            if "--RUN_BENTLEY_PUBLISHED" == upper:
                self.m_testRunningType = TEST_BentleyPublished
                continue

            if "--DUMP_LEAKING_TESTS" == upper:
                self.m_dumpLeakingTests = True
                continue

            if "--INDIVIDUALLY" == upper:
                self.m_isRunningIndividually = True
                continue

            if "--RUN_DEV_TESTS" == upper:
                self.m_isRunningDevTests = True
                continue

            if "--RUN_FIREBUG_TESTS" == upper:
                self.m_isRunningFirebugTests = True
                continue

            if "--RUN_ALL_DEV_TESTS" == upper:
                self.m_isRunningAllDevTests = True
                continue

            if "--PRINT_NON_FINISHING_TESTS" == upper:
                self.m_shouldPrintNonFinishingTests = True
                continue

            if 0 == upper.find ("--IGNORE_LEAK_FILE"):
                self.m_ignoreLeakFile = arg.split ("=")[1]
                continue

            if "--VS8" == upper:
                self.m_debugger = DEBUGGER_VS8
                continue

            if "--VS9" == upper:
                self.m_debugger = DEBUGGER_VS9
                continue

            if "--VS10" == upper:
                self.m_debugger = DEBUGGER_VS10
                continue

            if "--VS11" == upper:
                self.m_debugger = DEBUGGER_VS11
                continue

            if "--COVER" == upper:
                self.m_isRunningCoverage = True
                self.m_isMakingCoverageReport = True
                continue

            if "--JUST_COVER" == upper:
                self.m_isRunningCoverage = True
                continue

            if "--JUST_REPORT" == upper:
                self.m_isMakingCoverageReport = True
                continue

            if  "--TRUNCATE_OUTPUT" == upper:
                self.m_isRunningFromBentleyBuild = True
                continue

            if 0  == upper.find ("--OUTPUT_DIR"):
                self.m_outputDir = arg.split ("=")[1]
                continue

            if 0  == upper.find ("--DOXYGEN_XML_DIR"):
                self.m_doxygenXMLDir = arg.split ("=")[1]
                continue

            if 0  == upper.find ("--FILTER_FILE"):
                self.m_filterFile = arg.split ("=")[1]
                continue

            if 0  == upper.find ("--CODEWATCH"):
                self.m_codewatch = arg.split ("=")[1]
                continue

            if 0  == upper.find ("--LOG_FILE"):
                self.m_logFile = arg.split ("=")[1]
                global g_logFile
                g_logFile = open (self.m_logFile, "w") # TODO: This will have to be append if you are summing test results
                continue

            # If we get here, the argument should be passed through to the executable.
            remainingArgs.append (arg)
        self.m_dgnplatformTestArgs = remainingArgs

    #-------------------------------------------------------------------------#
    # bsimethod
    #---+---------+---------+---------+---------+---------+---------+---------#
    def VerifyArgs (self):
        error = False

        if self.m_isMakingCoverageReport:
            if not os.path.exists (self.m_doxygenXMLDir):
                PrintError ("Coverage reporting requires a valid doxygen xml dir, '" + self.m_doxygenXMLDir + "' doesn't exist.")
                error = True
        if self.m_isRunningCoverage or self.m_isMakingCoverageReport:
            if self.m_isTargetx64:
                PrintError ("Coverage can only run on x86.")
                error = True

        if error:
            Exit (1)

    #-------------------------------------------------------------------------#
    # bsimethod
    #---+---------+---------+---------+---------+---------+---------+---------#
    def GetTargetProcessorDir (self):
        if self.m_isTargetx64:
            return "Winx64"
        else:
            return "Winx86"

    #-------------------------------------------------------------------------#
    # bsimethod
    #---+---------+---------+---------+---------+---------+---------+---------#
    def GetExeLocation (self):
        outRoot = os.getenv ("OutRoot")
        dllLocation = os.path.join (outRoot, self.GetTargetProcessorDir(), "Product", TestProductMap[self.m_testRunningType], "Dlls")
        if not os.path.exists (dllLocation):
            PrintError ("Could not find " + dllLocation + ".")
            Exit (1)
        return dllLocation

    #-------------------------------------------------------------------------#
    # bsimethod
    #---+---------+---------+---------+---------+---------+---------+---------#
    def GetExeName (self):
        outpath = self.GetBuildLocation ()
        outpath = os.path.join (outpath, "run", "output")
        result = "{0} {1} --out_folder={2}\\ --testdll={3}".format (TestRunnerMap [self.m_testRunningType],
                TestHostMap[self.m_testRunningType],
                outpath,
                TestDllMap [self.m_testRunningType])

        return result

    #-------------------------------------------------------------------------#
    # bsimethod
    #---+---------+---------+---------+---------+---------+---------+---------#
    def PrintHelp(self):
        Log ("\n\n")
        Log ("------------")
        Log ("runtests.py help")
        Log (" This helps with locating and running either x86 or x64 versions of DgnPlatformTest.")
        Log (" It also can be used to locate output tmp files for using the output trimmer to only show")
        Log (" failing tests." )
        Log (" --x64(default)                                              : use x64 version")
        Log (" --x86                                                       : use x86 version")
        Log ("")
        Log (" --run_published(default)                                    : run published tests")
        Log (" --run_non_published                                         : run non-published tests")
        Log (" --run_regression                                            : run regression tests")
        Log (" --run_performance                                           : run performance tests")
        Log (" --run_scenario                                              : run scenario tests")
        Log (" --run_nonpub_scenario                                       : run non-published scenario tests")
        Log ("" )
        Log (" --print_non_finishing_tests                                 : prints list of tests that were available that didn't complete (either filtered out, or test crashed before they finished).")
        Log ("")
        Log (" --vs8                                                       : runs vs8 to debug the process")
        Log (" --vs9                                                       : runs vs9 to debug the process")
        Log (" --vs10                                                      : runs vs10 to debug the process")
        Log (" --vs11                                                      : runs vs11 to debug the process")
        Log ("")
        Log (" --coverage                                                  : runs executable through codewatch and generates reports")
        Log (" --doxygen_xml_dir                                           : doxygen xml output path")
        Log ("")
        Log ("------------")

    #-------------------------------------------------------------------------#
    # bsimethod
    #---+---------+---------+---------+---------+---------+---------+---------#
    def GetBuildLocation (self):
        if len (self.m_outputDir) > 0:
            return self.m_outputDir
        outRoot = os.getenv ("OutRoot")
        outputFolder = os.path.join (outRoot, self.GetTargetProcessorDir(), "build", "DgnPlatformTest")
        return outputFolder

    #-------------------------------------------------------------------------#
    # bsimethod
    #---+---------+---------+---------+---------+---------+---------+---------#
    def GetLogsDir (self):
        if len (self.m_outputDir) == 0:
            return self.MakeTmpDirIfNecessary ()

        outDir = os.path.join (self.m_outputDir, "run", "logs")
        if not os.path.exists (outDir):
            os.makedirs (outDir)
        return outDir

    #-------------------------------------------------------------------------#
    # bsimethod
    #---+---------+---------+---------+---------+---------+---------+---------#
    def GetFullExePath (self):
        return os.path.join (self.GetExeLocation (), self.GetExeName ())

    #-------------------------------------------------------------------------#
    # bsimethod
    #---+---------+---------+---------+---------+---------+---------+---------#
    def GetDebugger (self):
        regmap = {DEBUGGER_VS8:"SOFTWARE\\Wow6432Node\\Microsoft\\VisualStudio\\8.0",
                  DEBUGGER_VS9:"SOFTWARE\\Wow6432Node\\Microsoft\\VisualStudio\\9.0",
                  DEBUGGER_VS10:"SOFTWARE\\Wow6432Node\\Microsoft\\VisualStudio\\10.0",
                  DEBUGGER_VS11:"SOFTWARE\\Wow6432Node\\Microsoft\\VisualStudio\\11.0"}
        registryPath = regmap[self.m_debugger]

        reg = ConnectRegistry(None, HKEY_LOCAL_MACHINE)

        key = OpenKey (reg, registryPath)
        installDir = ""
        for i in range(1024):
            try:
                n,v,t = EnumValue(key,i)
                if (n == r"InstallDir"):
                    installDir = v
            except:
                pass
        if "" == installDir:
            map = {DEBUGGER_VS8:"VS2005Dir", DEBUGGER_VS9:"VS2008Dir", DEBUGGER_VS10:"VS2010Dir", DEBUGGER_VS11:"VS2012Dir"}
            installDir = "%" + map[self.m_debugger] + "%\\Common7\\IDE\\"
            if "" == installDir:
                print "Could not find registry entry for debugger [HKEY_LOCAL_MACHINE\\" + regmap[self.m_debugger] + "] or environment variable " + map[self.m_debugger] + "."
                Exit (1)
        return "\""+installDir  + "devenv.exe\" /useenv /debugexe"

    #-------------------------------------------------------------------------#
    # bsimethod
    #---+---------+---------+---------+---------+---------+---------+---------#
    def PrepareCommandFromShell (self):
        cmd = ""
        if self.m_debugger is not DEBUGGER_NONE:
            cmd = "start \"RunTests\" " + self.GetDebugger ()  + " "

        cmd += self.GetFullExePath()
        cmd += self.GetArgString ()
        return cmd

    #-------------------------------------------------------------------------#
    # bsimethod
    #---+---------+---------+---------+---------+---------+---------+---------#
    def GetArgString (self):
        argString = ""
        for arg in self.m_dgnplatformTestArgs:
            argString += " " + arg
        return argString

    #-------------------------------------------------------------------------#
    # bsimethod
    #---+---------+---------+---------+---------+---------+---------+---------#
    def PrepareCommandToRunTestsAndRedirectResults (self, testCommand, outputFile):
        cmd = testCommand
        return cmd

    #-------------------------------------------------------------------------#
    # bsimethod
    #---+---------+---------+---------+---------+---------+---------+---------#
    def MakeTmpDirIfNecessary (self):
        tmpDir = os.getenv ('tmp')
        outDir = os.path.join (tmpDir, "DgnPlatformTest")

        if not os.path.exists (outDir):
            os.system ("mkdir " + outDir)
        return outDir

    #-------------------------------------------------------------------------#
    # Print list of available tests to an output file.
    # bsimethod
    #---+---------+---------+---------+---------+---------+---------+---------#
    def PreparePrintListOfAvailableTestsCommand (self, testCommand, outFilename):
        cmd = testCommand + " --gtest_list_tests --no_filter_file"
        cmd += " > " + outFilename
        return cmd

    #-------------------------------------------------------------------------#
    # Print list of available tests to an output file.
    # bsimethod
    #---+---------+---------+---------+---------+---------+---------+---------#
    def PreparePrintListOfTestsThatWillRun (self, testCommand, outFilename):
        cmd = testCommand + " --gtest_list_tests --suppress_filter_message"
        cmd += " > " + outFilename
        return cmd

    #-------------------------------------------------------------------------#
    # bsimethod
    #---+---------+---------+---------+---------+---------+---------+---------#
    def ExecuteCommand (self, cmd, errorMessage ):
        print "\n***** Executing [" + cmd + "] *****\n"
        sys.stdout.flush ()

        result = os.system (cmd)
        if result is not 0:
            Log ("os.system returned nonzero result {0} \n".format (str(result)) + errorMessage)
        return result

    #-------------------------------------------------------------------------#
    # ForcedIndex is used when we are running individually and want to see
    # the overall progress since counting each test multiple times does not help.
    # bsimethod
    #---+---------+---------+---------+---------+---------+---------+---------#
    def ExecuteCommandThroughStripper (self, cmd, outfile, numberOfAllTests, forcedIndex=0):
        cmd2 = "%srcroot%util\\gtest\\BentleyTest\\striplines.py {0} {1}".format (outfile, str(numberOfAllTests))
        Log (cmd2)
        if forcedIndex > 0:
            cmd2 += " " + str (forcedIndex)

        Log ("\n***** Executing [" + cmd + " | " + cmd2 + "] *****\n")
        sys.stdout.flush ()

        exe = subprocess.Popen (cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE)
        utility = subprocess.Popen (cmd2, shell=True, stdin=exe.stdout)

        errors = exe.wait()
        utilityResult = utility.wait()

        sys.stdout.flush ()
        return errors

    #-------------------------------------------------------------------------#
    # bsimethod
    #---+---------+---------+---------+---------+---------+---------+---------#
    def TruncateResults (self, shouldPrintUnfinished, shouldPrintResults, shouldPrintStackUsage, testType):
        testResultsFilename = self.GeneratePathForTarget ("TestResults.txt", testType)
        testListFile = self.GeneratePathForTarget ("ListOfAllTests.txt", testType)

        parser = GtestStdoutResultParser (testResultsFilename, testListFile, self.m_isTargetx64)
        parser.ParseFile ()
        Log ("\n\n")
        if shouldPrintResults:
            Log (parser.TruncatedResultsToString (testResultsFilename, testType))
        if shouldPrintUnfinished:
            Log (parser.UnfinishedTestsToString (testResultsFilename))
        if shouldPrintStackUsage:
            Log (parser.StackPeakToString (testResultsFilename))

        sys.stdout.flush ()
        result = parser.GetErrorCount ()
        if result != 0:
            print result
        return result

    #-------------------------------------------------------------------------#
    # bsimethod
    #---+---------+---------+---------+---------+---------+---------+---------#
    def TruncateErrors (self, testType):
        testResultsFilename = self.GeneratePathForTarget ("TestResults.txt", testType)
        testListFile = self.GeneratePathForTarget ("ListOfAllTests.txt", testType)

        parser = GtestStdoutResultParser (testResultsFilename, testListFile, self.m_isTargetx64)
        parser.ParseFile ()
        return (parser.GetErrorCount (), parser.TruncatedErrorsToString (testType))

    #-------------------------------------------------------------------------#
    # bsimethod
    #---+---------+---------+---------+---------+---------+---------+---------#
    def PrintResultsAndUnfinishedTests (self, testResultsFilename, testListFile):
        parser = GtestStdoutResultParser (testResultsFilename, testListFile, self.m_isTargetx64)
        parser.ParseFile ()
        f = open (testResultsFilename, "r")
        for line in f:
            Log (line.strip() )
        forceFileFlushAndClose (f)
        Log ("\n\n")
        Log (parser.UnfinishedTestsToString (testResultsFilename))

        Log ("\n\n")
        Log (parser.StackPeakToString (testResultsFilename))

        outDir = self.GetLogsDir ()
        fileWithListOfUnfinishedTests   = os.path.join (outDir, "UnfinishedTests.txt")
        fileWithListOfFinishedTests     = os.path.join (outDir, "FinishedTests.txt")

        parser.UnfinishedTestListToFile (fileWithListOfUnfinishedTests)
        parser.FinishedTestListToFile (fileWithListOfFinishedTests)

        sys.stdout.flush ()
        return parser.GetErrorCount ()

    #-------------------------------------------------------------------------#
    # bsimethod
    #---+---------+---------+---------+---------+---------+---------+---------#
    def OutputFinishedAndNotFinishedFiles (self, testResultsFilename, testListFile, finishedTestsFilename, unfinishedTestsFilename):
        parser = GtestStdoutResultParser (testResultsFilename, testListFile, self.m_isTargetx64)
        parser.ParseFile ()
        parser.UnfinishedTestListToFile (unfinishedTestsFilename)
        parser.FinishedTestListToFile (finishedTestsFilename)

    #-------------------------------------------------------------------------#
    # bsimethod
    #---+---------+---------+---------+---------+---------+---------+---------#
    def GenerateFilenameForTarget (self, filename, testType):
        result = TestGroupNameMap [testType]
        return result + filename

    #-------------------------------------------------------------------------#
    # bsimethod
    #---+---------+---------+---------+---------+---------+---------+---------#
    def GeneratePathForTarget (self, filename, testType):
        outDir = self.GetLogsDir ()
        result = os.path.join (outDir, self.GenerateFilenameForTarget (filename, testType))
        return result

    #-------------------------------------------------------------------------#
    # bsimethod
    #---+---------+---------+---------+---------+---------+---------+---------#
    def RunAndGetNonFinishedTests (self, exe, shouldTruncateResults):
        # 1. Print out list of all tests to a file.
        fileWithAllTests = self.GeneratePathForTarget("ListOfAllTests.txt", self.m_testRunningType)
        cmd = self.PreparePrintListOfAvailableTestsCommand (exe, fileWithAllTests)
        result = self.ExecuteCommand (cmd, "Could not generate list of all tests from running [" + exe + "] into " + fileWithAllTests + "." )
        if result is not 0:
            Exit (result)

        # 1.5  Print out list of all tests  that are supposed to run.
        fileWithTestsThatWillRun = self.GeneratePathForTarget("ListOfAllTestsThatAreSupposedToRun.txt", self.m_testRunningType)
        cmd = self.PreparePrintListOfTestsThatWillRun (exe, fileWithTestsThatWillRun)
        result = self.ExecuteCommand (cmd, "Could not generate list of all tests from running [" + exe + "] into " + fileWithTestsThatWillRun + "." )
        if result is not 0:
            Exit (result)

        # count how many tests that should run so we can gauge how many tests we have completed.
        parser = GtestStdoutResultParser ("", fileWithTestsThatWillRun, self.m_isTargetx64)
        parser.PrepareListOfAllTests ()
        count = len(parser.m_listOfAllTests)

        # 2. Run the tests.
        #    TODO: If a test crashes, this would be a good location to
        #          re-launch the tests and filter out tests that were already run/tried to run.
        resultsLocation = self.GeneratePathForTarget("TestResults.txt", self.m_testRunningType)
        cmd = self.PrepareCommandToRunTestsAndRedirectResults (exe, resultsLocation)
        testReturnCode = self.ExecuteCommandThroughStripper (cmd, resultsLocation, count)

        # 3. Strip results to report only important information.
        if shouldTruncateResults:
            result = self.TruncateResults (True, False, True, self.m_testRunningType)      # Don't print results right now print them after we do everything else.
        else:
            result = self.PrintResultsAndUnfinishedTests (resultsLocation, fileWithAllTests)

        if not self.m_waitToSum:
            result = self.TruncateResults (True, True, True, self.m_testRunningType)

        if (0 != result and 0 == testReturnCode) or (0 == result and 0 != testReturnCode):
            Log ("Error : Inconsistency in return codes of the tests being run and the process that checks the result.\n")
            Log ("        This might have happened because someone called exit(0) before tests finished. This is considered an error.\n")
            Log ("        This might also happen if all of the tests pass but the program exited abnormally. \n")

            if (testReturnCode == 0):
                return 1

        return result

    #-------------------------------------------------------------------------#
    # TODO_KN: only use this when you want to run each test multiple times per run instead of all tests n times.
    #   This makes it easier to see what specifically is triggered from each test.
    # bsimethod
    #---+---------+---------+---------+---------+---------+---------+---------#
    def RunAndGetNonFinishedTestsIndividually (self, exe, shouldTruncateResults):
        result = 0
        # 1. Print out list of all tests to a file.
        fileWithAllTests = self.GeneratePathForTarget("ListOfAllTests.txt", self.m_testRunningType)
        cmd = self.PreparePrintListOfAvailableTestsCommand (exe, fileWithAllTests)
        result = self.ExecuteCommand (cmd, "Could not generate list of all tests from running [" + exe + "] into " + fileWithAllTests + "." )
        if result is not 0:
            Exit (result)

        # 1.5  Print out list of all tests  that are supposed to run.
        fileWithTestsThatWillRun = self.GeneratePathForTarget("ListOfAllTestsThatAreSupposedToRun.txt", self.m_testRunningType)
        cmd = self.PreparePrintListOfTestsThatWillRun (exe, fileWithTestsThatWillRun)
        result = self.ExecuteCommand (cmd, "Could not generate list of all tests from running [" + exe + "] into " + fileWithTestsThatWillRun + "." )
        if result is not 0:
            Exit (result)

        # count how many tests that should run so we can gauge how many tests we have completed.
        parser = GtestStdoutResultParser ("", fileWithTestsThatWillRun, self.m_isTargetx64)
        parser.PrepareListOfAllTests ()
        totalCount = len(parser.m_listOfAllTests)

        count = 0
        for item in parser.m_listOfAllTests:
            # 2. Run the tests.
            #    TODO: If a test crashes, this would be a good location to
            #          re-launch the tests and filter out tests that were already run/tried to run.
            outputFilename = "TestResults_" + item + ".txt"
            outputFilename = outputFilename.replace ("/", "_")
            outputFilename = outputFilename.replace ("\\", "_")

            resultsLocation = self.GeneratePathForTarget(outputFilename, self.m_testRunningType)
            cmd = "{0} --gtest_filter={1} --gtest_repeat=5 ".format (self.PrepareCommandToRunTestsAndRedirectResults (exe, resultsLocation), item)
            count += 1
            testReturnCode = self.ExecuteCommandThroughStripper (cmd, resultsLocation, totalCount, count)

        return result


    #-------------------------------------------------------------------------#
    # bsimethod
    #---+---------+---------+---------+---------+---------+---------+---------#
    def ParseIgnoreLeakFile (self, filename):
        print filename
        f = open (filename, "r")
        result = []
        for line in f.readlines():
            p = line.split ("#")
            fh = p[0].strip ()
            if "" is not fh:
                result.append (fh)
        f.close()
        return result

    #-------------------------------------------------------------------------#
    # bsimethod
    #---+---------+---------+---------+---------+---------+---------+---------#
    def DumpLeaksWithInfo (self, ignoreList, isPrintingIgnoreList):
        result = 0
        print ignoreList
        for tuple in self.GetListOfTestOutputsThatHaveMultipleMemleaks():
            file = tuple[0]
            pos = file.find ("TestResults_")
            testName = file[pos+len ("TestResults_"):len(file)-len(".txt")]

            if isPrintingIgnoreList:
                if testName in ignoreList:
                   Log ("warning (MemLeak): {0}".format (file))
            else:
                if testName not in ignoreList:
                    Log ("error (MemLeak): {0}".format (file))
                    result += 1
        return result

    #-------------------------------------------------------------------------#
    # bsimethod
    #---+---------+---------+---------+---------+---------+---------+---------#
    def DumpLeaksWithStackInfo (self):
        for tuple in self.GetListOfTestOutputsThatHaveMultipleMemleaks():
            file = tuple[0]
            pos = file.find ("TestResults_")
            testName = file[pos+len ("TestResults_"):len(file)-len(".txt")]
            Log ("\n\n")
            Log (file)
            count = 0
            for stackline in tuple[1]:
                Log ("\t" + stackline)
                count += 1

    #-------------------------------------------------------------------------#
    # bsimethod
    #---+---------+---------+---------+---------+---------+---------+---------#
    def DumpTestsWithLeaks (self):
        testNamesToIgnore = []
        Log (self.m_ignoreLeakFile)

        if "" != self.m_ignoreLeakFile:
            if not os.path.exists (self.m_ignoreLeakFile):
                Log ("Failed to find '{0}' which was passed in with --ignore_leak_file=*. Aborting printing leaks.".format(self.m_ignoreLeakFile))
                return 1
            testNamesToIgnore = self.ParseIgnoreLeakFile (self.m_ignoreLeakFile)

        result = 0
        Log ("\n\n***** Printing output files with suspected memleaks and top portion of stack trace... *****\n")
        self.DumpLeaksWithStackInfo ()

        Log ("\n\n***** Printing output files with suspected memleaks in ignore list (will not break build)... *****\n")
        result += self.DumpLeaksWithInfo (testNamesToIgnore, True)

        Log ("\n\n***** Printing output files with suspected memleaks (will break build)... *****\n")
        result += self.DumpLeaksWithInfo (testNamesToIgnore, False)
        if result > 0:
            msg = "NOTICE!!! Check stack traces above and output files for details and context of each leak."
            Log ("\n")
            Log ("!"*len(msg))
            Log (msg)
            Log ("!"*len(msg))

        Log ("\n\n***** Recommending commands to verify leaks are fixed... *****\n")
        Log (" Top leak should have a count no larger than 1.\n")

        platformArg = "--x86"
        if self.m_isTargetx64:
            platformArg = "--x64"

        for tuple in self.GetListOfTestOutputsThatHaveMultipleMemleaks():
            file = tuple[0]
            pos = file.find ("TestResults_")
            testName = file[pos+len ("TestResults_"):len(file)-len(".txt")]
            non = ""
            if -1 != file.find("NonPublished"):
                non = "--run_non_published"
            if -1 != file.find("Regression"):
                non = "--run_regression"
            if -1 != file.find("Scenario"):
                non = "--run_scenario"
            if -1 != file.find("Performance"):
                non = "--run_performance"

            text = "%srcroot%util\\gtest\\BentleyTest\\runtests.py {0} {1} --pagalloc --gtest_repeat=5 --disable_high_address --gtest_filter={2}".format (platformArg, non, testName)
            Log (text)

        if result == 0:
            result += self.VerifyLeakExists ()
        return result

    #-------------------------------------------------------------------------#
    # Output should be sorted based on count, therefore if the top number is greater than 1, chances are we have a "real" memleak
    # bsimethod
    #---+---------+---------+---------+---------+---------+---------+---------#
    def DoesTestFailMemLeakRules (self, filename):
        file = open (filename, "r")
        isLookingAtLeakers = False
        isReadingStack = False
        result = False
        stackLines = []
        for line in file.read ().split ("\n"):
            if line.startswith ("PAGALLOC: Leak summary ************************************"):
                isLookingAtLeakers = True
                continue
            if line.startswith ("PAGALLOC: Leak details ************************************"):
                isReadingStack = True
                continue

            if isLookingAtLeakers:  # really only need to look at the first line after leak summary
                # "PAGALLOC:  1. Root 0x000000008B476000 appeared     4 times totaling   4352 bytes."
                sections = line.split ("appeared")
                count = int(sections[1].split ("time")[0])
                if (count > 1):
                    result = True
                isLookingAtLeakers = False

            if isReadingStack:
                stackLines.append (line)
                if len(stackLines) > 5: # get summary line + top few stack lines
                    break
        file.close()
        return result, stackLines

    #-------------------------------------------------------------------------#
    # bsimethod
    #---+---------+---------+---------+---------+---------+---------+---------#
    def VerifyLeakExists (self):
        outDir = self.GetLogsDir ()
        wasFound = False
        for root, dirs, files in os.walk (outDir):
            for name in files:
                if (name.find ("TestResults_") > -1):
                    path = os.path.join(root, name)
                    if self.DoesTestFailMemLeakRules (path)[0]:
                        if (name.find("TestResults_MemLeak.ThisLeaksMem.txt") > -1):
                            wasFound = True
                            break
        if not wasFound:
            Log ("\n\nERROR: *MemLeak.ThisLeaksMem.txt was not found in leaked test results and should have been for published tests.\n This potentially indicates that pagalloc might have been broken, the test that is supposed to leak was not run, or the format for pagalloc's output has changed etc.\n")
            return 1
        return 0

    #-------------------------------------------------------------------------#
    # Output should be sorted based on count, therefore if the top number is greater than 1, chances are we have a "real" memleak
    # bsimethod
    #---+---------+---------+---------+---------+---------+---------+---------#
    def GetListOfTestOutputsThatHaveMultipleMemleaks (self):
        outDir = self.GetLogsDir ()
        result = []
        for root, dirs, files in os.walk (outDir):
            for name in files:
                if (name.find ("TestResults_") > -1):
                    if (name.find("TestResults_MemLeak.ThisLeaksMem.txt") > -1):
                        continue
                    path = os.path.join(root, name)
                    hasLeaks, stackLines = self.DoesTestFailMemLeakRules (path)
                    if hasLeaks:
                        result.append ((path, stackLines))
        return result

    #-------------------------------------------------------------------------#
    # bsimethod
    #---+---------+---------+---------+---------+---------+---------+---------#
    def GenerateFileWithListOfAllTests (self, exe, fileWithListOfAllTests):
        Log ("Generating list of all tests...")
        cmd = self.PreparePrintListOfAvailableTestsCommand (exe, fileWithListOfAllTests)
        result = self.ExecuteCommand (cmd, "Could not generate list of all tests from running [" + exe + "] into " + fileWithListOfAllTests + "." )
        if result is not 0:
            Exit (result)

    #-------------------------------------------------------------------------#
    # bsimethod
    #---+---------+---------+---------+---------+---------+---------+---------#
    def DoCoverage (self, outDir, exe, fileWithTestResults, logFile, sessionXML):
        Log ("Running coverage...")
        cmd = self.m_codewatch + " -t " + logFile + " -l 1 cover -s " + sessionXML + " -i 1 -f " + self.m_filterFile + " " + exe + " " + self.GetArgString ()
        cmd = cmd + " > " + fileWithTestResults
        result = self.ExecuteCommand (cmd, self.m_codewatch + " exited abnormally.")
        if result is not 0:
            Exit (result)
        return result

    #-------------------------------------------------------------------------#
    # bsimethod
    #---+---------+---------+---------+---------+---------+---------+---------#
    def DoCoverageReport (self, outDir, fileWithListOfFinishedTests, fileWithListOfUnfinishedTests, codewatchSessionXML):
        publishedNUnitXML   = os.path.join (outDir, "PublishedNUnit.xml")
        if TEST_Published == self.m_testRunningType:
            htmlName = os.path.join (outDir, "PublishedReport.html")
        elif TEST_NonPublished == self.m_testRunningType:
            htmlName = os.path.join (outDir, "NonPublishedReport.html")
        elif TEST_Scenario == self.m_testRunningType:
            htmlName = os.path.join (outDir, "ScenarioReport.html")
        else:
            Log ("Attempting to do coverage report on something other than Published, NonPublished, on non standard run type.")
            Exit (1)

        platform = 'x86' # currently x64 doesn't work.

        args = ['-FinishedTestsFilename' + fileWithListOfFinishedTests,
                '-UnfinishedTestsFilename' + fileWithListOfUnfinishedTests,
                #'-OutNunitXMLFilename' + publishedNUnitXML,
                '-OutHTMLFilename' + htmlName ,
                '-CodewatchXMLSession' + codewatchSessionXML,
                '-CodewatchFilterfile' + self.m_filterFile,
                '-DoxygenXMLDir' + self.m_doxygenXMLDir,
                '-platform' + platform]

        # Create the reports.
        cmd = "coverage\\MakeCoverageReport.py"
        for arg in args:
            cmd += " " + arg
        result = self.ExecuteCommand (cmd, "MakeCoverageReport.py failed.")
        result = 0
        if 0 != result:
            Exit (result)

    #-------------------------------------------------------------------------#
    # bsimethod
    #---+---------+---------+---------+---------+---------+---------+---------#
    def RunAndGetNonFinishedTestsCoverage (self, outDir, exe):
        fileWithTestResults             = os.path.join (outDir, self.GenerateFilenameForTarget ("TestResults.txt"))

        codewatchLog                    = os.path.join (outDir, "codewatch_trace.log")
        codewatchSessionXML             = os.path.join (outDir, "codewatch_session.xml")

        fileWithListOfUnfinishedTests   = os.path.join (outDir, "UnfinishedTests.txt")
        fileWithListOfFinishedTests     = os.path.join (outDir, "FinishedTests.txt")
        fileWithAllTests = self.GeneratePathForTarget("ListOfAllTests.txt", self.m_testRunningType)

        result = 0
        if self.m_isRunningCoverage:
            result = self.DoCoverage (outDir, exe, fileWithTestResults, codewatchLog, codewatchSessionXML)

        if self.m_isMakingCoverageReport:
            self.GenerateFileWithListOfAllTests (exe, fileWithAllTests)
            self.OutputFinishedAndNotFinishedFiles (fileWithTestResults, fileWithAllTests, fileWithListOfFinishedTests, fileWithListOfUnfinishedTests)

            self.DoCoverageReport (outDir, fileWithListOfFinishedTests, fileWithListOfUnfinishedTests, codewatchSessionXML)
        return result

    #-------------------------------------------------------------------------#
    # bsimethod
    #---+---------+---------+---------+---------+---------+---------+---------#
    def RunFromBentleyBuild (self):
        exe = self.GetFullExePath ()
        exe += self.GetArgString ()
        if self.m_isRunningIndividually:
            if self.m_dumpLeakingTests:
                result = self.DumpTestsWithLeaks()
            else:
                result = self.RunAndGetNonFinishedTestsIndividually (exe, True)
        else:
            result = self.RunAndGetNonFinishedTests(exe, True)
        return result

    #-------------------------------------------------------------------------#
    # bsimethod
    #---+---------+---------+---------+---------+---------+---------+---------#
    def RunForCoverage (self):
        exe = self.GetFullExePath ()
        outDir = self.GetLogsDir ()

        result = self.RunAndGetNonFinishedTestsCoverage (outDir, exe)
        return result

    #-------------------------------------------------------------------------#
    # bsimethod
    #---+---------+---------+---------+---------+---------+---------+---------#
    def RunFromCommand (self):
        cmd = self.PrepareCommandFromShell ()
        if self.m_shouldPrintNonFinishingTests:
            if self.m_isRunningIndividually:
                if self.m_dumpLeakingTests:
                    result = self.DumpTestsWithLeaks()
                else:
                    result = self.RunAndGetNonFinishedTestsIndividually (cmd, False)
            else:
                result = self.RunAndGetNonFinishedTests(cmd, False)
            return result
        else:
            if self.m_isRunningIndividually:
                result = self.RunAndGetNonFinishedTestsIndividually (cmd, False)
            else:
                result = self.ExecuteCommand (cmd, "")
            sys.stdout.flush ()
            return result

    #-------------------------------------------------------------------------#
    # bsimethod
    #---+---------+---------+---------+---------+---------+---------+---------#
    def RunTests (self):
        result = 0
        if self.m_isRunningCoverage or self.m_isMakingCoverageReport:
            result = self.RunForCoverage ()
        elif self.m_isRunningFromBentleyBuild:
            result = self.RunFromBentleyBuild ()
        else:
            result = self.RunFromCommand ()
        return result

    #-------------------------------------------------------------------------#
    # bsimethod
    #---+---------+---------+---------+---------+---------+---------+---------#
    def IsRunningAllTests (self):
        return self.m_isRunningFirebugTests or self.m_isRunningAllDevTests

    #-------------------------------------------------------------------------#
    # bsimethod
    #---+---------+---------+---------+---------+---------+---------+---------#
    def DumpErrorsIfExist (self, tuple):
        errors = tuple[0]
        msg = tuple[1]
        if errors:
            if not self.m_hasDumpedErrorSummaryHeader:
                self.m_hasDumpedErrorSummaryHeader = True
                Log ("\n\n")
                Log ("*"*80)
                Log ("* List of Failing Tests without details. Check above for call stacks and test printf output.")
                Log ("*"*80)
            Log (msg)

    #-------------------------------------------------------------------------#
    # bsimethod
    #---+---------+---------+---------+---------+---------+---------+---------#
    def Run (self):
        results = 0
        if self.m_isRunningDevTests or self.IsRunningAllTests():
            self.m_testRunningType = TEST_Published
            results += self.RunTests ()

            self.m_testRunningType = TEST_NonPublished
            results += self.RunTests ()

            self.m_testRunningType = TEST_Scenario
            results += self.RunTests ()

            self.m_testRunningType = TEST_NonPub_Scenario
            results += self.RunTests ()

            showFirebugWarning = False

            if self.IsRunningAllTests():
                self.m_testRunningType = TEST_Regression
                fbresults = self.RunTests ()

                if False: # performance tests are writing to wrong output dir - leaving for now.
                    self.m_testRunningType = TEST_Performance
                    fbresults += self.RunTests ()

                if fbresults > 0 and self.m_isRunningFirebugTests:
                    showFirebugWarning = True
                results += fbresults

            if self.m_isRunningFromBentleyBuild:
                self.m_testRunningType = TEST_Published
                self.TruncateResults (False, True, False, TEST_Published)

                self.m_testRunningType = TEST_NonPublished
                self.TruncateResults (False, True, False, TEST_NonPublished)

                self.m_testRunningType = TEST_Scenario
                self.TruncateResults (False, True, False, TEST_Scenario)

                self.m_testRunningType = TEST_NonPub_Scenario
                self.TruncateResults (False, True, False, TEST_NonPub_Scenario)

                if self.IsRunningAllTests():
                    self.m_testRunningType = TEST_Regression
                    self.TruncateResults (False, True, False, TEST_Regression)

                    if False: # performance tests are writing to wrong output dir - leaving for now.
                        self.m_testRunningType = TEST_Performance
                        self.TruncateResults (False, True, False, TEST_Performance)

                self.m_testRunningType = TEST_Published
                self.DumpErrorsIfExist (self.TruncateErrors (TEST_Published))

                self.m_testRunningType = TEST_NonPublished
                self.DumpErrorsIfExist (self.TruncateErrors (TEST_NonPublished))

                self.m_testRunningType = TEST_Scenario
                self.DumpErrorsIfExist (self.TruncateErrors (TEST_Scenario))

                self.m_testRunningType = TEST_NonPub_Scenario
                self.DumpErrorsIfExist (self.TruncateErrors (TEST_NonPub_Scenario))

                if self.IsRunningAllTests():
                    self.m_testRunningType = TEST_Regression
                    self.DumpErrorsIfExist (self.TruncateErrors (TEST_Regression))

                    if False: # performance tests are writing to wrong output dir - leaving for now.
                        self.m_testRunningType = TEST_Performance
                        self.DumpErrorsIfExist (self.TruncateErrors (TEST_Performance))

                if showFirebugWarning:
                    archString = "-x64"
                    if not self.m_isTargetx64:
                        archString = "-x86"
                    Log ("*"*80)
                    Log ("* WARNING!!! ")
                    Log ("* Firebug defined in shell. Additional tests run which returned and error that developer usually do not run.")
                    Log ("* To run these tests you run the following parts.")
                    Log ("*     bb -r dgnplatformtest -f dgnplatformtest -p RunDgnPlatformTest_All b {0} -f".format (archString))
                    Log ("*   or")
                    #Log ("*     RunDgnPlatformTest_Performance")
                    Log ("*     bb -r dgnplatformtest -f dgnplatformtest -p RunDgnPlatformTest_Regression b {0} -f".format (archString))
                    Log ("*"*80)

        else:
            self.m_waitToSum = False
            results += self.RunTests ()
        return results

#-----------------------------------------------------------------------------#
# This is the "runner" for the tests.  This can be used to make sure that the
#  output of the DgnPlatformTest gets put into one location.  This includes
#  stdout and stderr being logged to the same location so that parts of it
#  can be extracted instead of being too verbose.  This will return the
#  number of errors that were found when running the DgnPlatformTest.
# bsimethod
#-------+---------+---------+---------+---------+---------+---------+---------#
if __name__ == '__main__':
    runner = DgnPlatformTestRunner (sys.argv)
    result = runner.Run ()
    # we do this because if this is run from start /WAIT on TCC it will return the error.
	#  on cmd it returns 0 regardless of the error.
    FakeCleanExit (result)
