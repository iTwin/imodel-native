#----------------------------------------------------------------------------------------
#
#  $Source: gtest/RunTests.py $
#
#  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
#
#----------------------------------------------------------------------------------------
import os, sys
import subprocess
import platform
import shutil
import tempfile
import uuid

if os.name == 'nt':
    import _winreg

verbose=0

#-----------------------------------------------------------------------------#
#                                               Kevin.Nyman         11/09
#-------+---------+---------+---------+---------+---------+---------+---------#
def forceFileFlushAndClose (f):
    f.flush ()
    try:
        os.fsync(f.fileno())
    except:
        pass
    f.close() 

#-----------------------------------------------------------------------------#
#                                               Sam.Wilson          10/14
#-------+---------+---------+---------+---------+---------+---------+---------#
def getSrcRootEnvVar():
    if os.name == 'nt':
       return "%SrcRoot%"
    return "$SrcRoot"

#-----------------------------------------------------------------------------#
#                                               Kevin.Nyman         8/09
#-------+---------+---------+---------+---------+---------+---------+---------#
class GtestStdoutResultParser (): 

    #-------------------------------------------------------------------------#
    #                                           Kevin.Nyman         8/09
    #---+---------+---------+---------+---------+---------+---------+---------#
    def __init__ (self, filename, fileWithAllTests, arch):
        self.m_filename = filename
        self.m_listOfFailingTests = []
        self.m_listOfFailingTestNames = []
        self.m_listOfHaltingTests = [] 
        self.m_stackLines = []
        self.m_summaryList = []
        self.m_arch = arch
        self.m_finishedTests = 0
        self.m_numberOfTestsThatShouldRun = 0
        self.m_listOfAllTestNamesFilename = fileWithAllTests

        self.m_listOfAllTests = []
        self.m_listOfUnfinishedTests = []
        self.m_listOfFinishedTests = []
        self.m_parsingErrors = ""

    #-------------------------------------------------------------------------#
    #                                           Kevin.Nyman         10/09
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
                    # gtest 1.7, now outputs params of templated test. see util\gtest\test\gtest_list_tests_unittest.py.
                    # Cleanup so PrepareListOfTestsNotFinished can do its comparaison without using stringified pointer values.
                    # ex: 
                    #   CopyFromTests/0  # GetParam() = (0000008454964720, 000000845495E5C0)
                    fullNameNoTypedParam = fullName.split("#")[0].strip()
                    self.m_listOfAllTests.append (fullNameNoTypedParam)
        file.close()

        # if we don't find __START_TESTS__" at the start of a line, then the output is either malformed or the test exe did not run
        if startReading == False:
            self.m_parsingErrors = "***\n***Error: "+self.m_listOfAllTestNamesFilename+" - __START_TESTS__ was not found\n***"

    #-------------------------------------------------------------------------#
    #                                           Kevin.Nyman         10/09
    #---+---------+---------+---------+---------+---------+---------+---------#
    def PrepareListOfTestsNotFinished (self):
        if self.m_parsingErrors != "":
            Log (self.m_parsingErrors)
            return
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
    #                                           Kevin.Nyman         10/09
    #---+---------+---------+---------+---------+---------+---------+---------#
    def AddFinishedTest (self, testName):
        self.m_finishedTests += 1
        self.m_listOfFinishedTests.append (testName.split()[0])

    #-------------------------------------------------------------------------#
    #                                           Kevin.Nyman         8/09
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
                # gtest 1.7, now outputs params of templated tests. Cleanup the templated parts that might include pointer adresses.
                # ex: CopyFromTests/0, where GetParam() = (0000008454964720, 000000845495E5C0)
                failingTestName = failingTestName.split(",")[0].strip();
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
    #                                           Kevin.Nyman         8/09
    #---+---------+---------+---------+---------+---------+---------+---------#
    def PrintCommandLineToRun (self, num, arrayOfTestNames, command, wasCrash):
        result = "\nTo debug just the failing tests: \n"
        if wasCrash:
            result = "\nTo debug the crashing test: \n"

        platformArg = '-a'+self.m_arch

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

        cmdLine = getSrcRootEnvVar()+os.path.join("BeGTest","runtests.py ") + command

        if (numFailing > MAX):
            result += "WARNING::: Truncating command line to the first " + str(MAX) + " failing tests out of " + str (self.GetFailCount()) + " failing tests.\n"
        result += "    "+ cmdLine + " " + platformArg + " --vs12 --gtest_filter=" + gtestFilter + " --gtest_break_on_failure\n"
        if not wasCrash:
            result += "\nTo debug all tests: \n"
            result += "    " + cmdLine + " " + platformArg + " --vs12 --gtest_break_on_failure\n"
        return result

    #-------------------------------------------------------------------------#
    #                                           Kevin.Nyman         4/11
    #---+---------+---------+---------+---------+---------+---------+---------#
    def TruncatedErrorsToString (self, testType):
        result = ""

        if len (self.m_listOfFailingTestNames) > 0 or len (self.m_listOfHaltingTests) > 0:
            result += "Tests:\n"
        for item in self.m_listOfFailingTestNames: 
            result += " Error (Failed Test): {0}\n".format (item)

        for item in self.m_listOfHaltingTests: 
            result += " Error (Crashed Test): {0}\n".format (item[0])
       
        return result

    #-------------------------------------------------------------------------#
    #                                           Kevin.Nyman         8/09
    #---+---------+---------+---------+---------+---------+---------+---------#
    def TruncatedResultsToString (self, testResultsFilename, testType):
        result = ""
        seperator = "-"*80+"\n"
        starSep  = "*"*80+"\n"

        if verbose > 0:
            result += seperator
            result += seperator
            result += "Analyzing " + testResultsFilename  + " for test results.\n"
            result += seperator
            result += seperator

        areFailingTests = len (self.m_listOfFailingTests) > 0
        if areFailingTests:
            result += starSep 
            result += "***** Detected Failing Tests \n."
            result += starSep
            result += seperator

        for failblock in self.m_listOfFailingTests:
            for line in failblock:
                result += line + "\n"
            result += seperator 

        areCrashingTests = len (self.m_listOfHaltingTests) > 0
        if areCrashingTests:
            result += starSep
            result += "***** Detected Crashing Test \n"
            result += starSep
            result += seperator

        if not areCrashingTests and not areFailingTests:
            result += starSep
            result += "**** All Tests Passed\n"
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
            result += self.PrintCommandLineToRun (len(self.m_listOfHaltingTests), [self.m_listOfHaltingTests[0][0]], "", True)

        if self.GetFailCount () > 0:
            result += self.PrintCommandLineToRun (len(self.m_listOfFailingTestNames), self.m_listOfFailingTestNames, "", False)

        result += "\n\nSee " + testResultsFilename + " logging output"

        return result

    #-------------------------------------------------------------------------#
    #                                           Kevin.Nyman         8/09
    #---+---------+---------+---------+---------+---------+---------+---------#
    def UnfinishedTestsToString (self, testResultsFilename):
        result = ""    
        seperator = "-"*80 + "\n"

        if verbose > 0:
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
    #                                           Kevin.Nyman         8/09
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
    #                                           Kevin.Nyman         11/09
    #---+---------+---------+---------+---------+---------+---------+---------#
    def ListToFile (self, filename, list):
        f = open (filename, "w")
        for testname in list:
            f.write (testname + "\n")
        forceFileFlushAndClose (f)

    #-------------------------------------------------------------------------#
    #                                           Kevin.Nyman         11/09
    #---+---------+---------+---------+---------+---------+---------+---------#
    def FinishedTestListToFile (self, filename):
        self.ListToFile (filename, self.m_listOfFinishedTests)

    #-------------------------------------------------------------------------#
    #                                           Kevin.Nyman         11/09
    #---+---------+---------+---------+---------+---------+---------+---------#
    def UnfinishedTestListToFile (self, filename):
        self.ListToFile (filename, self.m_listOfUnfinishedTests)

    #-------------------------------------------------------------------------#
    #                                           Kevin.Nyman         8/09
    #---+---------+---------+---------+---------+---------+---------+---------#
    def GetErrorCount (self):
        return len (self.m_listOfFailingTests) + len (self.m_listOfHaltingTests)

    #-------------------------------------------------------------------------#
    #                                           Kevin.Nyman         8/09
    #---+---------+---------+---------+---------+---------+---------+---------#
    def GetFailCount (self):
        return len (self.m_listOfFailingTests)

