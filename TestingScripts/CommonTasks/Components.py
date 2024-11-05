#---------------------------------------------------------------------------------------------
#  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
#  See COPYRIGHT.md in the repository root for full copyright notice.
#---------------------------------------------------------------------------------------------
import os
import json

currFilePath = os.path.dirname(os.path.abspath(__file__))
f = open(os.path.join(currFilePath,"Components.json"))
Components = json.load(f)
f.close()

#-------------------------------------------------------------------------------------------
# helper methods to get info for each Component
# bsimethod
#-------------------------------------------------------------------------------------------
def CompForDll(dllToFind):
    for comp in Components:
        if Components[comp]['dll'].lower() == dllToFind.lower():
            return comp
def DllForComp(compToFind):
    for comp in Components:
        if comp.lower() == compToFind.lower():
            return Components[comp]['dll']
def ExeForComp(compToFind):
    for comp in Components:
        if comp.lower() == compToFind.lower():
            return Components[comp]['exe']
def CompForExe(exeToFind):
    for comp in Components:
        if Components[comp]['exe'].lower() == exeToFind.lower():
            return comp
def IgnoreForComp(compToFind):
    for comp in Components:
        if comp.lower() == compToFind.lower():
            return Components[comp]['ignore']

def PdbForComp(compToFind):
    for comp in Components:
        if comp.lower() == compToFind.lower():
            if 'pdb' in Components[comp].keys():
                return Components[comp]['pdb']
            else:
                return None

def RepoForComp(compToFind):
    for comp in Components:
        if compToFind.lower() in Components[comp]['product'].lower():
            if 'repo' in Components[comp].keys():
                repo = Components[comp]['repo']
                repoPath = os.getenv('SrcRoot')
                for path in repo:
                    repoPath = os.path.join(repoPath, path)
                return repoPath
            else:
                repoPath = os.path.join(os.getenv('SrcRoot'),'imodel-native', 'iModelCore', compToFind)
                return repoPath
    
def LogPathForComp(compToFind):
    for comp in Components:
        if comp.lower() == compToFind.lower():
            exeName = ExeForComp(comp)
            if exeName is None:
                return None
            base_path = os.path.join(os.getenv('OutRoot'), 'Winx64', 'build', 'RunGTest')
            log_path = base_path
            if 'special_path' in Components[comp].keys():
                for path in Components[comp]['special_path']:
                    log_path = os.path.join(log_path, path)
            else:
                log_path = os.path.join(log_path, comp+"-GTest")
            log_path = os.path.join(log_path, exeName, 'run', 'logs', 'test.log')
            return log_path

def ExePathForComp(compToFind):
    for comp in Components:
        if compToFind.lower() in Components[comp]['product'].lower():
            exeName = ExeForComp(comp)
            if exeName is None:
                return None
            base_path = os.path.join('%OutRoot%Winx64', 'Product')
            exe_path = base_path
            if 'special_path' in Components[comp].keys():
                for path in Components[comp]['special_path']:
                    exe_path = os.path.join(exe_path, path)
            else:
                exe_path = os.path.join(exe_path, comp+"-GTest")
            exe_path = os.path.join(exe_path, exeName+'.exe')
            return exe_path

def IgnorePathForComp(compToFind):
    ignore = IgnoreForComp(compToFind)
    if ignore is None:
        return None
    ignorePath = os.path.join(os.getenv('SrcRoot'), 'imodel-native')
    for path in ignore:
        ignorePath = os.path.join(ignorePath, path)
    ignorePath = os.path.join(ignorePath, 'ignore_list.txt')
    return ignorePath

def PdbPathForComp(compToFind):
    pdb = PdbForComp(compToFind)
    pdbRoot = os.path.join(os.getenv('OutRoot'), 'Winx64', 'build')
    dll = DllForComp(compToFind)
    if dll == 'iTwinECObjects.dll':
        pdbName = 'ECObjects'
    if dll == 'iTwinECPresentation.dll':
       pdbName = 'objects'
    if dll == 'iTwinGeom.dll':
       pdbName = 'BentleyGeom'
    if dll == 'iTwinGeoCoord.dll':
       pdbName = 'NonPublishedGeoCoord'
    else:
        pdbName = dll[:-4] #omit .dll
        
    if pdb is None:
        pdbPath = os.path.join(pdbRoot, compToFind, pdbName+'.pdb')
    else:
        pdbPath = pdbRoot
        for path in pdb:
            pdbPath = os.path.join(pdbPath, path)
        pdbPath = os.path.join(pdbPath, pdbName+'.pdb')
    return pdbPath

def RepoPathForComp(compToFind):
    repoPath = RepoForComp(compToFind)
    srcRoot = os.getenv('SrcRoot')
    if repoPath is None:
        repoPath = os.path.join(srcRoot,'imodel-native', 'iModelCore' ,compToFind)
    return repoPath

def CompForRepo(path):
    pathToFind = os.path.join(os.getenv('SrcRoot'), 'imodel-native', path)
    for comp in Components:
        repoPath = RepoForComp(comp)
        if repoPath.lower() in pathToFind.lower():
            return comp
    return ""

def MapPathForComp(compToFind):
    for comp in Components:
        if compToFind.lower() in Components[comp]['product'].lower():
            if 'map_repo' in Components[comp].keys():
                repo = Components[comp]['map_repo']
                repoPath = os.getenv('SrcRoot')
                for path in repo:
                    repoPath = os.path.join(repoPath, path)
                return repoPath
            else:
                repoPath = RepoPathForComp(compToFind)
                return repoPath
def AllDlls():
    dlls = []
    for comp in Components:
        if Components[comp]['dll'] not in dlls:
            dlls.append(Components[comp]['dll'].lower())
    return dlls
def AllComps():
    comps = []
    for comp in Components:
        if comp not in comps:
            comps.append(comp.lower())
    return comps
def AllCompsProper():
    comps = []
    for comp in Components:
        if comp not in comps:
            comps.append(comp)
    return comps

def GetName(compName):
	for comp in Components:
		if comp.lower() == compName.lower():
			return comp
	return None