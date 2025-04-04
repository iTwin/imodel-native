#---------------------------------------------------------------------------------------------
#  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
#  See LICENSE.md in the repository root for full copyright notice.
#---------------------------------------------------------------------------------------------
import copy, json, os, re, datetime, sys, threading, zipfile
from xml.dom import minidom

from . import azurecli, azuredevopsrestapi, bbutils, buildpaths, cmdutil, compat, globalvars, symlinks, targetplatform, utils, versionutils
from . import internal

s_licenseUrlNameMap = None
s_allowPrereleaseAnywhere = os.environ.get('BB_NUGET_ALLOW_PRERELEASE_ANYWHERE', None) # safety valve: set to non-zero for old behavoir
s_pkgQueryCache = {}
s_downloadLocks = {}
s_downloadXmls = {}
s_downloadUrls = {}
s_packageVersions = {}

TARGET_FRAMEWORK_DEFAULT = '.NETFramework4.7' # default for now
DEPENDENCY_VERSION_HIGHEST = True # default behavior of NuGet pre-2.8

LICENSE_SUFFIX = '-license.txt' # Decided on .txt even though some are actually HTML. They can be read regardless.

NuGetFeedTypes   = ['http', 'fileshare', 'localpkg']
NUGETFEED_http          = NuGetFeedTypes.index ('http')         # Use HTTP 
NUGETFEED_fileshare     = NuGetFeedTypes.index ('fileshare')    # Use a symlink to loose local files
NUGETFEED_localpkg      = NuGetFeedTypes.index ('localpkg')     # It's a nupkg file, but on the local disk

g_packageDownloadLock = threading.Lock()
g_changedPackages = set()

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def GetNugetPackage (pkgSource):
    if pkgSource.GetFeed().m_type == NUGETFEED_localpkg:
        return LocalNugetPackage (pkgSource)
    elif pkgSource.GetFeed().m_type == NUGETFEED_fileshare:
        return LooseFileNugetPackage (pkgSource)
    else:
        return RemoteNugetPackage (pkgSource)

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def TransformNugetName(name, platform, isStatic, isToolPart):
    if isToolPart:
        platform = targetplatform.GetHostPlatform()

    nugetName = "{0}_{1}".format(name, platform)
    if isStatic:
        nugetName = nugetName + "_static"
    return nugetName

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def GetTargetFrameworkFromPath(nextdir):
    # Attempt to find the targetFramework from the directory by assuming it is the next directory
    # under the 'lib'.
    
    # There seems to be more than just the 'lib' folder that can contain framework specific files.
    # The 'build', 'tools', and 'content' as well. Need to add support as well...
    
    def splitPath(path):
        folders = []
        while 1:
            path, folder = os.path.split(path)
            if folder != "":
                folders.append(folder)
            else:
                if path != "":
                    folders.append(path)
                    
                break

        folders.reverse()
        return folders

    fixedPath = splitPath(nextdir.strip('/\\')) # Will strip both, or either, '/' and '\\'.
    if 'lib' in fixedPath:
        libIdx = fixedPath.index('lib')
    else:
        return None

    targetFramework = None
    if len(fixedPath) > libIdx + 1:
        targetFramework = fixedPath[libIdx + 1]

    if not targetFramework:
        return None

    return NuGetPackageBase.GetSuggestedTargetFramework(targetFramework)

#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
class NuGetUsage (object):
    # This is for the NugetPackage in a Part.
    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def __init__(self, pkgSource, pkgFramework, platform, isStatic, isToolPart):
        self.m_pkgFramework = pkgFramework
        self.m_pkgSource = pkgSource
        # The idea behind these is that we can use it to transform the name appropriately.
        self.m_platform = platform
        self.m_isStatic = isStatic
        self.m_isToolPart = isToolPart

#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
class NuGetFeed (object):
    #---------------------------------------------------------------------------------------
    # bsimethod
    # Class: NuGetFeed
    #---------------------------------------------------------------------------------------
    def __init__(self, name, address, provider, feedType):
        self.m_name = name
        self.m_address = address
        self.m_provider = provider
        self.m_type = feedType or NUGETFEED_http

    #---------------------------------------------------------------------------------------
    # bsimethod
    # Class: NuGetFeed
    #---------------------------------------------------------------------------------------
    def GetRemoteAddress (self):
        return self.m_address

    #---------------------------------------------------------------------------------------
    # bsimethod
    # Class: NuGetFeed
    #---------------------------------------------------------------------------------------
    def GetAuthenticationHeader (self):
        # No authentication token required
        if self.m_provider == globalvars.CREDENTIAL_PROVIDER_BASIC:
            return None

        # Check if Azure CLI is forced globally
        if globalvars.useAzureCliAuth:
            return azurecli.GetAuthenticationHeader()

        # Check what provider is requested by build strategies
        if self.m_provider == globalvars.CREDENTIAL_PROVIDER_TOKEN:
            return globalvars.buildStrategy.GetAuthenticationToken(self.m_name)
        if self.m_provider == globalvars.CREDENTIAL_PROVIDER_MCP:
            return cmdutil.GetMicrosoftProviderToken(self.m_address)
        if self.m_provider == globalvars.CREDENTIAL_PROVIDER_AZ:
            return azurecli.GetAuthenticationHeader()
        if self.m_provider == globalvars.CREDENTIAL_PROVIDER_AUTO:
            return bbutils.ChooseAuthenticationHeader(self, self.m_name, self.m_address)
        return None

#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
class NuGetSource (object):
    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class: NuGetSource
    #-------------------------------------------------------------------------------------------
    def __init__(self, name, version, platforms=None, multiplatform=False, dependency=False, feed=None, alias=None):
        self.m_name = name
        self.m_version = version
        self.m_origVersion = version
        self.m_platforms = platforms
        self.m_packageDir = None
        self.m_remoteAddr = None
        self.m_authHeader = None
        self.m_multiplatform = multiplatform  # Attach _x64 etc. to the name for each platform.
        self.m_dependency = dependency # When we infer a package version based on a dependency of another package
        self.m_origName = name  # When we mutate for LKGs we keep the original around.
        self.m_feed = feed  # Feed passed in; None means use default.
        self.m_alias = alias if alias else name
        self.m_pkg = None

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class: NuGetSource
    #-------------------------------------------------------------------------------------------
    def __str__ (self):
        return self.m_name + '(' + str(self.m_version) + ')'

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class: NuGetSource
    #-------------------------------------------------------------------------------------------
    def Update (self, version=None, platforms=None, feed=None):
        if version:     self.m_version = version
        if platforms:   self.m_platforms = platforms
        if feed:        self.m_feed = feed

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class: NuGetSource
    #-------------------------------------------------------------------------------------------
    def IsToolPart (self):
        return (self.m_name.lower() in globalvars.buildStrategy.m_toolPackages['nuget'] or self.m_origName in globalvars.buildStrategy.m_toolPackages['nuget'])

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class: NuGetSource
    #-------------------------------------------------------------------------------------------
    def GetLocalDownloadDir (self):
        return buildpaths.GetToolCacheSourceRoot() if self.IsToolPart() else buildpaths.GetNuGetSourceLocation()

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class: NuGetSource
    #-------------------------------------------------------------------------------------------
    def GetVersion (self):
        return self.m_version

    #---------------------------------------------------------------------------------------
    # bsimethod
    # Class: NuGetSource
    #---------------------------------------------------------------------------------------
    def GetFeed (self):
        if self.m_feed:
            if not self.m_feed in globalvars.buildStrategy.m_nugetFeeds:
                raise utils.StrategyError ("Cannot find NuGet Feed {0} in Package {1}\n".format (self.m_feed, self.m_name))
            return globalvars.buildStrategy.m_nugetFeeds[self.m_feed]
        else:
            return globalvars.buildStrategy.m_defaultNugetPullFeed

    #---------------------------------------------------------------------------------------
    # bsimethod
    # Class: NuGetSource
    #---------------------------------------------------------------------------------------
    def GetLocalPath (self, version=None):
        name = self.m_name
        if version:
            name = '{0}.{1}'.format(name, version)
        elif not '*' in  self.m_version:
            name = '{0}.{1}'.format(name, self.m_version)
        return os.path.join (self.GetLocalDownloadDir(), name)

    #---------------------------------------------------------------------------------------
    # bsimethod
    # Class: NuGetSource
    #---------------------------------------------------------------------------------------
    def GetAuthenticationHeader (self):
        if not self.m_authHeader:
            self.m_authHeader = self.GetFeed().GetAuthenticationHeader()
        return self.m_authHeader

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class: NuGetSource
    #-------------------------------------------------------------------------------------------
    def GetRemoteAddress (self):
        if not self.m_remoteAddr:
            self.m_remoteAddr = self.GetFeed().GetRemoteAddress()
        return self.m_remoteAddr

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class: NuGetSource
    #-------------------------------------------------------------------------------------------
    def GetProvenanceFileName (self):
        return self.m_alias + '.provenance.txt'

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class: NuGetSource
    #-------------------------------------------------------------------------------------------
    def GetProvenanceFile (self):
        return os.path.join (self.GetLocalDownloadDir(), self.GetProvenanceFileName())

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class: NuGetSource
    #-------------------------------------------------------------------------------------------
    def GetResolvedVersion (self):
        if '*' in  self.m_version:
            self.GetPackageDirectory()
        return self.m_version

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class: NuGetSource
    #-------------------------------------------------------------------------------------------
    def GetPackageDirectory (self, baseDir=None):
        usingAlternateBaseDir = None != baseDir
        if not usingAlternateBaseDir and self.m_packageDir:
            return self.m_packageDir
        
        if not baseDir:
            baseDir = self.GetLocalDownloadDir()
        provFileName = os.path.join (baseDir, self.GetProvenanceFileName())
        if not os.path.exists (provFileName):
            return None

        # First line contains the provenance
        provLine = []
        with open (provFileName, 'rt') as provFile:
            provLine = provFile.read().strip().split('|')
        if provLine and len(provLine) > 1 and provLine[1]:
            if provLine[2] == 'tip': 
                utils.showStatusMsg ("Found old-style provenance file for NuGet Package '{0}'".format (self.m_name),utils.INFO_LEVEL_Important, utils.YELLOW)
                return None  # Old style proveance.txt; we could pull the nuget, but we'd have to figure out the correct prov.
            else:
                self.m_version = provLine[2]
                pkgDir = self.m_name+'.'+provLine[2]
                if not usingAlternateBaseDir:
                    self.m_packageDir = pkgDir
                return pkgDir

        return None

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class: NuGetSource
    #-------------------------------------------------------------------------------------------
    def GetPackagePath (self):
        downloadDir = self.GetLocalDownloadDir()
        if not downloadDir:
            return None
        packageDir = self.GetPackageDirectory()
        if not packageDir:
            return None
        return os.path.join (downloadDir, packageDir)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class: NuGetSource
    #-------------------------------------------------------------------------------------------
    def Clone (self, newName):
        newNugetSource = copy.deepcopy (self)
        if newNugetSource.m_alias == newNugetSource.m_name:
            newNugetSource.m_alias = newName
        newNugetSource.m_name = newName
        return newNugetSource

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class: NuGetSource
    #-------------------------------------------------------------------------------------------
    def GetPkg (self):
        if not self.m_pkg:
            self.m_pkg = GetNugetPackage (self)

        return self.m_pkg

