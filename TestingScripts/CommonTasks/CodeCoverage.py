#---------------------------------------------------------------------------------------------
#  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
#  See LICENSE.md in the repository root for full copyright notice.
#---------------------------------------------------------------------------------------------
import os, sys
import subprocess
import shutil

#Common Scripts to be used by any task
scriptsDir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
commonScriptsDir = os.path.join(scriptsDir, 'CommonTasks')
sys.path.append(commonScriptsDir)

#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
class CodeCoverage:
    def __init__(self):
        srcRoot = os.getenv('SrcRoot')
        outRoot = os.getenv('OutRoot')
        self.covExe = os.path.join(outRoot, 'tools', 'opencppcoverage', 'opencppcoverage.exe')
        mods = []
        mods.append('*iTwin*')
        self.modules = ' --modules=' + mods[0]
        self.sources = ' --sources=' + srcRoot
        self.reportPath = ''
        self.cmd = ''
        self.export = ' --export_type=cobertura:'
        self.exportExt = '.xml'

    def resetCommand(self):
        self.cmd = ''        
        
    def set_ReportPath(self, reportPath):
        self.reportPath = reportPath

    def set_SourcePath(self, sourcePath):
        self.sources = ' --sources=' + sourcePath
    
    def set_SourcePath2(self, sourcePath2):
        self.sources = str(self.sources) + ' --sources=' + sourcePath2

    def set_ExportType(self, type):
        if type.lower() == "html":
            self.export = ' --export_type=html:'
            self.exportExt = '.html'
        if type.lower() == "xml":
            self.export = ' --export_type=cobertura:'
            self.exportExt = '.xml'
           
        
    def covSingleTest(self, testExe, testName, overwrite=True):
        report = os.path.join(self.reportPath, testName + self.exportExt)
        if overwrite:
            self.export = ' --export_type=cobertura:'
            self.export = self.export + report
            self.cmd = self.covExe + self.modules + self.sources + self.export
            self.cmd = self.cmd + ' -- ' + testExe + ' --gtest_filter=' + testName
            return self.runCovCmd()
        else:
            if not os.path.exists(report):
                self.export = ' --export_type=cobertura:'
                self.export = self.export + report
                self.cmd = self.covExe + self.modules + self.sources + self.export
                self.cmd = self.cmd + ' -- ' + testExe + ' --gtest_filter=' + testName
                return self.runCovCmd()
            else:
                print ('Report exists: ' + report)
                return True

    def covTestCase(self, testExe, testName, overwrite=True):
        report = os.path.join(self.reportPath, testName + self.exportExt)
        if overwrite:
            self.export = ' --export_type=cobertura:'
            self.export = self.export + report
            self.cmd = self.covExe + self.modules + self.sources + self.export
            self.cmd = self.cmd + ' -- ' + testExe + ' --gtest_filter=' + testName + '.*'
            return self.runCovCmd()
        else:
            if not os.path.exists(report):
                self.export = ' --export_type=cobertura:'
                self.export = self.export + report
                self.cmd = self.covExe + self.modules + self.sources + self.export
                self.cmd = self.cmd + ' -- ' + testExe + ' --gtest_filter=' + testName + '.*'
                return self.runCovCmd()
            else:
                print ('Report exists: ' + report)
                return True

    def covTestExe(self, testExe):
        report = os.path.join(self.reportPath, os.path.basename(testExe) + self.exportExt)
        self.export = self.export + report
        self.cmd = self.covExe + self.modules + self.sources + self.export
        self.cmd = self.cmd + ' -- ' + testExe + ' --timeout=-1'
        return self.runCovCmd()

    def createZip(self, testExe):
        fileName = os.path.join(os.path.basename(testExe) + self.exportExt)
        report = os.path.join(self.reportPath, fileName)
        if os.path.exists(report):
            shutil.make_archive(report, 'zip', report)
            
    def runCovCmd(self):
        print (self.cmd)
        result = subprocess.call(self.cmd, shell=True)
        if result != 0:
            print ("Coverage tool failed for command: " + str(self.cmd))
            return False
        else:
            return True
    
    def editModules(self, moduleFilter):
        self.modules = moduleFilter