g_logFile = None
#-----------------------------------------------------------------------------#
# Helper to print error messages.
#                                               Kevin.Nyman         11/09
#-------+---------+---------+---------+---------+---------+---------+---------#
def Log (msg):
    print msg
    global g_logFile
    if None != g_logFile:
        g_logFile.write (msg + "\n")

#-----------------------------------------------------------------------------#
#                                               Kevin.Nyman         11/09
#-------+---------+---------+---------+---------+---------+---------+---------#
def Exit (result):
    Log ("\nRunTests.py exited with: " + str(result) + "\n")
    closeLogFileIfNecessary()
    exit (result)

#-----------------------------------------------------------------------------#
# print the error and return 0 (no errors) This is used for when called
# from start /WAIT runtests.py
#                                               Kevin.Nyman         11/09
#-------+---------+---------+---------+---------+---------+---------+---------#
def FakeCleanExit (result):
    Log ("\nRunTests.py exited with: " + str(result) + "\n")
    closeLogFileIfNecessary()
    exit (0)

#-----------------------------------------------------------------------------#
#                                               Kevin.Nyman         11/09
#-------+---------+---------+---------+---------+---------+---------+---------#
def closeLogFileIfNecessary ():
    global g_logFile
    if None != g_logFile:
        forceFileFlushAndClose (g_logFile)

