#--------------------------------------------------------------------------------------
#
#     $Source: TestFlakiness/TestFlakiness.py $
#
#  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
#
#--------------------------------------------------------------------------------------
import os
import csv
import re
import sys
import sqlite3
import string
import stat
import fnmatch
import time
import datetime
import random
import argparse
#Common Scripts to be used by any task
scriptsDir = os.getenv('SrcRoot')
commonScriptsDir = os.path.join(scriptsDir, 'DgnDbTestingScripts', 'CommonTasks')
sys.path.append(commonScriptsDir)

import CommitAndPushChanges as cm
import FindStream
from SqliteDatabase import Database
import Components as compo

testRE = re.compile ('^\s*TEST\s*\(\s*(\w+)\s*\,\s*(\w+)')
testfRE = re.compile ('^\s*TEST_F\s*\(\s*(\w+)\s*\,\s*(\w+)')
testPRE = re.compile ('^\s*TEST_P\s*\(\s*(\w+)\s*\,\s*(\w+)')

Deleted = "No"
testList = []
count = 0
ignoredList = []
testschemalist=[]
CurrentTime = time.time()
date = str(datetime.datetime.fromtimestamp(CurrentTime).strftime('%Y-%m-%d %H:%M:%S'))

dppath=os.path.join(*[os.getenv('SrcRoot'),"imodel02", "TestingScripts","TestFlakiness","TestsCatalog.db"])

reload(sys)
sys.setdefaultencoding('utf-8')

strings = ("@bsimethod" , "@bsiclass" , "@betest" , "@bsitest" , "@bsistruct")

components = ['Bentley','ECPresentation', 'BeHttp', 'DgnDomains','Planning','RealityModeling','DgnV8','Dwg','iModelBridge','ecobjects','Units', 'BeSecurity' , 'DgnPlatform','BeSQLite', 'ECDb','DgnDisplay','DgnClientFx' , 'GeomLibs', 'DgnDbSync','WSClient', 'ConstructionSchema','VersionCompare','ComponentModeling','GeoCoord']
#-------------------------------------------------------------------------------------------
# bsimethod                                     Ridha.Malik                     07/2017
#-------------------------------------------------------------------------------------------
def DBCreation():
    
    c.execute('''CREATE TABLE Stream
    (StreamId   INTEGER PRIMARY KEY,
    name        TEXT NOT NULL);''')
    print "Table Stream created successfully";

    c.execute('''CREATE TABLE Tests
    (TestId     INTEGER PRIMARY KEY,
    StreamId    INT,
    TestCase    TEXT    NOT NULL,
    Test        TEXT    NOT NULL,
    Author      TEXT    NOT NULL,
    File        TEXT    NOT NULL,
    Component   TEXT    NOT NULL,
    Owner       TEXT,
    Deleted     TEXT,
    Tag         TEXT,
    Date        DATETIME   NOT NULL,
    FOREIGN KEY (StreamId) REFERENCES Stream (StreamId));''')
    print "Table Tests created successfully";

    c.execute('''CREATE TABLE IgnoredTests
    (ig_Id      INTEGER PRIMARY KEY,
    TestId      INT,
    Ignored     TEXT    NOT NULL,
    Comment     TEXT    NOT NULL,
    Date        Text    NOT NULL,
    FOREIGN KEY (TestId) REFERENCES Tests(TestId) );''')
    print "Table IgnoredTests created successfully";

    c.execute('''CREATE TABLE ExecutionHistory
    (ex_Id      INTEGER PRIMARY KEY,
    TestId      INT,
    Platform    TEXT    NOT NULL,
    Device      TEXT    NOT NULL,
    Defect      TEXT    NOT NULL, 
    Date        TEXT    NOT NULL,
    FOREIGN KEY (TestId) REFERENCES Tests(TestId) );''')
    print "Table  ExeHistory created successfully";
    
    c.execute('''CREATE TABLE TestsDefectHistory
    (ex_Id      INTEGER PRIMARY KEY,
    TestId      INT,
    Platform    TEXT    NOT NULL,
    Device      TEXT    NOT NULL,
    Defect      TEXT    NOT NULL, 
    Date        TEXT    NOT NULL,
    FOREIGN KEY (TestId) REFERENCES Tests(TestId) );''')
    print "Table  TestsDefectHistory created successfully";
    
