#---------------------------------------------------------------------------------------------
#  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
#  See COPYRIGHT.md in the repository root for full copyright notice.
#---------------------------------------------------------------------------------------------
import os
import re

#-------------------------------------------------------------------------------------------
# bsiclass                                     Majd.Udidn   02/2019
#-------------------------------------------------------------------------------------------
class CPPTests:

    #----------------------------------------------------------------------------------------
    # bsimethod                                     Majd.Udidn   02/2019
    #----------------------------------------------------------------------------------------
    def __init__(self, cpp):
        self.cpp = cpp
        self.tests = []
        self.testschemalist=[]
        self.testRE = re.compile ('^\s*TEST\s*\(\s*(\w+)\s*\,\s*(\w+)')
        self.testfRE = re.compile ('^\s*TEST_F\s*\(\s*(\w+)\s*\,\s*(\w+)')
        self.testPRE = re.compile ('^\s*TEST_P\s*\(\s*(\w+)\s*\,\s*(\w+)')
        self.strings = ["@bsimethod" , "@bsiclass" , "@betest" , "@bsitest" , "@bsistruct"]

    #-------------------------------------------------------------------------------------------
    # This function finds and returns Author's name in the given line
    # bsimethod                                     Ayesha.Rahman                     07/2017
    #-------------------------------------------------------------------------------------------
    def find_authorNameInLine (self, line):
        line = line.strip()
        lineLower = line.lower()
        if any(s in lineLower for s in self.strings):
            line = line.replace("\t" , " ")
            lineparts = line.split(' ')
            found = False
            for i in range(0,len(lineparts)):
                if len(lineparts[i]) > 2:
                    if not ("@bsi" in lineparts[i] or "@betest" in lineparts[i] or "//" in lineparts[i] or "*" in lineparts[i] ):
                        found = True
                        authorParts = lineparts[i].split('.')
                        if len(authorParts) == 1: #The Name is not formatted like FirstName.SecondName rather it is FirstName SecondName with a space
                            authorName = lineparts[i] + "." + lineparts[i+1]
                            return authorName.title()
                        return lineparts[i].title()
            if(found == False):
                return "Unknown"
        else:
            return "UnKnown" # if it is not found

    #-------------------------------------------------------------------------------------------
    # bsimethod                                     Ayesha.Rahman                     08/2017
    #-------------------------------------------------------------------------------------------
    def find_authorNameInExtendedLines(self, lines,i):
        lineCount = 1
        while (1):
            line = lines[i-lineCount]
            line=line.strip()
            if not (line.startswith("//") or line.startswith("/*") or line.startswith("+") or  line.startswith("*") or  line.startswith("#")):
                break
            else:
                lineCount=lineCount+1
        linesTotal=lineCount
        linesTotal = int(linesTotal)
        for k in xrange(linesTotal):
            authorName = self.find_authorNameInLine(lines[i-k])
            if not authorName == "UnKnown":
                break
        return authorName

    #-------------------------------------------------------------------------------------------
    # Finds Author name from above lines
    # bsimethod                                     Ayesha.Rahman                     08/2017
    #-------------------------------------------------------------------------------------------
    def find_authorName(self, lines, i):
        for k in range(1,4):
            authorName = self.find_authorNameInLine(lines[i-k])
            if not authorName == "UnKnown":
                break
        if authorName == "UnKnown":
            authorName = self.find_authorNameInExtendedLines(lines,i)
        return authorName

    #-------------------------------------------------------------------------------------------
    # bsimethod                                     Ridha.Malik                    01/2018
    #-------------------------------------------------------------------------------------------    
    def test_hasSchema(self, testname):
        '''
            Find test has defined schema seprately above author header
        '''
        if len(self.testschemalist)!=0:
            for test in self.testschemalist:
                if testname==test[0]:
                    #print "testname",testname
                    return test[1]
            return -1
        else:
            return -1

    #-------------------------------------------------------------------------------------------
    # bsimethod                                     Ayesha.Rahman                     08/2017
    #-------------------------------------------------------------------------------------------
    def select_tag(self, line, lane):
        if line.startswith("#") and "(BENTLEY_WIN32)" in line and "(BENTLEY_WINRT)" in line and "||" in line and "|| defined (__unix__)" in line:
            return "Windows,WINRT,Unix"
        elif line.startswith("#ifdef _WIN32") or line.startswith("#if defined (BENTLEY_WIN32)") or line.startswith("#if !defined (BENTLEY_WINRT)") or line.startswith("#ifdef BENTLEY_WIN32") or line.startswith("#if defined (_WIN32)") or line.startswith("#ifdef USE_GTEST"):
            return "Windows"
        elif (" defined (BENTLEY_WINRT)" in line and not line.startswith("#")) or line.startswith("#if !defined (BENTLEY_WIN32)"):
            return "WINRT"
        elif (("#if defined" in lane or "#ifdef" in lane) and ("wip" in lane)):
            return "WIP"
        else:
            return "ALL"

    #-------------------------------------------------------------------------------------------
    # bsimethod                                     Ayesha.Rahman                     08/2017
    #-------------------------------------------------------------------------------------------
    def find_tagInAboveLines(self, lines, i):
        '''
            This function traverses the lines above the Test to find the tag on it and returns ALL if no tag is found
            SelectTag() : A function call to determine which tag is found in the line

        '''

        tag = "ALL"
        check = 1
        for j in range(i-1 , 0 , -1):
            linee = lines[j]
            lane = linee.lower()
            if not (linee.startswith("#")):
                continue
            if linee.startswith("#endif"):
                check = check+1
            if linee.startswith("#if"):
                check = check-1
            if check == 0 and linee.startswith("#if"):
                tag = self.select_tag(linee,lane)
                if tag == "ALL":
                    check = 1
                    continue
                else:
                    break
        return tag

    #-------------------------------------------------------------------------------------------
    # bsimethod                                     Ayesha.Rahman                     08/2017
    #-------------------------------------------------------------------------------------------
    def detect_test(self, line):
        '''
            This function does the pattern matching for detecting Tests in line
            testRE : Tests following pattern TEST()
            testFRE : Tests following pattern TEST_F()
            testPRE : Tests following pattern TEST_P()
        '''

        testMatch = self.testRE.search (line)
        if testMatch == None:
            testMatch = self.testfRE.search (line)
        if testMatch == None:
            testMatch = self.testPRE.search (line)
        return testMatch

    #-------------------------------------------------------------------------------------------
    # Search if test schema defined for test seprately 
    # bsimethod                                     Ridha.Malik                    01/2018
    #-------------------------------------------------------------------------------------------
    def defined_testSchema(self, line, lineno):
        Schemaregix= re.compile ('^\s*DEFINE_SCHEMA\s*\(\s*(\w+)\s*\,\s*(\w+)')
        testschema= Schemaregix.search (line)
        if testschema != None:
            self.testschemalist.append((testschema.group(1),lineno))
    #----------------------------------------------------------------------------------------
    # bsimethod                                     Majd.Udidn   02/2019
    #----------------------------------------------------------------------------------------
    def fill_tests(self):
            check=0
            openfile = open(self.cpp, "r")
            print 'Finding tests in: '+ self.cpp
            lines = openfile.readlines()
            i=0
            while(i<lines.__len__()):
                line = lines[i]
                line = line.strip()
                lane = line.lower()
                if line.startswith ('//') or line.startswith ('#'):
                    i=i+1
                    continue
                if line.startswith('/*'):
                    i=i+1
                    if line.endswith('*/'):
                        #print line
                        continue
                    if not(line.endswith('*/')) and '*/' in line:
                        #print line 
                        continue
                    if not(line.startswith('/*')) and '/*' in line:
                        #print line 
                        continue
                    #print line
                    check=check+1
                    continue 
                if line.endswith('*/') and check >0:
                    i=i+1
                    #print line
                    check=check-1
                    continue
                if check!=0:
                    i=i+1
                    continue
                self.defined_testSchema(line,i)
                testMatch = self.detect_test(line)
                if testMatch == None:
                    i=i+1
                    continue
                testClass = testMatch.group(1)
                testName = testMatch.group(2)
                Lineno = self.test_hasSchema(testName)
                Tag = self.find_tagInAboveLines(lines, i)
                ignored = 'R'
                comment=" "
                testNameToMatch = testClass + "." + testName
                # TO DO. Add check for Ignored tests as was in original code.
                if Lineno!=-1:
                    lineindex=Lineno
                else:
                    lineindex=i
                    self.testschemalist=[]
                #authorName = self.find_authorName(lines, lineindex)
                #if testName.startswith("DISABLED"):
                #    Tag ="Disabled"
                #owner=findOwner(component)
                self.tests.append(testNameToMatch)
                i=i+1
            check=0
    #----------------------------------------------------------------------------------------
    # bsimethod                                     Majd.Udidn   02/2019
    #----------------------------------------------------------------------------------------       
    def get_tests(self):
        self.fill_tests()
        return self.tests