#-----------------------------------------------------------------------------#
#                                               Kevin.Nyman         11/09
#-------+---------+---------+---------+---------+---------+---------+---------#
def PrintError (msg):
    Log ("ERROR: " + msg)

DEBUGGER_NONE = 0
DEBUGGER_VS11 = 1
DEBUGGER_VS12 = 2
DEBUGGER_VS14 = 3

#-----------------------------------------------------------------------------#
#                                               Kevin.Nyman         8/09
#-------+---------+---------+---------+---------+---------+---------+---------#
class DgnPlatformTestRunner:

    #-------------------------------------------------------------------------#
    #                                           Kevin.Nyman         8/09
    #---+---------+---------+---------+---------+---------+---------+---------#
    def __init__ (self, args):
        self.m_exeName = "";
        self.m_isRunningCoverage = False
        self.m_isMakingCoverageReport = False
        self.m_isRunningFromBentleyBuild = False
        self.m_dumpLeakingTests = False 
        self.m_testRunningType  = ""
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
        self.m_arch = os.getenv ("DEFAULT_TARGET_PROCESSOR_ARCHITECTURE")
        
        self.ParseArgs (args)
        self.VerifyArgs () 
        
    #-------------------------------------------------------------------------#
    #                                           Kevin.Nyman         8/09
    #---+---------+---------+---------+---------+---------+---------+---------#
    def ParseArgs (self, args):
        remainingArgs = []
        for i in range (1, len (args)):
            arg = args[i] 
            upper = arg.upper ()

            if 0 == upper.find("--EXENAME"):
                self.m_exeName = arg.split("=")[1]
                continue;

            if upper.startswith("-A"):
                if 2 == len(upper):
                    i += 1
                    arg = args[i]
                    self.m_arch = arg
                else:
                    self.m_arch = arg[2:]
                
                continue

            if "--ARCHITECTURE" == upper:
                self.m_arch = arg.split("=")[1]
                continue;

            if "--HELP" == upper:
                self.PrintHelp ()
                # Do not continue, this should get to BeGTest.exe

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

            if "--VS11" == upper:
                self.m_debugger = DEBUGGER_VS11
                continue

            if "--VS12" == upper:
                self.m_debugger = DEBUGGER_VS12
                continue

            if "--VS14" == upper:
                self.m_debugger = DEBUGGER_VS14
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

            # No longer supported in favor of the more common -a/--architecture pattern from BB.
            if "--X64" == upper or "--X86" == upper:
                Log("Ignoring argument '{0}'. Use the -a/--architecture argument instead.".format(arg))
                continue

            # If we get here, the argument should be passed through to the executable.
            remainingArgs.append (arg)

        self.m_dgnplatformTestArgs = remainingArgs

    #-------------------------------------------------------------------------#
    #                                           Kevin.Nyman         8/09
    #---+---------+---------+---------+---------+---------+---------+---------#
    def VerifyArgs (self):
        error = False

        if not self.m_exeName:
            PrintError ("You must specify the name of the test runner executable using the -exename= argument") 
            error = True 

        if not self.m_outputDir:
            PrintError ("You must specify the name of the output directory using the --output_dir= argument") 
            error = True 

        if not self.m_arch:
            PrintError ("Could not resolve architecture from environment or command line. Set via -a/--architecture or DEFAULT_TARGET_PROCESSOR_ARCHITECTURE.")
            error = True 

        if self.m_isMakingCoverageReport:
            if not os.path.exists (self.m_doxygenXMLDir):
                PrintError ("Coverage reporting requires a valid doxygen xml dir, '" + self.m_doxygenXMLDir + "' doesn't exist.")
                error = True
        if self.m_isRunningCoverage or self.m_isMakingCoverageReport:
            if self.m_arch.upper() != 'X86':
                PrintError ("Coverage can only run on x86.")
                error = True 

        if error:
            Exit (1)

    #-------------------------------------------------------------------------#
    #                                           Kevin.Nyman         8/09
    #---+---------+---------+---------+---------+---------+---------+---------#
    def GetExeLocation (self):
        return os.path.dirname(self.m_exeName)

    #-------------------------------------------------------------------------#
    #                                           Kevin.Nyman         8/09
    #---+---------+---------+---------+---------+---------+---------+---------#
    def GetExeName (self):
        return os.path.basename(self.m_exeName)

    #-------------------------------------------------------------------------#
    #                                           Kevin.Nyman         8/09
    #---+---------+---------+---------+---------+---------+---------+---------#
    def PrintHelp(self):
        Log ("\n\n")
        Log ("------------")
        Log ("runtests.py help")
        Log (" This helps with locating and running either x86 or x64 versions of BeGTest.")
        Log (" It also can be used to locate output tmp files for using the output trimmer to only show")
        Log (" failing tests." )
        Log ("")
        Log (" -exename:                                                   : Required: The filename (including the full path) of the test runner program to run.")
        Log (" -output_dir:                                                : Required: The directory where test runner output such as logs can be written.")
        Log (" --print_non_finishing_tests                                 : prints list of tests that were available that didn't complete (either filtered out, or test crashed before they finished).")
        Log ("")
        Log (" --vs11                                                      : runs vs11 to debug the process")
        Log ("")
        Log (" --coverage                                                  : runs executable through codewatch and generates reports")
        Log (" --doxygen_xml_dir                                           : doxygen xml output path")
        Log ("")
        Log (" -a|--architecture                                           : Optional (defaults to DEFAULT_TARGET_PROCESSOR_ARCHITECTURE from the environment). If provided as an argument, possible values are: 'x86', 'x64'.")
        Log ("")
        Log ("------------")

    #-------------------------------------------------------------------------#
    #                                           Kevin.Nyman         10/09
    #---+---------+---------+---------+---------+---------+---------+---------#
    def GetLogsDir (self):
        outDir = os.path.join (self.m_outputDir, "run", "logs")
        if not os.path.exists (outDir):
            os.makedirs (outDir)
        return outDir

    #-------------------------------------------------------------------------#
    #                                           Kevin.Nyman         8/09
    #---+---------+---------+---------+---------+---------+---------+---------#
    def GetFullExePath (self):
        return os.path.join (self.GetExeLocation (), self.GetExeName ())

    #---------------------------------------------------------------------------------------
    # Uses the registry to find the path of the requested devenv.exe.
    #                                                              Jeff.Marker     07/2013
    #---------------------------------------------------------------------------------------
    def GetVSIdePath(self):
        if os.name == 'nt':
            if self.m_debugger == DEBUGGER_VS11:
                regVersionString = "11.0"
            elif self.m_debugger == DEBUGGER_VS12:
                regVersionString = "12.0"
            elif self.m_debugger == DEBUGGER_VS14:
                regVersionString = "14.0"
            else:
                raise Exception("Unknown/unexpected m_debugger")

            try:
                hKey = _winreg.OpenKey(_winreg.HKEY_LOCAL_MACHINE, r"SOFTWARE\Microsoft\VisualStudio\{0}\Setup\VS".format(regVersionString), 0, _winreg.KEY_READ)
            except:
                hKey = _winreg.OpenKey(_winreg.HKEY_LOCAL_MACHINE, r"SOFTWARE\Microsoft\VisualStudio\{0}\Setup\VS".format(regVersionString), 0, _winreg.KEY_READ | _winreg.KEY_WOW64_32KEY)
        
            return '"' + _winreg.QueryValueEx (hKey, "EnvironmentPath")[0] + '"'
        else:
            return "*** not supported on this platform***"

    #---------------------------------------------------------------------------------------
    # Generates a full path to a candidate SLN file that does not exist.
    #                                                              Jeff.Marker     07/2013
    #---------------------------------------------------------------------------------------
    def GetSlnPath(self, slnBaseName):
        slnDir = tempfile.gettempdir()
        proposedSlnPath = os.path.join(slnDir, slnBaseName + ".sln")

        if not os.path.exists(proposedSlnPath):
            return proposedSlnPath

        slnSuffix = 1
        
        while os.path.exists(proposedSlnPath):
            proposedSlnPath = os.path.join(slnDir, "{0}-{1}.sln".format(slnBaseName, slnSuffix))
            slnSuffix += 1

        return proposedSlnPath

    #---------------------------------------------------------------------------------------
    # Writes an SLN file to debug the given EXE.
    #                                                              Jeff.Marker     07/2013
    #---------------------------------------------------------------------------------------
    def WriteSlnFile(self, slnFilePath, slnBaseName, exePath, exeArgs):
        slnGuid = uuid.uuid4()
        projGuid = uuid.uuid4()
        
        if self.m_debugger == DEBUGGER_VS11:
            slnFormatVersion = "12.00"
            slnVersionComment = "2012"
        elif self.m_debugger == DEBUGGER_VS12:
            slnFormatVersion = "12.00"
            slnVersionComment = "2013"
        elif self.m_debugger == DEBUGGER_VS14:
            slnFormatVersion = "14.00"
            slnVersionComment = "2015"
        else:
            raise Exception("Unknown/unexpected m_debugger")

        # SLN files must contain tabs; do NOT de-tab these strings.
        with open (slnFilePath, "w") as slnFile:
            slnFile.writelines('Microsoft Visual Studio Solution File, Format Version {0}\n'.format(slnFormatVersion))
            slnFile.writelines('# Visual Studio {0}\n'.format(slnVersionComment))
            slnFile.writelines('Project("{{{0}}}") = "{1}", "{2}", "{{{3}}}"\n'.format(str(slnGuid), slnBaseName, exePath, str(projGuid)))
            slnFile.writelines('	ProjectSection(DebuggerProjectSystem) = preProject\n')
            slnFile.writelines('		PortSupplier = 00000000-0000-0000-0000-000000000000\n')
            slnFile.writelines('		Executable = {0}\n'.format(exePath))
            slnFile.writelines('		RemoteMachine = {0}\n'.format(platform.node()))
            slnFile.writelines('		StartingDirectory = {0}\n'.format(os.path.split(slnFilePath)[0]))
            slnFile.writelines('		Arguments = {0}\n'.format(exeArgs.strip()))
            slnFile.writelines('		Environment = Default\n')
            slnFile.writelines('		LaunchingEngine = 00000000-0000-0000-0000-000000000000\n')
            slnFile.writelines('		LaunchSQLEngine = No\n')
            slnFile.writelines('		AttachLaunchAction = No\n')
            slnFile.writelines('	EndProjectSection\n')
            slnFile.writelines('EndProject\n')
            slnFile.writelines('Global\n')
            slnFile.writelines('	GlobalSection(SolutionConfigurationPlatforms) = preSolution\n')
            slnFile.writelines('		Debug|x64 = Debug|x64\n')
            slnFile.writelines('	EndGlobalSection\n')
            slnFile.writelines('	GlobalSection(ProjectConfigurationPlatforms) = postSolution\n')
            slnFile.writelines('		{{{0}}}.Debug|x64.ActiveCfg = Debug|x64\n'.format(str(projGuid)))
            slnFile.writelines('	EndGlobalSection\n')
            slnFile.writelines('	GlobalSection(SolutionProperties) = preSolution\n')
            slnFile.writelines('		HideSolutionNode = FALSE\n')
            slnFile.writelines('	EndGlobalSection\n')
            slnFile.writelines('EndGlobal\n')

    #---------------------------------------------------------------------------------------
    # Copies the appropriate seed SUO file next to the solution so it's used.
    #                                                              Jeff.Marker     07/2013
    #---------------------------------------------------------------------------------------
    def CopySuoFile(self, suoPath):
        if self.m_debugger == DEBUGGER_VS11:
            seedFileName = "SeedSlnPrefs.2012"
        elif self.m_debugger == DEBUGGER_VS12:
            seedFileName = "SeedSlnPrefs.2012"
        elif self.m_debugger == DEBUGGER_VS14:
            seedFileName = "SeedSlnPrefs.2012"
        else:
            raise Exception("Unknown/unexpected m_debugger")

        seedSuoPath = os.path.join(os.path.split(os.path.realpath(__file__))[0], seedFileName)
        shutil.copy(seedSuoPath, suoPath)

    #-------------------------------------------------------------------------#
    #                                           Kevin.Nyman         8/09
    #---+---------+---------+---------+---------+---------+---------+---------#
    def PrepareCommandFromShell (self):
        if self.m_debugger is DEBUGGER_NONE:
            return "{0} {1}".format(self.GetFullExePath(), self.GetArgString ())

        if os.name != 'nt':
            runDir = self.GetExeLocation()
            return "export LD_LIBRARY_PATH="+runDir+"\n"+os.path.joing (runDir, self.GetExeName())

        idePath = self.GetVSIdePath()
        exePath = self.GetFullExePath()
        slnBaseName = os.path.splitext(os.path.basename(exePath))[0]
        slnPath = self.GetSlnPath(slnBaseName)

        if self.m_debugger == DEBUGGER_VS11:
            suoExtensions = "v11.suo"
        elif self.m_debugger == DEBUGGER_VS12:
            suoExtensions = "v11.suo"
        elif self.m_debugger == DEBUGGER_VS14:
            suoExtensions = "v11.suo"
        else:
            raise Exception("Unknown/unexpected m_debugger")

        suoPath = "{0}.{1}".format(os.path.splitext(slnPath)[0], suoExtensions)

        self.WriteSlnFile(slnPath, slnBaseName, exePath, self.GetArgString())
        self.CopySuoFile(suoPath)
        
        return "{0} {1}".format(idePath, slnPath)
    
    #-------------------------------------------------------------------------#
    #                                           Kevin.Nyman         8/09
    #---+---------+---------+---------+---------+---------+---------+---------#
    def GetArgString (self):
        argString = ""
        for arg in self.m_dgnplatformTestArgs:
            argString += " " + arg

        return argString 

    #-------------------------------------------------------------------------#
    #                                           Kevin.Nyman         8/09
    #---+---------+---------+---------+---------+---------+---------+---------#
    def PrepareCommandToRunTestsAndRedirectResults (self, testCommand, outputFile):
        cmd = testCommand
        return cmd

    #-------------------------------------------------------------------------#
    # Print list of available tests to an output file. 
    #                                           Kevin.Nyman         10/09
    #---+---------+---------+---------+---------+---------+---------+---------#
    def PreparePrintListOfAvailableTestsCommand (self, testCommand, outFilename):
        cmd = testCommand + " --gtest_list_tests --no_filter_file"
        cmd += " > " + outFilename
        return cmd

    #-------------------------------------------------------------------------#
    # Print list of available tests to an output file. 
    #                                           Kevin.Nyman         10/09
    #---+---------+---------+---------+---------+---------+---------+---------#
    def PreparePrintListOfTestsThatWillRun (self, testCommand, outFilename):
        cmd = testCommand + " --gtest_list_tests --suppress_filter_message" 
        cmd += " > " + outFilename
        return cmd

    #-------------------------------------------------------------------------#
    #                                           Kevin.Nyman         10/09
    #---+---------+---------+---------+---------+---------+---------+---------#
    def ExecuteCommand (self, cmd, errorMessage ):
        if verbose > 0:
            print "\n***** Executing [" + cmd + "] *****\n"
            sys.stdout.flush ()
        
        if os.name != 'nt':
            os.environ['LD_LIBRARY_PATH'] = self.GetExeLocation()

        result = os.system (cmd)
        if result is not 0: 
            Log ("os.system returned nonzero result {0} \n".format (str(result)) + errorMessage)
        return result

    #-------------------------------------------------------------------------#
    # ForcedIndex is used when we are running individually and want to see
    # the overall progress since counting each test multiple times does not help.
    #                                           Kevin.Nyman         03/10 
    #---+---------+---------+---------+---------+---------+---------+---------#
    def ExecuteCommandThroughStripper (self, cmd, outfile, numberOfAllTests, forcedIndex=0):
        stripperCmd = "python " + os.path.join (os.getenv("SrcRoot"), "bsicommon", "build", "StripLines.py")

        cmd2 = stripperCmd + " " + outfile + " " + str(numberOfAllTests)
        Log (cmd2)
        if forcedIndex > 0:
            cmd2 += " " + str (forcedIndex)

        Log("\ncmd='{0}'".format(cmd))
        Log ("\n***** Executing [" + cmd + " | " + cmd2 + "] *****\n")
        sys.stdout.flush ()
         
        exe = subprocess.Popen (cmd.split(' '), stdin=subprocess.PIPE, stdout=subprocess.PIPE)
        utility = subprocess.Popen (cmd2, shell=True, stdin=exe.stdout)

        errors = exe.wait()
        utilityResult = utility.wait()

        sys.stdout.flush ()
        return errors 

    #-------------------------------------------------------------------------#
    #                                           Kevin.Nyman         10/09
    #---+---------+---------+---------+---------+---------+---------+---------#
    def TruncateResults (self, shouldPrintUnfinished, shouldPrintResults, shouldPrintStackUsage, testType):
        testResultsFilename = self.GeneratePathForTarget ("TestResults.txt", testType)
        testListFile = self.GeneratePathForTarget ("ListOfAllTests.txt", testType)

        parser = GtestStdoutResultParser (testResultsFilename, testListFile, self.m_arch)
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
    #                                           Kevin.Nyman         10/09
    #---+---------+---------+---------+---------+---------+---------+---------#
    def TruncateErrors (self, testType):
        testResultsFilename = self.GeneratePathForTarget ("TestResults.txt", testType)
        testListFile = self.GeneratePathForTarget ("ListOfAllTests.txt", testType)

        parser = GtestStdoutResultParser (testResultsFilename, testListFile, self.m_arch)
        parser.ParseFile ()
        return (parser.GetErrorCount (), parser.TruncatedErrorsToString (testType))

    #-------------------------------------------------------------------------#
    #                                           Kevin.Nyman         10/09
    #---+---------+---------+---------+---------+---------+---------+---------#
    def PrintResultsAndUnfinishedTests (self, testResultsFilename, testListFile):
        parser = GtestStdoutResultParser (testResultsFilename, testListFile, self.m_arch)
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
    #                                           Kevin.Nyman         10/09
    #---+---------+---------+---------+---------+---------+---------+---------#
    def OutputFinishedAndNotFinishedFiles (self, testResultsFilename, testListFile, finishedTestsFilename, unfinishedTestsFilename):
        parser = GtestStdoutResultParser (testResultsFilename, testListFile, self.m_arch)
        parser.ParseFile ()
        parser.UnfinishedTestListToFile (unfinishedTestsFilename)
        parser.FinishedTestListToFile (finishedTestsFilename)

    #-------------------------------------------------------------------------#
    #                                           Kevin.Nyman         03/10
    #---+---------+---------+---------+---------+---------+---------+---------#
    def GenerateFilenameForTarget (self, filename, testType):
        return "Tests" + filename

    #-------------------------------------------------------------------------#
    #                                           Kevin.Nyman         03/10
    #---+---------+---------+---------+---------+---------+---------+---------#
    def GeneratePathForTarget (self, filename, testType):
        outDir = self.GetLogsDir ()
        result = os.path.join (outDir, self.GenerateFilenameForTarget (filename, testType))
        return result

    #-------------------------------------------------------------------------#
    #                                           Kevin.Nyman         10/09
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
        parser = GtestStdoutResultParser ("", fileWithTestsThatWillRun, self.m_arch)
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
    #                                           Kevin.Nyman         10/09
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
        parser = GtestStdoutResultParser ("", fileWithTestsThatWillRun, self.m_arch)
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
    #                                           Kevin.Nyman         2/11
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
    #                                           Kevin.Nyman         2/11
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
    #                                           Kevin.Nyman         2/11
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
    #                                           Kevin.Nyman         1/11
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

        platformArg = '-a'+m_arch
       
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

            text = (getSrcRootEnvVar() + os.path.join ("BeGTest", "runtests.py") + "{0} {1} --pagalloc --gtest_repeat=5 --disable_high_address --gtest_filter={2}").format (platformArg, non, testName)
            Log (text)

        if result == 0:
            result += self.VerifyLeakExists ()
        return result

    #-------------------------------------------------------------------------#
    # Output should be sorted based on count, therefore if the top number is greater than 1, chances are we have a "real" memleak 
    #                                           Kevin.Nyman         12/10
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
    #                                           Kevin.Nyman         1/11
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
            Log ("\n\nERROR: *MemLeak.ThisLeaksMem.txt was not found in laaked test results and should have been for published tests.")
            return 1
        return 0 

    #-------------------------------------------------------------------------#
    # Output should be sorted based on count, therefore if the top number is greater than 1, chances are we have a "real" memleak 
    #                                           Kevin.Nyman         12/10
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
    #                                           Kevin.Nyman         11/09
    #---+---------+---------+---------+---------+---------+---------+---------#
    def GenerateFileWithListOfAllTests (self, exe, fileWithListOfAllTests):
        Log ("Generating list of all tests...")
        cmd = self.PreparePrintListOfAvailableTestsCommand (exe, fileWithListOfAllTests) 
        result = self.ExecuteCommand (cmd, "Could not generate list of all tests from running [" + exe + "] into " + fileWithListOfAllTests + "." )
        if result is not 0:
            Exit (result)

    #-------------------------------------------------------------------------#
    #                                           Kevin.Nyman         10/09
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
    #                                           Kevin.Nyman         10/09
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
    #                                           Kevin.Nyman         10/09
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
    #                                           Kevin.Nyman         10/09
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
    #                                           Kevin.Nyman         10/09
    #---+---------+---------+---------+---------+---------+---------+---------#
    def RunForCoverage (self):
        exe = self.GetFullExePath ()
        outDir = self.GetLogsDir ()

        result = self.RunAndGetNonFinishedTestsCoverage (outDir, exe)
        return result 

    #-------------------------------------------------------------------------#
    #                                           Kevin.Nyman         10/09
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
    #                                           Kevin.Nyman         8/09
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
    #                                           Kevin.Nyman         3/11
    #---+---------+---------+---------+---------+---------+---------+---------#
    def IsRunningAllTests (self):
        return self.m_isRunningFirebugTests or self.m_isRunningAllDevTests

    #-------------------------------------------------------------------------#
    #                                           Kevin.Nyman         3/11
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
    #                                           Kevin.Nyman         8/09
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

                if self.IsRunningAllTests():
                    self.m_testRunningType = TEST_Regression
                    self.DumpErrorsIfExist (self.TruncateErrors (TEST_Regression))

                    if False: # performance tests are writing to wrong output dir - leaving for now.
                        self.m_testRunningType = TEST_Performance 
                        self.DumpErrorsIfExist (self.TruncateErrors (TEST_Performance))

                if showFirebugWarning:
                    archString = '-a'+m_arch
                    Log ("*"*80)
                    Log ("* WARNING!!! ")
                    Log ("* Firebug defined in shell. Additional tests run which returned an error that developer usually do not run.")
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
#  output of the BeGTest gets put into one location.  This includes
#  stdout and stderr being logged to the same location so that parts of it 
#  can be extracted instead of being too verbose.  This will return the 
#  number of errors that were found when running the BeGTest.
#                                               Kevin.Nyman         8/09
#-------+---------+---------+---------+---------+---------+---------+---------#
if __name__ == '__main__':
    runner = DgnPlatformTestRunner (sys.argv)
    result = runner.Run ()
    # we do this because if this is run from start /WAIT on TCC it will return the error.
        #  on cmd it returns 0 regardless of the error.
    FakeCleanExit (result) 