#-------------------------------------------------------------------------------------------
# bsimethod                                     Ayesha.Rahman                     08/2017
#-------------------------------------------------------------------------------------------
def FindComponent(pathSplit):
    '''
        This function finds the component name from the path, in the component array and returns the match
        pathSplit : The path of the cpp file, split on the basis of \\
        components : The array of components name stored above
        return : The component found

    '''

    comp = ""
    for comp1 in pathSplit:
        for comp2 in components:
            if comp1 == comp2:
                comp = comp2
    return comp

#-------------------------------------------------------------------------------------------
# bsimethod                                     Ayesha.Rahman                     07/2017
#-------------------------------------------------------------------------------------------
def shouldParse (component):

    if component in components:
        return True
    else:
        return False

#-------------------------------------------------------------------------------------------
# bsimethod                                     Ayesha.Rahman                     07/2017
#-------------------------------------------------------------------------------------------
def findOwner (component):
    if component == "Bentley" or component == "DgnPlatform" or component == "DgnDisplay" or component == "DgnDbSync" or component == "DgnV8" or component=="iModelBridge":
        return "Sam.Wilson"
    if component == "ecobjects":
        return "Carole.MacDonalds"
    if component == "ECDb" or component == "BeSQLite":
        return "Krischan.Eberle"
    if component == "GeomLibs":
        return "Earlin.Lutz"
    if component == "DgnClientFx":
        return "Steve.Wilson"
    if component == "ConstructionSchema":
        return "Shaun.Sewall"
    if component == "DgnDomains" or component == "Planning":
        return "Raman.Ramananjum"
    if component == "Dwg":
        return "Don.Fu"
    if component == "BeHttp" or component=="BeSecurity":
        return "Vincas.Razma"
    if component == "WSClient": 
        return "Arturas.Januska"
    if component == "Units": 
        return "David.Fox-Rabinovitz"
    if component == "RealityModeling": 
        return "Marc.Bedard"
    if component == "ECPresentation": 
        return "Grigas.Petraitis"
    if component == "VersionCompare" or component == "ComponentModeling": 
        return "Diego.Pinate"
    if component == "GeoCoord": 
        return "Barry.Bentley"

#-------------------------------------------------------------------------------------------
# bsimethod                                     Ayesha.Rahman                     08/2017
#-------------------------------------------------------------------------------------------
def TestDetect(line):
    '''
        This function does the pattern matching for detecting Tests in line
        testRE : Tests following pattern TEST()
        testFRE : Tests following pattern TEST_F()
        testPRE : Tests following pattern TEST_P()
    '''

    testMatch = testRE.search (line)
    if testMatch == None:
        testMatch = testfRE.search (line)
    if testMatch == None:
        testMatch = testPRE.search (line)
    return testMatch



#-------------------------------------------------------------------------------------------
# bsimethod                                     Ayesha.Rahman                     08/2017
#-------------------------------------------------------------------------------------------
def SelectTag(line,lane):

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
def FindTagInAboveLines(lines,i):
    '''
        This function traverses the lines above the Test to find the tag on it and returns ALL if no tag is found
        SelectTag() : A function call to determine which tag is found in the line

    '''

    Tag = "ALL"
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
            Tag = SelectTag(linee,lane)
            if Tag == "ALL":
                check = 1
                continue
            else:
                break
    return Tag


#-------------------------------------------------------------------------------------------
# bsimethod                                     Ayesha.Rahman                     07/2017
#-------------------------------------------------------------------------------------------
def getComment(line):
    start = line.find('#')
    if start == -1: # It doesn't have a comment
        return " "
    else:
        return line[start + 1:]



