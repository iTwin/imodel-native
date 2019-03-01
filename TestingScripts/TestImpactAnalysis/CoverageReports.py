#--------------------------------------------------------------------------------------
#
#     $Source: TestImpactAnalysis/CoverageReports.py $
#
#  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
#
#--------------------------------------------------------------------------------------
import os, sys
import argparse
import sqlite3
#Common Scripts to be used by any task
scriptsDir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
commonScriptsDir = os.path.join(scriptsDir, 'CommonTasks')
sys.path.append(commonScriptsDir)

from CodeCoverage import CodeCoverage
import Components as cmp
from printing import printColored
from TestResults import TestResults
from TestsFromCpp import TestCatalog, CPPTests
from FindRequiredTests import getIgnoredTests

#-------------------------------------------------------------------------------------------
# bsimethod                                     Jeff.Marker
#-------------------------------------------------------------------------------------------
def fixPath(path):
    if not path or len(path) == 0:
        return path
    
    return os.path.normcase(os.path.realpath(path))

#-------------------------------------------------------------------------------------------
# bsimethod                                     Majd.Uddin    02/2019
#-------------------------------------------------------------------------------------------
def getChangedTests(reportPath, comp):
    tl = []
    mapFile = cmp.TiaMapPathForComp(comp)
    if not os.path.exists(mapFile):
        return tl
    timeToCompare = os.path.getmtime(mapFile)
    tcatalog = TestCatalog(cmp.RepoPathForComp(comp))
    sf = tcatalog.get_files()
    for sfn in sf:
        fileTime = os.path.getmtime(sfn)
        if fileTime > timeToCompare:
            cpp = CPPTests(sfn)
            tests = cpp.get_tests()
            for test in tests:
                if test not in tl:
                    tl.append(test)
    if len(tl) > 0 :
        # remove tests that are ignored
        ignored = getIgnoredTests(comp)
        for test in ignored:
            if test in tl:
                tl.remove(test)
        # Since unwanted tests can get in. Work around is to run tests and get list
        allTests = getTests(reportPath, comp)
        for test in list(tl):
            if test not in allTests:
                print 'removing: ' + test
                tl.remove(test)
    return tl
#-------------------------------------------------------------------------------------------
# bsimethod                                     Majd.Uddin    02/2019
#-------------------------------------------------------------------------------------------
def getTests(reportPath, comp):
    testList = []
    testLog = cmp.LogPathForComp(comp)
    print testLog
    if os.path.exists(testLog):
        tr = TestResults(testLog)
        testList = tr.getAllTests()
    else:
        print printColored('Test log not found. Running the tests first', 'cyan', True)
        testLogNew = os.path.join(reportPath, comp+'_test.log')
        cmdForTests = testExe + ' > ' + testLogNew
        print cmdForTests
        result = os.system(cmdForTests)
        if result != 0 : # Test run failed
            print printColored('Test execution failed for: ' + testExe, 'red', True)
            exit(-1)
        else:
            tr = TestResults(testLogNew)
            testList = tr.getAllTests()
    return testList

#-------------------------------------------------------------------------------------------
# bsimethod                                     Majd.Uddin    12/2017
#-------------------------------------------------------------------------------------------
def runCoverage(reportPath, comp, forceAll):
    print 'Running Coverage for comp: ' + comp
    results = {'NotNeeded': [], 'Passed': [], 'Failed': []}
    testExe = cmp.ExePathForComp(comp)
    if forceAll:
        testList = getTests(reportPath, comp)
    else: # only run for changed files
        testList = getChangedTests(reportPath, comp)
    print 'Total tests for coverage run: ' + str(len(testList))
    i = 0
    if len(testList) > 0: # we need to run coverage for some tests
        reportDir = os.path.join(reportPath, comp)
        if not os.path.exists(reportDir):
            os.mkdir(reportDir)
        cov = CodeCoverage()
        cov.set_ReportPath(reportDir)

        for test in testList:
            i = i + 1
            print printColored('***Running Coverage for Test: ' + str(i) + ' of ' + str(len(testList)), 'cyan', True)
            result = cov.covSingleTest(testExe, test)
            if result: # it passed
                results['Passed'].append(test)
            else:
                results['Failed'].append(test)

    return results

#-------------------------------------------------------------------------------------------
# bsimethod                                     Majd.Uddin    12/2017
#-------------------------------------------------------------------------------------------
def printResults(results, comp):
    print '\n*** Code coverage results for component:' + comp
    if len(results['Failed']) > 0: #Some tests failed
        print printColored('\n Code coverage failed for some tests. See list below.', 'red', True)
        for test in results['Failed']:
            print test
        print printColored(' Code coverage failed for some tests. See list above.\n', 'red', True)
    else:
        if len(results['NotNeeded']) > 0:
            print printColored('\n Code coverage not run for tests that have not changed. See list below.', 'cyan', True)
            for test in results['NotNeeded']:
                print test
            print printColored('\n Code coverage not run for tests that have not changed. See list above.\n', 'cyan', True)

        if len(results['Passed']) > 0:
            print printColored(' Code coverage passed for tests. See list below.', 'green', True)
            for test in results['Passed']:
                print test
            print printColored(' Code coverage passed for tests. See list above.\n', 'green', True)
    print '\n*** Code coverage results ended for component:' + comp
    
#-------------------------------------------------------------------------------------------
# bsimethod                                     Majd.Uddin    09/2017
#-------------------------------------------------------------------------------------------
def main():
        parser = argparse.ArgumentParser()
        parser.add_argument("--reportPath", help = "The path at which you want XML reports of Coverage e.g. --reportPath=D:\CoverageReports", required = True)
        parser.add_argument("--component", help = "The name of the component you want reports for e.g. --component=ECDb. If none is given, runs for all.")
        parser.add_argument("--forceAll", help = "Pass this argument to force generate Coverage report for all tests. Otherwise, it will run for changed tests only.", action='store_true')

        args = parser.parse_args()
        reportPath = args.reportPath
        if reportPath == None:
                print printColored('\n Script requires to get a folder where Coverage reports will be stored. Error.', 'red', True)
                exit(-1)
        if not os.path.exists(reportPath):
               print printColored('\n The report path is not valid: ' + reportPath, 'red', True)
               exit(-1)

        if args.component is None:#for all components
            comps = cmp.Components
        else:
            comps = [args.component]

        #Run for required components
        for comp in comps:
            results = runCoverage(reportPath, comp, args.forceAll)
            printResults(results, comp)  

if __name__ == '__main__':
    main()