#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
class NuGetPackageBase (object):
    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class: NuGetPackageBase
    #-------------------------------------------------------------------------------------------
    def __init__(self, pkgSource):
        self.m_pkgSource = pkgSource

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class: NuGetPackageBase
    #-------------------------------------------------------------------------------------------
    def GetName(self):
        return self.m_pkgSource.m_alias

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class: NuGetPackageBase
    #-------------------------------------------------------------------------------------------
    def DownloadFromNuGetSource(self, version, localDir):               raise utils.FatalError ('DownloadFromNuGetSource must be implemented by subclasses')
    def SearchVersionsFromServer(self, address, isSyncPull = False):    raise utils.FatalError ('SearchVersionsFromServer must be implemented by subclasses')
    def GetPackageVersion(self, version):                               raise utils.FatalError ('GetPackageVersion must be implemented by subclasses')

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class: NuGetPackageBase
    #-------------------------------------------------------------------------------------------
    def GetInfoInternal(self, pkgFile):
        archive = zipfile.ZipFile(pkgFile, 'r')
        nuspecList = [f for f in archive.filelist if f.filename.lower().endswith('.nuspec')]

        if len(nuspecList) != 1:
            # I'm told there will always be one.
            utils.showInfoMsg ('Incorrect number of nuspecs ({1}) found in {2}! All Nuspecs: {0}'.format (repr(nuspecList), len(nuspecList), pkgFile), utils.INFO_LEVEL_Important)
            return (None, None, None)

        xml = archive.open(nuspecList[0]).read()
        xmldoc = minidom.parseString(xml)
    #   print(xmldoc.toprettyxml(indent = '    ', encoding='UTF-8'))
        name = xmldoc.getElementsByTagName('id')[0].firstChild.nodeValue
        version = xmldoc.getElementsByTagName('version')[0].firstChild.nodeValue
        depsXml = xmldoc.getElementsByTagName('dependencies')
        return (name, version, depsXml)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class: NuGetPackageBase
    #-------------------------------------------------------------------------------------------
    def GetInfoFromPkgDir(self, pkgdir):
        nuspecFile = os.path.join (pkgdir, self.GetName()+'.nuspec')
        xmldoc = minidom.parse(nuspecFile)
        name = xmldoc.getElementsByTagName('id')[0].firstChild.nodeValue
        version = xmldoc.getElementsByTagName('version')[0].firstChild.nodeValue
        depsXml = xmldoc.getElementsByTagName('dependencies')
        return (name, version, depsXml)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class: NuGetPackageBase
    #-------------------------------------------------------------------------------------------
    def GetDepsFromPackage(self, pkgDir, targetFramework = None):
        name, version, depsXml = self.GetInfoFromPkgDir(pkgDir)

        def hasAttributes(attrNode):
            return attrNode.attributes and attrNode.hasAttribute ('id') and attrNode.hasAttribute ('version')

        def getDepsFromAttributes(deps, attrNode, targetFramework):
            if hasAttributes(attrNode):
                depDict = dict()
                for depKey in attrNode.attributes.keys():
                    depDict[depKey] = attrNode.attributes[depKey].value
                if targetFramework:
                    depDict['targetFramework'] = targetFramework
                deps.append(depDict)
            return deps

        deps = []
        for nodes in depsXml:
            for node in nodes.childNodes:
                if node.nodeName and node.nodeName == 'dependency':
                    deps = getDepsFromAttributes(deps, node, targetFramework)
                if targetFramework:
                    for child in node.childNodes:
                        if not hasAttributes(child):
                            continue
                        if node.attributes:
                            for groupKey in node.attributes.keys():
                                deps = getDepsFromAttributes(deps, child, node.attributes[groupKey].value)
                        else: # has id & version, but no group
                            deps = getDepsFromAttributes(deps, child, targetFramework)
        return (name, version, deps)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class: NuGetPackageBase
    #-------------------------------------------------------------------------------------------
    def GetLatestVersion (self, nugetSource):
        allVersions = self.SearchVersionsFromServer(nugetSource.GetRemoteAddress(), False)
        return allVersions[0]

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class: NuGetPackageBase
    #-------------------------------------------------------------------------------------------
    def GetLicenseDir (self, localDir):
        return os.path.join (localDir, 'licenses')

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class: NuGetPackageBase
    #-------------------------------------------------------------------------------------------
    def GetLicenseFileName (self, pkgName, version, localDir):
        return os.path.join (self.GetLicenseDir(localDir), '{0}.{1}{2}'.format(pkgName, version, LICENSE_SUFFIX))

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class: NuGetPackageBase
    #-------------------------------------------------------------------------------------------
    def IsFileIgnoredForNuGetUnzip(self, filename):
        if filename.endswith('/'):
            return True
        if filename.endswith('\\'):
            return True
        if filename.endswith('.nupkg'):
            return True
        if filename.startswith('_rels'):
            return True
        if filename == '[Content_Types].xml':
            return True
        return False

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class: NuGetPackageBase
    #-------------------------------------------------------------------------------------------
    def UnzipPackage(self, pkgDirName, pkgPathName):
        archive = zipfile.ZipFile(pkgPathName, 'r')
        for f in archive.filelist:
            if self.IsFileIgnoredForNuGetUnzip(f.filename):
                continue
            zipHandle = archive.open(f.filename)
            if not zipHandle:
                raise utils.BuildError ('Cannot open package {0} stream {1}\n'.format(pkgPathName, f.filename))
            fileBytes = zipHandle.read()
            outFileName = compat.unquoteUrl(f.filename)
            if not compat.isUnicode (outFileName): # Py 2 it's utf-8 and have to upconvert. Py 3 it's unicode already.
                outFileName = outFileName.decode('utf8')
            outFilePath = os.path.join(pkgDirName, outFileName)
            outDir = os.path.dirname(outFilePath)
            symlinks.makeSureDirectoryExists(outDir)
            outFile = open(outFilePath, 'wb')
            if not outFile:
                raise utils.BuildError ('Cannot write package {0} stream {1} to {2}\n'.format(pkgPathName, f.filename, outFilePath))
            outFile.write(fileBytes)
            outFile.close()
        utils.showInfoMsg ('Unzipped package {0} into {1}\n'.format (pkgPathName, pkgDirName), utils.INFO_LEVEL_RarelyUseful)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class: NuGetPackageBase
    #-------------------------------------------------------------------------------------------
    def GetBindingList (self, package, _packageFramework, currentPart, forPull=False):
    #    if currentPart.m_nugetPackages:
    #        for (partPackage, pkgFramework) in currentPart.m_nugetPackages:
    #            utils.showInfoMsg ("Part '{0}' contains NuGetPackage '{1}' framework {2}\n".format (package, partPackage, repr(pkgFramework)), utils.INFO_LEVEL_RarelyUseful)

        assemblyFileList = []
        bindingList = []
        if currentPart.m_bindings:
            if currentPart.m_bindings.m_assemblyFiles:
                for assemblyFileSources in currentPart.m_bindings.m_assemblyFiles:
                    assemblyFileList.extend(utils.splitSources(assemblyFileSources.m_val))
            # do we need to add miscFiles too?
            if currentPart.m_bindings.m_miscFiles:
                for assemblyFileSources in currentPart.m_bindings.m_miscFiles:
                    assemblyFileList.extend(utils.splitSources(assemblyFileSources.m_val))

        tag = None
        subDir = None
        if forPull:
            tag = self.m_pkgSource.GetVersion()
        else: # for build
            tag = self.GetVersionFromProvenance()

        # complain we can't find tag/version
        if not tag:
            if self.m_pkgSource.m_multiplatform:
                raise utils.BuildError ('Cannot find NuGet Package {0} for the current platform ({1}). Did you pull for a different platform?\n'.format(self.m_pkgSource.m_origName, currentPart.m_info.m_platform))
            raise utils.BuildError ('Cannot find NuGet Package {0}. Check if you have pulled it.\n'.format(package))

        # instead of linking everything in sight, only link the assembly files in the bindings?
        for assemblyFile in assemblyFileList:
            utils.showInfoMsg ("NuGet Package '{0}' AssemblyFile '{1}'\n".format (package, assemblyFile), utils.INFO_LEVEL_RarelyUseful)
            assemblyFileSplit = assemblyFile.replace ('/', '\\').split('\\') # e.g., 'NuGet\net45\EntityFramework.dll'
            if len(assemblyFileSplit) > 0 and assemblyFileSplit[0].lower() != 'nuget': 
                # Only things with nuget leading the path will be symlinked from the NuGet source.
                continue
            subDir = 'lib'
            targetFile = 'lib' # add lib prefix
            joinStart = 1
            for assemblyFileDir in assemblyFileSplit[joinStart:]: # skip 'nuget' (and maybe 'build')
                targetFile = os.path.join(targetFile, assemblyFileDir)
            targetFrameworkDir = None
            if len(assemblyFileSplit) > 2:
                targetFrameworkDir = assemblyFileSplit[joinStart] # e.g. 'net45'
            packageDir = '{0}.{1}'.format(package, tag)
            targetName = os.path.join(packageDir, targetFile)
            utils.showInfoMsg ("NuGet Package '{0}' AssemblyFile '{1}' targetName '{2}' targetFrameworkDir '{3}' appended to bindingList\n".format (package, assemblyFile, targetName, targetFrameworkDir), utils.INFO_LEVEL_RarelyUseful)
            bindingList.append((assemblyFile, targetName, targetFrameworkDir, subDir))

        return bindingList

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class: NuGetPackageBase
    #-------------------------------------------------------------------------------------------
    def GetBindingSubDirectory (self, nugetSource):
        if nugetSource.m_multiplatform:
            return ''
        return nugetSource.m_alias if nugetSource.m_alias != nugetSource.m_name else ''

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class: NuGetPackageBase
    #-------------------------------------------------------------------------------------------
    def LinkNugetToBuildContext (self, _package, targetFrameworkList, currentPart, subDir, isNative=False):
        # Building is just linking it into the right buildcontext.
    #    nugetSource = globalvars.buildStrategy.GetNugetSource (package)  # raises exception if fails
        outputBuildContext = currentPart.GetOutputBuildContextDir(currentPart.m_info.m_buildContext, currentPart.IsStatic())
        utils.showInfoMsg ("Connecting NuGet Package '{0}' to context '{1}'\n".format (self.GetName(), outputBuildContext), utils.INFO_LEVEL_SomewhatInteresting)
        
        # Link all files in the source direcctory
        linkedAnything = False
        for targetFrameworkDir in targetFrameworkList:
            if not targetFrameworkDir:
                continue
            if isNative:
                # For native we just link in the whole thing. There is a lot of variation on how things are stored (Lib, Build, etc) so it's best just to supply the whole thing.
                sourceDir = self.m_pkgSource.GetPackagePath()
            else:
                sourceDir = os.path.join(self.m_pkgSource.GetPackagePath(), subDir, targetFrameworkDir)
            utils.showInfoMsg ("Connecting NuGet Package '{0}' targetFrameworkDir '{1}' sourceDir {2}\n".format (self.GetName(), targetFrameworkDir, sourceDir), utils.INFO_LEVEL_RarelyUseful)
            for sourceFile in os.listdir (sourceDir):
                sourcePath = os.path.join (sourceDir, sourceFile)
                if not os.path.isdir(sourcePath):
                    linkPath = os.path.join (outputBuildContext, 'nuget', targetFrameworkDir, self.GetBindingSubDirectory (self.m_pkgSource), self.m_pkgSource.m_origName if isNative else '', sourceFile)
                    # set checkSame to False so matching files in nuget dependencies do not clash. JB 07/2015
                    # example case: Microsoft.Practices.Prism.SharedInterfaces.dll in Prism.Composition and Prism.Mvvm
                    utils.showInfoMsg ("Linking NuGet link '{0}' to source '{1}'\n".format (linkPath, sourcePath), utils.INFO_LEVEL_RarelyUseful)
                    currentPart.PerformBindToFile (linkPath, sourcePath, checkSame=False, checkTargetExists=True, skipIntermediateLinks=True)
                    linkedAnything = True
                else:
                    linkPath = os.path.join (outputBuildContext, 'nuget', targetFrameworkDir, self.GetBindingSubDirectory (self.m_pkgSource), self.m_pkgSource.m_origName if isNative else '', sourceFile)
                    utils.showInfoMsg ("Linking NuGet link '{0}' to source '{1}'\n".format (linkPath, sourcePath), utils.INFO_LEVEL_RarelyUseful)
                    currentPart.PerformBindToDir (linkPath, sourcePath, checkSame=False, checkTargetExists=True, skipIntermediateLinks=True)
                    linkedAnything = True

        # Link in the license if needed
        if linkedAnything:
            licSource = self.GetLicenseFileName (self.GetName(), self.m_pkgSource.GetResolvedVersion(), self.m_pkgSource.GetLocalDownloadDir())
            if licSource and os.path.exists (licSource):
                licLink = os.path.join (outputBuildContext, 'nuget', 'licenses', self.GetName()+LICENSE_SUFFIX)
                utils.showInfoMsg ("Linking NuGet license '{0}' ==> '{1}'\n".format (licSource, licLink), utils.INFO_LEVEL_RarelyUseful)
                currentPart.PerformBindToFile (licLink, licSource, checkSame=True, checkTargetExists=True, skipIntermediateLinks=True)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #
    # Class: NuGetPackageBase
    # Checks if the bindings list contains any files from the specific package and suggests bindings if no valid ones are used
    #-------------------------------------------------------------------------------------------
    def CheckAndWarnInvalidBindings(self, currentPart, bindings):
        # Normalize these up front
        bindings = [os.path.normpath(binding[0]).lower() for binding in bindings]
        bindingSug = ""
        for root, dirs, _ in os.walk(os.path.join(self.m_pkgSource.GetPackagePath(), 'lib')):
            for curdir in dirs:
                for curfile in os.listdir(os.path.join(root, curdir)):
                    subPath = os.path.join("Nuget", curdir, self.GetBindingSubDirectory (self.m_pkgSource), curfile)
                    if os.path.normpath(subPath).lower() in bindings:
                        return
                    bindingSug += "\t{0}\n".format(subPath)
        if bindingSug:
            utils.ShowAndDeferMessage("Your part <{0}> with NuGet package '{1}' has zero valid bindings. Consider adding one or more of the following to the part bindings depending on which runtime you are using (or add Type=\"Native\" if it's not a .NET package). The binding statement will create the link into your build context as well as in your output:\n{2}".format(currentPart.GetShortRepresentation(), self.GetName(), bindingSug), utils.INFO_LEVEL_VeryInteresting, utils.YELLOW)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class: NuGetPackageBase
    #-------------------------------------------------------------------------------------------
    def BuildAction (self, package, packageFramework, pkgSource, currentPart, allProcessedItems):

        # We want to bind in all subpackages. The rub is that most of the time there is no way to know which directory
        # of the subpackage to bind in. If it happens to correspond to the packageFramework that was passed in then
        # we use that. If we can't find the directory then we will just bind in everything. Sometimes it's older versions
        # like "net2" and sometimes it's custom-created directory names like "net45-full." So far we haven't found
        # a good way for people to present this information and nuget isn't willing to tell us, so it's the big pile.

        # Since converting from the packageFramework to a directory name is unreliable at best and impossible at worst,
        # we will stick with the directory name from the bindings throughout this method.

        # Look up targetFrameworkDir from the first binding statement.
        # That's not as good, but it will keep things working as we transition.
        bindingList = self.GetBindingList (package, packageFramework, currentPart, False)
        isNative = packageFramework and packageFramework.lower() == 'native'
        if not isNative and not pkgSource.IsToolPart():
            self.CheckAndWarnInvalidBindings(currentPart, bindingList)

        targetFrameworkDirList = []
        if not bindingList: # No bindings, no packageFramework. Check if is native
            if isNative:
                targetFrameworkDirList.append(packageFramework.lower())
            else:
                return
        for bindingItem in bindingList:
            if bindingItem[2] and not (bindingItem[2] in targetFrameworkDirList):
                targetFrameworkDirList.append(bindingItem[2])

        # link in any dependencies
        depsToProcess = [(package, None, None, None)] # Just following format of GetDepsListFromProvenance but only need package name
        while len(depsToProcess) > 0:
            curDep = depsToProcess.pop()
            
            # Don't reprocess items. Not thread-safe but worst case should be that it tries to do the same link at the same time, and linking is thread-safe.
            procString = '*'.join ([curDep[0], currentPart.GetPlatform().GetXmlName(), currentPart.m_info.GetStaticString(short=True), currentPart.m_info.m_buildContext]).lower()
            if len(targetFrameworkDirList) > 0:
                procString += '*' + '*'.join (targetFrameworkDirList).lower()
            if procString in allProcessedItems:
                continue
            allProcessedItems.add (procString)

            # Make sure the framework for the parent part exists, or get a list of all directories because we don't know what else to do
            if curDep[0] == package:
                nugetSource = pkgSource   # This works for now, but I suspect it will stop working once we encounter dependencies between bb-generated packages.
            else:
                nugetSource = globalvars.buildStrategy.GetNugetSource (curDep[0])
            packagePath = nugetSource.GetPackagePath()
            if not packagePath:
                continue  # package not downloaded; sometimes OK?

            # We decided internally to use "pkg/native" but it seems like the common approach is "pkg/lib/native" so for 
            # now we need to support both. Hopefully we will move to the latter.
            packageLib1 = packagePath
            packageLib2 = os.path.join(packagePath, 'lib')
            frameworkList = []
            if not (os.path.exists (packageLib1) or os.path.exists(packageLib2)) and not isNative:
                continue  # No Lib dir. Check if is native
            for packageLib in [packageLib1, packageLib2]:
                if not os.path.exists (packageLib):
                    continue
                if len(targetFrameworkDirList) > 0:
                    frameworkList += [x for x in targetFrameworkDirList if os.path.exists(os.path.join (packageLib, x))]
                else:
                    frameworkList += [x for x in os.listdir(packageLib) if os.path.isdir(os.path.join (packageLib, x))]
            
            if len(frameworkList) < 1:
                frameworkList.append('native' if isNative else '')

            self.LinkNugetToBuildContext (curDep[0], frameworkList, currentPart, 'lib', isNative)

            # Get the dependencies
            if packageFramework:
                targetFrameworkList = [ packageFramework ]
            else:
                targetFrameworkList = [ ]
                for targetFrameworkDir in frameworkList:
                    targetFramework = self.GetSuggestedTargetFramework(targetFrameworkDir)
                    if targetFramework:
                        targetFrameworkList.append(targetFramework)
            for targetFramework in targetFrameworkList:
                depList = self.GetDepsListFromProvenance (targetFramework)
                for dep in depList:
                    if self.TargetFrameworkFuzzyMatch(targetFramework, dep[2]): # filter based on targetFramework
                        utils.showInfoMsg ('Adding dep {0} targetFramework {1} dep targetFramework {2}\n'.format (dep[0], targetFramework, dep[2]), utils.INFO_LEVEL_RarelyUseful)
                        depsToProcess.append(dep)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class: NuGetPackageBase
    #-------------------------------------------------------------------------------------------
    @staticmethod
    def GetSuggestedTargetFramework(targetFrameworkDir):
        targetDict = { \
            "native":  "native", \
            "net20":   ".NETFramework2.0", \
            "net35":   ".NETFramework3.5", \
            "net40":   ".NETFramework4.0", \
            "net45":   ".NETFramework4.5", \
            "net451":  ".NETFramework4.5.1", \
            "net452":  ".NETFramework4.5.2", \
            "net46":   ".NETFramework4.6", \
            "net461":  ".NETFramework4.6.1", \
            "net462":  ".NETFramework4.6.2", \
            "net47":   ".NETFramework4.7", \
            "net471":  ".NETFramework4.7.1", \
            "net472":  ".NETFramework4.7.2", \
            "net48":   ".NETFramework4.8", \
            "portable-net4+sl40+netcore45+wp70":                ".NETPortable4.0-Profile2", \
            "portable-net4+sl40":	                            ".NETPortable4.0-Profile3", \
            "portable-net45+sl40+netcore45+wp70":               ".NETPortable4.0-Profile4", \
            "portable-net4+netcore45":	                        ".NETPortable4.0-Profile5", \
            "portable-net403+netcore45":	                    ".NETPortable4.0-Profile6", \
            "portable-net45+netcore45":	                        ".NETPortable4.5-Profile7", \
            "portable-net4+sl50":	                            ".NETPortable4.0-Profile14", \
            "portable-net403+sl40":	                            ".NETPortable4.0-Profile18", \
            "portable-net403+sl50":	                            ".NETPortable4.0-Profile19", \
            "portable-net45+sl40":	                            ".NETPortable4.0-Profile23", \
            "portable-net45+sl50":	                            ".NETPortable4.0-Profile24", \
            "portable-netcore451+wp81":	                        ".NETPortable4.6-Profile31", \
            "portable-netcore451+wpa81":	                    ".NETPortable4.6-Profile32", \
            "portable-net4+sl40+netcore45+wp8":	                ".NETPortable4.0-Profile36", \
            "portable-net4+sl50+netcore45":	                    ".NETPortable4.0-Profile37", \
            "portable-net403+sl40+netcore45":                   ".NETPortable4.0-Profile41", \
            "portable-net403+sl50+netcore45":                   ".NETPortable4.0-Profile42", \
            "portable-net451+netcore451":                       ".NETPortable4.6-Profile44", \
            "portable-net45+sl40+netcore45":                    ".NETPortable4.0-Profile46", \
            "portable-net45+sl50+netcore45":                    ".NETPortable4.0-Profile47", \
            "portable-net45+wp8":	                            ".NETPortable4.5-Profile49", \
            "portable-net45+netcore45+wp8":	                    ".NETPortable4.5-Profile78", \
            "portable-wpa81+wp81":	                            ".NETPortable4.6-Profile84", \
            "portable-net4+sl40+netcore45+wp71":                ".NETPortable4.0-Profile88", \
            "portable-net4+netcore45+wpa81":                    ".NETPortable4.0-Profile92", \
            "portable-net403+sl40+netcore45+wp70":              ".NETPortable4.0-Profile95", \
            "portable-net403+sl40+netcore45+wp71":              ".NETPortable4.0-Profile96", \
            "portable-net403+netcore45+wpa81":                  ".NETPortable4.0-Profile102", \
            "portable-net45+sl40+netcore45+wp71":               ".NETPortable4.0-Profile104", \
            "portable-net45+netcore45+wpa81":                   ".NETPortable4.5-Profile111", \
            "portable-net4+sl50+netcore45+wp8":                 ".NETPortable4.0-Profile136", \
            "portable-net403+sl40+netcore45+wp8":               ".NETPortable4.0-Profile143", \
            "portable-net403+sl50+netcore45+wp8":               ".NETPortable4.0-Profile147", \
            "portable-net451+netcore451+wpa81":                 ".NETPortable4.6-Profile151", \
            "portable-net45+sl40+netcore45+wp8":                ".NETPortable4.0-Profile154", \
            "portable-netcore451+wpa81+wp81":                   ".NETPortable4.6-Profile157", \
            "portable-net45+sl50+netcore45+wp8":                ".NETPortable4.0-Profile158", \
            "portable-net4+sl50+netcore45+wpa81":               ".NETPortable4.0-Profile225", \
            "portable-net403+sl50+netcore45+wpa81":             ".NETPortable4.0-Profile240", \
            "portable-net45+sl50+netcore45+wpa81":              ".NETPortable4.0-Profile255", \
            "portable-net45+netcore45+wpa81+wp8":               ".NETPortable4.5-Profile259", \
            "portable-net4+sl50+netcore45+wpa81+wp8":           ".NETPortable4.0-Profile328", \
            "portable-net403+sl50+netcore45+wpa81+wp8":         ".NETPortable4.0-Profile336", \
            "portable-net45+sl50+netcore45+wpa81+wp8":          ".NETPortable4.0-Profile344", \
            "portable-net4+sl5+netcore45+wpa81+wp8":            ".NETPortable0.0-net4+sl5+netcore45+wpa81+wp8",\
            "netstandard1.3":                                   ".NETStandard1.3", \
            "netstandard2.0":                                   ".NETStandard2.0", \
            "netcore50":                                        ".NETCore5.0", \
            "netcoreapp2.0":                                    ".NETCoreApp2.0", \
            "netcoreapp2.1":                                    ".NETCoreApp2.1", \
            "netcoreapp3.0":                                    ".NETCoreApp3.0", \
            "netcoreapp3.1":                                    ".NETCoreApp3.1", \
            }
        targetFramework = targetDict.get(targetFrameworkDir, None)
        if not targetFramework:
            # try just substituting as per CommonServiceLocator 1.3 example?
            if targetFrameworkDir and targetFrameworkDir.startswith('portable-'):
                targetFramework = targetFrameworkDir.replace('portable-', '.NETPortable0.0-')
        return targetFramework

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class: NuGetPackageBase
    #-------------------------------------------------------------------------------------------
    def TargetFrameworkFuzzyMatch(self, pkgFramework, depFramework):
        if pkgFramework == depFramework:
            return True
        if pkgFramework is None: # package had targetFramework identity crisis: for now, assume we need it?
            return True
        if depFramework is None: # dependency doesn't know targetFramework: again, assume we need it?
            return True
        if pkgFramework.lower() == depFramework.lower():
            return True
        pattern = r'(^[\.A-Za-z]*)([0-9\.]*)(.*)'
        pkgMatch = re.match(pattern, pkgFramework)
        depMatch = re.match(pattern, depFramework)
        if pkgMatch and depMatch:
            if pkgMatch.group(1) == depMatch.group(1):
                return True # for now, don't require more than non-numeric prefix (e.g., .NETFramework .NETStandard, .NETPortable) to match?
        return False

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class: NuGetPackageBase
    #-------------------------------------------------------------------------------------------
    def GetNuGetPackageFileName(self):
        return "{0}.{1}.nupkg".format(self.GetName(), self.m_pkgSource.m_version)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class: NuGetPackageBase
    #-------------------------------------------------------------------------------------------
    def Clean(self, nugetSource):
        packagePath = nugetSource.GetPackagePath()

        if packagePath is None:
            return
        
        nupkgFileName = self.GetNuGetPackageFileName()
        for item in os.listdir(packagePath):
            if item == nupkgFileName:
                continue

            itemPath = os.path.join(packagePath, item)

            if os.path.isdir(itemPath):
                utils.cleanDirectory(itemPath, deleteFiles=True)
                os.rmdir(itemPath)
            else:
                os.remove(itemPath)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class: NuGetPackageBase
    #-------------------------------------------------------------------------------------------
    def NupkgExists(self, nugetSource):
        packagePath = nugetSource.GetPackagePath()

        if packagePath is None:
            return False

        pathToNupkg = os.path.join(packagePath, self.GetNuGetPackageFileName())

        return os.path.exists(pathToNupkg)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class: NuGetPackageBase
    #-------------------------------------------------------------------------------------------
    def IsHealthy(self):
        packagePath = self.m_pkgSource.GetPackagePath()

        if packagePath is None:
            return False
        
        pathToNupkg = os.path.join(packagePath, self.GetNuGetPackageFileName())

        if not os.path.exists(pathToNupkg):
            return False

        with zipfile.ZipFile(pathToNupkg) as nupkg:
            for fileEntry in nupkg.filelist:
                filename = fileEntry.filename

                if self.IsFileIgnoredForNuGetUnzip(filename):
                    continue

                outFileName = compat.unquoteUrl(filename)
                if not compat.isUnicode(outFileName): # Py 2 it's utf-8 and have to upconvert. Py 3 it's unicode already.
                    outFileName = outFileName.decode('utf8')

                if not os.path.exists(os.path.join(packagePath, outFileName)):
                    return False

        return True

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class: NuGetPackageBase
    #-------------------------------------------------------------------------------------------
    def GetVersionFromProvenance (self):
        return self.GetVersionFromProvenanceFile(self.m_pkgSource.GetProvenanceFile())

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class: NuGetPackageBase
    #-------------------------------------------------------------------------------------------
    def GetVersionFromProvenanceFile (self, provFileName):
        version = None
        if os.path.exists (provFileName):
            provFile = open (provFileName, 'rt')
            provLine = provFile.read().strip().split('|')
            provFile.close()
            if len(provLine) >= 3 and provLine[2] and not 'tip' in provLine[2]: # Tip is a throwback to the old provenance files.
                version = provLine[2]
        return version

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class: NuGetPackageBase
    #-------------------------------------------------------------------------------------------
    def GetDepsListFromProvenance (self, pkgFramework):
        provFileName = self.m_pkgSource.GetProvenanceFile()
        depList = []
        if os.path.exists (provFileName):
            provFile = open (provFileName, 'rt')
            provLines = provFile.readlines()
            provFile.close()
            for provLine in provLines:
                provSplit = provLine.strip().split('|')
                if len(provSplit) > 1:
                    if provSplit[0] == 'DEP':
                        if len(provSplit) < 5: # need to fudge targetFramework since it is missing
                            depTargetFramework = pkgFramework
                        else:
                            depTargetFramework = provSplit[4]
                        depList.append( (provSplit[1], provSplit[2], depTargetFramework, provSplit[3]) )
        return depList

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class: NuGetPackageBase
    #-------------------------------------------------------------------------------------------
    def GetNugetProvenance (self, pkgFramework):
        provFileName = self.m_pkgSource.GetProvenanceFile()
        if not os.path.exists (provFileName):
            result = utils.GetExactCaseFilePath(provFileName)
            if not result:
                raise utils.BuildError ('Cannot open nuget provenance file {0}\n'.format(provFileName))
            else:
                provFileName = result[0]

        with open (provFileName, 'rt') as provFile:
            provLines = provFile.readlines()
        # First line is the local prov
        for provLine in provLines:
            provSplit = provLine.strip().split('|')
            if len(provSplit) > 1 and provSplit[0] == 'PKG':
                depTargetFramework = provSplit[4] if len(provSplit)>=5 else pkgFramework
                return (provSplit[1], provSplit[2], depTargetFramework, provSplit[3])

        raise utils.BuildError ('Could not find provenance in provenance file {0}\n'.format(provFileName))

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class: NuGetPackageBase
    #-------------------------------------------------------------------------------------------
    def GetAllNugetDepsFromProvenance (self, pkgFramework):
        allDeps = []
        processedDeps = set() # As-found dependencies so we don't revisit.

        targetFramework = pkgFramework if pkgFramework else TARGET_FRAMEWORK_DEFAULT
        (name, version, framework, url) = self.GetNugetProvenance (targetFramework)

        depsToProcess = [(name, version, framework, url, self.m_pkgSource.m_alias)]

        # Since we don't pull dependencies for aliased nugets, we should fetch them as dependencies
        if self.m_pkgSource.m_alias != self.GetName():
            return depsToProcess

        while len(depsToProcess) > 0:
            curDep = depsToProcess.pop()
            processedDeps.add (curDep)
            
            # Need to get the provenance from the prov file rather than the the dependency provenance which may have [] or whatever.
            curProv = self.GetNugetProvenance (targetFramework)
            resolvedDep = (curProv[0], curProv[1], curProv[2], curProv[3], curProv[0])  # Alias is the same as name for dependents

            if not resolvedDep in allDeps:
                allDeps.append (resolvedDep)

            # Get the dependencies
            depList = self.GetDepsListFromProvenance (targetFramework)
            for ldep in depList:
                if not ldep in depsToProcess and not ldep in processedDeps:
                    if self.TargetFrameworkFuzzyMatch(targetFramework, ldep[2]): # filter based on targetFramework
                        utils.showInfoMsg ('Adding ldep {0} targetFramework {1} ldep.targetFramework {2}\n'.format (ldep[0], targetFramework, ldep[2]), utils.INFO_LEVEL_RarelyUseful)
                        depsToProcess.append(ldep)
        return allDeps

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class: NuGetPackageBase
    #-------------------------------------------------------------------------------------------
    def UpdateProvenance (self, pkgFramework, version):
        # Store a provenance file for future use
        provFileName = self.m_pkgSource.GetProvenanceFile()
        provFile = open (provFileName, 'wt')
        if not provFile:
            raise utils.BuildError ('Cannot open nuget provenance file {0} for write\n'.format(provFileName))
        targetFramework = pkgFramework if pkgFramework else TARGET_FRAMEWORK_DEFAULT
        pkgBaseName = '{0}.{1}'.format(self.GetName(), version)
        pkgDirName = os.path.join(self.m_pkgSource.GetLocalDownloadDir(), pkgBaseName)
        #pkgPathName = os.path.join(pkgDirName, '{0}.nupkg'.format(pkgBaseName))
        pkgName, _, pkgDeps = self.GetDepsFromPackage(pkgDirName, targetFramework)
        provLine = '|'.join(['PKG', pkgName, version, self.m_pkgSource.GetRemoteAddress(), targetFramework])
        provFile.write (provLine + '\n')
        # pkgCheckFormat = "{0}/Packages(Id='{1}',Version='{2}')"
        for pkgDep in pkgDeps:
            depFramework = pkgDep.get('targetFramework', targetFramework) # try to get dep framework, but settle for package framework
            provLine = '|'.join(['DEP', pkgDep['id'], pkgDep['version'], self.m_pkgSource.GetRemoteAddress(), depFramework])
            provFile.write (provLine + '\n')
        provFile.close()
        return

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class: NuGetPackageBase
    #-------------------------------------------------------------------------------------------
    def VersionRange (self, versionRaw):
        versionExact = None
        versionMin   = versionRaw
        versionMax   = None
        inclusiveMin = True
        inclusiveMax = False
        if versionRaw:
            #nosuffix, suffix = versionutils.SplitPrereleaseSuffix(versionRaw)
            #versionInp = versionRaw.strip().replace('-', '.') # since may have dash notation coming in
            versionInp = versionRaw.strip()
            versionMin   = versionInp
            inclusiveMax = not versionInp.endswith(')')
            versionInvalPattern = r"^\(([^,]*)\)"
            versionInvalMatch = re.match(versionInvalPattern, versionInp)
            if versionInvalMatch:
                utils.showInfoMsg ("Invalid nuget version specification '{0}'\n".format (versionRaw), utils.INFO_LEVEL_Important)
                #raise utils.BuildError ("Invalid nuget version specification '{0}'".format (versionRaw))
            versionExactPattern = r"^\[([^,]*)\]"
            versionExactMatch = re.match(versionExactPattern, versionInp)
            if versionExactMatch:
                versionExact = versionExactMatch.group(1)
                versionMin   = versionExact
                versionMax   = versionExact
            if not versionExact:
                versionRangePattern = r"^[\[\(]([^,]*),([^\]^\)]*)[\]\)]"
                versionRangeMatch = re.match(versionRangePattern, versionInp)
                if versionRangeMatch:
                    inclusiveMin = not versionInp.startswith('(')
                    versionMin = versionRangeMatch.group(1)
                    versionMax = versionRangeMatch.group(2)
                    if not versionMax.strip() and not inclusiveMax:
                        versionMax = '*'
                    if not versionMin.strip() and not inclusiveMin:
                        versionMin = '*'
                    
        return versionMin, inclusiveMin, versionMax, inclusiveMax

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class: NuGetPackageBase
    #-------------------------------------------------------------------------------------------
    def VersionIsInexact (self, versionRaw):
        versionMin, _, versionMax, _ = self.VersionRange (versionRaw)
        return versionMin != versionMax

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class: NuGetPackageBase
    #-------------------------------------------------------------------------------------------
    def MeetsVersionRequirements(self, versionFoundStr, versionRequired):
        if not versionFoundStr:
            return False
        
        utils.showInfoMsg ("MeetsVersionRequirements required {0} found {1}\n".format (versionRequired, versionFoundStr), utils.INFO_LEVEL_RarelyUseful)
        versionFoundObj = versionutils.VersionWithSuffix(versionFoundStr)
        versionMinStr, inclusiveMin, versionMaxStr, inclusiveMax = self.VersionRange(versionRequired)
        versionMinObj = versionutils.VersionWithSuffix(versionMinStr)
        versionMaxObj = versionutils.VersionWithSuffix(versionMaxStr)

        metMinRequirement = True # assume no minimum
        # If this version is unbound, don't bother comparing
        if versionMinStr and not versionMaxObj.m_unbound:
            if inclusiveMin:
                metMinRequirement = versionFoundObj >= versionMinObj or versionMinObj.MatchVersionPadded(versionFoundObj)
            else:
                metMinRequirement = versionFoundObj >  versionMinObj

        metMaxRequirement = True # assume no maximum
        # If this version is unbound, don't bother comparing
        if versionMaxStr and not versionMaxObj.m_unbound:
            if inclusiveMax:
                metMaxRequirement = versionFoundObj <= versionMaxObj or versionMaxObj.MatchVersionPadded(versionFoundObj)
            else:
                metMaxRequirement = versionFoundObj <  versionMaxObj
        metBothRequirements = metMinRequirement and metMaxRequirement

        # safety valve for earlier behavior of not checking for pre-release packages
        if s_allowPrereleaseAnywhere: # os.environ.get('BB_NUGET_ALLOW_PRERELEASE_ANYWHERE', None)
            utils.showInfoMsg ("MeetsVersionRequirements found BB_NUGET_ALLOW_PRERELEASE_ANYWHERE={}, so NOT excluding prerelease packages\n".format (s_allowPrereleaseAnywhere), utils.INFO_LEVEL_RarelyUseful)
            return metBothRequirements

        # reject pre-release (having suffix) unless specifically requested
        if metBothRequirements:
            prereleaseFound = versionFoundObj.Suffix()
            if prereleaseFound:
                utils.showInfoMsg ("MeetsVersionRequirements found suffix {} on found {}\n".format (prereleaseFound, versionFoundStr), utils.INFO_LEVEL_RarelyUseful)
                wildcardList = [ '[*]', ]
                wildcardRequired = versionRequired in wildcardList
                prereleaseRequired = not wildcardRequired # initial guess
                if not wildcardRequired: # can only check non-wildcard requested versions
                    versionRequiredObj = versionutils.VersionWithSuffix(versionRequired)
                    prereleasseRequired = versionRequiredObj.Suffix()
                    utils.showInfoMsg ("MeetsVersionRequirements found suffix {} on required {}\n".format (prereleasseRequired, versionRequired), utils.INFO_LEVEL_RarelyUseful)
                if not prereleaseRequired:
                    utils.showInfoMsg ("MeetsVersionRequirements rejecting found {} because did not specify pre-release in required {}\n".format(versionFoundStr, versionRequired), utils.INFO_LEVEL_SomewhatInteresting)
                    return False
                else:
                    utils.showInfoMsg ("MeetsVersionRequirements allowing prerelease\n", utils.INFO_LEVEL_TemporaryDebugging)

        return metBothRequirements

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class: NuGetPackageBase
    #-------------------------------------------------------------------------------------------
    def UpdateSource (self, pkgFramework, version, reposDownloaded):
        alreadyDownloaded = False
        if not os.path.exists(self.m_pkgSource.GetLocalPath()):
            with g_packageDownloadLock:
                updatePackage = '|'.join(['PKG', self.GetName(), version])
                if updatePackage in g_changedPackages:
                    alreadyDownloaded = True
                else: g_changedPackages.add (updatePackage)
        if not alreadyDownloaded:
            self.DownloadFromNuGetSource(version, self.m_pkgSource.GetLocalDownloadDir())
        reposDownloaded.append (self.m_pkgSource)
        self.m_pkgSource.Update(version=str(versionutils.VersionWithSuffix(version)))
        self.UpdateProvenance (pkgFramework, version)
        return 0

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class: NuGetPackageBase
    #-------------------------------------------------------------------------------------------
    def Uninstall(self, version):
        pkgName = self.GetName()
        provFileName = self.m_pkgSource.GetProvenanceFile()
        localDir = self.m_pkgSource.GetLocalDownloadDir()
        pkgBaseName = '{0}.{1}'.format(pkgName, version)
        pkgDirName = os.path.join(localDir, pkgBaseName)
        self.m_pkgSource.m_packageDir = None # We need to reset the packageDir since it gets cached and then doesn't get updated after pull
        utils.showInfoMsg ("Uninstalling NuGet package {0} version {1} in directory {2}\n".format (pkgName, version, pkgDirName), utils.INFO_LEVEL_RarelyUseful)
        if os.path.exists(pkgDirName):
            utils.cleanDirectory (pkgDirName, deleteFiles=True)
            os.rmdir (pkgDirName)
        utils.deleteFileWithRetries(provFileName)
        return

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class: NuGetPackageBase
    #-------------------------------------------------------------------------------------------
    def GetAllNuGetVersionsFromDirectory (self, dirName, baseName):
        pattern = r"(.+?)\.(\d+\.\d+\.\d+\.\d+)\.nupkg"
        patternRe = re.compile(pattern)
        foundVersions = []
        for filename in os.listdir(dirName):
            if os.path.isfile (os.path.join(dirName, filename)):
                match = patternRe.search(filename)
                if match and match.group(1).lower() == baseName.lower():
                    foundVersions.append (versionutils.VersionWithSuffix(match.group(2)))
        return foundVersions

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class: NuGetPackageBase
    #-------------------------------------------------------------------------------------------
    def GetPackageFramework (self, curPart):
        pkgName = self.GetName()
        pkgFramework = None
        suggestedTargetFramework = None
        if not curPart.m_nugetBindingList:
            curPart.m_nugetBindingList = self.GetBindingList (pkgName, None, curPart, True)
        for _, _, targetFrameworkDir, _ in  curPart.m_nugetBindingList:
            if not suggestedTargetFramework and targetFrameworkDir and len(targetFrameworkDir) > 0:
                suggestedTargetFramework = self.GetSuggestedTargetFramework(targetFrameworkDir)
        if suggestedTargetFramework:
            pkgFramework = suggestedTargetFramework
        else:
            pkgFramework = TARGET_FRAMEWORK_DEFAULT
            utils.showInfoMsg ('Warning: NuGetPackage {0} on part {1} is missing targetFramework.\n    PartFile: {2}\n'.format (pkgName, curPart.GetShortRepresentation(), curPart.m_info.m_file), utils.INFO_LEVEL_SomewhatInteresting)

        return pkgFramework