#-------------------------------------------------------------------------------------------
# bsimethod                                     Ridha.Malik                     07/2017
#-------------------------------------------------------------------------------------------
def IgnoreList(directory,testNameToMatch):
    '''
        This function walks through the ignorelist.txt file to find if the given test is in the ignorelist or not
        ignored : this variable is either set 'Y' or 'R' based on whether the test was found in the file or not,respectively.

    '''


    ignored = "R"
    comment=" "
    for root2, dirs2, files2 in os.walk(directory):
        for file2 in files2:
            if file2 == "ignore_list.txt":
                openfile = open(os.path.join(root2,file2), "r")
                for line2 in openfile:
                    line2=line2.strip()
                    line2parts = line2.split(" ")
                    ignoreName = line2parts[0]
                    if ignoreName == testNameToMatch:
                        ignored = 'Y'
                        comment = getComment(line2)
                    if line2.startswith("#") or not line2.strip(): # It is a comment or empty line, let's move to next line
                            continue
                    else:
                        if ignoreName.startswith("*"):
                            ignoreName = ignoreName[1:]
                            pattern = re.compile(ignoreName)
                            if pattern.match(testNameToMatch): # Doing regex comparison
                                ignored = 'Y'
                                comment = getComment(line2)
                        elif ignoreName.endswith(".*"):
                            ignoreNamet = ignoreName.split(".")
                            ignoreName=ignoreNamet[0]
                            pattern = re.compile(ignoreName)
                            if pattern.match(testNameToMatch): # Doing regex comparison
                                ignored = 'Y'
                                comment = getComment(line2)
                        elif ignoreName.endswith("*"):
                            ignoreNamet = ignoreName.split("*")
                            ignoreName=ignoreNamet[0]
                            pattern = re.compile(ignoreName)
                            if pattern.match(testNameToMatch): # Doing regex comparison
                                ignored = 'Y'
                                comment = getComment(line2)
                        else:
                            pattern = re.compile(ignoreName)
                            if pattern.match(testNameToMatch) and "." not in ignoreName: # Doing regex comparison
                                ignored = 'Y'
                                comment = getComment(line2)

    return ignored,comment

#-------------------------------------------------------------------------------------------
# bsimethod                                     Ayesha.Rahman                     08/2017
#-------------------------------------------------------------------------------------------
def TraverseIgnoreList(fileSplit,component_Directory_Path,testNameToMatch):
    '''
        This function finds the path till the Tests Directory and sends it as an argument to another function
        The function was required for cases where there are more than one Tests folder in that case,
        the ignore file of the respective folder should be sent from which the Test belongs

    '''

    for name in fileSplit:
        if "cpp" in name.lower():
            fileSplit.remove(name)
    fileSplit.append("i")
    for name in fileSplit:
        if name.lower().endswith("tests"):
            path = os.path.join(component_Directory_Path,name)
            print path
            ignored,comment = IgnoreList(path,testNameToMatch)
            break
        else:
            ignored,comment = IgnoreList(component_Directory_Path,testNameToMatch)
            break
    return ignored,comment




#-------------------------------------------------------------------------------------------
# bsimethod                                     Ayesha.Rahman                     07/2017
#-------------------------------------------------------------------------------------------

    '''
        This function finds and returns Author's name in the given line

    '''

def FindNameInLine (line):
    line = line.strip()
    lineLower = line.lower()
    if any(s in lineLower for s in strings):
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
def FindAuthorNameInExtendedLines(lines,i):
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
        authorName = FindNameInLine(lines[i-k])
        if not authorName == "UnKnown":
            break
    return authorName



