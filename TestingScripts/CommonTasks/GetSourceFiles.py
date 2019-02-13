#--------------------------------------------------------------------------------------
#
#     $Source: TestImpactAnalysis/GetSourceFiles.py $
#
#  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
#
#--------------------------------------------------------------------------------------
import os, sys
import subprocess

#-------------------------------------------------------------------------------------------
# bsimethod                                     Majd.Uddin    01/2019
#-------------------------------------------------------------------------------------------
def getFilesGit():
    srcRoot = os.path.join(os.getenv('SrcRoot'), 'imodel02')

    # Get file names in last commit
    gitCmd = 'git show --oneline --name-only --pretty=""'
    sFiles = []

    try:
        result = subprocess.check_output(gitCmd, stderr=subprocess.STDOUT)
    except subprocess.CalledProcessError as e:
        print 'Error for git command: ' + gitCmd + '. The error is: ' + e.output
    lines = result.split('\n')
    for line in lines:
        if len(line) > 1:
            fullPath = os.path.join(srcRoot, line)
            if fullPath not in sFiles:
                sFiles.append(fullPath)
    for f in sFiles:
        if not os.path.exists(f):
            sFiles.remove(f)
    return sFiles

#-------------------------------------------------------------------------------------------
# bsimethod                                     Majd.Uddin    01/2019
#-------------------------------------------------------------------------------------------
def getFilesGitBranches(sourceBranch, targetBranch):
    srcRoot = os.path.join(os.getenv('SrcRoot'), 'imodel02')

    # Get file names in last commit
    gitCmd = 'git show --oneline --name-only --pretty="" ' + sourceBranch + ' ' + targetBranch
    sFiles = []

    try:
        result = subprocess.check_output(gitCmd, stderr=subprocess.STDOUT)
    except subprocess.CalledProcessError as e:
        print 'Error for git command: ' + gitCmd + '. The error is: ' + e.output
    lines = result.split('\n')
    for line in lines:
        if len(line) > 1:
            fullPath = os.path.join(srcRoot, line)
            if fullPath not in sFiles:
                sFiles.append(fullPath)
    for f in sFiles:
        if not os.path.exists(f):
            sFiles.remove(f)
    return sFiles

#-------------------------------------------------------------------------------------------
# bsimethod                                     Majd.Uddin    11/2017
#-------------------------------------------------------------------------------------------
def getFilesHg():
    srcRoot = os.getenv('SrcRoot')

    #Get repository name
    hgCmd = 'hg paths default'
    try:
        result = subprocess.check_output(hgCmd, stderr=subprocess.STDOUT)
    except subprocess.CalledProcessError as e:
        if e.output.startswith('not found'):
            print 'No hg repisotry at current path'
        else:
            print 'Unknown error for hg command: ' + hgCmd
        exit(-1)
    repoName = os.path.basename(result.strip())
    repoPath =  os.path.join(srcRoot, repoName)

    sFiles = []

    # First let's add files that are uncommitted
    hgCmd = 'hg stat'
    try:
        result = subprocess.check_output(hgCmd, stderr=subprocess.STDOUT)
    except subprocess.CalledProcessError as e:
        print 'Error for hg command: ' + hgCmd + '. The error is: ' + e.output
    lines = result.split('\n')
    for line in lines:
        if line.startswith('?') or line.startswith('M'):
            sfName = line.strip().split(' ')[1]
            fullPath = os.path.join(repoPath,sfName)
            if fullPath not in sFiles:
                sFiles.append(fullPath)

    # Now let's add files that are committed but not pushed
    hgCmd = 'hg outgoing -v'
    try:
        result = subprocess.check_output(hgCmd, stderr=subprocess.STDOUT)
    except subprocess.CalledProcessError as e:
        if 'no changes found' in e.output:
            print 'No outgoing changes.'
        else:
            print 'Error for hg command: ' + hgCmd + '. The error is: ' + e.output
    lines = result.split('\n')
    lineNo = 0
    for line in lines:
        lineNo = lineNo + 1
        if '[' in line:
            fileList = lines[lineNo]
            files = fileList.strip().split(' ')
            for sfName in files:
                fullPath = os.path.join(repoPath,sfName)
                if fullPath not in sFiles:
                    sFiles.append(fullPath)

    for f in sFiles:
        if not os.path.exists(f):
            sFiles.remove(f)
    return sFiles
