#--------------------------------------------------------------------------------------
#
#     $Source: CommonTasks/TestResults.py $
#
#  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
#
#--------------------------------------------------------------------------------------
import os
import re


#-------------------------------------------------------------------------------------------
# bsiclass                                     Majd.Uddin    11/2017
#-------------------------------------------------------------------------------------------
class TestResults:
   #----------------------------------------------------------------------------------------
   # bsimethod                                     Majd.Uddin    11/2017
   #----------------------------------------------------------------------------------------
   def __init__(self, logName, process=True):
      self.log = logName
      self.failedTests = []
      self.passedTests = []
      self.disabledTests = []
      self.architecture = ""
      if process:
         self.processLog()

   #----------------------------------------------------------------------------------------
   # bsimethod                                     Majd.Uddin    11/2017
   #----------------------------------------------------------------------------------------
   def processLog(self):
      #Patterns to match
      failedpat = re.compile (r"FAILED\s*]\s*(\w+\.\w+|\w+/\w+\.\w+/\d+)", re.I)
      passedpat = re.compile (r"OK\s*]\s*(\w+\.\w+|\w+/\w+\.\w+/\d+)", re.I)

      with open(self.log, 'r') as logfile:
         lines=logfile.readlines()
         for line in lines:
            failed = failedpat.search(line)
            passed = passedpat.search(line)
            if failed or passed:
               testName = line.split(']')[1].strip().split('(')[0].strip()
               testName = testName.replace('/', '_')
               if failed:
                   if testName not in self.failedTests:
                       self.failedTests.append(testName)
               if passed:
                   if testName not in self.passedTests:
                       self.passedTests.append(testName)
   #----------------------------------------------------------------------------------------
   # bsimethod                                     Majd.Uddin    11/2017
   #----------------------------------------------------------------------------------------
   def getFailedTestCount(self):
      return len(self.failedTests)
   #----------------------------------------------------------------------------------------
   # bsimethod                                     Majd.Uddin    11/2017
   #----------------------------------------------------------------------------------------
   def getFailedTests(self):
      return self.failedTests
   #----------------------------------------------------------------------------------------
   # bsimethod                                     Majd.Uddin    11/2017
   #----------------------------------------------------------------------------------------
   def getPassedTestCount(self):
      return len(self.passedTests)
   #----------------------------------------------------------------------------------------
   # bsimethod                                     Majd.Uddin    11/2017
   #----------------------------------------------------------------------------------------
   def getPassedTests(self):
      return self.passedTests
   #----------------------------------------------------------------------------------------
   # bsimethod                                     Majd.Uddin    11/2017
   #----------------------------------------------------------------------------------------
   def getTestCount(self):
      return (len(self.failedTests) + len(self.passedTests))
   #----------------------------------------------------------------------------------------
   # bsimethod                                     Majd.Uddin    11/2017
   #----------------------------------------------------------------------------------------
   def getAllTests(self):
      allTests = self.passedTests
      for test in self.failedTests:
         allTests.append(test)
      return allTests

   #----------------------------------------------------------------------------------------
   # bsimethod                                     Majd.Uddin    11/2017
   #----------------------------------------------------------------------------------------
   def getFixtureTestCount(self, testFixtureName):
      count = 0
      for test in self.passedTests:
         if test.startswith(testFixtureName):
            count = count + 1
      for test in self.failedTests:
         if test.startswith(testFixtureName):
            count = count + 1
      
      return count