#-------------------------------------------------------------------------------------------
# bsimethod                                     Ayesha.Rahman                     08/2017
#-------------------------------------------------------------------------------------------
    '''
        This function traverses the 3-4 lines above the Test to find the author
        FindNameInLine(): each of the given line is sent to this function for extracting the author name
        FindAuthorNameInExtendedLines(): This function call is made when the author isn't found in the above 3-4 lines

    '''

def FindAuthorNameInAbove4Lines(lines,i):
    for k in range(1,4):
        authorName = FindNameInLine(lines[i-k])
        if not authorName == "UnKnown":
            break
    if authorName == "UnKnown":
        authorName=FindAuthorNameInExtendedLines(lines,i)
    return authorName



def deletedTests():
    '''
        This function marks the Tests that are not in the Cpp files anymore, as Deleted in the Database

    '''

    c.execute("SELECT TestCase,Test,File,Component FROM Tests WHERE StreamId= ?",(StreamId,)) # with newly added data
    data = c.fetchall()
    for tests in data:
        found = False
        for del_test in ignoredList:
            if (tests[0]==del_test[0] and tests[1]==del_test[1] and tests[3]==del_test[2] and tests[2]==del_test[5]):
                found = True
                break
        if (found == False):
            dataa = "\"Yes\""
            #print "###",tests[1]
            sql = 'UPDATE Tests SET Deleted=' + str(dataa) + ' WHERE TestCase="' + str(tests[0]) + '" AND Test = "'+ str(tests[1]) +'" AND Component = "' + str(tests[3]) +'"'
            c.execute(sql)




#-------------------------------------------------------------------------------------------
# bsimethod                                     Ayesha.Rahman                     08/2017
#-------------------------------------------------------------------------------------------
def IgnoredTests():
    '''
        This function is used for maintaing a full record of Tests as to on what date were
        they added in the file or removed from the file

    '''

    c.execute("SELECT TestId, TestCase, Test, Component,Tag FROM Tests")
    itr = c.fetchall()

    c.execute('SELECT TestId FROM IgnoredTests')
    Results = c.fetchall()

    if len(Results) == 0:
        for test_in_db in itr:
            for new_test in ignoredList:
                if new_test[3] == "R":
                    continue
                else:
                    if (test_in_db[1]==new_test[0] and test_in_db[2]==new_test[1] and test_in_db[3].lower() == new_test[2].lower()) :
                        c.execute("INSERT INTO IgnoredTests(TestId, Ignored,  Comment, Date) VALUES (?, ?, ?, ?)", (test_in_db[0],new_test[3],new_test[4],date))
                        print "Added Tests on the Very First Run"
    else:
        for Test in itr:
            for ig_list in ignoredList:
                if (Test[1]==ig_list[0] and Test[2]==ig_list[1] and Test[3].lower() == ig_list[2].lower()):
                    found = False
                    for ig_table in Results:
                        if(Test[0] == ig_table[0]):
                            found = True
                            maxx = c.execute('SELECT * FROM IgnoredTests WHERE ig_Id = (SELECT  MAX(ig_Id) FROM IgnoredTests i2 WHERE i2.TestId = ?)',(Test[0],))
                            for maxim in maxx:
                                if not(maxim[2] == ig_list[3]):
                                    c.execute("INSERT INTO IgnoredTests(TestId, Ignored,  Comment, Date) VALUES (?, ?, ?, ?)", (Test[0],ig_list[3],ig_list[4],date))
                                    print "updated Ignored test History"
                    if(found == False):
                        if ig_list[3] == "R":
                            continue
                        c.execute("INSERT INTO IgnoredTests(TestId, Ignored,  Comment, Date) VALUES (?, ?, ?, ?)", (Test[0],ig_list[3],ig_list[4],date))
                        print "New Test Added as Ignored Test"



