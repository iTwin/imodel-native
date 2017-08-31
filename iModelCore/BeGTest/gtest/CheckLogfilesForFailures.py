#--------------------------------------------------------------------------------------
#
#     $Source: gtest/CheckLogfilesForFailures.py $
#
#  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
#
#--------------------------------------------------------------------------------------
import os, sys, re
import time
import xml.etree.ElementTree as ET
#search path for importing symlinks python script from bentleybuild
srcpath=os.environ.get ("SrcRoot")
searchpath=os.path.join(srcpath,"BentleyBuild")
sys.path.append(searchpath)
import bentleybuild.symlinks as symlinks

# Failure patterns to match
# [  FAILED  ] RleEditorTester.MaxRleRunSetPixels    
# [  FAILED  ] FileFormatTests/ExportTester.ToBestiTiff/21, where GetParam() = L"D:\\ATP\\Dataset\\Images_Files\\_forATPs\\Images\\jpeg\\Mosaic_Jpeg_WF\\TC06L0\\15.jpg"
failedpat = re.compile (r"FAILED\s*]\s*(\w+\.\w+|\w+/\w+\.\w+/\d+)", re.I)
summarypat = re.compile (r"\[==========\].*ran", re.I)
runpat = re.compile (r"RUN\s*]\s*(\w+\.\w+)", re.I)
mspat = re.compile (r"ms\s*\)", re.I)

RELATIVE_TREE_CONFIG_PATH = 'teamConfig' + os.sep + 'treeConfiguration.xml'
TREE_CONFIG_PATH = os.path.join(srcpath,RELATIVE_TREE_CONFIG_PATH)

#-------------------------------------------------------------------------------------------
# bsimethod                                     Ridha.Malik      08/2017
#-------------------------------------------------------------------------------------------
def ignoreflakytest(srcpath,testListf):
    for root, dirs, files in os.walk(srcpath):
        for file in files:
            if file == "ignore_list.txt":
               fpath=os.path.join(root,file)
               status=symlinks.isSymbolicLink(fpath)
               if status:
                   fpath1=symlinks.getFinalPath(fpath)
                   filep=open(fpath1,'a+')
                   for key,value in failedTestsDic.items():
                       filep.write("\n"+key+"\n")
                       for v in value:
                           filep.write(v+"\n")
                   filep.close()

#-------------------------------------------------------------------------------------------
# bsimethod                                     Ridha.Malik      08/2017
#-------------------------------------------------------------------------------------------
def FindStreamDetails():
    Buildconfig=""
    Stream=""
    tree = ET.parse(TREE_CONFIG_PATH)
    root = tree.getroot()
    for child in root.iter('Stream'):
        Stream=child.attrib
    Stream=Stream['Name']
    if(os.environ.get ("DEBUG"))!=None:
       Buildconfig="DEBUG"
    elif(os.environ.get ("NDEBUG"))!=None:
       Buildconfig="NDEBUG"
    elif "PRG" in [var.upper() for var in os.environ]:
       Buildconfig="NDEBUG"
    else:
        Buildconfig="Unknown"
    return Stream,Buildconfig

#-------------------------------------------------------------------------------------------
# bsimethod                                     Ridha.Malik      08/2017
#-------------------------------------------------------------------------------------------
def FindFailedTestFailures(lines,failedTestsList):
    failedTestsDic={}
    i=0
    start=0
    end=0
    for x in failedTestsList:
        for line in lines:
            if x in line and (runpat.search(line)!=None):
                   start=i
            if x in line and (failedpat.search(line)!=None) and (mspat.search(line)!=None):
                   end =i
            i=i+1
        i=0
        failedTestsDic.setdefault(x,[])
        Stream,Buildconfig=FindStreamDetails()
        details="Stream:"+Stream+", build configuration :"+Buildconfig+", architecture: "+Tragetplatform+", Date: "+time.strftime("%d/%m/%Y")
        failedTestsDic[x].append(details)
        for j in range(start+1,end):
            failedTestsDic[x].append(lines[j])
    return failedTestsDic

