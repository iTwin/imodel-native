#--------------------------------------------------------------------------------------
#
#  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
#
#--------------------------------------------------------------------------------------
import os, sys, shutil
import xml.etree.ElementTree as ET
import argparse
import time
import subprocess

#Common Scripts to be used by any task
scriptsDir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
commonScriptsDir = os.path.join(scriptsDir, 'CommonTasks')
sys.path.append(commonScriptsDir)

###BentleyBuild script for color printing
##bbDir = os.path.join(scriptsDir, 'bentleybuild', 'bentleybuild')
##sys.path.append(bbDir)
##import utils

#import CommitAndPushChanges as cm
from TestResults import TestResults
import Components as cmp
from CoverageReports import runCoverage, printResults
from GitCommands import GitCommand

#-------------------------------------------------------------------------------------------
# bsimethod                                    Majd.Uddin    12/2017
#-------------------------------------------------------------------------------------------
def getMapFromFile(fileName):
    tiaMap = {}
    # first get all files
    files = []
    fn, fx = os.path.splitext(os.path.basename(fileName))
    fp = fn.split('_')
    ff = ''
    for i in range(0, len(fp)):
        ff = ff + fp[i] + '_'
    for f in os.listdir(os.path.dirname(fileName)):
        if f.startswith(ff[:-1]):
            files.append(os.path.join(os.path.dirname(fileName),f))
    for fl in files:
        f = open(fl, 'r')
        lines = f.readlines()
        i = 0
        for line in lines:
            if line.strip().startswith('['):
                sfName = line.strip()[1:-1]
                if sfName not in tiaMap:
                    tiaMap.setdefault(sfName, {})
                tests = lines[i+1]
                testP = tests.split(' ')
                testP2 = testP[1].split('=')
                testList = testP2[1].split(':')
                for t in testList[:-1]:
                    tP = t.split('.')
                    testCase = tP[0]
                    test = tP[1]
                    if testCase not in tiaMap[sfName]:
                        tiaMap[sfName].setdefault(testCase, [])
                    if test not in tiaMap[sfName][testCase]:
                        tiaMap[sfName][testCase].append(test)
            i = i + 1

              
    return tiaMap
#-------------------------------------------------------------------------------------------
# bsimethod                                    Majd.Uddin    12/2017
#-------------------------------------------------------------------------------------------
def consolidateMaps(newtiaMap, fileName):
    print("\n 'Consolidating Maps for : " + fileName + "\n")
    oldtiaMap = getMapFromFile(fileName)
    for entry in newtiaMap:
        for entry1 in oldtiaMap:
            if entry == entry1:
                for testCase in newtiaMap[entry]:
                    for testCase1 in oldtiaMap[entry]:
                        if testCase == testCase1:
                            testName = newtiaMap[entry][testCase]
                            testName1 = oldtiaMap[entry][testCase]
                            if testName == testName1:
                                continue
                            else:
                                oldtiaMap[entry][testCase] = testName
    return oldtiaMap        
  
#-------------------------------------------------------------------------------------------
# bsimethod                                     Majd.Uddin    10/2017
#-------------------------------------------------------------------------------------------
def getTiaMapAll(covRoot, comp, dll=None):
    tiaMap = {}

    srcRoot = os.getenv('SrcRoot').lower()
    covPath = os.path.join(covRoot, comp)
    dllFilter = cmp.AllDlls()

    totalFiles = len(os.listdir(covPath))
    i = 1
    for file in os.listdir(covPath):
        fileName, fileExt = os.path.splitext(file)
        if fileExt == '.xml':
            testNameP = fileName.split('.')
            testCase = testNameP[0]
            testName = testNameP[1]
            print("Processing file " + str(i) + " of " + str(totalFiles) + ". File Name: " + os.path.join(covPath, file))
            i+=1
            tree = ET.parse(os.path.join(covPath, file))
            root = tree.getroot()
            drive = root.find('sources').find('source').text
            drive = drive + '\\'
            packages = root.find('packages')
            for pack in packages:
                packName = os.path.basename(pack.attrib['name'])
                if dll != None: #dll was passed
                    if packName.lower() == dll.lower():
                        tiaMap.setdefault(packName, {})
                else: # parse all dlls
                    if packName not in tiaMap:
                        tiaMap.setdefault(packName, {})
                classes = pack.find('classes')
                for class1 in classes:
                    sfName = class1.attrib['filename']
                    lineRate = float(class1.attrib['line-rate'])
                    if packName.lower() in dllFilter and packName in tiaMap:
                        if cmp.CompForDll(packName).lower() in sfName.lower() and lineRate > 0.0:
                            sfPath =  os.path.join(drive, sfName)
                            if srcRoot.lower() in sfPath.lower():
                                sfNtoAdd = sfPath[sfPath.find(srcRoot)+len(srcRoot):]
                                if sfNtoAdd not in tiaMap:
                                    tiaMap[packName].setdefault(sfNtoAdd, {})
                                if testCase not in tiaMap[packName][sfNtoAdd]:
                                    tiaMap[packName][sfNtoAdd].setdefault(testCase, [])
                                if testName not in tiaMap[packName][sfNtoAdd][testCase]:
                                   tiaMap[packName][sfNtoAdd][testCase].append(testName)
                      
    return tiaMap