#-------------------------------------------------------------------------------------------
# bsimethod                                     Ridha.Malik                    01/2018
#-------------------------------------------------------------------------------------------
def StreamId():
    '''
        Retruns the stream Id from database
    '''
    c.execute("SELECT  StreamId FROM Stream WHERE name = ?",(Stream,))
    result = c.fetchall()
    if len(result) == 0:
        c.execute("INSERT INTO Stream ( name ) VALUES ( ? )", (Stream,))
        c.execute("SELECT StreamId FROM Stream WHERE name = ?",(Stream,))
        result = c.fetchall()
        print " New stream name addedd "
        return result[0][0]
    return result[0][0]

#-------------------------------------------------------------------------------------------
# bsimethod                                     Ridha.Malik                    01/2018
#-------------------------------------------------------------------------------------------
def DefinedTestSchema(line,lineno):
    '''
        Search if test schema defined for test seprately 
    '''
    Schemaregix= re.compile ('^\s*DEFINE_SCHEMA\s*\(\s*(\w+)\s*\,\s*(\w+)')
    testschema= Schemaregix.search (line)
    if testschema != None:
        testschemalist.append((testschema.group(1),lineno))
        #print"$$$", testschema.group(1),lineno
        return testschemalist
    else:
        return testschemalist
#-------------------------------------------------------------------------------------------
# bsimethod                                     Ridha.Malik                    01/2018
#-------------------------------------------------------------------------------------------    
def TesthasSchema(testname):
    '''
        Find test has defined schema seprately above author header
    '''
    if len(testschemalist)!=0:
        for test in testschemalist:
            if testname==test[0]:
                #print "testname",testname
                return test[1]
        return -1
    else:
        return -1
        
#-------------------------------------------------------------------------------------------
# bsimethod                                     Ayesha.Rahman                     08/2017
#-------------------------------------------------------------------------------------------
    '''
        This function generates a CSV report in the end to summarize what the script did, everyday.
        You can find information like how many tests were added, deleted, put in ignored table , etc. in the report

    '''

def CsvReport():
    datee = date.split(" ")
    datee = datee[0]
    datee = datee+"%"
    dbCount = 0

    c.execute("SELECT count(Author) FROM Tests WHERE Author LIKE ? AND Date LIKE ? AND StreamId=?",("Unknown",datee,StreamId))
    author_count=c.fetchall()

    c.execute("SELECT * FROM Tests WHERE Author LIKE ? AND Date LIKE ? AND StreamId=?",("Unknown",datee,StreamId))
    unknown_author = c.fetchall()

    c.execute("SELECT COUNT(Deleted) FROM Tests WHERE Deleted=? AND Date LIKE ? AND StreamId=?",("Yes",datee,StreamId))
    delete_count = c.fetchall()

    c.execute("SELECT * FROM Tests WHERE Deleted = ? AND Date LIKE ? AND StreamId=?",("Yes",datee,StreamId))
    deleted_test = c.fetchall()

    #c.execute("SELECT COUNT(*) FROM (SELECT * FROM Tests INNER JOIN IgnoredTests WHERE Tests.TestId = IgnoredTests.TestId AND IgnoredTests.Date LIKE ? AND IgnoredTests.Ignored=?)",(datee,"Y"))
    #ignoredTests_count = c.fetchall()

    #c.execute('SELECT TestCase,Test,File,Component,Deleted,Tag,Ignored,Comment FROM Tests INNER JOIN IgnoredTests WHERE Tests.TestId = IgnoredTests.TestId AND IgnoredTests.Date LIKE ? AND IgnoredTests.Ignored=?',(datee,"Y"))
    #ignored_Tests = c.fetchall()

    #c.execute("SELECT COUNT(*) FROM (SELECT * FROM Tests INNER JOIN IgnoredTests WHERE Tests.TestId = IgnoredTests.TestId AND IgnoredTests.Date LIKE ? AND IgnoredTests.Ignored=?)",(datee,"R"))
    #RemoveFrom_IgnoreList_count = c.fetchall()

    #c.execute('SELECT TestCase,Test,File,Component,Deleted,Tag,Ignored,Comment FROM Tests INNER JOIN IgnoredTests WHERE Tests.TestId = IgnoredTests.TestId AND IgnoredTests.Date LIKE ? AND IgnoredTests.Ignored=?',(datee,"R"))
    #RemoveFrom_IgnoreList = c.fetchall()

    with open("TestsFlakinessReport.csv" , "wb") as f:
        writer = csv.writer(f)
        writer.writerow(('Component','Test Count'))
        for x in components:
            c.execute('select count(Component) from Tests where Component = ? AND Deleted = ? AND StreamId=?',(x,"No",StreamId))
            itr = c.fetchall()
            dbCount = dbCount+itr[0][0]
            writer.writerow((x,itr[0][0]))
        writer.writerow(('Database Test Count',dbCount))
        writer.writerow(('Unknown Authors Count',author_count[0][0]))
        writer.writerow(('TestId','TestCase' ,'Test' ,'Author','File', 'Component','Owner','Stream' ,'Deleted', 'Tag','Date'))
        writer.writerows(unknown_author)
        writer.writerow(('Deleted Test Count',delete_count[0][0]))
        writer.writerow(('TestId','TestCase' ,'Test' ,'Author','File', 'Component','Owner','Stream' ,'Deleted', 'Tag','Date'))
        writer.writerows(deleted_test)
        '''
        writer.writerow(('Ignored Tests Count',ignoredTests_count[0][0]))
        writer.writerow(('TestCase' ,'Test' ,'File', 'Component','Deleted', 'Tag','Ignored','Comment'))
        writer.writerows(ignored_Tests)
        writer.writerow(('Tests Removed From IgnoreList Count',RemoveFrom_IgnoreList_count[0][0]))
        writer.writerow(('TestCase' ,'Test' ,'File', 'Component','Deleted', 'Tag','Ignored','Comment'))
        writer.writerows(RemoveFrom_IgnoreList)
        '''
