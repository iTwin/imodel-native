#--------------------------------------------------------------------------------------
#
#     $Source: CommonTasks/Components.py $
#
#  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
#
#--------------------------------------------------------------------------------------
import os
import FindStream

Components = {'BeHttp':                 {'dll': 'BeHttpM02.dll', 'exe': 'BeHttpTest',
                                         'ignore':['BeHttp','Tests','UnitTests'],
                                         'owner': 'Vincas Razma'},
              'Bentley':                {'dll': 'BentleyM02.dll', 'exe': 'BentleyTest',
                                         'ignore':['Bentley','Tests'],
                                         'pdb': ['Bentley', 'BentleyDll'],
              'BeSecurity':             {'dll': 'BeSecurityM02.dll', 'exe': 'BeSecurityTest',
                                         'ignore':['BeSecurity','Tests'],
                                         'pdb': ['BeSecurity', 'BeSecurityLibrary'],
                                         'owner': 'Vincas Razma'},
                                         'owner': 'Ramanujam Raman'},
              'BeSQLite':               {'dll': 'BeSQLiteM02.dll', 'exe': 'BeSQLiteTest',
                                        'ignore':['BeSQLite','Tests'],
                                         'pdb': ['BeSQLite', 'BeSQLite'],
                                         'owner': 'Krischan Eberle'},
              'DgnPlatform':            {'dll': 'DgnPlatformM02.dll', 'exe': 'DgnPlatformTest',
                                         'ignore':['DgnPlatform','Tests','DgnProject'],
                                         'pdb': ['DgnPlatform', 'DgnPlatformDll', 'DgnPlatform'],
                                         'owner': 'Ramanujam Raman'},
              'ECDb':                   {'dll': 'BeSQLiteECM02.dll', 'exe': 'ECDbTest',
                                         'ignore':['ECDb','Tests'],
                                         'owner': 'Krischan Eberle'},
              'ECObjects':              {'dll': 'ECObjectsM02.dll', 'exe': 'ECObjectsTest',
                                         'ignore':['ECObjects','test'],
                                         'pdb': ['ECObjects', 'ECObjects'],
                                         'owner': 'Colin Kerr'},
              'ECPresentation':         {'dll': 'ECPresentationM02.dll', 'exe': 'ECPresentationTest',
                                        'ignore':['ECPresentation','Tests'],
                                        'pdb': ['ECPresentation', 'lib'],
                                         'owner': 'Grigas Petraitis'},
              'GeomLibs':               {'dll': 'BentleyGeomM02.dll', 'exe': 'GeomLibsTest',
                                         'ignore':['GeomLibs','geom','test'],
                                         'pdb': ['GeomLibs', 'delivery', 'basegeom','symbols'],
                                         'owner': 'Earlin Lutz'},                   
              'Units':                  {'dll': 'UnitsM02.dll', 'exe': 'UnitsTest',
                                         'ignore':['Units','Tests'],
                                         'owner': 'Colin Kerr'},
              'Licensing':               {'dll': 'LicensingM02.dll', 'exe': 'LicensingTest',
                                         'ignore':['LicensingCrossPlatform','Tests','UnitTests'],
                                         'pdb': ['Licensing'],
                                         'owner': 'Jeff Marker',
                                          'repo': ['imodel02', 'imodelCore', 'LicensingCrossPlatform']},
              'WSClient':               {'dll': 'WebServicesClientM02.dll', 'exe': 'WSClientTest',
                                         'ignore':['WSClient','Tests','UnitTests'],
                                         'pdb': ['WSClient', 'Client'],
                                         'owner': 'Vincas Razma'}
            }

Architectures = { 'Winx64': {'log':'RunGTest'},
                  'AndroidARM64':{'log':'RunANJU'},
                  'WinRTx64':{'log':'RunCppUnit'},
                  'iOSARM64':{'log':'RunXCTest'},
                }
#-------------------------------------------------------------------------------------------
# helper methods to get info for each Component
# bsimethod                                     Majd.Uddin    12/2017
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
def OwnerForComp(compToFind):
    for comp in Components:
        if comp.lower() == compToFind.lower():
            return Components[comp]['owner']
        
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
        if comp.lower() == compToFind.lower():
            if 'repo' in Components[comp].keys():
                return Components[comp]['repo']
            else:
                return  os.path.join(os.getenv('SrcRoot'),'imodel02', 'iModelCore', compToFind)
    
