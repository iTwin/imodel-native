#--------------------------------------------------------------------------------------
#
#  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
#
#--------------------------------------------------------------------------------------
import xml.etree.ElementTree as ET
import os, sys
import subprocess
import datetime
import time
import argparse

#Common Scripts to be used by any task
scriptsDir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
commonScriptsDir = os.path.join(scriptsDir, 'CommonTasks')
sys.path.append(commonScriptsDir)

from CodeCoverage import CodeCoverage
import Components as cmp
import FindStream

#-------------------------------------------------------------------------------------------
# Class to manipulate Excel Reporting
# bsiclass                                     Majd.Uddin    01/2018
#-------------------------------------------------------------------------------------------
class ExcelReport:
    def __init__(self, fileName):
        import xlsxwriter
        self.wb = xlsxwriter.Workbook(fileName)
        self.addDefaultFormats()
    def addSheet(self, sheetName):
        return self.wb.add_worksheet(sheetName)
    def getFormats(self):
        return self.formats
    def addDefaultFormats(self):
        self.heading = self.wb.add_format({'color': 'blue',
                             'bold': 1,
                             'size': 14,
                             'align': 'center',
                             'text_wrap': 1})
        self.heading2 = self.wb.add_format({'color': 'green',
                             'bold': 1,
                             'size': 12,
                             'align': 'center',
                             'border': 2 })
        self.heading3 = self.wb.add_format({'color': 'green',
                             'bold': 1,
                             'size': 12,
                             'align': 'center'})
        self.border = self.wb.add_format({'left': 2,
                              'right': 2})
        self.link = self.wb.add_format({'color': 'blue',
                            'underline': True})
        self.percent = self.wb.add_format()
        self.percent.set_num_format('0%')

        self.num = self.wb.add_format()
        self.num.set_num_format('#,##0')
        self.num2 = self.wb.add_format({'top' : 2,
                            'bottom' : 2,
                            'num_format' : '#,##0'})
        self.percent2 = self.wb.add_format({'top' : 2,
                                'bottom' : 2,
                                'num_format' : '0%'})
        self.percent3 = self.wb.add_format({'align' : 'center',
                                'font_size' : 16,
                                'num_format' : '0%',
                                'right' : 2,
                                'left': 2})

        self.percent3.set_align('vcenter')
        self.percent4 = self.wb.add_format({'align' : 'center',
                                'font_size' : 16,
                                'num_format' : '0%',
                                'right' : 2,
                                'left': 2,
                                'bottom': 2})

        self.percent.set_align('vcenter')

        self.high = self.wb.add_format({'align' : 'center',
                            'font_size' : 16,
                            'font_color' : 'blue',
                            'right' : 2,
                            'left' : 2})
        self.high.set_align('vcenter')
        self.leftborder = self.wb.add_format({'left': 2})
        self.bottomborder = self.wb.add_format({'bottom' : 2})
        self.topborder = self.wb.add_format({'top' : 2})
        self.good = self.wb.add_format({'bg_color' : "#C6EFCE"})
        self.bad = self.wb.add_format({'bg_color' : "#FFC7CE"})
        self.neutral = self.wb.add_format({'bg_color' : "#D9D9D9"})

        
    def __del__(self):
        self.wb.close()