#-------------------------------------------------------------------------------------------
# bsimethod                                     Ridha.Malik                  01/2018
#-------------------------------------------------------------------------------------------
def UpdateDefectHistory(id,testClass,testName,component):
    
    ignorelistfile=compo.IgnorePathForComp(component)
    regarc=re.compile (r"architecture: .*,", re.I)
    regdate=re.compile (r"Date: .*", re.I)
    regdevice=re.compile (r"Device: .*,", re.I)
    test=testClass+"."+testName
    
    if ignorelistfile != None:
        openfile = open(ignorelistfile, "r")
        print ignorelistfile
        lines = openfile.readlines()
        i=0
        while(i<lines.__len__()):
            line = lines[i]
            line = line.strip()
            if line.startswith(test):
                try:
                    testline1=lines[i+3].strip()
                    testline2=lines[i+2].strip()
                except (ValueError,IndexError):
                    return "NULL"
                architecture=regarc.search(testline1)
                date=regdate.search(testline1)
                device=regdevice.search(testline1)
                if architecture!=None and "TFS#" in testline2 and date!=None and device!=None:
                    architecture=architecture.group(0).split(',')
                    architecture=architecture[0].split(" ")
                    architecture=architecture[1]
                    TFSno=testline2.split("TFS#")[1]
                    date=date.group(0).split(' ')[1]
                    device=device.group(0).split(" ")[1].split(",")[0]
                    c.execute("SELECT TestId,Platform,Device,Defect FROM TestsDefectHistory WHERE TestId= ? AND Platform = ?  AND Device = ? AND Defect=?",(id,architecture,device,TFSno))
                    result = c.fetchall()
                    if len(result)==0:
                        c.execute("INSERT INTO TestsDefectHistory (TestId,Platform,Device,Defect,Date) VALUES (?, ?, ?, ?, ?)", (id,architecture,device,TFSno,date))
                        result = c.fetchall()               
            i=i+1
            
