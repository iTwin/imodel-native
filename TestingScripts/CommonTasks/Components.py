#--------------------------------------------------------------------------------------
#
#  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
#
#--------------------------------------------------------------------------------------
import os
import FindStream

Components = {'BeHttp':             {'dll': 'BeHttpM02.dll', 'exe': 'BeHttpTest',
                                     'ignore':['iModelCore', 'BeHttp','Tests','UnitTests'],
                                     'owner': 'Vincas Razma',
                                     'product': 'BeHttp-Gtest'},
              'Bentley':            {'dll': 'BentleyM02.dll', 'exe': 'BentleyTest',
                                     'ignore':['iModelCore', 'Bentley','Tests'],
                                     'pdb': ['Bentley', 'BentleyDll'],
                                     'product': 'Bentley-Gtest'},
              'BeSecurity':         {'dll': 'BeSecurityM02.dll', 'exe': 'BeSecurityTest',
                                     'ignore':['iModelCore', 'BeSecurity','Tests'],
                                     'pdb': ['BeSecurity', 'BeSecurityLibrary'],
                                     'owner': 'Vincas Razma',
                                     'owner': 'Ramanujam Raman',
                                     'product': 'BeSecurity-Gtest'},
              'BeSQLite':           {'dll': 'BeSQLiteM02.dll', 'exe': 'BeSQLiteTest',
                                     'ignore':['iModelCore', 'BeSQLite','Tests'],
                                     'pdb': ['BeSQLite', 'BeSQLite'],
                                     'owner': 'Krischan Eberle',
                                     'product': 'BeSQLite-Gtest'},
              'DgnPlatform':        {'dll': 'DgnPlatformM02.dll', 'exe': 'DgnPlatformTest',
                                     'ignore':['iModelCore', 'DgnPlatform','Tests','DgnProject'],
                                     'pdb': ['DgnPlatform', 'DgnPlatformDll', 'DgnPlatform'],
                                     'owner': 'Ramanujam Raman',
                                     'product': 'DgnPlatform-Gtest'},
              'ECDb':               {'dll': 'BeSQLiteECM02.dll', 'exe': 'ECDbTest',
                                     'ignore':['iModelCore', 'ECDb','Tests'],
                                     'owner': 'Krischan Eberle',
                                     'product': 'ECDb-Gtest'},
              'ECObjects':          {'dll': 'ECObjectsM02.dll', 'exe': 'ECObjectsTest',
                                     'ignore':['iModelCore', 'ECObjects','test'],
                                     'pdb': ['ECObjects', 'ECObjects'],
                                     'owner': 'Colin Kerr',
                                     'product': 'ECObjects-Gtest'},
              'ECPresentation':     {'dll': 'ECPresentationM02.dll', 'exe': 'ECPresentationTest',
                                     'ignore':['iModelCore', 'ECPresentation','Tests'],
                                     'pdb': ['ECPresentation', 'lib'],
                                     'owner': 'Grigas Petraitis',
                                     'product': 'ECPresentation-Gtest'},
              'GeomLibs':           {'dll': 'BentleyGeomM02.dll', 'exe': 'GeomLibsTest',
                                     'ignore':['iModelCore', 'GeomLibs','geom','test'],
                                     'pdb': ['GeomLibs', 'delivery', 'basegeom','symbols'],
                                     'owner': 'Earlin Lutz',
                                     'product': 'GeomLibs-Gtest'},                   
              'Units':              {'dll': 'UnitsM02.dll', 'exe': 'UnitsTest',
                                     'ignore':['iModelCore', 'Units','Tests'],
                                     'owner': 'Colin Kerr',
                                     'product': 'Units-Gtest'},
              'Licensing':          {'dll': 'LicensingM02.dll', 'exe': 'LicensingUnitTest',
                                     'ignore':['iModelCore', 'LicensingCrossPlatform','Tests','UnitTests'],
                                     'pdb': ['Licensing'],
                                     'owner': 'Jeff Marker',
                                     'product': 'Licensing-Gtestunit',
                                     'repo': ['imodel02', 'imodelCore', 'LicensingCrossPlatform'],
                                     'special_path': ['Licensing-GTestUnit']},
              'WSClient':           {'dll': 'WebServicesClientM02.dll', 'exe': 'WSClientTest',
                                     'ignore':['iModelCore', 'WSClient','Tests','UnitTests'],
                                     'pdb': ['WSClient', 'Client'],
                                     'owner': 'Vincas Razma',
                                     'product': 'WSClient-Gtest'},
              'DgnV8Converter':     {'dll': 'DgnV8ConverterM02.dll', 'exe': 'DgnV8ConverterTests',
                                     'pdb': ['DgnDbSync', 'DgnV8ConverterTests'],
                                     'ignore': ['iModelBridgeCore', 'DgnDbSync', 'DgnV8', 'Tests'],
                                     'owner': 'Carole MacDonald',
                                     'product': 'DgnV8ConverterTests',
                                     'repo': ['imodel02', 'iModelBridgeCore', 'DgnDbSync'],
                                     'special_path': ['DgnV8ConverterTests']},
              'MstnBridge':         {'dll': 'iModelBridgeM02.dll', 'exe': 'MstnBridgeTests',
                                     'pdb': ['MstnBridge', 'MstnBridgeTests'],
                                     'owner': 'Abeesh Basheer',
                                     'product': 'MstnBridgeTests',
                                     'repo': ['imodel02', 'Bridges', 'Mstn'],
                                     'special_path': ['MstnBridgeTests']},
              'DwgImporter':        {'dll': 'DwgBridge.dll', 'exe': 'DwgImporterTests',
                                     'product': 'DwgImporterTests',
                                     'ignore': ['iModelBridgeCore', 'DgnDbSync', 'Dwg', 'Tests'],                                     
                                     'repo': ['imodel02', 'iModelBridgeCore', 'DgnDbSync', 'Dwg'],
                                     'special_path': ['DwgImporterTests']},
              'BuildingDomain':     {'dll': 'BuildingDomainM02.dll', 'exe': 'BuildingDomainTests',
                                     'product': 'buildingdomaintests\\assemblies\\',
                                     'ignore': ['Domains', 'Building', 'Tests'],                                         
                                     'repo': ['imodel02', 'Domains', 'Building'],
                                     'special_path': ['BuildingDomainTests', 'Assemblies']},
              'StructuralDomains':  {'dll': 'StructuralDomainM02.dll', 'exe': 'StructuralDomainTests',
                                     'product': 'structuraldomainstested\\',
                                     'ignore': ['Domains', 'Structural', 'Tests'],
                                     'repo': ['imodel02', 'Domains', 'Structural'],
                                     'special_path': ['StructuralDomainsTested']}, 
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
        if compToFind.lower() in Components[comp]['product'].lower():
            if 'repo' in Components[comp].keys():
                repo = Components[comp]['repo']
                repoPath = os.getenv('SrcRoot')
                for path in repo:
                    repoPath = os.path.join(repoPath, path)
                return repoPath
            else:
                repoPath = os.path.join(os.getenv('SrcRoot'),'imodel02', 'iModelCore', compToFind)
                return repoPath
    
def LogPathForComp(compToFind):
    for comp in Components:
        if comp.lower() == compToFind.lower():
            exeName = ExeForComp(comp)
            if exeName is None:
                return None
            base_path = os.path.join('%OutRoot%Winx64', 'build', 'RunGTest')
            log_path = base_path
            if 'special_path' in Components[comp].keys():
                for path in Components[comp]['special_path']:
                    log_path = os.path.join(log_path, path)
            else:
                log_path = os.path.join(log_path, comp+'-GTest')
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
                exe_path = os.path.join(exe_path, comp+'-GTest')
            exe_path = os.path.join(exe_path, exeName+'.exe')
            return exe_path

def IgnorePathForComp(compToFind):
    ignore = IgnoreForComp(compToFind)
    if ignore is None:
        return None
    ignorePath = os.path.join(os.getenv('SrcRoot'), 'imodel02')
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
    repoPath = RepoForComp(compToFind)
    stream = findstream()
    srcRoot = os.getenv('SrcRoot')
    if repoPath is None:
        if stream == "imodel02":
            repoPath = os.path.join(srcRoot,'imodel02', 'iModelCore' ,compToFind)
            print "Stream: " + str(stream)
        else:
            repoPath = os.path.join(srcRoot ,compToFind)
    else:
        print "Repository Path = " + str(repoPath)
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
