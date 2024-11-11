#---------------------------------------------------------------------------------------------
#  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
#  See LICENSE.md in the repository root for full copyright notice.
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
            return Components[comp]['pdb']
            
def RepoForComp(compToFind):        
    for comp in Components:
        if comp.lower() == compToFind.lower():
            repoPath = os.path.join(os.getenv('SrcRoot'),'imodel-native', 'iModelCore', compToFind)  # This is the path to the component source folder relative to source root
            return repoPath

def ExePathForComp(compToFind):
    exeName = ExeForComp(compToFind)
    exe_path = os.path.join(os.getenv('OutRoot'),'Winx64', 'Product')
    exe_path = os.path.join(exe_path, compToFind+"-GTest")
    exe_path = os.path.join(exe_path, exeName+'.exe')
    return exe_path

def IgnorePathForComp(compToFind):
    ignore = IgnoreForComp(compToFind)
    ignorePath = os.path.join(os.getenv('SrcRoot'), 'imodel-native')
    for path in ignore:
        ignorePath = os.path.join(ignorePath, path)
    ignorePath = os.path.join(ignorePath, 'ignore_list.txt')
    return ignorePath

def PdbPathForComp(compToFind):
    pdbPath = os.path.join(os.getenv('OutRoot'), 'Winx64', 'build')
    pdb = PdbForComp(compToFind)
    for path in pdb:
        pdbPath = os.path.join(pdbPath, path)
    return pdbPath

def RepoPathForComp(compToFind):
    repoPath = RepoForComp(compToFind)
    srcRoot = os.getenv('SrcRoot')
    if repoPath is None:
        repoPath = os.path.join(srcRoot,'imodel-native', 'iModelCore' ,compToFind)
    return repoPath

def AllComps():
    comps = []
    for comp in Components:
        comps.append(comp.lower())
    return comps

def GetName(compName):
	for comp in Components:
		if comp.lower() == compName.lower():
			return comp
	return None