#-------------------------------------------------------------------------------------------
# bsimethod                                     Ridha.Malik                  01/2018
#-------------------------------------------------------------------------------------------            
def parsecomponent(component):
    if component=="DgnV8":
        return "DgnV8Converter"
    if component=="ConstructionSchema":
        return "ConstructionPlanning"
    else:
        return component
    
#-------------------------------------------------------------------------------------------
# bsimethod                                     Ayesha.Rahman                     07/2017
#-------------------------------------------------------------------------------------------
def TestRecord(srcDirectory,Stream,StreamId):
    '''
        This function is the core function of the Script, all calls to other functions are
        made from this function.The WORKFLOW of the function is as follows:
        - Traverses all the folders and sub-folders in the source directory until the Tests/Test
          folder is found
        - Walks through each Cpp file in the Folder to find Tests.
        - Traverses a few lines above the test to find the Author name and the tag
        - Finds other required information like TestName,Owner,File,Comonent,etc.
        - Adds the Test in the Database if doesn't exist already, updates the information
          of the existing Test otherwise

    '''
    check=0
    for root,allDirs,allFiles in os.walk(srcDirectory):
        for componentDirectory in allDirs:
            if componentDirectory.lower() == "test" or componentDirectory.lower() == "tests":
                component_Directory_Path = os.path.join(root,componentDirectory)
                for root2t,dirs2t,filest in os.walk(component_Directory_Path):
                    for file in filest:
                        if file.endswith('.cpp'):
                            fullFile = os.path.join(root2t,file)
                            pathSplit = fullFile[len(srcDirectory):].split('\\')
                            component = FindComponent(pathSplit)

                            if shouldParse(component):
                                print "*** Listing tests in: " + component
                                openfile = open(fullFile, "r")
                                print fullFile
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
                                    testschemalist=DefinedTestSchema(line,i)
                                    testMatch = TestDetect(line)
                                    if testMatch == None:
                                        i=i+1
                                        continue
                                    testClass = testMatch.group(1)
                                    testName = testMatch.group(2)
                                    Lineno=TesthasSchema(testName)
                                    Tag = FindTagInAboveLines(lines,i)
                                    ignored = 'R'
                                    comment=" "
                                    testNameToMatch = testClass + "." + testName
                                    fileSplit = fullFile[len(component_Directory_Path)+1:].split('\\')
                                    ignored,comment=TraverseIgnoreList(fileSplit,component_Directory_Path,testNameToMatch)
                                    if Lineno!=-1:
                                       lineindex=Lineno
                                    else:
                                        lineindex=i
                                        testschemalist=[]
                                    authorName = FindAuthorNameInAbove4Lines(lines,lineindex)
                                    if testName.startswith("DISABLED"):
                                        Tag ="Disabled"
                                    filePath= os.path.join('%SrcRoot%', 'imodel02', 'iModelCore')+fullFile[len(srcDirectory):]
                                    ignoredList.append((testClass, testName,component,ignored,comment,filePath))
                                    owner=findOwner(component)
                                    c.execute("SELECT TestId,Author,Deleted, Tag FROM Tests WHERE TestCase = ? AND Test = ?  AND File = ? AND Component = ?  AND StreamId = ?",(testClass,testName,filePath,component,StreamId))
                                    result = c.fetchall()
                                    if not len(result) == 0:
                                        if not (result[0][3]== Tag):
                                            c.execute("UPDATE Tests SET Tag = ? WHERE TestId = ?",(Tag,result[0][0]))
                                        if (result[0][2]=="Yes"):
                                            c.execute("UPDATE Tests SET Deleted = ? WHERE TestId = ?",("No",result[0][0]))
                                        if not (authorName == result[0][1]):
                                            c.execute("UPDATE Tests SET Author = ? WHERE TestId = ?",(authorName,result[0][0]))
                                    elif len(result) == 0:
                                        #This portion of the code is for those tests that have been moved to a new repo.
                                        c.execute("SELECT * FROM Tests WHERE TestCase = ? AND Test = ? AND StreamId = ?",(testClass,testName,StreamId))
                                        data = c.fetchall()
                                        if not len(data) == 0:
                                            if not (data[0][5]==component or data[0][4]==filePath):
                                                c.execute("UPDATE Tests SET Component = ? WHERE TestId = ?",(component,data[0][0]))
                                                c.execute("UPDATE Tests SET File = ? WHERE TestId = ?",(filePath,data[0][0]))
                                                c.execute("UPDATE Tests SET Author = ? WHERE TestId = ?",(authorName,data[0][0]))
                                                c.execute("UPDATE Tests SET Deleted = ? WHERE TestId = ?",("No",data[0][0]))
                                                c.execute("UPDATE Tests SET Owner = ? WHERE TestId = ?",(owner,data[0][0]))
                                            if  (data[0][5]==component and data[0][4]!=filePath):
                                                c.execute("UPDATE Tests SET Component = ? WHERE TestId = ?",(component,data[0][0]))
                                                c.execute("UPDATE Tests SET File = ? WHERE TestId = ?",(filePath,data[0][0]))
                                                c.execute("UPDATE Tests SET Author = ? WHERE TestId = ?",(authorName,data[0][0]))
                                                c.execute("UPDATE Tests SET Deleted = ? WHERE TestId = ?",("No",data[0][0]))                                              
                                        else:
                                            print "&&& New Test",testClass,testName
                                            c.execute("INSERT INTO Tests (TestCase, Test, Author, File, Component, Owner, Deleted, Tag, Date,StreamId) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?,?)",(testClass,testName,authorName,filePath,component,owner,'No',Tag,date,StreamId))
                                            testid=c.lastrowid
                                            component=parsecomponent(component)
                                            UpdateDefectHistory(testid,testClass,testName,component)
                                    i=i+1
                        check=0
    deletedTests()
    #IgnoredTests()
    CsvReport()