#-------------------------------------------------------------------------------------------
# bsimethod                                     Sam.Wilson      05/2016
#-------------------------------------------------------------------------------------------
def checkLogFileForFailures(logfilename):
    global exename
    exename = ''
    foundSummary = False
    global failedTestsList
    failedTestsList = ""
    lastTestRun = ""
    colon = ''
    anyFailures = False
    lineNo = 0
    summaryLineNo = 0
    summarystr = logfilename + '\n'
    lines=''
    global failedTestsDic
    failedTestsDic={}
    with open(logfilename, 'r') as logfile:
        lines=logfile.readlines()
        for line in lines:

            lineNo = lineNo + 1

            # Name of test runner exe is always the first line
            if lineNo == 1:
                exename = line.strip('\n')
                continue
            
            if not foundSummary:
                # We don't look for much in the log before the summary. 
                if summarypat.search(line) != None:
                    summarystr = summarystr + line
                    foundSummary = True
                    summaryLineNo = 0
                else:
                    # We do keep track of the last test run, in case we need to report a crash or early exit
                    run = runpat.search(line)
                    if run != None:
                        lastTestRun = run.group(1)

                continue

            else:
                # The summary has the results, either the count of tests run or a list of failing tests.
                summaryLineNo = summaryLineNo + 1

                if summaryLineNo < 3:
                    summarystr = summarystr + line
                    continue

                if line.find('FAILED TEST') != -1:
                    anyFailures = True

                failed = failedpat.search(line)
                if failed != None:
                    failedTestsList = failedTestsList + colon + failed.group(1)
                    colon = ':'
                    continue

        if failedTestsList!='' and ignorefailure:
           failedTestsListemp=failedTestsList.split(':')
           failedTestsDic=FindFailedTestFailures(lines,failedTestsListemp)

    if not anyFailures and foundSummary:
        return '',summarystr

    #advicestr = '************ Failures from: ' + logfilename + ' ******************'
    advicestr = ''

    # If we never got to the summary, that means that the last test to run was interrupted by a crash
    if not foundSummary:
        advicestr = advicestr + "\n"
        advicestr = advicestr + "\n*** CRASH ***"
        advicestr = advicestr + "\n"
        advicestr = advicestr + "\nERROR: Test crashed in " + exename + " - " + lastTestRun
        advicestr = advicestr + "\n"
        advicestr = advicestr + "\nTo run just the crashing test, run the following command in your debugger:"
        advicestr = advicestr + "\n    " + exename + " --gtest_break_on_failure --gtest_filter=" + lastTestRun
        advicestr = advicestr + "\n"

    # Report all failures that we saw in the log
    if len(failedTestsList) != 0:
        advicestr = advicestr + "\n"
        advicestr = advicestr + "\nERROR: Test(s) failed in " + exename + " - " + failedTestsList
        advicestr = advicestr + "\n"
        advicestr = advicestr + "\nTo re-run failing tests, run the following command in your debugger:"
        advicestr = advicestr + "\n    " + exename + " --gtest_break_on_failure --gtest_filter=" + failedTestsList
        advicestr = advicestr + "\n"

    if anyFailures or not foundSummary:
        # When we detect failing or crashing tests, print the whole log. That will then go into bb's build log.
        # The user will want to scroll up to see complete details.
        print '************ ' + logfilename + ' ******************'
        with open(logfilename, 'r') as logfile:
            for line in logfile.readlines():
                print line,
        print '*********************************************************************************'

    return advicestr,summarystr

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
    ignorefailure = False
    Tragetplatform=''
    if len(sys.argv) > 2 and int(sys.argv[2]) != 0:
        breakonfailure = True
    if len(sys.argv) >3 and str(sys.argv[3]) =="True":
        ignorefailure = True
        Tragetplatform=sys.argv[4]
    advicestr = ''
    summarystr = ''
    exename = ''
    failedTestsDic=''
    for root,dirs,files in os.walk (dir, topdown=True, onerror=None, followlinks=True):
        for file in files:
            if not file.endswith('.log'):
                continue;

            checkedCount = checkedCount + 1
            path = os.path.join(root, file)
            adviceForThisLog,summarystrThisLog = checkLogFileForFailures(path)
            summarystr = summarystr + '\n\n' + summarystrThisLog
            if 0 != len(adviceForThisLog):
                advicestr = advicestr + '\n\n' + adviceForThisLog
     
    if checkedCount == 0:
        print 'no logfiles found in ' + dir
        exit(0)

    print '{0} test product logs found'.format(checkedCount)

    print summarystr

    if 0 == len(advicestr):
        print "All tests passed."
        exit (0)

    statusfilename=os.path.join(dir,"CheckStatus.txt")
    if len(failedTestsDic)!=0 and ignorefailure:
       if "using" in exename:
           exename=exename.split("using ")
           srcpath=os.path.dirname(exename[1])
       else:
           srcpath=os.path.dirname(exename)
       ignoreflakytest(srcpath,failedTestsDic)
       file=open(statusfilename,'wb')
       file.write("FlakyTests")
       file.close()
    elif(os.path.isfile(statusfilename)):
        os.remove(statusfilename)
    print advicestr
    exit(breakonfailure)