#-------------------------------------------------------------------------------------------
# Class to manage OpenCPP coverage for all components
# bsiclass                                     Majd.Uddin    12/2017
#-------------------------------------------------------------------------------------------
class CoverageOpenCPP:
    def __init__(self, comps):
        self.comps = {}
        for comp in comps:
            self.comps.setdefault(comp.lower(), {'methods': {}, 'files': {}, 'lines': {'total':0, 'covered':0}})
        TeamConfigPath = os.path.join(os.getenv('SrcRoot'), 'teamConfig', 'treeConfiguration.xml')
        self.streamName = FindStream.FindStreamDetails(TeamConfigPath)
    def set_ReportPath(self, reportPath):
        self.repoPath = reportPath
    def addComp(self, comp):
        if comp not in self.comps:
            self.comps.setdefault(comp.lower(), {'methods': {}, 'files': {}, 'lines': {'total':0, 'covered':0}})
    #-------------------------------------------------------------------------------------------
    # bsimethod                                     Majd.Uddin    12/2017
    #-------------------------------------------------------------------------------------------
    def getMethodName(self, comp, file, line):
        for c in self.comps:
            if c.lower() == comp.lower():
                methods = self.comps[c]['methods']
                for m1 in methods:
                    if file.lower() == methods[m1]['fileName'].lower() and line == methods[m1]['lineNo']:
                        return m1
        return None

    #-------------------------------------------------------------------------------------------
    # bsimethod                                     Majd.Uddin    12/2017
    #-------------------------------------------------------------------------------------------
    def methodsFromPDBs(self):
        winSdk = os.getenv('Win10SdkDir')
        if winSdk is None:
            print 'Please set Win10SkDir environment variable for dbh.exe'
            exit(-1)
        else:
            dbhPath = os.path.join(winSdk, 'Debuggers', 'x86', 'dbh.exe')
            
        if not os.path.exists(self.repoPath):
            os.mkdir(self.repoPath)

        #Gather method details for all components
        for comp in self.comps:
            methods = self.comps[comp]['methods']
            pdbFile = cmp.PdbPathForComp(comp)
            dumpFile = os.path.join(self.repoPath, comp+'_dump.txt')
            if os.path.exists(pdbFile):
                if not os.path.exists(dumpFile):
                    print 'Generating dump for: ' + comp+'.pdb'            
                    cmd = '"' + dbhPath + '" ' + pdbFile + ' dump >' + dumpFile
                    print cmd
                    result = subprocess.call(cmd, shell=True)
                    if result is not 0:
                        print 'Dump command failed: ' + cmd
                        exit(-1)
                print 'Processing dump file: ' + comp+'_dump.txt'
                with open(dumpFile, 'r') as f:
                    lines = f.readlines()
                    for line in lines:
                        if len(line) > 1:
                            lineP = line.split('\t')
                            method = lineP[1]
                            if 'bvector<' in method.lower() or ('<' in method.lower() and '>' in method.lower()) or '::operator' in method.lower():
                                continue
                            fullFileName = lineP[len(lineP) - 3]
                                          
                            fileName = os.path.basename(fullFileName)
                            fName, fExt = os.path.splitext(fileName)
                            lineNo = int(lineP[len(lineP) - 2])
                            skipMethod = False
                            baseRoot = os.path.dirname(os.path.dirname(os.getenv('SrcRoot')))
                            if fExt == ".h" and baseRoot.lower() in fullFileName.lower():
                                with open(fullFileName, 'r') as f2:
                                    lines2 = f2.readlines()
                                    fullLine = lines2[lineNo - 1].strip()
                                    if not '_EXPORT' in fullLine:
                                        skipMethod = True
                            if method.lower().startswith('bentleyb0200::') and method not in methods and not skipMethod:
                                methods.setdefault(method, {})
                                methods[method]['fileName'] = fileName
                                methods[method]['lineNo'] = lineNo
                                methods[method]['covered'] = False
            else:
                print 'PDB file could not be located: ' + pdbFile
            
    #-------------------------------------------------------------------------------------------
    # bsimethod                                     Majd.Uddin    12/2017
    #-------------------------------------------------------------------------------------------
    def runCoverage(self, methods, reportType=None):
        covSuccess = False
        logPath = ''
        #Attempt to run Coverage against all possible exes
        for comp in self.comps:
            print 'Try to run Coverage for: ' + comp
            testExe = cmp.ExePathForComp(comp)
            if reportType: #local report, always generate it
                cov = CodeCoverage()
                cov.set_ReportPath(self.repoPath)
                # Calling filter for Dll's
                if comp.lower() == 'mstnbridgetests':
                    moduleFilter = '*M02*'  #"*Bridge*M02.dll"
                    cov.editModules(moduleFilter)

                if 'Bridges\\Mstn' in str(cmp.RepoPathForComp(comp)):
                    logPath = str(cmp.RepoPathForComp(comp))
                    cov.set_SourcePath(str(cmp.RepoPathForComp(comp)).replace('Bridges\\Mstn','iModelBridgeCore')) #Mstn is using iModelBridgeCore
                    cov.set_SourcePath2(str(cmp.RepoPathForComp(comp)).replace('Bridges\\Mstn','iModelCore')) #Mstn is also using iModelCore
                elif 'Bridges\\Dwg' in str(cmp.RepoPathForComp(comp)):
                    cov.set_SourcePath(str(cmp.RepoPathForComp(comp)).replace('Bridges\\Dwg','iModelBridgeCore')) #Mstn is using iModelBridgeCore
                else:
                    cov.set_SourcePath(cmp.RepoPathForComp(comp))
                    logPath = str(cmp.RepoPathForComp(comp))
                
                cov.set_ExportType('html')
                if cov.covTestExe(testExe):  # if a single run completes, mark it success
                    covSuccess = True
                if methods:
                    self.methodsHtml()
                cov.createZip(testExe)
                cov.deleteLogs(logPath) #delete log file
            else:
                if not os.path.exists(os.path.join(self.repoPath, os.path.basename(testExe)+'.xml')):
                    cov = CodeCoverage()
                    cov.set_ReportPath(self.repoPath)
                    if 'Bridges\\Mstn' in str(cmp.RepoPathForComp(comp)):
                        cov.set_SourcePath(str(cmp.RepoPathForComp(comp)).replace('Bridges\\Mstn','iModelBridgeCore')) #Mstn is using iModelBridgeCore
                        cov.set_SourcePath2(str(cmp.RepoPathForComp(comp)).replace('Bridges\\Mstn','iModelCore')) #Mstn is also using iModelCore
                        logPath = str(cmp.RepoPathForComp(comp))
                    elif 'Bridges\\Dwg' in str(cmp.RepoPathForComp(comp)):
                        cov.set_SourcePath(str(cmp.RepoPathForComp(comp)).replace('Bridges\\Dwg','iModelBridgeCore')) #Mstn is using iModelBridgeCore
                        logPath = str(cmp.RepoPathForComp(comp))
                    else:
                        cov.set_SourcePath(cmp.RepoPathForComp(comp))
                        logPath = str(cmp.RepoPathForComp(comp))
                    if cov.covTestExe(testExe):  # if a single run completes, mark it success
                        covSuccess = True
                else: #XML output is already there.
                    covSuccess = True
                cov.deleteLogs(logPath)
        return covSuccess

    #-------------------------------------------------------------------------------------------
    # bsimethod                                     Majd.Uddin    12/2017
    #-------------------------------------------------------------------------------------------
    def coverageAll(self):
        for f in os.listdir(self.repoPath):
            fileName, fileExt = os.path.splitext(f)
            if fileExt == ".xml":
                print 'Processing file: ' + f
                tree = ET.parse(os.path.join(self.repoPath,f))
                root = tree.getroot()
                packages = root.find('packages')
                for pack in packages:
                    comp = cmp.CompForDll(os.path.basename(pack.attrib['name']))
                    if comp is not None:#Ignore DLLs not needed
                        comp = comp.lower()
                        print 'Checking methods in component: ' + comp
                        self.addComp(comp)
                        totalLines = 0
                        coveredLines = 0
                        classes = pack.find('classes')
                        for class1 in classes:
                            sfName = os.path.basename(class1.attrib['filename'])
                            if sfName not in self.comps[comp]['files']:
                                self.comps[comp]['files'].setdefault(sfName, {'covered':False})
                            lineRate = float(class1.attrib['line-rate'])
                            if lineRate > 0.0:
                                self.comps[comp]['files'][sfName]['covered'] = True
                            lines = class1.find('lines')
                            totalLines = totalLines + len(lines)
                            for line in lines:
                                lineNo = int(line.attrib['number'])
                                if int(line.attrib['hits']) > 0:
                                    coveredLines = coveredLines + 1
                                    method = self.getMethodName(comp, sfName, lineNo)                       
                                    if method is not None:
                                        self.comps[comp]['methods'][method]['covered'] = True
                        if totalLines > self.comps[comp]['lines']['total']:
                            self.comps[comp]['lines']['total'] = totalLines
                        if coveredLines > self.comps[comp]['lines']['covered']:
                            self.comps[comp]['lines']['covered'] = coveredLines

                            
    #-------------------------------------------------------------------------------------------
    # bsimethod                                     Majd.Uddin    12/2017
    #-------------------------------------------------------------------------------------------
    def printSummary(self):
        print '\n*** Results \n'
        for c in self.comps:
            print '\nComponent: ' + c
            methods = self.comps[c]['methods']
            print 'Total Methods    : ' + str(len(methods))
            covered = 0
            for m in methods:
                if methods[m]['covered']:
                   covered = covered + 1

            print 'Covered Methods  : ' + str(covered)
            print 'Uncovered Methods: ' + str(len(methods) - covered)

            files = self.comps[c]['files']
            print 'Total Files      : ' + str(len(files))
            covered = 0
            for f in files:
                if files[f]['covered']:
                   covered = covered + 1

            print 'Covered Files    : ' + str(covered)
            print 'Uncovered Files  : ' + str(len(files) - covered)

            print 'Total Lines      : ' + str(self.comps[c]['lines']['total'])
            print 'Covered Lines    : ' + str(self.comps[c]['lines']['covered'])
    #-------------------------------------------------------------------------------------------
    # bsimethod                                     Majd.Uddin    12/2017
    #-------------------------------------------------------------------------------------------
    def addSummary(self, rp, wsSumm):
        wsSumm.set_column(0,1,22)
        wsSumm.set_column(3,6,8)
        wsSumm.set_column(8,11,8)
        wsSumm.set_column(13,16,8)
        wsSumm.set_column(2,2,0.2)
        wsSumm.set_column(7,7,0.2)
        wsSumm.set_column(12,12,0.2)

        wsSumm.merge_range('A1:R1', "Code Coverage Report", rp.heading)
        wsSumm.merge_range('B2:D2', "Stream: " + self.streamName, rp.heading3)
        wsSumm.merge_range('N2:P2', "Dated: " + str(datetime.date.today()), rp.heading3)
        wsSumm.merge_range('A4:A5', "Component", rp.heading2)
        wsSumm.merge_range('B4:B5', "DLL", rp.heading2)
        wsSumm.write(4,2, "", rp.border)
        wsSumm.merge_range ('D4:G4', 'Files', rp.heading2)
        wsSumm.write(4,3, "Total", rp.bottomborder)
        wsSumm.write(4,4, "Visited", rp.bottomborder)
        wsSumm.write(4,5, "% Visited", rp.bottomborder)
        wsSumm.write(4,6, "UnVisited", rp.bottomborder)
        wsSumm.write(4,7, "", rp.border)
        wsSumm.merge_range ('I4:L4', 'Lines', rp.heading2)
        wsSumm.write(4,8, "Total", rp.bottomborder)
        wsSumm.write(4,9, "Visited", rp.bottomborder)
        wsSumm.write(4,10, "% Visited", rp.bottomborder)
        wsSumm.write(4,11, "UnVisited", rp.bottomborder)
        wsSumm.write(4,12, "", rp.border)
        wsSumm.merge_range ('N4:R4', 'Functions', rp.heading2)
        wsSumm.write(4,13, "Total", rp.bottomborder)
        wsSumm.write(4,14, "Visited", rp.bottomborder)
        wsSumm.write(4,15, "% Visited", rp.bottomborder)
        wsSumm.write(4,16, "UnVisited", rp.bottomborder)
        wsSumm.write(4,17, "Ignored", rp.bottomborder)
        wsSumm.write(4,18, "", rp.leftborder)

        wsSumm.write(22,0, "Notes")
        wsSumm.write(23,0, "1. Rows are color coded on the basis of % Function Visited:")
        wsSumm.write(24,0, "Great > 70%", rp.good)
        wsSumm.write(24,1, "Basic < 50%", rp.bad)
        wsSumm.merge_range('D25:E25', "Moderate 50-70%", rp.neutral)
        wsSumm.write(25,0, "2. The report for each Component is generated by applying three filters:")
        wsSumm.write(26,1, "a. Only selected DLL is checked")
        wsSumm.write(27,1, "b. Only files in it's directory are checked e.g. %SrcRoot%ECDb for ECDb")
        wsSumm.write(28,1, "c. Only functions that have BentleyB0200 namespace are checked")
        wsSumm.write(29,0, "3. Click on each Component name to see list of Unvisited Functions and Files")
        wsSumm.write(30,0, "4. To go back to high level summary:")
        wsSumm.write_url(32,1, 'internal:Summary!A1', rp.link, "Function Coverage Summary")

        wsSumm.conditional_format('A6:R19', {'type':     'formula',
                                            'criteria': '=$P6>0.7',
                                            'format':   rp.good})
        wsSumm.conditional_format('A6:R19', {'type':     'formula',
                                            'criteria': '=AND(($P6<=0.7), ($P6>=0.5))',
                                            'format':   rp.neutral})
        wsSumm.conditional_format('A6:R19', {'type':     'formula',
                                            'criteria': '=$P6<0.5',
                                            'format':   rp.bad})
        return wsSumm
    #-------------------------------------------------------------------------------------------
    # bsimethod                                     Majd.Uddin    12/2017
    #-------------------------------------------------------------------------------------------
    def addHighSummary(self, rp, wsHigh, FuncCov=True):
        wsHigh.hide_gridlines(2)
        wsHigh.merge_range('D2:F2', "Code Coverage Report", rp.heading)
        if FuncCov:
            wsHigh.merge_range('D3:F3', "Function Coverage Summary", rp.heading)
        else:
            wsHigh.merge_range('D3:F3', "Line Coverage Summary", rp.heading)
        wsHigh.write('D5', "Stream: " + self.streamName, rp.heading3)
        wsHigh.write('F5', "Dated: " + str(datetime.date.today()), rp.heading3)

        total = len(self.comps)
        perRow = 4
        rows = total/perRow
        if total%perRow > 0:
            rows = rows + 1

        startRow = 6
        endRow = startRow + (2*rows)
        startCol = 3

        rowHeight = 42
        wsHigh.set_row(1, 15)
        wsHigh.set_row(2, 15)
        for i in range(startRow, endRow):
            wsHigh.set_row(i, rowHeight)

        wsHigh.set_column(startCol, startCol+perRow-1,18)
        wsHigh.set_column(startCol+perRow,startCol+perRow, 5)
        wsHigh.set_column(startCol+perRow+1,startCol+perRow+1, 18)

        wsHigh.write(startRow,startCol+perRow+1, "Legend", rp.heading3)
        wsHigh.write(startRow+1,startCol+perRow+1, "Great     > 70%", rp.good)
        wsHigh.write(startRow+2,startCol+perRow+1, "Moderate  50-70%", rp.neutral)
        wsHigh.write(startRow+3,startCol+perRow+1, "Basic     < 50%", rp.bad)


        for i in range(0, perRow):
            wsHigh.write(startRow-1,startCol+i, "", rp.bottomborder)

        counter = 0
        compStart = 6
        for i in range(0, 2*rows, 2):
            for k in range(0, perRow):
                if counter < total:
                    wsHigh.write(startRow+i,startCol+k, '=Components!A'+str(compStart+counter), rp.high)
                    if FuncCov:
                        wsHigh.write(startRow+1+i,startCol+k, '=Components!P'+str(compStart+counter), rp.percent4)
                    else:
                        wsHigh.write(startRow+1+i,startCol+k, '=Components!K'+str(compStart+counter), rp.percent4)                        
                    counter = counter  + 1
                else:
                    wsHigh.write(startRow+i,startCol+k, "", rp.high)
                    wsHigh.write(startRow+1+i,startCol+k, "", rp.percent4)
                    
        for i in range(0, perRow):
            wsHigh.write(endRow,startCol+i, "", rp.topborder)


        wsHigh.write(endRow+2,startCol, "To see detailed report, go to Component Summary:")
        wsHigh.write_url(endRow+3,startCol+1, 'internal:Components!A1', rp.link, "Component Summary")


        cols = {1:'A', 2:'B', 3:'C', 4:'D', 5:'E', 6:'F', 7:'G', 8:'H', 9:'I', 10:'J'}

        for i in range(0, 2*rows, 2):
            cellToWatch = cols[startCol+1]+'$'+str(startRow+i+2)
            wsHigh.conditional_format(startRow+i,startCol,startRow+i+1,startCol+perRow-1,
                                                {'type':     'formula',
                                                'criteria': '=AND(('+ cellToWatch +'>0.7), NOT(ISBLANK(' + cellToWatch + ')))',
                                                'format':   rp.good})
            wsHigh.conditional_format(startRow+i,startCol,startRow+i+1,startCol+perRow-1,
                                                {'type':     'formula',
                                                'criteria': '=AND((' + cellToWatch +'<=0.7), (' + cellToWatch + '>=0.5), NOT(ISBLANK(' + cellToWatch + ')))',
                                                'format':   rp.neutral})
            wsHigh.conditional_format(startRow+i,startCol,startRow+i+1,startCol+perRow-1,
                                                {'type':     'formula',
                                                'criteria': '=AND(('+ cellToWatch +'<0.5), NOT(ISBLANK(' + cellToWatch + ')))',
                                                'format':   rp.bad})

        if FuncCov:
            rp.wb.define_name('Chart', '='+wsHigh.get_name()+'!$D$2:$'+cols[startCol+perRow+2]+'$'+str(endRow))
        else:
            rp.wb.define_name('ChartLine', '='+wsHigh.get_name()+'!$D$2:$'+cols[startCol+perRow+2]+'$'+str(endRow))



    #-------------------------------------------------------------------------------------------
    # bsimethod                                     Majd.Uddin    12/2017
    #-------------------------------------------------------------------------------------------
    def writeToExcel(self):
        print '\n*** writing to Excel \n'
        self.xlsReport = os.path.join(self.repoPath, self.streamName+'_CovSummary.xlsx')
        rp = ExcelReport(self.xlsReport)
        wsHigh = rp.addSheet('FunctionSummary')
        wsHigh2 = rp.addSheet('LineSummary')
        wsSumm = rp.addSheet('Components')
        wsSumm = self.addSummary(rp, wsSumm)
        startRow = 5
        startCol = 0
        
        i = 0

        TotalFiles = TotalMethods = TotalLines = CoveredFiles = CoveredMethods = CoveredLines = 0
        for comp in self.comps:
            methods = self.comps[comp]['methods']
            wsComp = rp.addSheet(comp)
            wsComp.write_url(0, 0, 'internal:Components!A1', rp.link, "Home")
            wsComp.merge_range('C1:N1', cmp.DllForComp(comp), rp.heading)
            wsComp.merge_range('B2:E2', "Unvisited Functions", rp.heading)
            wsComp.merge_range('J2:L2', "Unvisited Files", rp.heading)
            wsComp.merge_range('S2:V2', "Ignored Functions", rp.heading)
            wsComp.freeze_panes(2, 0)

            compName = cmp.GetName(comp)
            if compName.startswith('Construction'):
                compName = 'Const.Plan.'
            wsSumm.write_url(startRow + i, startCol, 'internal:'+comp+'!A1', rp.link, compName)
            wsSumm.write(startRow + i, startCol + 1, cmp.DllForComp(comp))
            wsSumm.write(startRow + i, startCol + 2, "", rp.border)

            #Write file summary and unvisited files
            files = self.comps[comp]['files']
            total = len(files)
            TotalFiles = TotalFiles + total
            wsSumm.write(startRow + i, startCol + 3, total, rp.num)
            covered = 0
            sRow = 2
            k = 0
            for f in files:
                if files[f]['covered']:
                    covered = covered + 1
                else:
                    wsComp.write(sRow + k, 9, k+1)
                    wsComp.write(sRow + k, 10, f)
                    k = k + 1
            wsSumm.write(startRow + i, startCol + 4, covered, rp.num)
            CoveredFiles = CoveredFiles + covered
            if float(total) > 0.0:
                wsSumm.write(startRow + i, startCol + 5, (float(covered) / float(total)), rp.percent)
            else:
                wsSumm.write(startRow + i, startCol + 5, 0.0, rp.percent)

            wsSumm.write(startRow + i, startCol + 6, total - covered, rp.num)
            wsSumm.write(startRow + i, startCol + 7, "", rp.border)

            #Write line summary
            total = self.comps[comp]['lines']['total']
            TotalLines = TotalLines + total
            covered = self.comps[comp]['lines']['covered']
            CoveredLines = CoveredLines + covered
            wsSumm.write(startRow + i, startCol + 8, total, rp.num)
            wsSumm.write(startRow + i, startCol + 9, covered, rp.num)
            if float(total) > 0.0:
                wsSumm.write(startRow + i, startCol + 10, (float(covered) / float(total)), rp.percent)
            else:
                wsSumm.write(startRow + i, startCol + 10, 0.0, rp.percent)

            wsSumm.write(startRow + i, startCol + 11, total - covered, rp.num)
            wsSumm.write(startRow + i, startCol + 12, "", rp.border)            

            #Write Method Summary and unvisited methods
            total =  len(methods)
            TotalMethods = TotalMethods + total
            covered = 0
            sRow = 2
            j = 0
            for m in methods:
                if methods[m]['covered']:
                    covered = covered + 1
                else:
                    wsComp.write(sRow + j, 0, j+1)
                    wsComp.write(sRow + j, 1,  m)
                    j = j + 1
            wsSumm.write(startRow + i, startCol + 13, total, rp.num)
            wsSumm.write(startRow + i, startCol + 14, covered, rp.num)
            CoveredMethods = CoveredMethods + covered
            if float(total) > 0.0:
                wsSumm.write(startRow + i, startCol + 15, (float(covered) / float(total)), rp.percent)
            else:
                wsSumm.write(startRow + i, startCol + 15, 0.0, rp.percent)
            wsSumm.write(startRow + i, startCol + 16, total - covered, rp.num)
            wsSumm.write(startRow + i, startCol + 18, "", rp.leftborder)

            i = i + 1

        #write totals
        endRow = startRow + len(self.comps)
        wsSumm.merge_range(endRow, 0, endRow, 1, "Totals", rp.heading2)
        wsSumm.write(endRow, startCol + 2, "", rp.border)
        wsSumm.write(endRow, startCol + 3, TotalFiles, rp.num2)
        wsSumm.write(startRow + len(self.comps), startCol + 4, CoveredFiles, rp.num2)
        if float(TotalFiles) > 0.0:
            wsSumm.write(endRow, startCol + 5, (float(CoveredFiles) / float(TotalFiles)), rp.percent2)
        else:
            wsSumm.write(sendRow, startCol + 5, 0.0, rp.percent2)
        wsSumm.write(endRow, startCol + 6, TotalFiles - CoveredFiles, rp.num2)

        wsSumm.write(endRow, startCol + 7, "", rp.border)
        wsSumm.write(endRow, startCol + 8, TotalLines , rp.num2)
        wsSumm.write(endRow, startCol + 9, CoveredLines, rp.num2)
        if float(TotalFiles) > 0.0:
            wsSumm.write(endRow, startCol + 10, (float(CoveredLines) / float(TotalLines)), rp.percent2)
        else:
            wsSumm.write(endRow, startCol + 10, 0.0, rp.percent2)
        wsSumm.write(startRow + i, startCol + 11, TotalLines - CoveredLines, rp.num2)

        wsSumm.write(endRow, startCol + 12, "", rp.border)
        wsSumm.write(endRow, startCol + 13, TotalMethods, rp.num2)
        wsSumm.write(endRow, startCol + 14, CoveredMethods, rp.num2)
        if float(TotalFiles) > 0.0:
            wsSumm.write(endRow, startCol + 15, (float(CoveredMethods) / float(TotalMethods)), rp.percent2)
        else:
            wsSumm.write(endRow, startCol + 15, 0.0, rp.percent2)
        wsSumm.write(endRow, startCol + 16, TotalMethods - CoveredMethods, rp.num2)
        wsSumm.write(endRow, startCol + 17, "", rp.num2)        
        wsSumm.write(endRow, startCol + 18, "", rp.leftborder)

        #Now add high level summary
        self.addHighSummary(rp, wsHigh)
        self.addHighSummary(rp, wsHigh2, False)
    #-------------------------------------------------------------------------------------------
    # bsimethod                                     Majd.Uddin    05/2018
    #-------------------------------------------------------------------------------------------
    def methodsHtml(self):
        
        print '\n*** writing methods to html \n'
        self.runCoverage(None)
        self.methodsFromPDBs()
        self.coverageAll()

        for comp in self.comps:
            methods = self.comps[comp]['methods']
            compName = cmp.GetName(comp)
            exeName = os.path.basename(cmp.ExePathForComp(comp))
            moduleFile = os.path.join(self.repoPath, exeName +'.html', 'Modules', cmp.DllForComp(comp)[:-4]+'_methods.html')
            html_file = open(moduleFile,"w")
            html_file.write('<html><body>')
            html_file.write('<h1>')
            html_file.write(exeName)
            html_file.write('</h1>')
            html_file.write('<table border="1">')
            html_file.write('<tr><th>')
            html_file.write('Method Name')
            html_file.write('</th><th>')
            html_file.write('File Name')
            html_file.write('</th></tr>')

            total = len(methods)
            covered = 0
            f = open(os.path.join(self.repoPath, exeName+'.html', 'methods.html'),"w")
            for m in methods:
                if methods[m]['covered']:
                    html_file.write('<tr bgcolor="#dfd"><td>')
                    html_file.write (m)
                    html_file.write('</td><td><a href="')
                    html_file.write(os.path.join(cmp.DllForComp(comp)[:-4], methods[m]['fileName']+'.html'))
                    html_file.write('" /a>')
                    html_file.write(methods[m]['fileName'])
                    html_file.write('</td><tr/>')                    
                    covered = covered + 1
                else:
                    html_file.write('<tr bgcolor="#fdd"><td>')
                    html_file.write (m)
                    html_file.write('</td><td><a href="')
                    html_file.write(os.path.join(cmp.DllForComp(comp)[:-4], methods[m]['fileName']+'.html'))
                    html_file.write('" /a>')
                    html_file.write(methods[m]['fileName'])
                    html_file.write('</td><tr/>')                    
            
            html_file.write('</body></html>')
            html_file.close()
            uncovered = total - covered
            covPercent = int(round((float(covered) / float(total)) * 100.0))
            uncovPercent = int(round((float(uncovered) / float(total)) * 100.0))
            message = """<!doctype html>
            <html xmlns="http://www.w3.org/1999/xhtml">
            <head>
                <meta charset="utf-8"/>
                <title>"""
            message = message + exeName
            message = message + """
                </title>
                <link rel="stylesheet" type="text/css" href="third-party/css/style.css"/>
                <script src="third-party/RGraph/libraries/RGraph.common.core.js" type="text/javascript"></script>
                <script src="third-party/RGraph/libraries/RGraph.common.dynamic.js" type="text/javascript"></script>
                <script src="third-party/RGraph/libraries/RGraph.common.tooltips.js" type="text/javascript"></script>
                <script src="third-party/RGraph/libraries/RGraph.pie.js" type="text/javascript"></script>
                <script src="third-party/JQuery/jquery-1.11.1.min.js"></script>
            </head>
            <body>
            <script type="text/javascript">
                $(document).ready(function ()
                {
                    
                    new RGraph.Pie('pi_"""
            message = message + cmp.DllForComp(comp)
            message = message + """', ["""
            message = message + str(covered)
            message = message + """, """
            message = message + str((len(methods) - covered))
            message = message + """])
                        .set('labels', ['Cover"""
            message = message + str(covPercent)
            message = message + """%','Uncover """
            message = message + str(uncovPercent)
            message = message + """%'])
                        .set('colors', ['rgb(0,255,0)','rgb(255,0,0)'])
                        .set('linewidth', 1)
                        .set('shadow', true)
                        .set('labels.sticks', true)
                        .set('labels.sticks.length', 10)
                        .draw();
            
              })
            </script>    
            <h1>"""
            message = message + exeName
            message = message + """
                    </h1>
                    <h4>
                    
                    </h4>
                    <table id="rounded-corner">
                        <thead>
                            <tr>
                                <th scope="col" class="rounded-left">Coverage</th>
                                <th scope="col">Total Methods</th>
                                <th scope="col" class="rounded-right">Items</th>
                            </tr>
                        </thead> 
                        <tbody>
                            
                            <tr>
                                <td>
                        <canvas id="pi_"""
            message = message + cmp.DllForComp(comp)
            message = message + """" width="280" height="100">Cover"""
            message = message + str(covPercent)
            message = message + """%</canvas>
                                </td>            
                                <td>"""
            message = message + str(total)
            message = message + """
                                </td>
                                <td>
                                <a href = "
                                """
            message = message + moduleFile
            message = message + """ " />"""
            message = message + cmp.DllForComp(comp)
            message = message + """
                              </td>
                            </tr>        
                            
             
                        </tbody>
                        <tfoot>
                            <tr>
                            <td colspan="2" class="rounded-foot-left"></td>
                            <td class="rounded-foot-right"></td>
                            </tr>
                        </tfoot>            
                    </table>
                </body>
                </html>            

                """

            f.write(message)
            f.close()

    #-------------------------------------------------------------------------------------------
    # bsimethod                                     Majd.Uddin    01/2018
    #-------------------------------------------------------------------------------------------
    def uploadResults(self):
        import SharePointData
        print '\n*** uploading to SharePoint \n'
        reportName = os.path.basename(self.xlsReport)
        copyPath = os.path.join(self.repoPath, 'ServerCopy')
        if not os.path.exists(copyPath):
            os.mkdir(copyPath)
        SharePointData.downloadFile('Testing Reports', reportName , copyPath)
        archFile = os.path.join(copyPath, time.strftime('%Y-%m-%d')  + '_' + reportName)
        os.rename(os.path.join(copyPath, reportName), archFile)
        fp1 = SharePointData.uploadFile('Testing Reports\\Archive\\CodeCoverage\\'+ self.streamName, archFile)
        print 'Old report archived at: ' + fp1
        
        fp = SharePointData.uploadFile('Testing Reports', self.xlsReport)
        print 'Report uploaded to: ' + fp