#---Entry point of the Script ---#
if __name__ == '__main__':

    parser = argparse.ArgumentParser(description='Tests Database.')

    parser.add_argument('--srcpath',help='path of your src directory for which want to maintain test catalog')
    parser.add_argument('--stream' ,help='Name of the Stream for which you want to maintain tests record,If not given it determines it self')
    parser.add_argument('--db' ,dest="db" ,action="store_true", default=False, help='Already have a database?')
    parser.add_argument('--commitpush', dest="commitpush" ,action="store_true" ,default=False, help='Do you want to commit and push changes?')

    args = parser.parse_args()

    directory = args.srcpath
    Stream = args.stream
    dbChoice = args.db
    commitpush=args.commitpush
    
    if Stream == None:
        TeamConfigPath = os.path.join(os.getenv('SrcRoot'), 'teamConfig', 'treeConfiguration.xml')
        Stream = FindStream.FindStreamDetails(TeamConfigPath)
        Stream = Stream.lower()
    else:
         Stream = args.stream.lower()
    db=Database(dppath)
    db.connectDatabase()
    c=db.cursor
    if not dbChoice:
        DBCreation()
        StreamId = StreamId()
        print "Stream id",StreamId
        TestRecord(directory,Stream,StreamId)
        db.saveChanges()
    else:
        StreamId= StreamId()
        print "Stream id",StreamId
        TestRecord(directory,Stream,StreamId)
        db.saveChanges()
        if commitpush:
            filepath=os.getcwd()
            filename="TestsCatalog.db"
            message="Updated Database"
            status=cm.CommitAndPush(filepath,filename,message,"Maha.Nasir")
            while status!=0:
               dppath=os.path.join(*[os.getenv('SrcRoot'),"imodel02", "TestingScripts","TestFlakiness","TestsCatalog.db"])
               db=Database(dppath)
               db.connectDatabase()
               c=db.cursor
               TestRecord(directory,Stream,StreamId)
               db.saveChanges()
               status=cm.CommitAndPush(filepath,filename,message,"Maha.Nasir")
    sys.exit()