#-------------------------------------------------------------------------------------------
# bsimethod                                     Majd.Uddin    10/2017
#-------------------------------------------------------------------------------------------
def writeMapToFiles(mapDir, tiaMap, comp):
    for dll in tiaMap:
        if len(tiaMap[dll]) > 0: # It has entires
            fileName = os.path.join(mapDir, 'TIAMap_' + comp + '_' + cmp.CompForDll(dll) + '.txt')
            print("\n Writing map for: " + fileName + "\n")
            writeMapToFile(fileName, tiaMap[dll], comp)
        else:
            print("\n 'No entries found. Skipping: " + dll + "\n")

#-------------------------------------------------------------------------------------------
# bsimethod                                     Majd.Uddin    09/2017
#-------------------------------------------------------------------------------------------
def writeMapToFile(fileName, tiaMap, comp):
    oldtiaMapfile = os.path.join(os.getenv('SrcRoot'), 'imodel02', 'TestingScripts', 'TestImpactAnalysis', 'TIAMaps', os.path.basename(fileName))
    if os.path.exists(oldtiaMapfile): #TiaMap is there, so we need to conslidate
        tiaMap = consolidateMaps(tiaMap, oldtiaMapfile)

    mapFile = open(fileName,'w') 
    specialComps = ['GeoCoord', 'ConstructionPlanning', 'Planning']
    testLogDir = os.path.join(os.getenv('OutRoot'), 'Winx64', 'build','RunGTest')
    for name in tiaMap:
        mapFile.write('\n[' + name + ']\n')
        testExe = cmp.ExePathForComp(comp)
        testLog = cmp.LogPathForComp(comp)
        mapFile.write(testExe + ' --gtest_filter=')
        if not os.path.exists(testLog):
            testLog = os.path.join(os.path.dirname(os.path.dirname(fileName)), 'test.log')
        tr = TestResults(testLog)
        for testCase in tiaMap[name]:
            len1 = len(tiaMap[name][testCase])
            len2 = tr.getFixtureTestCount(testCase)
            if len2 > 0:
                percent = (float(len1) / float(len2))
            else:
                percent = 0.0
            if len1 >= len2 or percent > 0.5:
                mapFile.write(testCase + '.*:')
            else:
                for test in tiaMap[name][testCase]:
                    mapFile.write(testCase + '.' + test + ':')

#-------------------------------------------------------------------------------------------
# bsimethod                                     Majd.Uddin    02/2019
#-------------------------------------------------------------------------------------------
def adjustMapFiles(map_dir):
    limit = 999
    for f in os.listdir(map_dir):
        full_path = os.path.join(map_dir, f)
        statinfo = os.stat(full_path)
        size_kb = int(statinfo.st_size/1024)
        if size_kb > limit:
            filename, file_extension = os.path.splitext(os.path.basename(full_path))
            parts = int(size_kb / limit) + 1
            f1 = open(full_path, 'r')
            lines = f1.readlines()
            entries = (len(lines) - 1) / 2
            partlines = ((entries / parts) * 2) - 4
            last = 0
            for i in range(0, parts + 1):
                if i == 0:
                    start = i*partlines
                else:
                    start = i*partlines - 1
                end = (i+1)*partlines - 1
                last = end
                if i == 0:
                    f_name = os.path.join(map_dir, os.path.basename(filename) + file_extension)
                else:
                    f_name = os.path.join(map_dir, os.path.basename(filename) + '_' + str(i) + file_extension)
                f2 = open(f_name, 'w')
                print 'writing to file: ' + str(f_name)
                for line in lines[start:end]:
                    f2.write(line)
                #add empty line to ensure file gets updated
                f2.write('\n')
            #if remaining entires
            if last < len(lines):
                f_name2 = os.path.join(map_dir, os.path.basename(filename) + '_' + str(i+1) + file_extension)
                f3 = open(f_name2, 'w')
                print 'writing to file: ' + str(f_name2)
                for line in lines[last:]:
                    f3.write(line)
            f1.close()
            #os.remove(full_path)