#-------------------------------------------------------------------------------------------
# bsimethod                                     Majd.Uddin    12/2017
#-------------------------------------------------------------------------------------------
def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--reportPath", help="Path to store reports. If not specified, defaults to LogFiles\\CodeCoverage")
    parser.add_argument("--component", help = "The name of the component you want Code Coverage for e.g. --component=ECDb. If none is given, it runs for all")
    parser.add_argument('--uploadResults', action='store_true', dest='uploadResults', help='If specified, will upload results to SharePoint')
    parser.add_argument('--localReport', action='store_true', dest='localReport', help='If specified, generates an HTML report on the box')
    parser.add_argument('--excelReport', action='store_true', dest='excelReport', help='If specified, generates an Excel report with color coding')
    parser.add_argument('--methods', action='store_true', dest='methods', help='If specified, generates an Html report for methods')    
    args = parser.parse_args()
  
    comps = []
    if args.component == None:
        comps = cmp.AllComps()
    else:
        if 'LICENSING-GTESTUNIT' in str(args.component).upper(): #if product name is sent as component name (Special Case for licensing product)
            args.component = str(args.component).upper().replace('-GTESTUNIT','')
        elif '-GTEST' in str(args.component).upper(): #if product name is sent as component name
            args.component = str(args.component).upper().replace('-GTEST','')
        comps.append(args.component)
        print "Component Name: " + str(args.component)

    if not args.reportPath:
        repoPath = os.path.join(os.getenv('OutRoot'), 'winx64', 'LogFiles', 'CodeCoverage')
    else:
        repoPath = args.reportPath

    reportType = None
    if args.localReport:
        reportType = 'html'
     
    cov = CoverageOpenCPP(comps)
    cov.set_ReportPath(repoPath)
    if cov.runCoverage(args.methods, reportType):
        if args.excelReport: #For global report, we need methods and Excel report
            cov.methodsFromPDBs()
            cov.coverageAll()
            cov.writeToExcel()
        if args.uploadResults:
            cov.uploadResults()
        print '\nReports are at: ' + repoPath + '\n'
    else:
        print '\n\nError running Code Coverage. Please ensure that you have OpenCPP Coverage tool in your source\n\n'
        exit(-1)

main()