#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
class RemoteNugetPackage (NuGetPackageBase):

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class: RemoteNugetPackage
    #-------------------------------------------------------------------------------------------
    def __init__(self, pkgSource):
        NuGetPackageBase.__init__ (self, pkgSource)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class: RemoteNugetPackage
    #-------------------------------------------------------------------------------------------
    def plServUrlPost(self, url, packageName, packageVersion, licenseUrl):
        data = {
                "Name": packageName,
                "Version": packageVersion,
                "Type": "nuget",
                "LicenseUrl": licenseUrl
            }
        return compat.getUrlWithJson (url, data, proxyDict=bbutils.GetHttpProxyDict(url))

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class: RemoteNugetPackage
    #-------------------------------------------------------------------------------------------
    def SearchVersionsFromServer(self, address, isSyncPull = False):
        name = self.GetName()
        httpAuth = self.m_pkgSource.GetAuthenticationHeader ()
        pkgQueryFormat =                               r"{0}/FindPackagesById()?id='{1}'&semVerLevel=2.0.0"
        pkgQueryUrl = pkgQueryFormat.format(address, name)
        pattern = r".*\/Packages\(Id='([^']*)',Version='([^']*)'"
        utils.showInfoMsg ('Finding versions of {0} via {1}\n'.format (name, pkgQueryUrl), utils.INFO_LEVEL_SomewhatInteresting)
        versionsFound = []
        versionsObj = []
        if bbutils.IsRemoteAddress(pkgQueryUrl):
            if pkgQueryUrl in s_pkgQueryCache:
                pkgQueryContent = s_pkgQueryCache[pkgQueryUrl]
                utils.showInfoMsg ("NuGet package {0} versions info found in cache\n".format (name), utils.INFO_LEVEL_RarelyUseful)
            else:
                _, pkgQueryContent = utils.getUrlWithDataWithRetries(pkgQueryUrl, None, {}, httpAuth)
                s_pkgQueryCache[pkgQueryUrl] = pkgQueryContent
            if pkgQueryContent:
                xmldoc = minidom.parseString(pkgQueryContent)
                if xmldoc:
                    feeds = utils.getDomElementsByName(xmldoc, 'feed')
                    if len(feeds) < 1:
                        utils.showInfoMsg ("NuGet package {0} entry has no feed\n".format (name), utils.INFO_LEVEL_RarelyUseful)
                        return versionsFound
                    entries = utils.getDomElementsByName(feeds[0], 'entry')
                    for entry in entries:
                        ids = utils.getDomElementsByName(entry, 'id')
                        if len(ids) < 1:
                            utils.showInfoMsg ("NuGet package {0} entry has no id\n".format (name), utils.INFO_LEVEL_RarelyUseful)
                        else:
                            idValue = ids[0].childNodes[0].nodeValue
                            match = re.search(pattern, idValue)
                            if match:
                                if match.group(1).lower() == name.lower(): # package Id must match exactly; Search brings back other Ids too
                                    # NB: version OR suffix may also include "+" delimited semver 2.0 "build metadata" which we must ignore!
                                    versionStr, suffix = versionutils.SplitPrereleaseSuffix(match.group(2))
                                    if suffix:
                                        versionStr = versionStr + suffix
                                    versionsObj.append(versionutils.VersionWithSuffix(versionStr))
                                    utils.showInfoMsg ("Found        package {0} version '{1}'\n".format (name, versionStr), utils.INFO_LEVEL_RarelyUseful)
        elif isSyncPull:
            version = self.GetVersionFromProvenanceFile(os.path.join(address, name + '.provenance.txt'))
            if version:
                versionsObj.append(version)
        else:
            versionsObj.extend (self.GetAllNuGetVersionsFromDirectory(address, self.GetName()))
            # pattern = r".+?\.(\d+\.\d+\.\d+\.\d+)\.nupkg"
            # for filename in os.listdir(os.path.join(address, name)):
            #     match = re.search(pattern, filename)
            #     if match:
            #         versionsObj.append(versionutils.VersionWithSuffix(match.group(1)))
        versionsSorted = sorted(versionsObj, reverse=DEPENDENCY_VERSION_HIGHEST)
        versionsFound = [str(v) for v in versionsSorted]
        return versionsFound

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class: RemoteNugetPackage
    #-------------------------------------------------------------------------------------------
    # Actually gets the package via HTTP or rsync and unzip contents
    def GetPackage(self, pkgGetUrl, pkgName, pkgPathName, version, localDir, httpAuth = None):
        if bbutils.IsRemoteAddress(pkgGetUrl):
            utils.showStatusMsg ('Downloading package {0} from {1}\n'.format (pkgName, pkgGetUrl), utils.INFO_LEVEL_Interesting)
            httpCode, pkgGetContent = utils.getUrlWithDataWithRetries(pkgGetUrl, None, {}, httpAuth)
            if 200 != httpCode:
                # special case for semver 2.0 versions with "build metdata" appended
                urlParts = pkgGetUrl.rsplit('/', 1) # version part of typical download url is after final slash
                if len(urlParts) > 1:
                    versionStr, buildMetadata = versionutils.SplitBuildMetadataFrom(urlParts[1])
                    if buildMetadata:
                        utils.showInfoMsg ('Retrying package {} without semver 2.0 build metadata {}\n'.format (pkgName, buildMetadata), utils.INFO_LEVEL_RarelyUseful)
                        httpCode, pkgGetContent = utils.getUrlWithDataWithRetries('/'.join([urlParts[0], versionStr]), None, {}, httpAuth)
            if pkgGetContent:
                pkgFile = open(pkgPathName, 'wb')
                if not pkgFile:
                    raise utils.BuildError ('Cannot open package file {0} for write\n'.format(pkgPathName))
                pkgFile.write(pkgGetContent)
                utils.showInfoMsg ('Wrote Nuget Package {0} {1} to file {2}\n'.format (pkgName, version, pkgPathName), utils.INFO_LEVEL_RarelyUseful)
                pkgFile.close()
            else:
                raise utils.BuildError ('Cannot download package {0} version {1} from remote URL {2} to localDir {3} http returned {4}\n'.format(pkgName, version, pkgGetUrl, localDir, httpCode))
        else:
            utils.showInfoMsg ('Copying package {0} from {1} to {2}\n'.format (pkgName, pkgGetUrl, localDir), utils.INFO_LEVEL_SomewhatInteresting)
            status = cmdutil.roboCopyFiles(pkgGetUrl, localDir)
            if status != 0:
                raise utils.BuildError ('Cannot copy package {0} version {1} from directory {2} to localDir {3}\n'.format(pkgName, version, pkgGetUrl, localDir))

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class: RemoteNugetPackage
    #-------------------------------------------------------------------------------------------
    # Gets and uzips package, but with locking
    def GetUnzipPackage(self, pkgGetUrl, pkgName, pkgPathName, version, localDir, pkgDirName, httpAuth = None):
        lock = s_downloadLocks.get(pkgGetUrl, None)
        if not lock:
            s_downloadLocks[pkgGetUrl] = lock = threading.RLock()
            utils.showInfoMsg ('Created lock to download package {} version {} from {}\n'.format (pkgName, version, pkgGetUrl), utils.INFO_LEVEL_RarelyUseful)
        utils.showInfoMsg ('Waiting for lock to download package {} version {} from {}\n'.format (pkgName, version, pkgGetUrl), utils.INFO_LEVEL_RarelyUseful)
        with lock:
            self.GetPackage(pkgGetUrl, pkgName, pkgPathName, version, localDir, httpAuth)
            self.UnzipPackage(pkgDirName, pkgPathName)
        utils.showInfoMsg ('Released lock to download package {} version {} from {}\n'.format (pkgName, version, pkgGetUrl), utils.INFO_LEVEL_RarelyUseful)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class: RemoteNugetPackage
    #-------------------------------------------------------------------------------------------
    def _GetV2MetadataXml(self, url_registry, package_id, version_text, httpAuth = None):
        co_name = sys._getframe().f_code.co_name # pylint: disable=protected-access
        utils.showInfoMsg('{} url={} package={} version={}\n'.format(co_name, url_registry, package_id, version_text), utils.INFO_LEVEL_RarelyUseful)
        cache_key = '/'.join([url_registry.lower(), package_id.lower(), version_text.lower()])
        xmldoc = s_downloadXmls.get(cache_key, None)
        if xmldoc:
            utils.showInfoMsg('{} returning xml from cache\n'.format(co_name), utils.INFO_LEVEL_RarelyUseful)
            return xmldoc, 200
        url_metadata = "{}/Packages(Id='{}',Version='{}')".format(url_registry, package_id, version_text)
        http_code, nuspec_str = utils.getUrlWithDataWithRetries(url_metadata, None, {}, httpAuth)
        if 200 == http_code:
            xmldoc = minidom.parseString(nuspec_str)
            s_downloadXmls[cache_key] = xmldoc
            utils.showInfoMsg('{} added xml to cache\n'.format(co_name), utils.INFO_LEVEL_RarelyUseful)
        return xmldoc, http_code

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class: RemoteNugetPackage
    #-------------------------------------------------------------------------------------------
    def _GetV2DownloadUrl(self,url_registry, package_id, version_text, raise_error, httpAuth=None):
        co_name = sys._getframe().f_code.co_name # pylint: disable=protected-access
        utils.showInfoMsg('{} url={} package={} version={}\n'.format(co_name, url_registry, package_id, version_text), utils.INFO_LEVEL_RarelyUseful)
        cache_key = '/'.join([url_registry.lower(), package_id.lower(), version_text.lower()])
        # TODO: implement caching even if the query fails
        content_src = s_downloadUrls.get(cache_key, None)
        if content_src:
            utils.showInfoMsg('{} returning url {} from cache\n'.format(co_name, content_src), utils.INFO_LEVEL_RarelyUseful)
            return content_src
        xmldoc, http_code = self._GetV2MetadataXml(url_registry, package_id, version_text, httpAuth)
        utils.showInfoMsg('_GetV2MetadataXml return code {}\n'.format(http_code), utils.INFO_LEVEL_RarelyUseful)
        if 200 == http_code:
            # VERY important for finding correct download URL!
            content_list = xmldoc.getElementsByTagName('content')
            if len(content_list) > 0:
                attribute_dict = content_list[0].attributes
                if attribute_dict and len(attribute_dict) > 0:
                    for attribute in attribute_dict.keys():
                        attr_key = str(attribute).strip()
                        if attr_key.lower() == 'src':
                            utils.showInfoMsg('{} found src "{}"\n'.format(co_name, attribute_dict[attr_key].value), utils.INFO_LEVEL_RarelyUseful)
                            content_src = attribute_dict[attr_key].value
        if content_src:
            s_downloadUrls[cache_key] = content_src
            utils.showInfoMsg('{} added url {} to cache\n'.format(co_name, content_src), utils.INFO_LEVEL_RarelyUseful)
        else:
            err = '{} cannot find download url for package {} version {} from registry {}\n'.format(co_name, package_id, version_text, url_registry)
            if raise_error:
                raise utils.BuildError (err)
            else:
                utils.showInfoMsg(err, utils.INFO_LEVEL_RarelyUseful)
        return content_src

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class: RemoteNugetPackage
    #-------------------------------------------------------------------------------------------
    def _GetV2PackageVersion(self, url_registry, package_id, version_text, raise_error, httpAuth=None):
        utils.showInfoMsg('{} url={} package={} version={}\n'.format('_GetV2PackageVersion', url_registry, package_id, version_text), utils.INFO_LEVEL_RarelyUseful)
        cache_key = '/'.join([url_registry.lower(), package_id.lower(), version_text.lower()])
        # TODO: implement caching even if the query fails
        packageVersionFound = s_packageVersions.get(cache_key, None)
        if packageVersionFound:
            utils.showInfoMsg('{} returning url {} from cache\n'.format('_GetV2PackageVersion', packageVersionFound), utils.INFO_LEVEL_RarelyUseful)
            return packageVersionFound
        xmldoc, http_code = self._GetV2MetadataXml(url_registry, package_id, version_text, httpAuth)
        utils.showInfoMsg('_GetV2MetadataXml return code {}\n'.format(http_code), utils.INFO_LEVEL_RarelyUseful)
        if 200 == http_code:
            nugetVersion = xmldoc.getElementsByTagName('d:Version')
            if len(nugetVersion) > 0:
                packageVersionFound = nugetVersion[0].firstChild.nodeValue
        if packageVersionFound:
            s_packageVersions[cache_key] = packageVersionFound
            utils.showInfoMsg('{} added package version {} to cache\n'.format('_GetV2PackageVersion', packageVersionFound), utils.INFO_LEVEL_RarelyUseful)
        else:
            err = '{} cannot find package version for package {} version {} from registry {}\n'.format('_GetV2PackageVersion', package_id, version_text, url_registry)
            if raise_error:
                raise utils.BuildError (err)
            else:
                utils.showInfoMsg(err, utils.INFO_LEVEL_RarelyUseful)
        return packageVersionFound

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class: RemoteNugetPackage
    #-------------------------------------------------------------------------------------------
    # get package via HTTP or rsync and unzip contents
    def GetDownloadUrl(self, pkgAddress, pkgName, version, raise_error, httpAuth=None):
        pkgGetUrl = self._GetV2DownloadUrl(pkgAddress, pkgName, version, raise_error, httpAuth)
        return pkgGetUrl

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class: RemoteNugetPackage
    #-------------------------------------------------------------------------------------------
    # get package version via HTTP or rsync
    def GetPackageVersion(self, version):
        packageVersion = self._GetV2PackageVersion(self.m_pkgSource.GetRemoteAddress(), self.GetName(), version, False, self.m_pkgSource.GetAuthenticationHeader())
        return packageVersion

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class: RemoteNugetPackage
    #-------------------------------------------------------------------------------------------
    # get package via HTTP or rsync and unzip contents
    def DownloadFromNuGetSource(self, version, localDir):
        pkgAddress = self.m_pkgSource.GetRemoteAddress()
        pkgName = self.GetName()
        pkgBaseName = '{0}.{1}'.format(pkgName, version)
        pkgGetUrl = self.GetDownloadUrl(pkgAddress, pkgName, version, True, self.m_pkgSource.GetAuthenticationHeader())
        pkgDirName = os.path.join(localDir, pkgBaseName)
        pkgPathName = os.path.join(pkgDirName, '{0}.nupkg'.format(pkgBaseName))
        symlinks.makeSureDirectoryExists(pkgDirName)
        self.GetUnzipPackage(pkgGetUrl, pkgName, pkgPathName, version, localDir, pkgDirName, self.m_pkgSource.GetAuthenticationHeader())

        return pkgPathName

