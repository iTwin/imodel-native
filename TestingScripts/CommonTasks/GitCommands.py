#--------------------------------------------------------------------------------------
#
#     $Source: CommonTasks/GetSourceFiles.py $
#
#  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
#
#--------------------------------------------------------------------------------------
import os, sys
import subprocess

#-------------------------------------------------------------------------------------------
# bsiclass                                     Majd.Uddin    09/2017
#-------------------------------------------------------------------------------------------
class GitCommand:
    def __init__(self):
        self.res = ""
    def execute(self, cmd):
        self.cmd = cmd        
        print '\nRunning git command: ' + self.cmd
        try:
            result = subprocess.check_output(self.cmd, stderr=subprocess.STDOUT)
        except subprocess.CalledProcessError as e:
            print 'Error for git command: ' + self.cmd + '. The error is: ' + e.output
            return False
        self.res = result
        return True
    def status(self):
        self.stats = []
        success = self.execute('git status --porcelain')
        if not success:
            return False
        else:
            lines = self.res.split('\n')
            for line in lines:
                if len(line.strip()) > 0:
                    self.stats.append(line.strip())
        return True
        
    def switch_branch(self, bn):
        exists = self.branch_exists(bn)
        if exists:
            cmd = 'git checkout ' + bn
        else:
            cmd = 'git checkout -b ' + bn
        success = self.execute(cmd)
        return success

    def branch_exists(self, bn):
        bexist = False
        success = self.execute('git branch')
        if not success:
            return False
        else:
            lines = self.res.split('\n')
            for line in lines:
                for lp in line.split(' '):
                    if bn in lp:
                        bexist = True
        return bexist

    def files_last_commit(self):
        srcRoot = os.path.dirname(self.work_dir)
        success = self.execute('git show --oneline --name-only --pretty=""')
        if not success:
            return []
        else:
            lines = self.res.split('\n')
            for line in lines:
                if len(line) > 1:
                    fullPath = os.path.join(srcRoot, line)
                    if fullPath not in sFiles:
                        sFiles.append(fullPath)
            for f in sFiles:
                if not os.path.exists(f):
                    sFiles.remove(f)
            return sFiles

    def files_incoming(self):
        srcRoot = os.path.dirname(self.work_dir)
        success = self.execute('git fetch && git diff --name-only ..origin')
        if not success:
            return []
        else:
            lines = self.res.split('\n')
            for line in lines:
                if len(line) > 1:
                    fullPath = os.path.join(srcRoot, line)
                    if fullPath not in sFiles:
                        sFiles.append(fullPath)
            for f in sFiles:
                if not os.path.exists(f):
                    sFiles.remove(f)
            return sFiles

    def files_branches(self, sb, tb):
        srcRoot = os.path.dirname(self.work_dir)
        success = self.execute('git show --oneline --name-only --pretty="" ' + sb + ' ' + tb)
        if not success:
            return []
        else:
            lines = self.res.split('\n')
            for line in lines:
                if len(line) > 1:
                    fullPath = os.path.join(srcRoot, line)
                    if fullPath not in sFiles:
                        sFiles.append(fullPath)
            for f in sFiles:
                if not os.path.exists(f):
                    sFiles.remove(f)
            return sFiles