def LogPathForComp(compToFind):
    exeName = ExeForComp(compToFind)
    if exeName is None:
        return None
    outRoot = os.getenv('OutRoot')
    if compToFind.lower() == 'dgnv8converter': # it has different naming convention
        logPath = os.path.join(outRoot, 'Winx64', 'build', 'RunGTest', exeName, exeName, 'run', 'logs', 'test.log')
    else:
        logPath = os.path.join(outRoot, 'Winx64', 'build', 'RunGTest', compToFind+'-GTest', exeName, 'run', 'logs', 'test.log')
    return logPath

def ExePathForComp(compToFind):
    exeName = ExeForComp(compToFind)
    if exeName is None:
        return None
    if compToFind.lower() == 'dgnv8converter': # it has different naming convention
        exePath = os.path.join('%OutRoot%Winx64', 'Product', exeName, exeName+'.exe')
    else:
        exePath = os.path.join('%OutRoot%Winx64', 'Product', compToFind+'-GTest', exeName+'.exe')
    return exePath

def IgnorePathForComp(compToFind):
    ignore = IgnoreForComp(compToFind)
    if ignore is None:
        return None
    ignorePath = os.path.join(os.getenv('SrcRoot'), 'imodel02', 'iModelCore')
    for path in ignore:
        ignorePath = os.path.join(ignorePath, path)
    ignorePath = os.path.join(ignorePath, 'ignore_list.txt')
    return ignorePath

def PdbPathForComp(compToFind):
    pdb = PdbForComp(compToFind)
    pdbRoot = os.path.join(os.getenv('OutRoot'), 'winx64', 'build')
    dll = DllForComp(compToFind)
    stream = findstream()
    if stream == "imodel02":
        if 'M02' in dll:
            pdbName = dll[:dll.find('M02')]
        else:
            pdbName = dll[:-4] #omit .dll
    else:
        if 'B02' in dll:
            pdbName = dll[:dll.find('B02')]
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

def findstream():
    TeamConfigPath = os.path.join(os.getenv('SrcRoot'), 'teamConfig', 'treeConfiguration.xml')
    streamName = FindStream.FindStreamDetails(TeamConfigPath)
    return streamName

def RepoPathForComp(compToFind):
    repo = RepoForComp(compToFind)
    stream = findstream()
    srcRoot = os.getenv('SrcRoot')
    if repo is None:
        if stream == "imodel02":
            repoPath = os.path.join(srcRoot,'imodel02', 'iModelCore' ,compToFind)
            print "Stream: " + str(stream)
        else:
            repoPath = os.path.join(srcRoot ,compToFind)
    else:
        repoPath = srcRoot
        for path in repo:
            repoPath = os.path.join(repoPath, path)
            print "Stream: " + str(stream)
    return repoPath
    
def TiaMapPathForComp(compToFind):
    exeName = ExeForComp(compToFind)
    if exeName is None:
        return None
    mapRoot = os.path.join(os.getenv('SrcRoot'), 'imodel02', 'TestingScripts', 'TestImpactAnalysis', 'TIAMaps')
    if compToFind.lower() == 'dgnv8converter': # it has different naming convention
        tiaPath = os.path.join(mapRoot, 'TIAMap_' +compToFind+ '_Bentley.txt')
    else:
        tiaPath = os.path.join(mapRoot, 'TIAMap_' +compToFind+ '_' +compToFind+'.txt')
    return tiaPath
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

def GetName(compName):
	for comp in Components:
		if comp.lower() == compName.lower():
			return comp
	return None
def FindArchitectureDeatils(architecture):
    for arch in Architectures:
        if arch.lower() == architecture.lower():
            return arch
    return None
    
def AndriodLogPathForComp(compToFind,architecture):
    arch = FindArchitectureDeatils(architecture)
    if arch is None:
        return None
    outRoot = os.getenv('OutRoot')
    for comp in Components:
        if comp.lower() == compToFind.lower():
            logPath = os.path.join(outRoot, arch,'static','build',Architectures[arch]['log'] , comp+'-AndroidJUnit','test.log')
            return logPath
    
def UwpLogPathForComp(compToFind,architecture):
    arch = FindArchitectureDeatils(architecture)
    if arch is None:
        return None
    outRoot = os.getenv('OutRoot')
    for comp in Components:
        if comp.lower() == compToFind.lower():
            logPath = os.path.join(outRoot, arch, 'build',Architectures[arch]['log'] , comp+'-UwpTest','test.log')
            return logPath
        
def IosLogPathForComp(compToFind,architecture):
    arch = FindArchitectureDeatils(architecture)
    if arch is None:
        return None
    outRoot = os.getenv('OutRoot')
    for comp in Components:
        if comp.lower() == compToFind.lower():
            logPath = os.path.join(outRoot, arch, 'static', 'build', Architectures[arch]['log'] , comp+'-iOSXCTest','XCTest.log')
            return logPath