#-------------------------------------------------------------------------------------------
# bsimethod                                     Majd.Uddin    02/2019
#-------------------------------------------------------------------------------------------
def copyMapFiles(mapDir, comps):
    srcDir = os.path.join(os.getenv('SrcRoot'), 'imodel02', 'TestingScripts', 'TestImpactAnalysis', 'TIAMaps')
    for comp in comps:
        compMapDir = os.path.join(mapDir)
        if os.path.exists(compMapDir):
            for file in os.listdir(compMapDir):
                shutil.copy2(os.path.join(compMapDir, file), srcDir)

#-------------------------------------------------------------------------------------------
# bsimethod                                     Majd.Uddin    10/2017
#-------------------------------------------------------------------------------------------
def pushMapFiles():
    os.chdir(os.path.dirname(os.path.realpath(__file__)))
    gc = GitCommand()
    success = gc.execute('git pull')
    if not success:
        return False
    # Now move to branch
    bn = 'pushmaps'
    success = gc.switch_branch(bn)
    if not success:
        return False
    # Check status
    fs = []
    success = gc.status()
    if not success:
        return False
    for f in gc.stats:
        fp = f.split(' ')
        if len(fp) > 0:
            print fp[0]
            fn = os.path.basename(fp[1])
            if (fn.startswith('TIAMap')):
                fs.append(fp[1])
    # If there are files. Add, commit and push them
    if len(fs) > 0:
        for f1 in fs:
            gc.execute('git add ' + f1)
        success = gc.execute('git commit -m "Update TIA Map files."')
        if not success:
            return False
        success = gc.execute('git push origin ' + bn)
        if not success:
            return False
    else:
        print 'No Map files were changed, hence no push.'
    # Back to master branch
    success = gc.switch_branch('master')
    if not success:
        return False
    return True

#-------------------------------------------------------------------------------------------
# bsimethod                                     Majd.Uddin    09/2017
#-------------------------------------------------------------------------------------------
def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--covReports", help = "The path at XML reports of Coverage are --covReports=D:\CoverageReports", required = True)
    parser.add_argument("--component", help = "The name of the component you want map for e.g. --component=ECDb. If none is given, it runs for all")
    parser.add_argument("--DLL", help = "The DLL name against which you want map for e.g. --DLL=BeSQLiteB02.dll")
    parser.add_argument("--pushChanges", help = "If specified, will also push Map files.", action='store_true')
    parser.add_argument("--forceAll", help = "Pass this argument to force generate Coverage report for all tests. Otherwise, it will run for changed tests only.", action='store_true')

    args = parser.parse_args()
    covRoot = args.covReports
    comp = args.component
    dll = args.DLL
    comps = []
    if covRoot == None:
        print('\n Script requires to get a folder where Coverage reports are stored. Error.')
        exit(-1)
    if not os.path.exists(covRoot):
        os.mkdir(covRoot)
    if comp == None:
        comps = cmp.AllCompsProper()
        comps.remove('BeHttp')

    else:
        comps.append(comp)

    for component in comps:
        print '***Generating TiaMaps for component: ' + component
        compDir = os.path.join(covRoot, component)
        results = runCoverage(covRoot, component, args.forceAll)
        printResults(results, component) # for logging purpose
        
        if os.path.exists(compDir):
            mapDir = os.path.join(compDir, 'TIAMaps')
            if not os.path.exists(mapDir):
                os.mkdir(mapDir)

            tiaMap = getTiaMapAll(covRoot, component, dll)
            writeMapToFiles(mapDir, tiaMap, component)
            adjustMapFiles(mapDir)
            #copyMapFiles(mapDir, comps)
    if args.pushChanges:
        status = pushMapFiles()
        if not status:
            print('\n Map files were generated but could not be pushed to the server \n')
            exit(-1)
        else:
            print 'Map files pushed successfully.'
    
main()