#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
class LocalNugetPackage (NuGetPackageBase):
    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class: LocalNugetPackage
    #-------------------------------------------------------------------------------------------
    def __init__(self, pkgSource):
        NuGetPackageBase.__init__ (self, pkgSource)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class: LocalNugetPackage
    #-------------------------------------------------------------------------------------------
    def GetPackageVersion(self, version):
        dirToCheck = self.m_pkgSource.GetFeed().m_address
        fileToFind = '{0}.{1}'.format(self.GetName(), version)

        for pkgName in os.listdir (dirToCheck):
            pkgFileName = os.path.join(dirToCheck, pkgName)
            if os.path.isfile (pkgFileName) and pkgFileName.lower() == fileToFind.lower():
                return version
        return None

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class: LocalNugetPackage
    #-------------------------------------------------------------------------------------------
    def SearchVersionsFromServer(self, address, isSyncPull = False):
        versionsObj = []
        if isSyncPull:
            version = self.GetVersionFromProvenanceFile(os.path.join(address, self.GetName() + '.provenance.txt'))
            if version:
                versionsObj.append(version)
        else:
            versionsObj.extend (self.GetAllNuGetVersionsFromDirectory(address, self.GetName()))

        versionsSorted = sorted(versionsObj, reverse=DEPENDENCY_VERSION_HIGHEST)
        versionsFound = [str(v) for v in versionsSorted]
        return versionsFound

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class: LocalNugetPackage
    #-------------------------------------------------------------------------------------------
    def DownloadFromNuGetSource(self, version, localDir):
        pkgBaseName = '{0}.{1}'.format(self.GetName(), version)
        pkgDirName = os.path.join(localDir, pkgBaseName)
        pkgPathName = os.path.join(pkgDirName, '{0}.nupkg'.format(pkgBaseName))
        symlinks.makeSureDirectoryExists(pkgDirName)

        localPkg = os.path.join (self.m_pkgSource.GetRemoteAddress(), pkgBaseName + '.nupkg')

        symlinks.createFileSymLink (pkgPathName, localPkg, checkSame=True)
        self.UnzipPackage (pkgDirName, pkgPathName)

        return pkgPathName