#-------------------------------------------------------------------------------------------
# bsiclass                                     Majd.Udidn   02/2019
#-------------------------------------------------------------------------------------------
class TestCatalog:
    #----------------------------------------------------------------------------------------
    # bsimethod                                     Majd.Udidn   02/2019
    #----------------------------------------------------------------------------------------
    def __init__(self, dir):
        self.dir = dir
        self.source_files = []
       
    #----------------------------------------------------------------------------------------
    # bsimethod                                     Majd.Udidn   02/2019
    #----------------------------------------------------------------------------------------       
    def fill_filles(self):
        for root,allDirs,allFiles in os.walk(self.dir):
            for componentDirectory in allDirs:
                if componentDirectory.lower() == "test" or componentDirectory.lower() == "tests":
                    component_Directory_Path = os.path.join(root,componentDirectory)
                    for root2t,dirs2t,filest in os.walk(component_Directory_Path):
                        for file in filest:
                            if file.endswith('.cpp'):
                                fullFile = os.path.join(root2t,file)
                                if fullFile not in self.source_files:
                                    self.source_files.append(fullFile)

    #----------------------------------------------------------------------------------------
    # bsimethod                                     Majd.Udidn   02/2019
    #----------------------------------------------------------------------------------------       
    def get_files(self):
        self.fill_filles()
        return self.source_files

# tcatalog = TestsCatalog(r'C:\BSW\work\imodel02\src\imodel02\iModelCore\LicensingCrossPlatform')
# tcatalog.fill_filles()
# files = tcatalog.get_files()
# for f in files:
#     print f

# cpp = CPPTests(r'C:\BSW\work\imodel02\src\imodel02\iModelCore\LicensingCrossPlatform\Tests\UnitTests\Published\ClientTests.cpp')
# cpp.fill_tests()
# tests = cpp.get_tests()
# for test in tests:
#     print test