#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
class LooseFileNugetPackage (NuGetPackageBase):
    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class: LooseFileNugetPackage
    #-------------------------------------------------------------------------------------------
    def __init__(self, pkgSource):
        NuGetPackageBase.__init__ (self, pkgSource)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class: LooseFileNugetPackage
    #-------------------------------------------------------------------------------------------
    def GetPackageVersion(self, version):
        dirToCheck = os.path.join (self.m_pkgSource.GetFeed().m_address, self.GetName())
        nameLower = self.GetName().lower()

        for pkgName in os.listdir (dirToCheck):
            pkgFileName = os.path.join(dirToCheck, pkgName)
            if os.path.isfile (pkgFileName) and pkgName.lower().startswith(nameLower) and pkgName.lower().endswith('.nuspec'):
                xmldoc = minidom.parse(pkgFileName)
                version = xmldoc.getElementsByTagName('version')[0].firstChild.nodeValue
                return version
        return None

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class: LooseFileNugetPackage
    #-------------------------------------------------------------------------------------------
    def SearchVersionsFromServer(self, address, isSyncPull = False):
        versionsObj = []
        if isSyncPull:
            version = self.GetVersionFromProvenanceFile(os.path.join(address, self.GetName() + '.provenance.txt'))
            if version:
                versionsObj.append(version)
        else:
            foundVer = self.GetPackageVersion (None)
            if foundVer:
                versionsObj.append (versionutils.VersionWithSuffix(foundVer))

        versionsSorted = sorted(versionsObj, reverse=DEPENDENCY_VERSION_HIGHEST)
        versionsFound = [str(v) for v in versionsSorted]
        return versionsFound

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class: LooseFileNugetPackage
    #-------------------------------------------------------------------------------------------
    def DownloadFromNuGetSource(self, version, localDir):
        pkgBaseName = '{0}.{1}'.format(self.GetName(), version)
        pkgDirName = os.path.join(localDir, pkgBaseName)
        localFilesDir = os.path.join (self.m_pkgSource.GetRemoteAddress(), self.GetName())
        symlinks.createDirSymLink (pkgDirName, localFilesDir, checkSame=True)
        return pkgDirName

#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
class Nuspec(object):
    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class: NuSpec
    #-------------------------------------------------------------------------------------------
    def __init__(self, path, name, _version, description, platformTag, isNative):
        # NuGet removes leading 0s, so we'll do the same here
        self.m_version = None
        self.m_path = path
        self.m_name = name
        self.m_description = description
        self.m_dependencies = dict() # key is targetFramwork, value is a list of (names, version) of other products to be added to be set as dependents
        self.m_files = set() # these should be a list of the source of the dependencies... i.e. %outroot%build\...\
        self.m_platformTag = platformTag
        self.m_isNative = isNative

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class: NuSpec
    #  NuGet removes leading 0s, so we'll do the same here
    #-------------------------------------------------------------------------------------------
    @staticmethod
    def FormatVersionString(version):
        return '.'.join(['0' if len(x.lstrip('0')) < 1 else x.lstrip('0') for x in version.split('.')])

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class: NuSpec
    #-------------------------------------------------------------------------------------------
    def GetVersion(self):
        # We don't want to store the version in the object until it's needed; otherwise we need to be able
        #  to purge the part cache if someone changes just the version.
        if not self.m_version:
            (relV, majV, minV, subminV) = utils.GetVersionForProvenance()
            version = "{0}.{1}.{2}.{3}".format(relV, majV, minV, subminV)
            self.m_version = Nuspec.FormatVersionString(version)
        return self.m_version

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class: NuSpec
    #-------------------------------------------------------------------------------------------
    def AddFiles(self, files, target = None):
        if target is None:
            target = "\\"
        else:
            # normalizes paths from part files; helps prevent duplicate entries in nuspec files which result in bad archives
            target = target.rstrip('\\/')

        files = os.path.abspath (files)

        utils.showInfoMsg ("Adding to {0} nuspec, {1}: {2} : {3}\n".format(self.m_name, self.m_path, files, target), utils.INFO_LEVEL_Interesting)
        self.m_files.add((files, target))

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class: NuSpec
    #-------------------------------------------------------------------------------------------
    def AddDependency(self, name, version, targetFramework = None):
        if (targetFramework and ("native" in targetFramework)):
            utils.showInfoMsg("Not adding dependency {0}.{1} to nuspec because it is a native dependency.".format(name, version), utils.INFO_LEVEL_Interesting)
            return

        if targetFramework not in self.m_dependencies:
            self.m_dependencies[targetFramework] = set()
        
        self.m_dependencies[targetFramework].add((name, version))

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class: NuSpec
    #-------------------------------------------------------------------------------------------
    def GetFileName(self):
        name = "{0}.{1}.nuspec".format(self.m_name, self.GetVersion())
        return os.path.join(self.m_path, name)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class: NuSpec
    #-------------------------------------------------------------------------------------------
    def GetTargetFrameworks(self):
        targetFrameworks = []

        for targetFramework in self.m_dependencies:
            if targetFramework is not None and targetFramework not in targetFrameworks:
                targetFrameworks.append(targetFramework)

        for curfile in self.m_files:
            targetFramework = GetTargetFrameworkFromPath(curfile[1])
            if targetFramework is not None and targetFramework not in targetFrameworks:
                targetFrameworks.append(targetFramework)

        return targetFrameworks

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class: NuSpec
    #-------------------------------------------------------------------------------------------
    def WriteDependencies(self, dom, node):
        for targetFramework in self.m_dependencies:
            groupElm = dom.createElement("group")

            if targetFramework is not None:
                groupElm.setAttribute("targetFramework", targetFramework)

            # Native nugets are just zips; the dependencies don't get used. We don't write native dependencies so this is symmetric.
            if not self.m_isNative:
                for dependency, version in self.m_dependencies[targetFramework]:
                    dependElm = dom.createElement("dependency")
                    dependElm.setAttribute("id", dependency)
                    dependElm.setAttribute("version", "{0}".format(version))
                    groupElm.appendChild(dependElm)

            node.appendChild(groupElm)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class: NuSpec
    #-------------------------------------------------------------------------------------------
    def WriteToNuspec(self, dom, node, licenseFile=None):
        metadataElm = dom.createElement("metadata")

        licenseData = licenseFile

        # id
        idElm = dom.createElement("id")
        idTxt = dom.createTextNode(licenseData['name'] + "_{0}".format(self.m_platformTag) if licenseData else self.m_name)
        idElm.appendChild(idTxt)
        metadataElm.appendChild(idElm)

        # version
        versionElm = dom.createElement("version")
        versionTxt = dom.createTextNode(licenseData['version'] if licenseData and 'version' in licenseData and licenseData['version'] else self.GetVersion())
        versionElm.appendChild(versionTxt)
        metadataElm.appendChild(versionElm)

        # authors
        authorElm = dom.createElement("authors")
        authorTxt = dom.createTextNode(licenseData['authors'] if licenseData else "Bentley Systems, Incorporated")
        authorElm.appendChild(authorTxt)
        metadataElm.appendChild(authorElm)

        # owners
        ownerElm = dom.createElement("owners")
        ownerTxt = dom.createTextNode(licenseData['authors'] if licenseData else "Bentley Systems, Incorporated")
        ownerElm.appendChild(ownerTxt)
        metadataElm.appendChild(ownerElm)

        # licenseUrl
        licenseElm = dom.createElement("licenseUrl")
        licenseTxt = dom.createTextNode(licenseData['licenseUrl'] if licenseData else internal.EULA_PDF)
        licenseElm.appendChild(licenseTxt)
        metadataElm.appendChild(licenseElm)

        # copyright
        cpyrightElm = dom.createElement("copyright")
        if licenseData:
            cpyrightTxt = dom.createTextNode(u"{0} License: {1}".format(licenseData['copyright'], licenseData['SPDX-ID']))
        else:
            cpyrightTxt = dom.createTextNode(r"Copyright (c) {0} Bentley Systems, Incorporated. All rights reserved.".format(datetime.datetime.now().year))
        cpyrightElm.appendChild(cpyrightTxt)
        metadataElm.appendChild(cpyrightElm)

        # description
        descElm = dom.createElement("description")
        descTxt = dom.createTextNode(licenseData['description'] if licenseData else (self.m_description if self.m_description else (self.m_name + " " + self.GetVersion())))
        descElm.appendChild(descTxt)
        metadataElm.appendChild(descElm)

        # language
        langElm = dom.createElement("language")
        langTxt = dom.createTextNode("en-US")
        langElm.appendChild(langTxt)
        metadataElm.appendChild(langElm)

        if licenseData:
            # projectUrl
            projectUrlElm = dom.createElement("projectUrl")
            projectUrlTxt = dom.createTextNode(licenseData['homepage'])
            projectUrlElm.appendChild(projectUrlTxt)
            metadataElm.appendChild(projectUrlElm)

        # Dependencies
        dependElm = dom.createElement("dependencies")
        self.WriteDependencies(dom, dependElm)
        metadataElm.appendChild(dependElm)

        node.appendChild(metadataElm)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class: NuSpec
    #-------------------------------------------------------------------------------------------
    def WriteFiles(self, dom, node, baseDir):
        filesElm = dom.createElement("files")
    
        for src, target in self.m_files:
            # Convert src to a relative path
            sourceFile = os.path.relpath (src, baseDir)
            fileElm = dom.createElement("file")
            fileElm.setAttribute("src", sourceFile)
            fileElm.setAttribute("target", target)
            filesElm.appendChild(fileElm)

        node.appendChild(filesElm)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class: NuSpec
    #-------------------------------------------------------------------------------------------
    def WriteToXml(self, licenseFilePath=None):
        fileName = os.path.abspath(self.GetFileName())
        utils.showInfoMsg ("Writing nuspec {0}\n".format(fileName), utils.INFO_LEVEL_Interesting)

        licenseFile = None
        if licenseFilePath:
            with open(licenseFilePath, 'rt', encoding='utf-8') as licFile:
                licenseFile = json.load(licFile)

        try:
            baseDir = os.path.dirname(fileName)
            symlinks.makeSureDirectoryExists(baseDir)

            nuspecDom = minidom.Document()

            pkgNode = nuspecDom.createElement("package")
            pkgNode.setAttribute("xmlns", "http://schemas.microsoft.com/packaging/2011/08/nuspec.xsd")
            nuspecDom.appendChild(pkgNode)

            self.WriteToNuspec(nuspecDom, pkgNode, licenseFile)
            self.WriteFiles(nuspecDom, pkgNode, baseDir)

            with open(fileName, 'w', encoding='utf8') as nuspecFile:
                nuspecDom.writexml(nuspecFile, "", "    ", "\n", "UTF-8")
            nuspecDom.unlink()

        except Exception as err:
            return 1, "Error generating nuspec file: {0}".format (err)

        return 0, "Succeeded"

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def __getNugetExe():
    if os.name == 'nt':
        if 'NugetPath' in os.environ:
            return os.path.join (os.environ['NugetPath'], 'nuget.exe')

        try:
            return utils.GetToolFile (globalvars.TOOLS_NUGET, os.path.join('tools', 'nuget.exe'))
        except utils.BuildError:
            pass

        if 'ToolCache' in os.environ:
            cwExe = os.path.join (buildpaths.GetToolCacheSourceRoot(), 'bsitools', 'nuget', 'nuget.exe')
            if os.path.exists (cwExe):
                return cwExe
            utils.showInfoMsg ("Cannot find nuget in ToolCache\n", utils.INFO_LEVEL_VeryInteresting)

        outputToolsNugetPath = os.path.join(buildpaths.GetToolsOutputRoot(), 'NuGet.CommandLine', 'tools', 'nuget.exe')
        if os.path.exists(outputToolsNugetPath):
            return outputToolsNugetPath

        import distutils.spawn
        nugetPath = distutils.spawn.find_executable("nuget.exe")
        if nugetPath:
            return nugetPath

        raise utils.PartPullError ("Cannot find nuget in SrcRoot/bsitools, NugetPath, ToolCache, or PATH\n", None)

    elif os.name == 'posix':
        return os.getenv('NugetCommandPath', 'nuget')

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def RunNuget(nuspec, destPath):
    nugetExe = __getNugetExe()

    def outProc (outputLine):
        utils.showInfoMsg (outputLine, utils.INFO_LEVEL_SomewhatInteresting)
        outputLine = outputLine.strip()

        if 'Could not find' in outputLine or 'Cannot create a package' in outputLine or 'not a valid' in outputLine:
            raise utils.PartBuildError("Failed to create nuget package: {0}\n".format(outputLine), None)
        else:
            pass

    symlinks.makeSureDirectoryExists (destPath)

    cmd = [nugetExe]
    cmd.extend(['pack'])
    cmd.extend([nuspec.GetFileName()])
    cmd.extend(['-OutputDirectory', destPath])

    cmdutil.runAndWait (cmd, outputProcessor=outProc)

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def PushToNuget(nugetPath, apiKey, nugetServer):
    nugetExe = __getNugetExe()
    allOutput = []

    def outProc (outputLine):
        knownErrors = ['Pushing took too long', 'File does not exist', 'Forbidden', 'Please provide credentials', 'InternalServerError', ]
        utils.showInfoMsg (outputLine, utils.INFO_LEVEL_SomewhatInteresting)
        outputLine = outputLine.strip()
        allOutput.append(outputLine)

        for known in knownErrors:
            if known in outputLine:
                raise utils.PartBuildError("Failed to push nuget package: {0}\n".format(outputLine), None)
            else:
                pass

    localEnv = os.environ.copy()

    # Set credentials for nugets that are pushed to ADO server using Azure CLI
    if "dev.azure.com" in nugetServer:
        authHeader = azurecli.GetAuthenticationHeader()
        organization = nugetServer[nugetServer.find("dev.azure.com")+14:nugetServer.find("/_packaging")]
        authToken = azuredevopsrestapi.GetSessionToken(authHeader, organization, azuredevopsrestapi.SCOPE_PACKAGING_WRITE)
        endpointDefinition = "{{\"endpointCredentials\": [{{\"endpoint\":\"{0}\", \"username\":\"optional\", \"password\":\"{1}\"}}]}}".format(nugetServer, authToken)
        utils.showInfoMsg ("Setting VSS_NUGET_EXTERNAL_FEED_ENDPOINTS to {0}\n".format(endpointDefinition.replace(authToken, "**replaced in logging for security purposes**" if utils.isUnattendedBuild() else authToken)), utils.INFO_LEVEL_RarelyUseful)
        utils.appendEnvVar (localEnv, "VSS_NUGET_EXTERNAL_FEED_ENDPOINTS", endpointDefinition)

    cmd = [nugetExe]
    cmd.extend(['push'])
    cmd.extend([nugetPath])
    cmd.extend(['-Source', nugetServer, '-verbosity', 'detailed', '-apiKey', apiKey, '-Timeout', '1800'])
    #e.g. nuget.exe push TestPackage.1.1.0.nupkg -Source http://nuget.myserver.com/nuget/Default -verbosity detailed -apiKey <KEY> -Timeout 1800
    status = cmdutil.runAndWait (cmd, outputProcessor=outProc, procEnv=localEnv)
    utils.showInfoMsg ('{} returned status {}\n'.format(cmd, status), utils.INFO_LEVEL_RarelyUseful)
    return status, allOutput



