#---------------------------------------------------------------------------------------------
#  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
#  See LICENSE.md in the repository root for full copyright notice.
#---------------------------------------------------------------------------------------------
import datetime, json, os, re, stat, sys, copy, threading
import xml.parsers.expat

from . import bbutils, bdfcache, buildpaths, compat, symlinks, cmdutil, utils, repository, builddescriptionfile, targetplatform, azuredevopsrestapi
from . import lkgs, nugetpkg, translationkit, globalvars, universalpkg, variableexpander, versionutils, azurecli, internal

if compat.py3:
    import pickle
else:
    import cPickle as pickle

provenanceErrors = ['warn', 'stop']
buildOptions     = ['always', 'never', 'once']
pullOptions      = ['always', 'ifnecessary']

ERROR_OPTION_Stop       = 0
ERROR_OPTION_UseLkg     = 1
ERROR_OPTION_Continue   = 2
onErrorOptions   = ['stop', 'uselkg', 'continue']

DEFER_OPTION_Normal     = 0
DEFER_OPTION_Never      = 'Never'

STRATEGY_VERSION = 13

s_createVersionLock = threading.Lock()
g_packageDownloadLock = threading.Lock()

g_changedPackages = set()

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def ensureStrategyDir ():
    if not utils.isBsi():
        return

    if "SrcRoot" not in os.environ:
        raise utils.StrategyError ('Error: Environment variable "SrcRoot" must be defined; did you forget to run your environment batch file / shell script?')

    stratDir = GetDefaultStrategyDir()
    if not os.path.exists (stratDir):
        raise utils.StrategyError ('"Error: BuildStrategies repository ({0}) does not exist. Do you need to bootstrap?\n'.format(stratDir))

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def IsValidDeferValue (value):
    return value == DEFER_OPTION_Never or re.match (r'^[0-9]+$', value)

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def GetDefaultStrategyDir ():
    basePath = os.environ['BB_BASE_STRAT_DIR'] if 'BB_BASE_STRAT_DIR' in os.environ else "${SrcRoot}" + os.sep + "BuildStrategies"
    return symlinks.normalizePathName(basePath)

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def checkForStratFile (val):
    if os.path.isfile(val):
        return val

    pathVar = "BuildStrategyPath"
    if pathVar not in os.environ:
        return None

    for path in os.environ[pathVar].split(";"):
        test = os.path.join (path, val)
        if os.path.isfile(test):
            return test
    return None

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def findBuildStratFile (val, rootName):
    expandedVal = os.path.expandvars(val)
    test = checkForStratFile (expandedVal)
    if test: return test

    drive, tail = os.path.splitdrive (expandedVal)
    _, ext = os.path.splitext (tail)
    if "" != drive and '.xml' == ext.lower():  # Full path was provided; nothing good will come of testing further.
        raise utils.StrategyError ("Can't open BuildStrategy File '{0}'\n".format (expandedVal))

    val = val + ".BuildStrategy.xml"
    test = checkForStratFile (val)
    if test: return test

    if None != rootName:
        rootDir = os.path.dirname (rootName)
        newval = symlinks.normalizePathName(os.path.join (rootDir, val))
        if os.path.isfile (newval):
            return newval

    # Check base build strategies
    defaultStrategyPath = GetDefaultStrategyDir()
    test = os.path.join (defaultStrategyPath, val)    
    if os.path.isfile(test):
        return test

    # Also look in Branches directory
    test = os.path.join (defaultStrategyPath, "branches", val)    
    if os.path.isfile(test):
        return test

    # And in Teams directory
    test = os.path.join (defaultStrategyPath, "teams", val)    
    if os.path.isfile(test):
        return test

    # Return this so user will get a better error message.
    return os.path.join (defaultStrategyPath, val)

#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
class SdkSource (lkgs.LKGSource):
    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def __init__(self, name, sourceType, prgServer, devServer, version, platforms, prgOutputDir, productDir, streamName):
        self.m_prgOutputDir = prgOutputDir
        self.m_streamName = streamName
        self.m_productDir = productDir
        self.m_versionDir = None
        lkgs.LKGSource.__init__ (self, name, sourceType, prgServer, devServer, version, platforms)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def Update (self, **kwargs): # prgOutputDir, productDir, streamName

        def GetValue (name, default):
            val = kwargs.get (name, None)
            return val if val else default # Val can be '' or None

        self.m_prgOutputDir =   GetValue ('prgOutputDir', self.m_prgOutputDir)
        self.m_productDir =     GetValue ('productDir', self.m_productDir)
        self.m_streamName =     GetValue ('streamName', self.m_streamName)

        lkgs.LKGSource.Update (self, **kwargs)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetPrgOutputDir (self):
        return self.m_prgOutputDir

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def ResolveVersion (self):
        # Set up the version information.  For compatability with LkgSource
        utils.showInfoMsg ("Using SdkSource version from strategy {0}={1}\n".format (self.m_name, self.m_version), utils.INFO_LEVEL_SomewhatInteresting)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def UpdateVersion (self, newVersion):
        # Update with a version from a BDF
        self.m_version = newVersion

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetLocalBaseDir (self, platform):
        return os.path.join (buildpaths.GetSdkSourceLocation(), platform.GetDirName(), self.m_name)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetLocalSdkDir (self, platform, version):
        if not version:
            version = self.m_version
        return os.path.join (self.GetLocalBaseDir(platform), version, self.m_productDir)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetCurrentSdkDir (self, platform):
        if not self.m_versionDir:
            verList = os.listdir (self.GetLocalBaseDir(platform))
            verList.sort()
            self.m_versionDir = verList[-1]

        return os.path.join (self.GetLocalBaseDir (platform), self.m_versionDir)

#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
class NpmFeed (object):
    #---------------------------------------------------------------------------------------
    # bsimethod
    #---------------------------------------------------------------------------------------
    def __init__(self, name, address, provider):
        self.m_name = name
        self.m_address = address
        self.m_provider = provider

    #---------------------------------------------------------------------------------------
    # bsimethod
    #---------------------------------------------------------------------------------------
    def GetName(self):
        return self.m_name

    #---------------------------------------------------------------------------------------
    # bsimethod
    #---------------------------------------------------------------------------------------
    def GetRemoteAddress (self):
        return self.m_address

    #---------------------------------------------------------------------------------------
    # bsimethod
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
class ADOBuildArtifactSource (object):
    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:ADOBuildArtifactSource
    #-------------------------------------------------------------------------------------------
    def __init__(self, alias, artifactName, version, definitionId, project, organization, buildId = None):
        self.m_alias = alias
        self.m_artifactName = artifactName
        self.m_origVersion = version
        self.m_version = None if version == None or '*' in version else version
        self.m_definitionId = definitionId
        self.m_project = project
        self.m_organization = organization
        self.m_buildId = buildId

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:ADOBuildArtifactSource
    #-------------------------------------------------------------------------------------------
    def UpdateProvenance (self):
        # Store a provenance file for future use
        provFileName = self.GetProvenanceFile()
        
        with open (provFileName, 'wt') as provFile:
            provLine = '|'.join(['ADOBUILDARTIFACT', self.m_artifactName, str(self.m_buildId), str(self.m_definitionId), self.m_project, self.m_organization])
            provFile.write (provLine + '\n')
            provFile.close()
        return

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:ADOBuildArtifactSource
    #-------------------------------------------------------------------------------------------
    def ResolveDownloadInfo (self):
        if not self.m_buildId:
            authHeader = azurecli.GetAuthenticationHeader()
            self.m_buildId = azuredevopsrestapi.GetBuildId(authHeader, self.m_organization, self.m_project, self.m_definitionId, self.m_origVersion, None)
        if not self.m_version:
            authHeader = azurecli.GetAuthenticationHeader()
            self.m_version = azuredevopsrestapi.GetBuildVersionFromBuildId(authHeader, self.m_organization, self.m_project, self.m_buildId)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:ADOBuildArtifactSource
    #-------------------------------------------------------------------------------------------
    def ReadProvenance (self):
        provFileName = self.GetProvenanceFile()
        with open (provFileName, 'rt') as provFile:
            provLines = provFile.readlines()
        if len(provLines) != 1:
            raise utils.BuildError ('Azure Devops build artifact Prov file {0} is the wrong size; expected 1 line, found {1}'.format(provFileName, len(provLines)))
        splitLine = provLines[0].strip().split('|')
        if len(splitLine) < 5:
            raise utils.BuildError ('Azure Devops build artifact Prov file {0} is bad; expected 5 pieces, found {1} ({2})'.format(provFileName, len(splitLine), repr(splitLine)))
        return (splitLine[1], splitLine[2], splitLine[3], splitLine[4], splitLine[5])

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:ADOBuildArtifactSource
    #-------------------------------------------------------------------------------------------
    def IsUpToDate(self):
        provFileName = self.GetProvenanceFile()
        if os.path.exists(provFileName) and os.path.exists(self.GetLocalPath()):
            (artifactName, buildId, definitionId, project, organization) = self.ReadProvenance()
            if artifactName == self.m_artifactName and buildId == str(self.m_buildId) and definitionId == str(self.m_definitionId) and project == self.m_project and organization == self.m_organization:
                #Provenance exists and its the same
                return True
        return False

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:ADOBuildArtifactSource
    #-------------------------------------------------------------------------------------------
    def GetProvenanceFileName (self):
        return self.m_alias + '.provenance.txt'

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:ADOBuildArtifactSource
    #-------------------------------------------------------------------------------------------
    def GetProvenanceFile (self):
        return os.path.join (buildpaths.GetADOBuildArtifactSourceLocation(), self.GetProvenanceFileName())

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:ADOBuildArtifactSource
    #-------------------------------------------------------------------------------------------
    def GetLocalPath (self):
        return os.path.join (buildpaths.GetADOBuildArtifactSourceLocation(), self.m_alias)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:ADOBuildArtifactSource
    #-------------------------------------------------------------------------------------------
    def UpdateSource(self, artifactsUpdated):
        if self.IsUpToDate():
            utils.showInfoMsg ("ADO Build Artifact '{0}' is up-to-date\n".format (self.m_alias),utils.INFO_LEVEL_SomewhatInteresting)
            return 0

        # Delete old provenance file and artifact folder
        if os.path.exists(self.GetProvenanceFile()):
            os.remove(self.GetProvenanceFile())
        if os.path.isdir(self.GetLocalPath()):
            utils.cleanDirectory(self.GetLocalPath, deleteFiles=True)

        authHeader = azurecli.GetAuthenticationHeader()
        downloadUrl = azuredevopsrestapi.GetArtifactDownloadUrl(authHeader, self.m_organization, self.m_project, self.m_buildId, self.m_artifactName)
        azuredevopsrestapi.DownloadAndExtractBuildArtifactWithGivenName(authHeader, downloadUrl, buildpaths.GetADOBuildArtifactSourceLocation(), self.m_alias)
        self.UpdateProvenance()
        artifactsUpdated.append (self)

        return 0

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def BuildAction (self, thisPart):
        outputBuildContext = thisPart.GetOutputBuildContextDir(thisPart.m_info.m_buildContext, thisPart.IsStatic())
        utils.showInfoMsg ("Connecting ADO Build Artifact '{0}' to context '{1}'\n".format (self.m_alias, outputBuildContext), utils.INFO_LEVEL_SomewhatInteresting)
        sourcePath = self.GetLocalPath()

        linkPath = os.path.join (outputBuildContext, 'adobuildartifact', self.m_alias)
        utils.showInfoMsg ("Linking ADO Build Artifact link '{0}' to source '{1}'\n".format (linkPath, sourcePath), utils.INFO_LEVEL_RarelyUseful)
        thisPart.PerformBindToDir (linkPath, sourcePath, checkSame=False, checkTargetExists=True, skipIntermediateLinks=True)

#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
class UpackFeed (object):
    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def __init__(self, name, address, feed, feedType):
        self.m_name = name
        self.m_address = os.path.expandvars(address)
        self.m_feed = feed
        self.m_type = feedType

#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
class UpackSource (object):
    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def __init__(self, name, version, platforms=None, feed=None, partFile=None, partName=None, multiPlatform=False, alias=None):
        self.m_name = name
        self.m_origName = name  # For multi-platform this gives us a way to recreate.
        self.m_version = version
        self.m_origVersion = version
        self.m_platforms = platforms
        self.m_feed = feed  # Feed passed in; None means use default.
        self.m_partFile = partFile
        self.m_partName = partName
        self.m_remoteAddr = None
        self.m_upkg = None
        self.m_multiPlatform = multiPlatform
        self.m_platform = None
        self.m_alias = alias if alias else name

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetVersionFromProvenance (self):
        _, version, _, _ = self.GetProvenance()
        if not version:
            return None

        return versionutils.VersionWithSuffix (version)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetLocalPath (self):
        name = self.m_name
        if not '*' in  self.m_version:
            name = '{0}.{1}'.format(name, self.m_version)
        return os.path.join (self.GetLocalDownloadDir(), name)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def HasPartInfo (self):
        return self.m_partFile and self.m_partName

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetLocalDownloadDir (self):
        return buildpaths.GetToolCacheSourceRoot() if self.IsToolPart() else buildpaths.GetUpackSourceLocation()

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetProvenance (self):
        provFileName = self.GetProvenanceFile()
        if not os.path.exists (provFileName):
            return None, None, None, None
        
        return self.ReadProvenance (provFileName)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------  
    def ReadProvenance (self, provFileName):
        with open (provFileName, 'rt') as provFile:
            provLines = provFile.readlines()
        if len(provLines) != 1:
            raise utils.BuildError ('Upack Prov file {0} is the wrong size; expected 1 line, found {1}'.format(provFileName, len(provLines)))
        splitLine = provLines[0].split('|')
        if len(splitLine) < 5:
            raise utils.BuildError ('Upack Prov file {0} is bad; expected 5 pieces, found {1} ({2})'.format(provFileName, len(splitLine), repr(splitLine)))
        return (splitLine[1], splitLine[2], splitLine[3], splitLine[4])

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def UpdateProvenance (self):
        # Store a provenance file for future use
        pkgFeed = self.GetUpkg().GetFeed()
        provFileName = self.GetProvenanceFile()
        with open (provFileName, 'wt') as provFile:
            provLine = '|'.join(['UPKG', self.m_name, self.m_version, pkgFeed.m_address, pkgFeed.m_feed])
            provFile.write (provLine + '\n')
            provFile.close()
        return

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def UpdateSource (self, reposDownloaded, pullToRev, retries=5):
        alreadyDownloaded = False

        if pullToRev:
            self.m_version = pullToRev
        # Check if package exists
        self.GetUpkg ().ResolveVersion (pullToRev, retries)
        if self.PackageIsUpToDate():
            utils.showInfoMsg ("Upack {0} is up-to-date with version {1}\n".format (self.m_name, self.m_version),utils.INFO_LEVEL_SomewhatInteresting)
            return 0

        status = 0
        if not os.path.exists(self.GetLocalPath ()):
            with g_packageDownloadLock:
                updatePackage = '|'.join (['UPKG', self.m_name, self.m_version])
                if updatePackage in g_changedPackages:
                    alreadyDownloaded = True
                else: g_changedPackages.add (updatePackage)

            if not alreadyDownloaded:
                status = self.GetUpkg ().Download (retries)

        if status == 0:
            self.UpdateProvenance ()
            reposDownloaded.append (self)
        return status

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def Update (self, version=None, platforms=None, feed=None, partFile=None, partName=None, multiPlatform=None):
        if version:
            self.m_version = version
            self.m_origVersion = version
        if platforms:   self.m_platforms = platforms
        if feed:        self.m_feed = feed
        if partFile:    self.m_partFile = partFile
        if partName:    self.m_partName = partName
        if multiPlatform: self.m_multiPlatform = multiPlatform

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def PackageIsUpToDate (self):
        # Check if package exists
        name, version, _, _ = self.GetProvenance()
        matching = (name == self.m_name and version == self.m_version)
        pkgExists = os.path.exists (self.GetLocalPath()) # Convenience for devs, in case we delete directory but not prov file
        return matching and pkgExists

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetPackageDirectory (self, baseDir=None):
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
            self.m_version = provLine[2]
            pkgDir = self.m_name+'.'+provLine[2]
            return pkgDir

        return None

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetProvenanceFileName (self):
        return self.m_alias + '.provenance.txt'

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetProvenanceFile (self):
        return os.path.join (self.GetLocalDownloadDir(), self.GetProvenanceFileName())

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def IsToolPart (self):
        return (self.m_name.lower() in globalvars.buildStrategy.m_toolPackages['upack'] or self.m_origName.lower() in globalvars.buildStrategy.m_toolPackages['upack'])

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetUpkg (self, curStrat=None):
        buildStrategy = curStrat or globalvars.buildStrategy
        # We wait to resolve this until needed so that the feed can be overridden in later strategies.
        if not self.m_upkg:
            if self.m_feed:
                if not self.m_feed in buildStrategy.m_upkgFeeds:
                    raise utils.StrategyError ("Cannot find Package {0} in Upack Feed {1}\n".format (self.m_name, self.m_feed))
                self.m_upkg =  universalpkg.GetUniversalPackage (self, buildStrategy.m_upkgFeeds [self.m_feed])
            else:
                if not buildStrategy.m_defaultUpkgPullFeed:
                    raise utils.StrategyError ("No DefaultUpackFeed has been provided. Unable to set up Upack.\n")
                self.m_upkg = universalpkg.GetUniversalPackage (self, buildStrategy.m_defaultUpkgPullFeed)

        return self.m_upkg

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    @staticmethod
    def GetNameWithPlatform (name, platform):
        return '{0}_{1}'.format(name, platform.GetXmlName())

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def CreateRepoEntries (self, curStrat):
        name = self.m_alias
        if curStrat.HasLocalRepository(name):
            return

        # For UPacks that have a partfile, create a local and remote repo so these can get into the DAG as parts. 
        upkg = self.GetUpkg(curStrat)
        # Version may not be resolved since this is done before the pull.
        upkg.ResolveVersionLocal(ignoreErrors=True)
        localRepo = repository.CreateLocalRepository (name, globalvars.REPO_UPACK, self.GetLocalPath())
        localRepo.SetUpackSource (self)
        curStrat.AddLocalRepository (localRepo)

        remoteRepo = repository.RemoteRepository (name, upkg.GetFeed().m_address, None, upkg.GetFeed().m_feed)
        curStrat.GetRemoteRepositoryList().AddPackageEntry(remoteRepo)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def MakeToolExecutable (self):
        # For tool packages on non-windows make files in all dirs executable.
        # Originally just wanted to do the root directory, but nodejs is more complex.
        if os.name == 'nt' or not self.IsToolPart():
            return

        pkdDir = os.path.join (self.GetLocalDownloadDir(), self.GetPackageDirectory())
        fileList = utils.listAllFiles (pkdDir)
        for pkgFile in fileList:
            fullFile = os.path.join (pkdDir, pkgFile)
            if os.path.isdir(fullFile):
                continue
            st = os.stat(fullFile)
            os.chmod(fullFile, st.st_mode | stat.S_IXUSR) # Execute permission for user
            utils.showStatusMsg ("Updating executable status of '{0}'".format (fullFile),utils.INFO_LEVEL_SomewhatInteresting)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def CreatePlatformSpecificCopy (self, newAlias, platform):
        newSrc = copy.copy (self)
        newSrc.m_name = UpackSource.GetNameWithPlatform (self.m_name, platform)
        newSrc.m_alias = newAlias
        newSrc.m_platform = platform
        newSrc.m_multiPlatform = False
        return newSrc

#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
class PartStrategy (object):
    def __init__(self, name, fromfile):
        self.m_name = name
        self.m_buildFromSource = globalvars.BUILD_OPTION_Once
        self.m_onError = ERROR_OPTION_Stop
        self.m_fromfile = fromfile
        self.m_saveLKG   = 'all'
        self.m_LKGSource = None
        self.m_locks = []
        self.m_versionString = None
        self.m_exclude = None

    def DoesMatch (self, buildContext, partName):
        if not utils.NameMatch (buildContext, self.m_name[0]):
            return False
        if not utils.NameMatch (partName, self.m_name[1]):
            return False
        return True

    def RequiresSource (self):
        return self.m_buildFromSource != globalvars.BUILD_OPTION_Never

    def RequiresLKG (self):
        return self.m_onError == ERROR_OPTION_UseLkg or not self.RequiresSource()

    def ShouldSaveLKG (self, platform):
        if self.m_saveLKG == 'all':
            return True

        if platform.IsExcluded (self.m_saveLKG):     # Yeah, the method is called IsExcluded and this is an inclusion list, so return True if it's "Excluded".
            return True
        return False
             
    #--------------------------------------------------------------------------------
    # bsimethod
    #--------------------------------------------------------------------------------
    def Duplicate (self):
        newStrat = copy.deepcopy (self)
        return newStrat

    def __str__(self):
        return "PartStrategy<{0},{1}> From={2}".format(self.m_name[0], self.m_name[1], self.m_fromfile)

    def Dump (self, indent):
        lkgString = self.m_saveLKG
        if type(lkgString) is list:
            lkgString = [x.GetXmlName() for x in self.m_saveLKG]

        msg = " "*indent + "PartStrategy<{0},{1}> Build={2}, OnError={3}, LKGSource={4}, SaveLKG={5}, Exclude-{6} Locks=[{7}] From={8}\n".format( \
            self.m_name[0], self.m_name[1],buildOptions[self.m_buildFromSource], onErrorOptions[self.m_onError], \
            self.m_LKGSource, lkgString, self.m_exclude, ','.join(self.m_locks), self.m_fromfile)

        utils.showInfoMsg (msg, utils.INFO_LEVEL_Essential)

#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
class RepoStrategy (object):
    def __init__(self, name, getSource, fromfile):
        self.m_name = name
        self.m_getSource = getSource
        self.m_fromfile = fromfile

    def DoesMatch (self, partFileName):
        return utils.NameMatch (partFileName, self.m_name)

    def __str__(self):
        return "RepositoryStrategy<{0}>, From={1}".format(self.m_name, self.m_fromfile)

    def Dump (self, indent):
        utils.showInfoMsg( " "*indent + "RepositoryStrategy<{0}> GetSource={1}, From={2}\n".format(self.m_name, pullOptions[self.m_getSource], self.m_fromfile),
            utils.INFO_LEVEL_Essential )


#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
class StrategyEnvVariable (object):
    def __init__(self, name, value, from_location):
        self.m_name = name
        self.m_value = value
        self.m_from = from_location
             
    def GetName(self):
        return self.m_name

    def GetValue(self):
        return self.m_value

    def GetFrom(self):
        return self.m_from

    def Duplicate (self):
        self_copy = copy.deepcopy (self)
        return self_copy

    def __str__(self):
        return "StrategyEnvVariable Name={0} Value={1} From={2}".format(self.m_name, self.m_value, self.m_from)


    def Dump (self, indent):
        # Do not use __str__ for Dump as it has different purpose.
        msg = " "*indent + str(self) + "\n"

        utils.showInfoMsg (msg, utils.INFO_LEVEL_Essential)

#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
class DeferStrategy (object):
    def __init__(self, name, build):
        self.m_name = name
        self.m_build = build

    def DoesMatch (self, deferType):
        return utils.NameMatch (deferType, self.m_name)

    def __repr__(self):
        return "DeferStrategy {0}={1}".format(self.m_name, self.m_build)

#--------------------------------------------------------------------------------
# @bsimethod
#--------------------------------------------------------------------------------
def parseBoolFromXml (boolString, description):
    boolString = boolString.lower()
    if "false" == boolString:
        return False
    elif "true" == boolString:
        return True
    raise utils.StrategyError ("{0} must be 'True' or 'False'. '{1}' found.\n".format (description, boolString))

#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
class BuildControl (object):
    def __init__(self):
        self.m_saveLkgsNuGet        = False     # Controls whether "bb savelkgs" calls SaveNugetAction
        self.m_saveLkgsLegacy       = False     # Controls whether "bb savelkgs" does anything (in the non-NuGet way)
        self.m_saveProducts         = False     # Controls whether "bb saveproduct" does anything
        self.m_saveNuget            = False     # Controls whether "bb savenuget" does anything
        self.m_saveInstallSetToLKG  = False
        self.m_isBundleBuild        = False
        self.m_deployLanguages      = False     # this seems to do nothing?
        self.m_pullUsingPartPlatforms = False
        self.m_includeSubProductBundles = True
        self.m_useUpgradeCodeInComponentGuidSeed = False
        self.m_useLKGTryLocalBDFFirst = False

#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
class RepositoryOptions (object):
    def __init__(self):
        self.m_tag = None
        self.m_branch = None
        self.m_revision = None
        self.m_pullToTip = False
        self.m_forceCheckout = False
        self.m_legacyTag = None
        self.m_legacyPullOnce = True
        self.m_skipPull = False

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:RepositoryOptions
    #-------------------------------------------------------------------------------------------
    def GetLegacyTuple(self):
        return (self.m_legacyTag, self.m_legacyPullOnce, self.m_forceCheckout)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:RepositoryOptions
    #-------------------------------------------------------------------------------------------
    def Dump(self):
        return "Tag={0}, Branch={1}, Revision={2}, pullToTip={3}, forceCheckout={4}, legacyTag={5}, legacyPullOnce={6}, skipPull={7}".format (self.m_tag, self.m_branch, self.m_revision, self.m_pullToTip, self.m_forceCheckout, self.m_legacyTag, self.m_legacyPullOnce, self.m_skipPull)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:RepositoryOptions
    #-------------------------------------------------------------------------------------------
    def SetLegacyTag(self, legacyTag):
        if legacyTag:
            self.m_legacyTag = legacyTag

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:RepositoryOptions
    #-------------------------------------------------------------------------------------------
    def SetLegacyPullOnce(self, pullOnce):
        if pullOnce:
            self.m_legacyPullOnce = pullOnce

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:RepositoryOptions
    #-------------------------------------------------------------------------------------------
    def SetRevision(self, revision):
        self.m_revision = revision
        self.m_pullToTip = False

#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
class BuildStrategy (object):
    def __init__(self):
        self.m_strategyName = None
        self.m_default = PartStrategy(('*','*'), None)
        self.m_localRepos = {}
        self.m_repoOptions = {}
        self.m_lkgSources = {}
        self.m_lkgServers = {}
        self.m_sdkSources = {}
        self.m_nugetSources = {}
        self.m_upackSources = {}
        self.m_adoBuildArtifactSources = {}
        self.m_remoteRepoLists = repository.RemoteRepositoryList()
        self.m_defaultRepo = None
        self.m_defaultPartFile = None
        self.m_defaultPartName = None
        self.m_pullToBdfName = None
        self.m_defaultLKG = None
        self.m_defaultBdfServer = None
        self.m_defaultBdfServerName = None
        self.m_forceLkgType = None
        self.m_defaultOutputRootDir = os.environ.get ('OutRoot', None)
        self.m_defaultPlatform = [targetplatform.GetHostPlatform ()]
        self.m_lkgPlatforms = None
        self.m_onlyPlatforms = None
        self.m_pullToBDFIgnoreList = None
        self.m_defaultOnProvenanceErrors = 0
        self.m_onProvenanceErrors = []
        self.m_partStrategies = []
        self.m_accessedFiles = {}
        self.m_additionalImports = []
        self.m_usePartStrategyFindCache = True
        self.m_foundPartStrategyCache = {}
        self.m_wixBundleStrategy = None
        self.m_numThreads = -1
        self.m_toolsetPart = None
        self.m_deferStrategies = []
        self.m_useLastKnownGood = []
        self.m_toolsetForPlatform = {}
        self.m_toolVersionForPlatform = {}
        self.m_dotNetRuntimeForPlatform = {}
        self.m_windowsSdkVersionForPlatform = {}
        self.m_buildControl = BuildControl()
        self.m_lkgBdfForPull = None
        self.m_targetVersionSource = None  # As read
        self.m_targetVersion = False  # Lazy-parsed m_targetVersionSource
        self.m_envVariables = {}   # EnvVariables that are defined in the strategy file (xml tag EnvVariable)
        self.m_nugetFeeds = {}
        self.m_defaultNugetPullFeed = None
        self.m_defaultNugetPushFeed = None
        self.m_npmFeeds = {}
        self.m_defaultNpmPullFeed = NpmFeed("BentleyAdo", internal.NPM_SERVER, globalvars.CREDENTIAL_PROVIDER_BASIC)
        self.m_defaultNpmPushFeed = NpmFeed("BentleyAdo", internal.NPM_SERVER, globalvars.CREDENTIAL_PROVIDER_BASIC)
        self.m_upkgFeeds = {}
        self.m_defaultUpkgPullFeed = None # Must be defined in strategy
        self.m_defaultUpkgPushFeed = None # Must be defined in strategy
        self.m_strategyAliases = None
        self.m_repoPullMethod = None
        self.m_stratRepos = set()
        self.m_toolParts = []
        self.m_toolPackages = {'upack':set(), 'nuget':set(), 'repo':set()}
        self.m_l10nProduct = None
        self.m_cacheTime = None
        self.m_envForBuild = None
        self.m_aliasTeamConfigRepoList = None
        self.m_languages = "en"

        # Stuff used for caching only
        self.m_version = STRATEGY_VERSION
        self.m_bbVersion = globalvars.bbVersion
        self.m_bbModTime = None

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    @staticmethod
    def CacheFileName(): return os.path.join (buildpaths.GetBBCacheDir(), 'strategy.cache')

    #---------------------------------------------------------------------------------------
    # bsimethod
    #---------------------------------------------------------------------------------------
    def IsMicrosoftProviderRequired (self):
        if 'USE_NUGET_MSCP' in os.environ:  # Currently this forces auto to favor MCP
            return True

        for node in self.m_nugetSources:
            nugetSource = self.m_nugetSources[node]
            if nugetSource.m_feed and nugetSource.m_feed in self.m_nugetFeeds and self.m_nugetFeeds[nugetSource.m_feed].m_provider == globalvars.CREDENTIAL_PROVIDER_MCP:
                return True

        for node in self.m_lkgServers:
            if self.m_lkgServers[node].m_provider == globalvars.CREDENTIAL_PROVIDER_MCP:
                return True
        return False

    #---------------------------------------------------------------------------------------
    # bsimethod
    #---------------------------------------------------------------------------------------
    def GetAuthenticationToken (self, name):
        # To allow for different Nuget source authentication tokens, you can set BENTLEYADO_AUTH or similar where the first
        #   part matches the name of the feed. Since we're Azure DevOps, SYSTEM_ACCESSTOKEN is set more commonly, including
        #   in the classic pipeline when you toggle on OAuth sharing.
        feedEnv = "{}_AUTH".format(name.upper())
        token = os.environ.get(feedEnv, None)
        if token:
            utils.showInfoMsg ("Using auth token from {}.\n".format(feedEnv), utils.INFO_LEVEL_SomewhatInteresting)
            return token

        utils.showInfoMsg ("No auth token found for feed {}.\n".format(name), utils.INFO_LEVEL_SomewhatInteresting)
        return None

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def StoreStrategyCache (self):
        pickle.dump (self, open (self.CacheFileName(), "wb"))

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def RecreateStrategy (self, strategyName, additionalImports, partStrategyOverlays=None, envVariablesChangelog=None, repoPullMethod=None, commandBdf=None, repoOptionsChangeLog=None):
        utils.showRewritableLine ('Recreating build strategy', utils.INFO_LEVEL_VeryInteresting)

        self.m_strategyName = strategyName
        self.m_additionalImports = additionalImports or []

        self.m_partStrategyOverlays = []  # Just for bb debug
        self.m_LkgBdfs = []               # Keep during strategy read in case there's a match situation

        self.m_envVariablesChangelog = [] # Just for bb debug
        self.m_repoOptionsChangeLog = [] # Just for bb debug
        self.m_repoPullMethod = repoPullMethod  # A method for when repo needs to be pulled because it is used in a strategy alias.
        self.m_commandLineBdf = commandBdf  # If a BDF is passed on the command line it is needed for aliases.
        self.m_aliasTeamConfigRepoList = repository.RemoteRepositoryList() # When looking up aliases we need the repo list from BuildStrategies

         # Turn off caching while reading the buildstrategies.
        self.m_usePartStrategyFindCache = False
        self.m_stratDefaultExpansionVars = self.RegisterExpanderVariables ()

        # Process main strategies
        for singleStrategy in strategyName.split(";"):
            self.DoStrategyFile (singleStrategy, None)

        if translationkit.IsLanguageBuild() and not globalvars.programOptions.useLocalizedDir and translationkit.L10N_REPOSITORY_LISTS_BUILDSTRATEGY not in additionalImports:
            self.m_additionalImports.insert(0, translationkit.L10N_REPOSITORY_LISTS_BUILDSTRATEGY)

        # Process additional imports
        for additionalImport in self.m_additionalImports:
            self.DoStrategyFile(additionalImport, None)
        self.m_usePartStrategyFindCache = True
        self.MarkDefaultTools()

        # After reading, update the LKG sources to include a server element.  Once the strategies
        #   are read, the locations are set.
        self.SetLKGServers () 

        if partStrategyOverlays != None:
            partStrategyOverlays.extend (self.m_partStrategyOverlays)

        if envVariablesChangelog != None:
            envVariablesChangelog.extend (self.m_envVariablesChangelog)

        if repoOptionsChangeLog != None:
            repoOptionsChangeLog.extend (self.m_repoOptionsChangeLog)

        # Create an internal BDF for pull that is the accumulation of all the pieces.
        self.m_lkgBdfForPull = self.SetupPullBdf (self.m_LkgBdfs)

        # Free stuff we don't want to cache
        del (self.m_envVariablesChangelog)
        del (self.m_repoOptionsChangeLog)
        del (self.m_LkgBdfs)
        del (self.m_partStrategyOverlays)
        del (self.m_aliasTeamConfigRepoList)
        self.m_repoPullMethod = None
        self.m_commandLineBdf = None
        self.m_envForBuild = None
        self.m_stratDefaultExpansionVars = None # These include object references so do not cache but reset in post-cache load.
        utils.setExpanderStrategy (None)

        self.m_bbModTime = utils.GetNewestBentleyBuildFileTime()
        self.m_cacheTime = datetime.datetime.utcnow().strftime('%Y-%m-%d %H:%M:%S')

        if utils.HasXmlParseErrors(): # Don't store if any errors because then they won't reproduce.
            raise utils.FatalError (message="There were XML parsing errors in the strategy files; please make sure the XML passes the XSD check.")
        if utils.isBsi():
            self.StoreStrategyCache () # Always cache the last one to disk

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    @staticmethod
    def ClearStrategyCache ():
        cacheFileName = BuildStrategy.CacheFileName()
        if os.path.exists (cacheFileName):
            os.remove (cacheFileName)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    @staticmethod
    def ValidateStrategy (buildStrategy, strategyName, additionalImports):
        if not buildStrategy:
            utils.showInfoMsg ('Strategy format changed\n', utils.INFO_LEVEL_VeryInteresting)
        elif buildStrategy.m_version != STRATEGY_VERSION :
            utils.showInfoMsg ('Strategy cache version is out of date {0} != {1}; flushing strategy cache\n'.format (buildStrategy.m_version, STRATEGY_VERSION), utils.INFO_LEVEL_VeryInteresting)
        elif buildStrategy.m_bbVersion != globalvars.bbVersion :
            utils.showInfoMsg ('BentleyBuild version is out of date {0} != {1}; flushing strategy cache\n'.format (buildStrategy.m_bbVersion, globalvars.bbVersion), utils.INFO_LEVEL_VeryInteresting)
        elif  buildStrategy.AnyAccessedFileChanged():
            utils.showInfoMsg ('Strategy files have been modified; flushing strategy cache\n', utils.INFO_LEVEL_VeryInteresting)
        elif  buildStrategy.AnyBentleyBuildFileChanged():
            utils.showInfoMsg ('BentleyBuild files have been modified; flushing strategy cache\n', utils.INFO_LEVEL_VeryInteresting)
        elif strategyName.lower() != buildStrategy.m_strategyName.lower():
            utils.showInfoMsg ('Different build strategies are specified({0} != {1}); flushing strategy cache\n'.format (strategyName, buildStrategy.m_strategyName), utils.INFO_LEVEL_VeryInteresting)
        elif len(additionalImports) != len(buildStrategy.m_additionalImports) or \
               len(set (additionalImports).intersection(buildStrategy.m_additionalImports)) != len(additionalImports):
            utils.showInfoMsg ('Different additional imports are specified; flushing strategy cache\n', utils.INFO_LEVEL_VeryInteresting)
        elif not buildStrategy.m_defaultOutputRootDir or (buildStrategy.m_defaultOutputRootDir + os.sep != os.environ.get('OutRoot', None) and buildStrategy.m_defaultOutputRootDir != os.environ.get('OutRoot', None)):
            utils.showInfoMsg ('Default OutRoot is different; flushing strategy cache\n', utils.INFO_LEVEL_VeryInteresting)
        else:
            return True # Good strategy
        return False

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    @staticmethod
    def GetBuildStrategy (strategyName, additionalImports, forceLoad, repoPullMethod, commandBdf):
        cacheFileName = BuildStrategy.CacheFileName()
        if not forceLoad and os.path.exists (cacheFileName):
            try:
                strat = pickle.load (open (cacheFileName, "rb"))
            except:
                strat = None

            if strat:
                currentStrategyLanguages = translationkit.getLanguagesFromCSV(strat.m_languages)
                addL10nStrategy = translationkit.IsLanguageBuild() if translationkit.isLanguageSettingsInitialized() else 'en' not in currentStrategyLanguages or len(currentStrategyLanguages) > 1
                if addL10nStrategy and not globalvars.programOptions.useLocalizedDir and translationkit.L10N_REPOSITORY_LISTS_BUILDSTRATEGY not in additionalImports:
                    additionalImports.insert(0, translationkit.L10N_REPOSITORY_LISTS_BUILDSTRATEGY)

            if BuildStrategy.ValidateStrategy (strat, strategyName, additionalImports):
                strat.PostCacheLoad ()
                return strat

        strat = BuildStrategy ()
        strat.RecreateStrategy (strategyName, additionalImports, repoPullMethod=repoPullMethod, commandBdf=commandBdf)
        strat.PostCacheLoad ()
        return strat

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def PostCacheLoad (self):
        # Since the pickled Platform and Server objects will always be a personal copy, update them all to the single global version; useful for comparisons.
        # More importantly it creates the thread lock objects used by LKG Sources. These can't be pickled.
        for lkgSourceDict in [self.m_lkgSources, self.m_sdkSources]:
            for lkgSource in lkgSourceDict.values():
                # Update Platforms
                if lkgSource.m_platforms:
                    lkgSource.m_platforms = [targetplatform.FindPlatformByXMLName (platform.GetXmlName()) for platform in lkgSource.m_platforms]

                # Update servers. Shouldn't be necessary; pickle handles internal references.
                # lkgSource.SetServer (self.m_lkgServers [lkgSource.GetServerName().lower()])
                
                # Add thread lock
                lkgSource.AddThreadLock () 

        self.UpdateCachedEnvVariables() # Lock in the environment so we don't have to keep recreating it.
        self.m_stratDefaultExpansionVars = self.RegisterExpanderVariables () # These are in terms of the object so we don't want to use cached.

        # Add in the additional data from the config file. 
        self.m_remoteRepoLists.AddEntriesFromConfig ()

        # Initialize global languages object since it's not saved to the cache
        translationkit.setLanguagesCSV (self.m_languages)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def AnyBentleyBuildFileChanged (self):
        return utils.GetNewestBentleyBuildFileTime() != self.m_bbModTime

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def AddAccessedFile (self, fileanme):
        if fileanme in self.m_accessedFiles:
            return

        modTime = 0 if not os.path.exists (fileanme) else os.stat (fileanme).st_mtime
        self.m_accessedFiles[fileanme] = modTime

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def AnyAccessedFileChanged (self):
        for aFile, modTime in self.m_accessedFiles.items():
            if not os.path.exists (aFile):
                return True
            st = os.stat (aFile)
            if modTime != st.st_mtime:
                return True
        return False

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def RegisterExpanderVariables (self):
        utils.setExpanderStrategy (self)
        variableBindings = [
                ("strategy.name",                           "self.m_strategyName"),
                ("strategy.defaultRepo",                    "self.m_defaultRepo"),
                ("strategy.defaultPartFile",                "self.GetDefaultPartFileName()"),
                ("strategy.defaultPartFileBase",            "self.GetDefaultPartFileBasename()"),
                ("strategy.defaultPartName",                "self.m_defaultPartName"),
                ]

        return variableexpander.CreateVariableList (self, variableBindings)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def SetLKGServers (self):
        for lkgSourceDict in [self.m_lkgSources, self.m_sdkSources]:
            for lkgSource in lkgSourceDict.values():

                # If server types and names are forced on us, just update the LKG source at this time
                if self.m_forceLkgType:
                    lkgSource.Update (type=self.m_forceLkgType)

                # Now associate the correct server with the source for later lookup
                serverName = lkgSource.GetServerName().lower()
                lkgServer = self.m_lkgServers [serverName]
                if not lkgServer:
                    raise utils.StrategyError ('Server name ' + lkgSource.GetServerName() + ' not found for lkgSource ' + lkgSource.GetName())

                lkgSource.SetServer (lkgServer)

                # Resolve any version information
                lkgSource.ResolveVersion ()
                
        # Also set the default BDF server to be the server instead of the server name
        if self.m_defaultBdfServerName:
            if not self.m_defaultBdfServerName.lower() in self.m_lkgServers:
                raise utils.StrategyError ('Server name ' + self.m_defaultBdfServerName + ' not found for DefaultLastKnownGoodSource BDFServer')
            self.m_defaultBdfServer = self.m_lkgServers [self.m_defaultBdfServerName.lower()]
            
    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def SetLKGServersForBdf (self):
        if not self.m_defaultBdfServer:
            utils.showInfoMsg ('No default BDF server set; skipping reset of LKG servers.\n', utils.INFO_LEVEL_SomewhatInteresting)
            return
        utils.showInfoMsg ('Updating all LKG servers to default BDF server "{0}".\n'.format(self.m_defaultBdfServer.m_name), utils.INFO_LEVEL_SomewhatInteresting)
        # When pulling from a BDF file, we need to reset all the LKG servers to be pulling from the verisioned
        # PRG server.  Of course if we're in PRG and using a fileshare then we're ok.
        for lkgSourceDict in [self.m_lkgSources, self.m_sdkSources]:
            for lkgSource in lkgSourceDict.values():
                if not lkgSource.IsServerVersioned():
                    lkgSource.Update (type=lkgs.LKGSOURCE_prg, prgServer=self.m_defaultBdfServer)
                    lkgSource.SetServer (self.m_defaultBdfServer)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetSdkSource (self, name):
        if not name.lower() in globalvars.buildStrategy.m_sdkSources:

            raise utils.StrategyError ('Unable to find SdkSource Strategy for {0}'.format(name))
            
        return globalvars.buildStrategy.m_sdkSources[name.lower()]

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetNugetSource (self, name, version='*'):
        lname = name.lower()
        if lname in self.m_nugetSources:
            return self.m_nugetSources[lname]

        # We might have inferred a dependency source at pull time, which could have been 
        # removed from the strategy on a cache clear. See if it's still around...
        nugetSource = nugetpkg.NuGetSource(name, version, dependency=True)
        if os.path.exists(nugetSource.GetProvenanceFile()):
            # Just in case we're going off the wrong version this time, update it with the one stored in provenance
            nugetSource.Update(nugetSource.GetResolvedVersion(), None)
            self.m_nugetSources[lname] = nugetSource
            return nugetSource

        def addSource (fullName, foundName):
            # May as well add it in case we need it again
            newNugetSource = self.m_nugetSources[foundName].Clone (fullName)
            self.m_nugetSources[newNugetSource.m_name.lower()] = newNugetSource
            return newNugetSource

        if '_' in lname: # Often end with _platformName for internal packages
            ubarIndex = lname.rfind('_')
            postfix = lname[ubarIndex+1:]
            if targetplatform.FindPlatformByXMLName (postfix) != None:
                lname = lname[0:ubarIndex]
                if lname in self.m_nugetSources:
                    return addSource (name, lname) # May as well add it
                elif '_' in lname:  # May have _stream name as well
                    ubarIndex = lname.rfind('_')
                    lname = lname[0:ubarIndex]
                    if lname in self.m_nugetSources:
                        return addSource (name, lname) # May as well add it

        msg = "NuGetSource {0} was not found in strategy so a default one will be created with version *".format(name)
        utils.showInfoMsg (msg, utils.INFO_LEVEL_RarelyUseful)

        # Unable to find NuGetSource Strategy for Package Id, 
        # thus created nuget source and added to nugetSources list
        nugetSource = nugetpkg.NuGetSource(name, version, dependency=False)
        self.m_nugetSources[lname] = nugetSource

        return nugetSource

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def AddNugetSource (self, source):
        self.m_nugetSources[source.m_name.lower()] = source

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetNpmFeedDefault (self):
        return self.m_defaultNpmPullFeed

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetNpmFeed (self, name):
        return self.m_npmFeeds.get(name, self.m_defaultNpmPullFeed)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetADOBuildArtifactSource(self, alias):
        artifactSource = None
        if alias.lower() in self.m_adoBuildArtifactSources:
            artifactSource = self.m_adoBuildArtifactSources[alias.lower()]
        return artifactSource

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetUpackSource (self, alias, platform):
        if not platform:
            src = self.m_upackSources.get(alias.lower(), None)
            if not src:
                src = UpackSource(alias, '*') # Generate one with default feed. This is when you don't want to bother adding it to the strat; you just want the latest every time.
                self.m_upackSources[alias.lower()] = src
            return src

        # For tool parts we use Host platform
        src = self.m_upackSources.get(alias.lower(), None) # Get generic one to check if it's a tool part
        if src and src.IsToolPart():
            platform = targetplatform.GetHostPlatform()

        # If there's one that involves the platform, use that
        fullAlias = UpackSource.GetNameWithPlatform (alias, platform)
        platSrc = self.m_upackSources.get(fullAlias.lower(), None)
        if platSrc:
            return platSrc

        # See if there's one that doesn't use the platform
        if src:
            # If it doesn't use the platform but it should, then create a platform-specific copy
            if platform and src.m_multiPlatform:
                # Create platform package and add it
                newSrc = src.CreatePlatformSpecificCopy(fullAlias, platform)
                self.m_upackSources[newSrc.m_alias.lower()] = newSrc
                src = newSrc

        if not src:
            src = UpackSource(alias, '*') # Generate one with default feed. This is when you don't want to bother adding it to the strat; you just want the latest every time.
            self.m_upackSources[alias.lower()] = src
        return src

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def MarkDefaultTools (self):
        for toolName, toolType in globalvars.defaultTools:
            self.MarkToolPart (toolName, toolType)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def MarkToolPart (self, toolName, toolType):
        if toolType == globalvars.REPO_UPACK:
            self.m_toolPackages['upack'].add(toolName.lower())
        elif toolType == globalvars.REPO_NUGET:
            self.m_toolPackages['nuget'].add(toolName.lower())
        elif toolType == globalvars.REPO_GIT:
            self.m_toolPackages['repo'].add(toolName.lower())
        else:
            raise utils.BuildError('Unsupported Default Tool Type: {0} {1}'.format(toolName, toolType))

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetDefaultNugetPushLocation (self):
        return self.m_defaultNugetPushFeed.m_address

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetToolsetForPlatform (self, platform):
        return self.m_toolsetForPlatform.get (platform.GetXmlName(), (None, None))[0]

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetToolVersionForPlatform (self, platform):
        return self.m_toolVersionForPlatform.get (platform.GetXmlName(), None)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetDotNetRuntimeForPlatform (self, platform):
        return self.m_dotNetRuntimeForPlatform.get (platform.GetXmlName(), None)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetWindowsSdkVersionForPlatform (self, platform):
        return self.m_windowsSdkVersionForPlatform.get (platform.GetXmlName(), None)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetLibTypeForPlatform (self, platform):
        return True if self.m_toolsetForPlatform.get (platform.GetXmlName(), (None, globalvars.LIB_TYPE_Dynamic))[1] == globalvars.LIB_TYPE_Static else False

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetDeferStrategy (self, name):
        for deferItem in self.m_deferStrategies:
            if deferItem.DoesMatch (name):
                return deferItem.m_build

        return DEFER_OPTION_Normal

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def PartIsSkippedByDeferStrategy (self, deferName):
        if not deferName:
            return False
        return (DEFER_OPTION_Never == self.GetDeferStrategy(deferName.lower()))

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def EnsureUpackLocalRepository (self, repoName, platform, partFile, partName):
        upackSrc = self.GetUpackSource (repoName, platform)
        if (partFile and partName) and not upackSrc.HasPartInfo(): # When upack is not specified in strategy but still has partfile / partname
            upackSrc.Update (partFile=partFile, partName=partName)
        upackSrc.CreateRepoEntries (self)
        return upackSrc.m_alias

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetVersionValue (self, rootPart):
        # Look this up as late as possible since it may go out to the BDF server to get the next available value
        # Because of this we need a thread lock.
        if self.m_targetVersion != False:
            return self.m_targetVersion

        with s_createVersionLock:
            if not self.m_targetVersionSource:
                self.m_targetVersion = None # None means checked but not found. False means unchecked.
                return self.m_targetVersion

            def getnextVersionFromServer (versionStr):
                prodName = rootPart.GetPrgDirName()
                while versionStr[-1] in ['*', '.']:
                    versionStr = versionStr[0:-1]
                from . import prodversionaction
                nextVer = prodversionaction.GetNextVersion (prodName, versionStr, False)
                return versionutils.Version (nextVer)

            if self.m_targetVersionSource[0].isdigit():
                # It is a version string being stored in the Strategy; should be productName:1.2.3.* format
                utils.showInfoMsg ('Treating DefaultTarget VersionSource as a version to check on the BDF server because it starts with a digit: {0}'.format (self.m_targetVersionSource), utils.INFO_LEVEL_SomewhatInteresting)
                if not self.m_targetVersionSource.endswith ('*'):
                    raise utils.StrategyError ('Version string in DefaultTarget element must contain a trailing wildcard (*) if it specifies a number to look up in the BDF server.')

                self.m_targetVersion = getnextVersionFromServer (self.m_targetVersionSource)

            elif self.m_targetVersionSource.startswith('${'):  
                # If it starts wht ${SrcRoot} then it's a file name. At this point the filename format is just a . or - seperated number.
                utils.showInfoMsg ('Treating DefaultTarget VersionSource as a filename to check on the BDF server because it starts with a source reference: {0}'.format (self.m_targetVersionSource), utils.INFO_LEVEL_SomewhatInteresting)
    #            filename = globalvars.variableExpander.ExpandString (self.m_targetVersionSource) # have to fight ${} vs $()
                filename = os.path.expandvars (self.m_targetVersionSource)
                with open (filename, 'rt') as verFile:
                    verLine = verFile.readline()
                self.m_targetVersion = versionutils.Version (verLine)
                
                # Some products prefer a 3 digit version, so assume more minor digits are 0.
                self.m_targetVersion = self.m_targetVersion.GetFilledInWildcardVersion (versionutils.Version.InitFromList ([0, 0, 0, 0]))
            else:
                # Last choice is an environment variable that contains a version number. If it has a * then do the server lookup.
                utils.showInfoMsg ('Treating DefaultTarget VersionSource as a environment variable: {0}'.format (self.m_targetVersionSource), utils.INFO_LEVEL_SomewhatInteresting)
                if not self.m_targetVersionSource in os.environ:
                    raise utils.StrategyError ('Version Environment Variable "{0}"" not defined'.format(self.m_targetVersionSource))
                verString = os.environ[self.m_targetVersionSource]
                if '*' in verString:
                    self.m_targetVersion = getnextVersionFromServer (verString)
                else:
                    self.m_targetVersion = versionutils.Version (verString)

            utils.showInfoMsg ('Using Build Version {1} From Strategy: {0}\n'.format (self.m_targetVersionSource, self.m_targetVersion), utils.INFO_LEVEL_VeryInteresting)
            return self.m_targetVersion

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def SetupPullBdf (self, lkgBdfList):
        # Create a compbined BDF with all the source locations so that we will pull source to match.
        if not lkgBdfList:
            return None

        newBdf = builddescriptionfile.BuildDescription ()
        # I think we just need Hg, Git, and CVS repos. Add in rsync and bundles for bundles
        for lkgBdf in lkgBdfList:
            for hgRepoName in lkgBdf.GetAllRepoNames ():
                hgRepo = lkgBdf.m_repoGuidDict[hgRepoName]
                newBdf.UpdateEntry (hgRepo.Name, hgRepo.Guid, hgRepo.Url, fromLKG=True)

            for gitRepoName in lkgBdf.GetAllGitRepoNames ():
                gitRepo = lkgBdf.m_gitRepoGuidDict[gitRepoName]
                newBdf.UpdateGitEntry (gitRepo.Name, gitRepo.Guid, gitRepo.Url, fromLKG=True)

            for cvsTagName in lkgBdf.GetAllCvsNames ():
                cvsTag = lkgBdf.m_repoTagDict[cvsTagName]
                newBdf.UpdateCvsEntry (cvsTag.Name, cvsTag.Tag, cvsTag.Url, cvsTag.Path)

            for rsyncName in lkgBdf.GetAllRsyncNames ():
                rsyncData = lkgBdf.m_rsyncRepoDict[rsyncName.lower()]
                newBdf.UpdateRsyncEntry (rsyncData.Name, rsyncData.Tag, rsyncData.Url, rsyncData.Path)

            for bundleName in lkgBdf.GetAllBundles ():
                bundleEntry = lkgBdf.GetBundleEntry(bundleName)
                newBdf.UpdateBundleEntry (bundleName, bundleEntry.Path, bundleEntry.OrganizationName, bundleEntry.Project, bundleEntry.ArtifactName, bundleEntry.AzureBuildId)

        return newBdf

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def DownloadStrategyFile (self, stratRepo, strategyName):
        self.m_stratRepos.add (stratRepo)

        localRepo = self.FindLocalRepository (stratRepo)
        stratFileName = symlinks.normalizePathName(os.path.join(localRepo.GetLocalDir(), strategyName + ".BuildStrategy.xml"))

        # This is a bit tricky. I wanted to only pull in this situation if the repository doesn't exist, but
        # there's a bit of a chicken-and-egg problem if the BuildStrategies change but you don't have the repo changes.
        # I think in general you'll get both in the reread of strategies, but sometimes it will take a second pull to catch up. 
        if self.m_repoPullMethod and not os.path.exists(stratFileName):
            try:
                remoteRepo = self.m_aliasTeamConfigRepoList.FindRepository (stratRepo, localRepo.m_repoType)
            except utils.StrategyError:
                remoteRepo = None
            pullToRev = self.m_aliasTeamConfigRepoList.GetRevision(localRepo, False)
            self.m_repoPullMethod (stratRepo, strategyName, self, remoteRepo=remoteRepo, pullToRev=pullToRev)
    
        # Always return the name of the strategy
        return stratFileName

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def ProcessIncludes (self, dom):
        baseStrategies = utils.getDomElementsByName(dom, "ImportStrategy")
        
        # Process repository info for current build strategy so we know how to correctly download the strategy files. Should only be done once.
        for baseStrategy in baseStrategies:
            if baseStrategy.getAttribute("Repository") != '':
                self.ProcessRemoteRepositoryLists (dom)
                self.ProcessLocalRepositories (dom)
                self.ProcessRepoTags (dom)
                break

        for baseStrategy in baseStrategies:
            repo = baseStrategy.getAttribute("Repository") if baseStrategy.getAttribute("Repository") != '' else None
            subdirectory = baseStrategy.getAttribute("SubDir") if baseStrategy.getAttribute("SubDir") != '' else None
            strategyName = baseStrategy.getAttribute ("Name")

            if repo:
                strategyName = self.DownloadStrategyFile (repo, os.path.join(subdirectory, strategyName))

            self.DoStrategyFile (strategyName, globalvars.currentStrategyFile)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def ProcessAliases (self, dom):
        if self.m_strategyAliases == None:
            self.m_strategyAliases = {}

        stratAliases = utils.getDomElementsByName(dom, "StrategyAlias")
        for alias in stratAliases:
            name = alias.getAttribute("Name")
            if "" == name:
                raise utils.StrategyError ("No Name given for StrategyAlias")
            strat = alias.getAttribute("Alias")
            if "" == strat:
                raise utils.StrategyError ("No Alias given for StrategyAlias {0}".format(name))
            self.m_strategyAliases[name.lower()] = (name, strat)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def ProcessLocalRepositories (self, dom):
        repos = utils.getDomElementsByName(dom, "LocalRepository")
        for repo in repos:
            name = repo.getAttribute("Name")
            if "" == name:
                raise utils.StrategyError ("No Name given for LocalRepository")
            localDir = repo.getAttribute("Directory")
            if "" == localDir:
                raise utils.StrategyError ("No Directory given for LocalRepository " + name)

            repoTypeString = repo.getAttribute("Type")
            if "" != repoTypeString:
                repoType = utils.resolveOption (repoTypeString, globalvars.repoTypes, "Repository Type for " + name)
            else:
                repoType = 0

            if 'BBPW' in os.environ and repoType == globalvars.repoTypes.index(globalvars.REPO_NUGET): # DMS speciality code until they update
                continue  # PW still has some Nugets mixed in...
                
            localRepo = repository.CreateLocalRepository (name, repoType, localDir)

            if name.lower() not in self.m_repoOptions:
                self.m_repoOptions[name.lower()] = RepositoryOptions()

            repoOptions = self.m_repoOptions[name.lower()]
            repoOptions.m_skipPull = utils.getOptionalBoolAttr('SkipPull', False, 'SkipPull', repo)
            repoOptions.m_pullToTip = utils.getOptionalBoolAttr('PullToTip', False, 'PullToTip', repo)


            if repoOptions.m_pullToTip and repoOptions.m_skipPull:
                raise utils.StrategyError("SkipPull cannot be used when PullToTip is true.")

            if name.lower() in globalvars.skipPull:
                repoOptions.m_pullToTip = False
                repoOptions.m_skipPull = True
            self.m_repoOptionsChangeLog.append('RepositoryOptions Name={0}, {1}'.format(name, repoOptions.Dump()))

            self.m_localRepos[name.lower()] = localRepo

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def ProcessRemoteRepositoryLists (self, dom):
        # Read in from the strategy file
        self.m_remoteRepoLists.AddStrategyEntries (dom)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    class PartStrategyOverlay (object):
        #-------------------------------------------------------------------------------------------
        # bsimethod
        #-------------------------------------------------------------------------------------------
        def __init__ (self, dom, partName):
            self.m_buildFromSource  = None
            self.m_onError          = None
            self.m_saveLKG          = None
            self.m_LKGSource        = None
            self.m_locksAdded       = []
            self.m_locksRemoved     = []
            self.m_lockMessages     = ''
            self.m_versionString    = None
            self.m_exclude          = False

            if dom:
                self.FromDom (dom, partName)

        #-------------------------------------------------------------------------------------------
        # bsimethod
        #-------------------------------------------------------------------------------------------
        def FromDom (self, dom, partName):
            self.m_buildFromSource =             self.ResolveOption (dom, partName, "BuildFromSource", buildOptions)
            self.m_onError =                     self.ResolveOption (dom, partName, "OnError", onErrorOptions)

            saveLKG = dom.getAttribute ("SaveLKG")
            if saveLKG == '':
                self.m_saveLKG = None
            else:
                if saveLKG == 'true' or saveLKG == '*':  # Allow true for historical reasons
                    self.m_saveLKG = 'all'
                elif saveLKG == 'false' or saveLKG == 'none':  # Allow false for historical reasons
                    self.m_saveLKG = []
                else:
                    self.m_saveLKG = []
                    def handleNoPlatformMatch (platformCsv, platformWildcard):
                        raise utils.StrategyError(" PartStrategy SaveLKG option contains unrecognized platform, '{0}', in Platforms='{1}'.".format(platformWildcard, platformCsv))

                    self.m_saveLKG = targetplatform.GetPlatformsFromCSV (saveLKG, handleNoPlatformMatch=handleNoPlatformMatch)

            lkgSource = dom.getAttribute ("LastKnownGoodSource")
            self.m_LKGSource = None if "" == lkgSource else lkgSource
            
            locksAdded = dom.getAttribute ("Locked")
            self.m_locksAdded = [] if "" == locksAdded else locksAdded.split(',')

            locksRemoved = dom.getAttribute ("Unlocked")
            self.m_locksRemoved = [] if "" == locksRemoved else locksRemoved.split(',')

            versionString = dom.getAttribute ("Version")
            self.m_versionString = None if "" == versionString else versionString
            
            exclude = dom.getAttribute ("Exclude")
            self.m_exclude = None if "" == exclude else (exclude.lower() == 'true')

            self.m_lockMessages = ''  # Have to get message that we didn't do something because it was locked back to the report section

        #-------------------------------------------------------------------------------------------
        # bsimethod
        #-------------------------------------------------------------------------------------------
        def ResolveOption (self, dom, partName, itemName, options):
        
            itemAttr = dom.getAttribute (itemName)
            if '' == itemAttr:
                return None
            source = itemName + " value of " + repr(partName)
            return utils.resolveOption (itemAttr, options, source)
            
        #-------------------------------------------------------------------------------------------
        # bsimethod
        #-------------------------------------------------------------------------------------------
        def ItemLocked (self, partStrategy, itemName):
            lowerName = itemName.lower()
            
            if lowerName in partStrategy.m_locks:
                self.m_lockMessages += (' (skipping {0} because it is locked)'.format(itemName))
                return True

        #-------------------------------------------------------------------------------------------
        # bsimethod
        #-------------------------------------------------------------------------------------------
        def ApplyOverlay (self, partStrategy):
            # First apply unlocks before applying any changes in this PartStrategy
            for item in self.m_locksRemoved:
                name = item.lower()
                if name in partStrategy.m_locks:
                    partStrategy.m_locks.remove (name)
        
            # Apply changes
            # Just doing a few locks for now.  May need more.
            if self.m_saveLKG != None and not self.ItemLocked (partStrategy, 'SaveLKG'):
                partStrategy.m_saveLKG = self.m_saveLKG
            if self.m_buildFromSource != None and not self.ItemLocked (partStrategy, 'BuildFromSource'):
                partStrategy.m_buildFromSource = self.m_buildFromSource
            if self.m_onError != None and not self.ItemLocked (partStrategy, 'OnError'):
                partStrategy.m_onError = self.m_onError
            if self.m_LKGSource != None:
                partStrategy.m_LKGSource = self.m_LKGSource
            if self.m_versionString != None:
                partStrategy.m_versionString = self.m_versionString
            if self.m_exclude != None:
                partStrategy.m_exclude = self.m_exclude

            # Finally apply any new locks after applying any changes in this PartStrategy
            for item in self.m_locksAdded:
                name = item.lower()
                if not name in partStrategy.m_locks:
                    partStrategy.m_locks.append (name)

        #-------------------------------------------------------------------------------------------
        # bsimethod
        #-------------------------------------------------------------------------------------------
        def Dump (self, partName, addedStr):
            msg = "PartStrategy [{2}] <{0},{1}>".format (partName[0], partName[1], addedStr)

            if self.m_buildFromSource != None:
                msg = msg + " Build={0}".format (buildOptions[self.m_buildFromSource])
            if self.m_onError != None:
                msg = msg + " OnError={0}".format (onErrorOptions[self.m_onError])
            if self.m_LKGSource != None:
                msg = msg + " LastKnownGoodSource={0}".format (self.m_LKGSource)
            if self.m_saveLKG != None:
                msg = msg + " SaveLKG={0}".format (self.m_saveLKG)
            if self.m_exclude != None:
                msg = msg + " Exclude={0}".format (self.m_exclude)
            if self.m_locksAdded:
                msg = msg + " AddingLocks={0}".format (','.join(self.m_locksAdded))
            if self.m_locksRemoved:
                msg = msg + " RemovingLocks={0}".format (','.join(self.m_locksRemoved))
            msg = msg + self.m_lockMessages
            return msg

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def InterpretDefaultPartStrategy (self, dom):
        partName = ('*','*')
        overlay = self.PartStrategyOverlay (dom, partName)
        return self.AddInterpretPartStrategy (dom, partName, None, overlay)
    
    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def AddInterpretPartStrategy (self, _dom, partName, partStrat, overlay):
        if None == partStrat:
            partStrat = self.FindPartStrategy (partName[0], partName[1]).Duplicate()
        partStrat.m_name = partName
        partStrat.m_fromfile = globalvars.currentStrategyFile
        
        overlay.ApplyOverlay (partStrat)
        return partStrat

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def UpdateAllPartStrategies (self, partName, overlay):
        for partStrat in self.m_partStrategies:
            if utils.NameMatch (partStrat.m_name[0], partName[0]) and utils.NameMatch (partStrat.m_name[1], partName[1]):
                overlay.ApplyOverlay (partStrat)

        if ('*', '*') == partName:
            overlay.ApplyOverlay (self.m_default)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def InterpretPartStrategy (self, dom, partName):
        partStrat = self.FindExactPartStrategy (partName[0], partName[1])
        overlay = self.PartStrategyOverlay (dom, partName)
        returnStrat = None
        
        if ('*', '*') == partName:
            # Just apply to matching
            addedStr = 'Applied'
        elif partStrat != None:
            # Start with previous; apply changes; put at front of queue, and remove previous
            returnStrat = self.AddInterpretPartStrategy (dom, partName, partStrat.Duplicate(), overlay)
            self.m_partStrategies.remove (partStrat)
            addedStr = 'Updated'
        else:
            returnStrat = self.AddInterpretPartStrategy (dom, partName, None, overlay)
            addedStr = 'Added'
            
        self.m_partStrategyOverlays.append (overlay.Dump (partName, addedStr))  # store this for bb debug

        # Update all intervening strats
        self.UpdateAllPartStrategies (partName, overlay)
        
        return returnStrat

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def ProcessPartStrategy (self, dom):
        buildContextName= dom.getAttribute ("PartFile")
        if "" == buildContextName:
            raise utils.StrategyError ("No PartFile for PartStrategy")

        partName = dom.getAttribute ("PartName")
        if "" == partName:
            raise utils.StrategyError ("No PartName for PartStrategy")

        return self.InterpretPartStrategy (dom, (buildContextName, partName))

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def _getStratLevel (self, partStrat):
        # Wildcards lowers priority.  It feels the most natural to have the options you specifed 
        #   for a specific part take precedence over anything specified for a wildcard
        #  1  file, part
        #  2  file, part*
        #  3  file, *
        #  4  file*, part
        #  5  file*, part*
        #  6  file*, *
        #  7  *, part
        #  8  *, part*
        #  9  *, *

        def getType (name):
            if name == '*':
                return 3
            elif '*' in name:
                return 2
            else:
                return 1

        (filestr, part) = partStrat.m_name
        fileType = (getType (filestr) - 1) * 3    # 0, 3, 6
        partType = getType (part)                 # 1, 2, 3
        return fileType + partType

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def ProcessPartStrategies (self, dom):
        partStrategies = utils.getDomElementsByName(dom, "PartStrategy")
        for partStratDom in partStrategies:
            partStrat = self.ProcessPartStrategy (partStratDom)
            if partStrat:
                stratLevel = self._getStratLevel (partStrat)
                if '*' in partStrat.m_name[0] or '*' in partStrat.m_name[1]:
                    # Put all wildcard strategies after all specific strategies
                    iStrat = 0
                    while (iStrat < len (self.m_partStrategies)):
                        if ( self._getStratLevel(self.m_partStrategies[iStrat]) >= stratLevel):
                            break
                        iStrat += 1
                    self.m_partStrategies.insert (iStrat, partStrat)
                else: # Simple case of a specific strategy
                    self.m_partStrategies.insert (0, partStrat)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def AddDeferredStrategy (self, deferStrat):
        # If it already exists, update
        for item in self.m_deferStrategies:
            if item.m_name == deferStrat.m_name:
                item.m_build = deferStrat.m_build
                return
    
        # Put all wildcard strategies after all specific strategies. Full wildcard is last and there is only one.
        if '*' == deferStrat.m_name:
            self.m_deferStrategies.append (deferStrat)
        elif '*' in deferStrat.m_name:
            iStrat = 0
            found = False
            while (iStrat < len (self.m_deferStrategies)):
                if '*' in self.m_deferStrategies[iStrat].m_name:
                    self.m_deferStrategies.insert (iStrat, deferStrat)
                    found = True
                    break
                iStrat += 1
            if not found:
                self.m_deferStrategies.append (deferStrat)
        else:
            self.m_deferStrategies.insert (0, deferStrat)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def ProcessDeferStrategies (self, dom):
        deferStrategies = utils.getDomElementsByName(dom, "DeferStrategy")
        for deferStratDom in deferStrategies:

            name = deferStratDom.getAttribute ("Name")
            if "" == name:
                raise utils.StrategyError ("No Name for DeferStrategy")

            build = deferStratDom.getAttribute ("BuildPass")
            if "" == build:
                raise utils.StrategyError ('No BuildPass for DeferStrategy "{0}"'.format (name))
                
            # Can be a number or Never
            if not IsValidDeferValue(build):
                raise utils.StrategyError ("DeferStrategy BuildPass must be a number or {0}\n".format(DEFER_OPTION_Never))
                
            # cast build to an int if applicable to compare with DEFER_OPTION_Normal later.
            deferStrat = DeferStrategy (name, int(build) if build != DEFER_OPTION_Never else build)
            self.AddDeferredStrategy (deferStrat)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def ProcessToolsets (self, dom):
        # Process the DefaultToolset elements and put the toolset right onto the platform object.
        toolsetItems = utils.getDomElementsByName(dom, "DefaultToolset")
        for tsItem in toolsetItems:
            tsName = tsItem.getAttribute ("Toolset")
            if "" == tsName:
                raise utils.StrategyError ("A DefaultToolset element must have a valid 'Toolset' attribute")

            tsVersion = tsItem.getAttribute ("ToolVersion")

            tsLibType = tsItem.getAttribute ("LibType")
            if not tsLibType:
                tsLibType = globalvars.LIB_TYPE_Dynamic

            dotNetRuntime = tsItem.getAttribute ("TargetDotNetRuntime")
            if not dotNetRuntime:
                dotNetRuntime = None

            windowsSdkVersion = tsItem.getAttribute ("TargetWindowsSDKVersion")
            if not windowsSdkVersion:
                windowsSdkVersion = None

            tsPlatform = tsItem.getAttribute ("Platform")
            if "" == tsPlatform:
                raise utils.StrategyError ("A DefaultToolset element must have a valid 'Platform' attribute. Failing element has values: {0}".format 
                    (' '.join([tsItem.attributes.item(x).name+'='+tsItem.attributes.item(x).value for x in range(tsItem.attributes.length)])))
                
            platformList = targetplatform.GetPlatformsFromWildcard (tsPlatform)
            if not platformList:
                raise utils.StrategyError ("Unknown Platform '{0}' in DefaultToolset element with toolset={1}. Valid choices are: {2}".format (tsPlatform, tsName, targetplatform.GetXmlOptions()))

            for tplat in platformList:
                self.m_toolsetForPlatform [tplat.GetXmlName()] = (tsName, tsLibType)
                self.m_toolVersionForPlatform [tplat.GetXmlName()] = tsVersion
                self.m_dotNetRuntimeForPlatform [tplat.GetXmlName()] = dotNetRuntime
                self.m_windowsSdkVersionForPlatform [tplat.GetXmlName()] = windowsSdkVersion


    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def ProcessStrategyEnvVariables (self, dom):
        # Process the EnvVariable elements.
        strategyEnvVariableItems = utils.getDomElementsByName(dom, "EnvVariable")
        for evItem in strategyEnvVariableItems:
            evName = evItem.getAttribute ("Name")
            if "" == evName:
                raise utils.StrategyError ("An EnvVariable element must have a valid 'Name' attribute")

            # As a precaution we are not allowing strange environment names.
            match = re.search(r"[^a-zA-Z0-9_-]", evName)
            if match != None:
                raise utils.StrategyError ("An EnvVariable element 'Name' attribute should not contain '%s' sign. Name should contain alpha-numeric, underscore or dash characters only." % match.group(0))
            
            evValue = evItem.getAttribute ("Value")

            cmdEvName = compat.getStringForEnv(evName)
            cmdEvValue = compat.getStringForEnv(evValue)

            if not cmdEvValue:
                operation = "Removed"
                del self.m_envVariables[cmdEvName]
                self.m_envVariablesChangelog.append("StrategyEnvVariable [{0}] Name={1} Value={2} From={3}".format(operation, cmdEvName, cmdEvValue, globalvars.currentStrategyFile))
            else:
                operation = "Updated" if cmdEvName in self.m_envVariables else "Added"
                envVar = StrategyEnvVariable(cmdEvName, cmdEvValue, globalvars.currentStrategyFile)
                self.m_envVariables[cmdEvName] = envVar
                self.m_envVariablesChangelog.append("StrategyEnvVariable [{0}] Name={1} Value={2} From={3}".format(operation, envVar.GetName(), envVar.GetValue(), envVar.GetFrom()))

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetStrategyDefinedEnvVariables (self):
        return list(self.m_envVariables.values())

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetEnvVariables (self):
        # Create merged environment from os.environ and in the strategy defined envVariables
        bbEnvVariables = self.m_envVariables.copy()
        osEnvVariables = { name : StrategyEnvVariable(name, value, "SHELL") for name, value in os.environ.items()}

        def AddEnvIfNotThere (n, v):
            if not n in bbEnvVariables and not n in osEnvVariables:
                bbEnvVariables[n] = StrategyEnvVariable(n, v, "SHELL")

        # AddEnvIfNotThere ('BSI', '1') Cannot do this because we now have open source
        AddEnvIfNotThere ('BMAKE_OPT', r'-I$(SrcRoot)bsicommon/PublicSDK')
        AddEnvIfNotThere ('BBPYTHONCMD','"{0}"'.format(sys.executable))
        srcRoot = os.path.expandvars('${SrcRoot}')
        bsicommonDirName = os.path.join (srcRoot, 'BsiCommon')
        if os.path.exists (bsicommonDirName):
            AddEnvIfNotThere ('SrcBsiCommon', bsicommonDirName+os.path.sep)
        bsitoolsDirName = os.path.join (srcRoot, 'bsitools')
        if os.path.exists (bsitoolsDirName):
            AddEnvIfNotThere ('SrcBsiTools', bsitoolsDirName+os.path.sep)

        # Update this way so strategy overrides env.
        osEnvVariables.update(bbEnvVariables)
        return list(osEnvVariables.values())

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def UpdateCachedEnvVariables (self):
        self.m_strategyEnv = None
        self.m_strategyEnv = self.GetEnv()

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetEnv (self):
        if self.m_envForBuild:
            return self.m_envForBuild.copy()
        return { ev.GetName() : ev.GetValue() for ev in self.GetEnvVariables()}

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def ExpandEnvironmentVariables (self, varToExpand):
        # I tried relying on os.path.expandvars, but there is only one os.eviron and it hits threading issues. Fell back to our implementation.
        if not '$' in varToExpand:
            return varToExpand
        expandedVar = varToExpand
        origStratEnvVars = self.GetEnv()
        origStratEnvVars['ToolCache'] = buildpaths.GetToolCacheSourceRoot()
        stratEnvVars = {name.lower() : origStratEnvVars[name] for name in origStratEnvVars.keys() }
        ve = variableexpander.VariableExpander()

        # Do the env vars from env + strategy first. They are ${}
        expandedVar =ve.ExpandCurlyBraceString (expandedVar, stratEnvVars, self.m_stratDefaultExpansionVars)

        # Originally the expander variables were all $() for the strat defaults like default
        expandedVar =ve.ExpandParenString (expandedVar, None, self.m_stratDefaultExpansionVars)

        # If there is anything left, run it through the stock python code
        if '$' in expandedVar:
            expandedVar = os.path.expandvars (expandedVar)

#        print ('====> Expanded: {0} ==> {1}'.format(varToExpand, expandedVar))
#        if '$' in expandedVar:
#            print ('====> Unresolved variables {0} ==> {1}'.format(varToExpand, expandedVar))
        return expandedVar

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def ExpandInternalVariables (self, varToExpand):
        if not '$' in varToExpand:
            return varToExpand
        expandedVar = varToExpand
        ve = variableexpander.VariableExpander()
        expandedVar =ve.ExpandParenString (expandedVar, None, self.m_stratDefaultExpansionVars)

#        print ('====> Expanded(Internal): {0} ==> {1}'.format(varToExpand, expandedVar))
#        if '$' in expandedVar:
#            print ('====> Unresolved variables(Internal) {0} ==> {1}'.format(varToExpand, expandedVar))
        return expandedVar

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def ProcessPullBDF (self, dom):
        pullToBDF = utils.getDomElementsByName(dom, "PullToBuildDescriptionFile")
        if len (pullToBDF) > 0:
            if pullToBDF[0].hasAttribute ("Name"):
                pullToBDFName = pullToBDF[0].getAttribute ("Name")
                if "" == pullToBDFName and "" == self.m_pullToBdfName:
                    raise utils.StrategyError ("A PullToBDF element cannot have a blank 'Name' attribute")
                self.m_pullToBdfName = pullToBDFName
            else:
                if "" == self.m_pullToBdfName:
                    raise utils.StrategyError ("A PullToBDF element cannot have a blank 'Name' attribute")

            if pullToBDF[0].hasAttribute ("IgnoreRepositories"):
                bdfIgnoreRepositoriesStr = pullToBDF[0].getAttribute ("IgnoreRepositories")
                if "" != bdfIgnoreRepositoriesStr:
                    self.m_pullToBDFIgnoreList = [repo.lower() for repo in bdfIgnoreRepositoriesStr.split(",")]
            
    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def ProcessDefaultTarget (self, dom):
        target = utils.getDomElementsByName(dom, "DefaultTarget")
        if len(target) > 0:
            repo = target[0].getAttribute("Repository")
            if "" != repo:
                self.m_defaultRepo = repo
            pf = target[0].getAttribute("PartFile")
            if "" != pf:
                self.m_defaultPartFile = pf
            pn = target[0].getAttribute("PartName")
            if "" != pn:
                self.m_defaultPartName = pn
            rootDir = target[0].getAttribute("OutputRootDir")
            if "" != rootDir:
                self.m_defaultOutputRootDir = symlinks.normalizePathName (rootDir)
            numPartThreads = target[0].getAttribute("PartThreads")
            if "" != numPartThreads: 
                if int (numPartThreads) < 0 or int (numPartThreads) > 100:
                    raise utils.StrategyError ("PartThreads ({0}) must be between 0 and 100.  \n".format (int(numPartThreads)) + \
                    "Maybe machines can now work with more than 100 threads, but it's probably an error.")
                self.m_numThreads = numPartThreads

            # These have been moved to BuildControl element, but we will continue to read them and put up a warning.
            self.m_buildControl.m_isBundleBuild            = utils.getOptionalBoolAttr ('IsBundleBuild', self.m_buildControl.m_isBundleBuild, 'IsBundleBuild', target[0])
            self.m_buildControl.m_saveInstallSetToLKG      = utils.getOptionalBoolAttr ('IsSaveInstallSetToLKG', self.m_buildControl.m_saveInstallSetToLKG, 'IsSaveInstallSetToLKG', target[0])
            self.m_buildControl.m_deployLanguages          = utils.getOptionalBoolAttr ('DeployLanguagesToProductsAndInstallers', self.m_buildControl.m_deployLanguages, 'DeployLanguagesToProductsAndInstallers', target[0])
            self.m_buildControl.m_saveNuget                = utils.getOptionalBoolAttr ('SaveNugetProducts', self.m_buildControl.m_saveNuget, 'SaveNugetProducts', target[0])

            # The warnings are commented off until we remove all the current usage.
#            if target[0].getAttribute("IsBundleBuild"):
#                utils.ShowAndDeferMessage ("Warning: Attribute IsBundleBuild is now 'BundleBuild' on the BuildControl element. Use in DefaultTarget is deprecated.\n", utils.INFO_LEVEL_Important, utils.YELLOW)
#                
#            if target[0].getAttribute("IsSaveInstallSetToLKG"):
#                utils.ShowAndDeferMessage ("Warning: Attribute IsSaveInstallSetToLKG is now 'SaveInstallSetToLKG' on the BuildControl element. Use in DefaultTarget is deprecated.\n", utils.INFO_LEVEL_Important, utils.YELLOW)
#                
#            if target[0].getAttribute("DeployLanguagesToProductsAndInstallers"):
#                utils.ShowAndDeferMessage ("Warning: Attribute DeployLanguagesToProductsAndInstallers is now 'DeployLanguagesToProductsAndInstallers' on the BuildControl element. Use in DefaultTarget is deprecated.\n", utils.INFO_LEVEL_Important, utils.YELLOW)

            # Code later in translationkit.py needs a language, and will assume "en".
            # If you don't set it up initially here, it's re-created umpteen times later, creating a performance penalty.
            # Save the attribute value to the strategy so that when we load the strategy from cache we can set the languages properly and not run in English
            self.m_languages = target[0].getAttribute("Languages") or "en"
            translationkit.setLanguagesCSV (self.m_languages)

            platformStr = target[0].getAttribute("Platform")
            if "" != platformStr:
                self.m_defaultPlatform = targetplatform.ResolvePlatformList (platformStr.strip(), False, "ERROR: Unknown architecture '{0}'. \n DefaultTarget@Platform must be {1}\n")

            lkgPlatformStr = target[0].getAttribute("LKGPlatforms")
            if "" != lkgPlatformStr:
                lkgPlatformList = [platform.lower() for platform in lkgPlatformStr.split(",")]
                newPlatList = []
                for lkgPlat in lkgPlatformList:
                    curPlat = targetplatform.FindPlatformByXMLName (lkgPlat.strip())
                    if not curPlat:
                        raise utils.StrategyError ("DefaultTarget@LKGPlatforms found bad platform '"+lkgPlat+"' must be " + ', '.join (targetplatform.GetXmlOptions()))
                    newPlatList.append (curPlat)
                self.m_lkgPlatforms = newPlatList

            # Store as onlyPlatform because it will also be used for m_lkgPlatforms
            excludePlatforms = target[0].getAttribute("ExcludePlatforms")
            if len (excludePlatforms) > 0:
                def handleNoPlatformMatch (csv, nonvalidMatch):
                    raise utils.PartBuildError ("DefaultTarget ExcludePlatforms='{0}' has platform wildcard '{1}' that does not match any platform, must be one of {2}.".format(csv, nonvalidMatch, targetplatform.GetXmlOptions().__str__()), self, target[0])

                excludePlatformsList = []
                targetplatform.UpdatePlatformListByName (excludePlatformsList, excludePlatforms, handleNoPlatformMatch)
                allPlatforms = set (targetplatform.PlatformConstants)
                self.m_onlyPlatforms = list (allPlatforms.difference (set (excludePlatformsList)))

            onlyPlatforms = target[0].getAttribute("OnlyPlatforms")
            if len (onlyPlatforms) > 0:
                def handleNoPlatformMatch2 (csv, nonvalidMatch):
                    raise utils.PartBuildError ("DefaultTarget OnlyPlatforms='{0}' has platform wildcard '{1}' that does not match any platform, must be one of {2}.".format(csv, nonvalidMatch, targetplatform.GetXmlOptions().__str__()), self, target[0])

                self.m_onlyPlatforms = []  # OnlyPlatforms beats ExcludePlatforms
                targetplatform.UpdatePlatformListByName (self.m_onlyPlatforms, onlyPlatforms, handleNoPlatformMatch2)

            versionSourceStr = target[0].getAttribute("VersionSource")
            if "" != versionSourceStr:
                self.m_targetVersionSource = versionSourceStr

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def ProcessBuildControl (self, dom):
        target = utils.getDomElementsByName(dom, "BuildControl")
        if len(target) > 0:
            bcElement = target[0]
            # Pass in previous values as default; keep any previously set values for attributes that aren't explicitly set.
            self.m_buildControl.m_saveLkgsNuGet         = utils.getOptionalBoolAttr ('SaveLkgsNuGet', self.m_buildControl.m_saveLkgsNuGet, 'SaveLkgsNuGet', bcElement)
            self.m_buildControl.m_saveLkgsLegacy        = utils.getOptionalBoolAttr ('SaveLkgsLegacy', self.m_buildControl.m_saveLkgsLegacy, 'SaveLkgsLegacy', bcElement)
            self.m_buildControl.m_saveProducts          = utils.getOptionalBoolAttr ('SaveProducts', self.m_buildControl.m_saveProducts, 'SaveProducts', bcElement)
            self.m_buildControl.m_saveNuget             = utils.getOptionalBoolAttr ('SaveNugetProducts', self.m_buildControl.m_saveNuget, 'SaveNugetProducts', bcElement)
            self.m_buildControl.m_isBundleBuild         = utils.getOptionalBoolAttr ('BundleBuild', self.m_buildControl.m_isBundleBuild, 'BundleBuild', bcElement)
            self.m_buildControl.m_saveInstallSetToLKG   = utils.getOptionalBoolAttr ('SaveInstallSetToLKG', self.m_buildControl.m_saveInstallSetToLKG, 'SaveInstallSetToLKG', bcElement)
            self.m_buildControl.m_deployLanguages       = utils.getOptionalBoolAttr ('DeployLanguagesToProductsAndInstallers', self.m_buildControl.m_deployLanguages, 'DeployLanguagesToProductsAndInstallers', bcElement)
            self.m_buildControl.m_pullUsingPartPlatforms = utils.getOptionalBoolAttr ('PullUsingPartPlatforms', self.m_buildControl.m_pullUsingPartPlatforms, 'PullUsingPartPlatforms', bcElement)
            self.m_buildControl.m_includeSubProductBundles = utils.getOptionalBoolAttr ('IncludeSubProductBundles', self.m_buildControl.m_includeSubProductBundles, 'IncludeSubProductBundles', bcElement)
            self.m_buildControl.m_useUpgradeCodeInComponentGuidSeed = utils.getOptionalBoolAttr ('UseUpgradeCodeInComponentGuidSeed', self.m_buildControl.m_useUpgradeCodeInComponentGuidSeed, 'UseUpgradeCodeInComponentGuidSeed', bcElement)
            self.m_buildControl.m_useLKGTryLocalBDFFirst = utils.getOptionalBoolAttr ('UseLKGTryLocalBDFFirst', self.m_buildControl.m_useLKGTryLocalBDFFirst, 'UseUpgradeCodeInComponentGuidSeed', bcElement)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def ProcessToolsetPart (self, dom):
        target = utils.getDomElementsByName(dom, "ToolsetPart")
        if len(target) > 0:
            self.m_toolsetPart = ['', '', '']
            repo = target[0].getAttribute("Repository")
            self.m_toolsetPart[0] = repo if "" != repo else None
                
            pf = target[0].getAttribute("PartFile")
            self.m_toolsetPart[1] = pf if "" != pf else None

            pn = target[0].getAttribute("PartName")
            if "" != pn:
                self.m_toolsetPart[2] = pn
            else:
                raise utils.StrategyError ("ToolsetPart must supply a PartName")

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def ProcessToolParts(self, dom):
        toolParts = utils.getDomElementsByName(dom, "ToolPart")

        for toolPart in toolParts:
            repo = toolPart.getAttribute("Repository") or None
            partfile = toolPart.getAttribute("PartFile") or None
            partname = toolPart.getAttribute("PartName") or None
            platforms = toolPart.getAttribute("Platforms") or None
            includeInTranskitString = toolPart.getAttribute("IncludeInTranskit") or None

            if includeInTranskitString:
                includeInTranskit = utils.resolveBoolean (includeInTranskitString, "IncludeInTranskit flag")
            else:
                includeInTranskit = False

            platformList = None
            if platforms:
                platformList = targetplatform.GetPlatformsFromWildcard (platforms)
                if not platformList:
                    raise utils.StrategyError ("Unknown Platform '{0}' in ToolPart element {1}:{2}:{3}. Valid choices are: {4}".format (platforms, repo, partfile, partname, targetplatform.GetXmlOptions()))

            if repo and partfile and partname:
                skipAddition = False
                for tool in self.m_toolParts:
                    if tool[3].lower() == partfile.lower() and tool[0] == None:
                        raise utils.StrategyError("Can't have ToolParts defined in a PartFile with the same name in both repository and packages")
                    if tool[3].lower() == partfile.lower() and (tool[0].lower() != repo.lower()):
                        raise utils.StrategyError("Can't have ToolParts defined in a PartFile with the same name but different Repositories")
                    if tool[0].lower() == repo.lower() and tool[3].lower() == partfile.lower() and tool[3].lower() == partname.lower():
                        skipAddition = True
                        utils.showInfoMsg ('ToolPart with Repository={0}, PartFile={1}, PartName={2} has already been defined. Skipping addition.\n'.format(repo, partfile, partname), utils.INFO_LEVEL_RarelyUseful)
                if not skipAddition:
                    self.m_toolParts.append((repo, None, None, partfile, partname, platformList, includeInTranskit))
            else:
                raise utils.StrategyError("ToolPart does not have the required attributes (Repository, PartFile, PartName)")

        toolPackages = utils.getDomElementsByName(dom, "ToolPackage")

        for toolPackage in toolPackages:
            pkgType = toolPackage.getAttribute("PkgType") or None
            pkgName = toolPackage.getAttribute("Name") or None
            partfile = toolPackage.getAttribute("PartFile") or None
            partname = toolPackage.getAttribute("PartName") or None
            platforms = toolPackage.getAttribute("Platforms") or None
            includeInTranskitString = toolPackage.getAttribute("IncludeInTranskit") or None

            if includeInTranskitString:
                includeInTranskit = utils.resolveBoolean (includeInTranskitString, "IncludeInTranskit flag")
            else:
                includeInTranskit = False

            platformList = None
            if platforms:
                platformList = targetplatform.GetPlatformsFromWildcard (platforms)
                if not platformList:
                    raise utils.StrategyError ("Unknown Platform '{0}' in ToolPart element {1}:{2}:{3}. Valid choices are: {4}".format (platforms, repo, partfile, partname, targetplatform.GetXmlOptions()))

            if pkgType and pkgName and partfile and partname:
                skipAddition = False
                for tool in self.m_toolParts:
                    if tool[3].lower() == partfile.lower() and tool[0] != None:
                        raise utils.StrategyError("Can't have ToolParts defined in a PartFile with the same name in both repository and packages")
                    if not tool[0] and tool[3].lower() == partfile.lower() and (tool[1].lower() != pkgType.lower() or tool[2].lower() != pkgName.lower()):
                        raise utils.StrategyError("Can't have ToolParts defined in a PartFile with the same name but different packages")
                    if not tool[0] and tool[1].lower() == pkgType.lower() and tool[2].lower() == pkgName.lower() and tool[3].lower() == partfile.lower() and tool[3].lower() == partname.lower():
                        skipAddition = True
                        utils.showInfoMsg ('ToolPart with PkgType={0}, Name={1} PartFile={2}, PartName={3} has already been defined. Skipping addition.\n'.format(pkgType, pkgName, partfile, partname), utils.INFO_LEVEL_RarelyUseful)
                if not skipAddition:
                    self.m_toolParts.append((None, pkgType, pkgName, partfile, partname, platformList, includeInTranskit))
            else:
                raise utils.StrategyError("ToolPackage does not have the required attributes (PkgType, Name, PartFile, PartName)")

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def ProcessL10nProduct(self, dom):
        l10nProduct = utils.getDomElementsByName(dom, "L10nProduct")

        if len(l10nProduct) > 0:
            name = l10nProduct[0].getAttribute("Name") or None
            if name:
                self.m_l10nProduct = name
            else:
                raise utils.StrategyError("L10nProduct does not have the required Name attribute defined")

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def ProcessProvenance (self, dom):
        defaultProv = utils.getDomElementsByName (dom, "DefaultProvenance")
        if len (defaultProv) > 0:
            defaultOnError = defaultProv[0].getAttribute("OnError")
            if "" == defaultOnError:
                raise utils.StrategyError ("No OnError attribute specified for DefaultProvenance element")

            self.m_defaultOnProvenanceErrors = utils.resolveOption (defaultOnError, provenanceErrors, "OnError value of DefaultProvenance")

        provs = utils.getDomElementsByName(dom, "Provenance")
        for prov in provs:
            onError  = prov.getAttribute("OnError")
            if "" == onError:
                raise utils.StrategyError ("No OnError attribute specified for Provenance element")

            provRepo = prov.getAttribute("Repository")
            if "" == provRepo:
                raise utils.StrategyError ("No Repository attribute specified for Provenance element")

            onProvenanceError = (provRepo, utils.resolveOption (onError, provenanceErrors, "OnError value of Provenance"))
            self.m_onProvenanceErrors.insert (0, onProvenanceError)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def ProcessDefaults (self, dom):
        self.ProcessDefaultTarget (dom)
        self.ProcessProvenance (dom)
        self.ProcessToolsetPart (dom)

        defpartStrat = utils.getDomElementsByName(dom, "DefaultPartOptions")
        if len(defpartStrat) > 0:
            self.m_default = self.InterpretDefaultPartStrategy (defpartStrat[0])

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def _GetConsumingProductVersions (self, lkgName, subName, version):
        # Ask the BDF server for all products that use this particular LKG version, then filter by the ones that match the requested product
        serverName, headers = builddescriptionfile.BuildDescription.GetServerAndHeaders(None)
        requestUrl = "{0}api/products?Product={1}&Flags=Consumers&Release={2}&Major={3}&Minor={4}&Build={5}".format(serverName, subName, version[0], version[1], version[2], version[3])

        try:
            _, response = compat.getUrlWithHeaders (requestUrl, headers, proxyDict=bbutils.GetHttpProxyDict(requestUrl))
            jsonText = json.loads (response)
        except Exception as e:
            raise utils.StrategyError ('\n{0}'.format(e))

        if not jsonText:
            utils.showInfoMsg ('No consumers found on BDF server for the product {0}:{1}-{2}-{3}-{4}\n'.format(subName, version[0], version[1], version[2], version[3]), utils.INFO_LEVEL_VeryInteresting)
        
        allVersions = []
        for retVer in jsonText['LKG Consumers']:
            if retVer['Name'].lower() == lkgName.lower():
                allVersions.append ((retVer['ReleaseVersion'], retVer['MajorVersion'], retVer['MinorVersion'], retVer['BuildNumber']))
        return allVersions

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def FindMatchingLkg (self, lkgName, matchLkgs, prodName, prodVersionList):
        # The goal here is to find a version of lkgName that has parts that match the lkg named
        # in matchLkgs. 
        utils.showInfoMsg ('Trying to match {0} to {1}\n'.format(lkgName, matchLkgs), utils.INFO_LEVEL_VeryInteresting)

        # Get the matching BDF which we should have pulled earlier
        matchBdf = None
        for bdf in self.m_LkgBdfs:
            if matchLkgs.lower() == bdf.GetPrgBuildName().lower():
                matchBdf = bdf
                break

        # It should have been in the list if it is specified before the match item.
        if not matchBdf:
            utils.showInfoMsg ('UseLastKnownGood [{0}:{1}] Could not find matching BDF {2}; was it specified before this line?\n'.format(prodName, '-'.join(prodVersionList), matchLkgs), utils.INFO_LEVEL_VeryInteresting)
            return bdfcache.GetCachedBdfWithStatusCheck (prodName, prodVersionList)

        # Check the cache
        def cacheMessageOutput (message):
            utils.showInfoMsg (message, utils.INFO_LEVEL_SomewhatInteresting)
            
        cacheMatchBdf, status, _ = bdfcache.GetMatchingLkg (lkgName, matchBdf, outputFunc=cacheMessageOutput)
        if cacheMatchBdf and not status:
            return cacheMatchBdf

        for lkgSourceName in matchBdf.GetSubPartLKGSourceNames():
            if lkgSourceName.lower() == lkgName.lower():
                lkgVersionInBdf = versionutils.Version.InitFromList (matchBdf.GetLKGVersion(lkgSourceName))
                versionRequirement = versionutils.Version.InitFromList (prodVersionList)
                if versionRequirement.MatchVersion(lkgVersionInBdf):
                    utils.showInfoMsg ('Found matching UseLastKnownGood [{0} {1}] in [{2} {3}]\n'.format(lkgName, lkgVersionInBdf, matchBdf.GetPrgBuildName(), '-'.join([str(x) for x in matchBdf.GetProductVersion()])), utils.INFO_LEVEL_SomewhatInteresting)
                    baseLkgSource = self.m_lkgSources.get (lkgName.lower(), None)
                    if baseLkgSource and baseLkgSource.m_version:
                        utils.showInfoMsg ('Overriding LKGSource {0} version {1} to matched version {2}\n'.format(lkgName, baseLkgSource.m_version, lkgVersionInBdf), utils.INFO_LEVEL_VeryInteresting)

                    # Get the final BDF from the server
                    matchingServerBDF = bdfcache.GetCachedBdfWithStatusCheck (lkgName, lkgVersionInBdf.m_verQuad)

                    bdfcache.SetMatchingLkg (lkgName, lkgVersionInBdf, matchBdf)
                    return matchingServerBDF
                else:
                    utils.showInfoMsg ('{0} {1} found in {2} {3} but version requirement {4} not met\n'.format(lkgName, lkgVersionInBdf, matchLkgs, '.'.join([str(x) for x in matchBdf.GetProductVersion()]), versionRequirement), utils.INFO_LEVEL_RarelyUseful)
                    return bdfcache.GetCachedBdfWithStatusCheck (prodName, prodVersionList)

        # Get the BDF from the server
        serverBDF = bdfcache.GetCachedBdfWithStatusCheck (prodName, prodVersionList)

        # Get the lists of items from both BDFs and figure out what matches. I am doing this based on subpart versions rather than repository GUIDs
        # because the former are ordered which makes it more searchable. Also it eliminates the stuff like bsicommon that everyone shares.
        matchBdfLKGSources = set (matchBdf.GetSubPartLKGSourceNames())
        curBdfLKGSources = set (serverBDF.GetSubPartLKGSourceNames())
        lkgSourcesToMatch = matchBdfLKGSources & curBdfLKGSources

        # Do a quick check; if this version happens to match then we're golden. Someone may have specified the version and gotten it right.
        count = 0
        for lkgSourceName in lkgSourcesToMatch:
            if matchBdf.GetLKGVersion(lkgSourceName) == serverBDF.GetLKGVersion(lkgSourceName):
                count += 1

        if count == len (lkgSourcesToMatch):
            utils.showInfoMsg ('No matching LKGs for {0} in {1}\n'.format(lkgName, matchLkgs), utils.INFO_LEVEL_RarelyUseful)
            return serverBDF

        # For each part get the versions of the LKG where the subpart versions match the matchLkgs product.
        matchingVersions = set()
        for lkgSourceName in lkgSourcesToMatch:
            context = lkgSourceName
            versionToCheck = matchBdf.GetLKGVersion(lkgSourceName)
            verToCheckStr = '-'.join ([str(v) for v in versionToCheck])
            listOfVersions = self._GetConsumingProductVersions (lkgName, context, versionToCheck)
            utils.showInfoMsg ('Looking for usage of {0} version {1}  in product {2}: found list {3}\n'.format(context, verToCheckStr, lkgName, repr(listOfVersions)), utils.INFO_LEVEL_RarelyUseful)
            if not listOfVersions:
                raise utils.StrategyError (message="No way to match LastKnownGoodSource {0} to {1}; Could not find match for part {2} version {3}\n".format (lkgName, matchLkgs, context, verToCheckStr))
            if not matchingVersions:
                matchingVersions.update (listOfVersions) # First time - just add to list
            else:
                # Keep only the versions that all parts agree on.
                intersect = matchingVersions & set (listOfVersions)
                if len(intersect) == 0:
                    raise utils.StrategyError (message="No way to match LastKnownGoodSource {0} to {1}; Could not find match for combination of parts\n".format (lkgName, matchLkgs))
                matchingVersions = intersect

        # Pick the newest choice.
        sortableVersions = [versionutils.Version.InitFromList(x) for x in matchingVersions]
        sortableVersions.sort()
        utils.showInfoMsg ('Found matching LKG {0} in list {1}\n'.format(lkgName, repr(sortableVersions)), utils.INFO_LEVEL_RarelyUseful)

        utils.showInfoMsg ('Found matching UseLastKnownGood [{0} {1}] matches [{2} {3}]\n'.format(lkgName, sortableVersions[-1], matchBdf.GetPrgBuildName(), '.'.join([str(x) for x in matchBdf.GetProductVersion()])), utils.INFO_LEVEL_SomewhatInteresting)
        baseLkgSource = self.m_lkgSources.get (lkgName.lower(), None)
        if baseLkgSource and baseLkgSource.m_version:
            utils.showInfoMsg ('Overriding LKGSource {0} version {1} to matched version {2}\n'.format(lkgName, baseLkgSource.m_version, sortableVersions[-1]), utils.INFO_LEVEL_VeryInteresting)

        # Get the final BDF from the server
        matchingServerBDF = bdfcache.GetCachedBdfWithStatusCheck (lkgName, sortableVersions[-1].m_verQuad)

        bdfcache.SetMatchingLkg (lkgName, sortableVersions[-1], matchBdf)
        return matchingServerBDF

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def UpdateChildLkgSources (self, lkgName, matchLkgs):
        # Need some way to cache this so maybe we don't have to do this on a build, although changing to a different strategy may still cause it to hit the server.

        lkgSource = self.m_lkgSources.get (lkgName.lower(), None)
        if not lkgSource:
            raise utils.StrategyError (message="No LastKnownGoodSource found for UseLastKnownGood; could not find '{0}'\n".format (lkgName))

        # Choose the version if specified
        lkgSource.ResolveVersion()
        if lkgSource.m_version:
            ver = versionutils.Version (lkgSource.m_version)
            versionList = [str(v) if v != None else '*' for v in ver.m_verQuad]
        else:
            versionList = ['*', '*', '*', '*']

        # Download BDF file from Azure build artifact
        lkgSource.SetServer (self.m_lkgServers [lkgSource.GetServerName().lower()])
        if lkgSource.m_server.m_type == lkgs.LKGTYPE_azurebuildartifact:
            serverBDF = bdfcache.GetBdfFromAzureArtifact (lkgSource.m_server, lkgSource.GetName(), lkgSource.m_version)
        elif matchLkgs:
            serverBDF = self.FindMatchingLkg (lkgName, matchLkgs, lkgSource.GetName(), versionList)
        else:
            if lkgSource.m_server.m_type == lkgs.LKGTYPE_local and self.m_buildControl.m_useLKGTryLocalBDFFirst:
                serverBDF = lkgSource.m_server.GetBDF(lkgSource.GetName(), targetplatform.GetHostPlatform(), lkgSource.m_version, False)
            else:
                serverBDF = bdfcache.GetCachedBdfWithStatusCheck (lkgSource.GetName(), versionList)

        bdfVersionString = '-'.join([str(x) for x in serverBDF.GetProductVersion()])
        # Lock in the actual version we used. For primary LKGs this will make sure it's consistent and speed up BDF lookup (in the cache).
        # For matching LKGs it helps when you need a few extra bits that weren't saved with the LKG of the initial product.
        lkgSource.UpdateVersion (bdfVersionString)
        utils.showInfoMsg ('UseLastKnownGood [{0}:{1}] updating LKGSource version to {2}\n'.format(lkgSource.GetName(), '-'.join(versionList), lkgSource.m_version), utils.INFO_LEVEL_SomewhatInteresting)

        self.m_LkgBdfs.append (serverBDF) # Keep them around while pulling in case there's a match-to item.
        utils.showInfoMsg ('UseLastKnownGood selected bdfserver:{0}:{1}\n'.format(serverBDF.GetPrgBuildName(), bdfVersionString), utils.INFO_LEVEL_Interesting)

        # The full list of BuildContexts that we hit is the cominbation of all the stuff listed in the LKGPartMap (from LKgs) plus BuildContexts (from Source)
        contextsToChange = set([x.lower() for x in serverBDF.m_usedContexts])
        for context in serverBDF.m_lkgMapEntries:
            contextsToChange.add (context.split(':')[0].lower())

        # Check all the existing part strategies and update any that match this partfile.
        foundStarStrat = set()
        for partStrat in self.m_partStrategies:
            if not '*' in  partStrat.m_name[0]:
                if partStrat.m_name[0].lower() in contextsToChange:
                    overlay = self.PartStrategyOverlay (None, partStrat.m_name)
                    overlay.m_buildFromSource = 1
                    overlay.m_LKGSource = lkgSource.m_name
                    overlay.ApplyOverlay (partStrat)
                    self.m_partStrategyOverlays.append (overlay.Dump (partStrat.m_name, 'Updated by UseLastKnownGood [{0}:{1}]'.format(lkgSource.m_name, '-'.join(versionList))))
                    utils.showInfoMsg ('UseLastKnownGood [{0}:{1}] updating LKG Source for {2}\n'.format(lkgSource.m_name, '-'.join(versionList), partStrat.m_name), utils.INFO_LEVEL_SomewhatInteresting)

                    if partStrat.m_name[1] == '*':
                        foundStarStrat.add (partStrat.m_name[0].lower())

        # We want to add a PartFile:* strategy for each of these if there isn't one already.
        for context in (contextsToChange - foundStarStrat):
            # Cook up a part strategy
            partStrat = self.FindPartStrategy (context, '*').Duplicate()  # should always be a copy of default
            partStrat.m_buildFromSource = globalvars.BUILD_OPTION_Never
            partStrat.m_LKGSource = lkgSource.m_name
            partStrat.m_name = (context, '*')

            stratLevel = self._getStratLevel (partStrat)
            # Put all wildcard strategies after all specific strategies
            iStrat = 0
            while (iStrat < len (self.m_partStrategies)):
                if ( self._getStratLevel(self.m_partStrategies[iStrat]) >= stratLevel):
                    break
                iStrat += 1
            self.m_partStrategies.insert (iStrat, partStrat)
            msg = "PartStrategy [Added by UseLastKnownGood [{0}:{1}]] <{2},*> BuildFromSource=False LKGSource={3}".format (lkgSource.GetName(), '-'.join(versionList), context, partStrat.m_LKGSource)
            self.m_partStrategyOverlays.append (msg)  # store this for bb debug

        # Update the in-memory SDKSource versions to those specified with UseLastKnownGood
        for sdkSourceOrig in serverBDF.GetAllSdkSourceNames():
            sdkSource = sdkSourceOrig.lower()
            if sdkSource in self.m_sdkSources:
                sdkVersion = serverBDF.GetSdkSourceVersionString (sdkSourceOrig)
                utils.showInfoMsg ('UseLastKnownGood [{0}:{1}] updating SDK Source version for {2} to {3}\n'.format(lkgSource.GetName(), '-'.join(versionList), sdkSourceOrig, sdkVersion), utils.INFO_LEVEL_SomewhatInteresting)
                self.m_sdkSources[sdkSource].Update(version=sdkVersion)

        # Update the in-memory Nuget versions to those specified with UseLastKnownGood
        for nugetPkg in serverBDF.GetAllNugetPackageNames():
            nugetPkgLower = nugetPkg.lower()
            nugetVersion = serverBDF.GetNugetVersionString(nugetPkg)
            if nugetPkgLower in self.m_nugetSources:
                utils.showInfoMsg ('UseLastKnownGood [{0}:{1}] updating Nuget Source version for {2} to {3}\n'.format(lkgSource.GetName(), '-'.join(versionList), nugetPkg, nugetVersion), utils.INFO_LEVEL_SomewhatInteresting)
                self.m_nugetSources[nugetPkgLower].m_version = nugetVersion
            else:
                utils.showInfoMsg ('UseLastKnownGood [{0}:{1}] adding Nuget Source {2} with version {3}\n'.format(lkgSource.GetName(), '-'.join(versionList), nugetPkg, nugetVersion), utils.INFO_LEVEL_SomewhatInteresting)
                self.m_nugetSources[nugetPkgLower] = nugetpkg.NuGetSource(nugetPkg, nugetVersion)

        # Update the in-memory Upack versions to those specified with UseLastKnownGood
        for (upkgName, upkgAlias, upkgVer, url, feedName) in serverBDF.GetAllUpacks():
            nameLower = upkgName.lower()
            feed = None
            for fname in self.m_upkgFeeds:
                item = self.m_upkgFeeds[fname]
                if item.m_address.lower() == url.lower() and item.m_feed.lower() == feedName.lower():
                    feed = fname
                    break

            if nameLower in self.m_upackSources:
                utils.showInfoMsg ('UseLastKnownGood [{0}:{1}] updating Universal Package Source version for {2} to {3}\n'.format(lkgSource.GetName(), '-'.join(versionList), upkgName, upkgVer), utils.INFO_LEVEL_SomewhatInteresting)
                self.m_upackSources[nameLower].Update(version=upkgVer, feed=feed)
            else:
                # If the source does not already exist, create it. Multiplatforms will always fall in this category as a per-platform will be created.
                utils.showInfoMsg ('UseLastKnownGood [{0}:{1}] adding Universal Package Source version {2} ({3})\n'.format(lkgSource.GetName(), '-'.join(versionList), upkgName, upkgVer), utils.INFO_LEVEL_SomewhatInteresting)
                self.m_upackSources[nameLower] = UpackSource (upkgName, upkgVer, None, feed, None, None, None, upkgAlias)

        # Update the in-memory ADOBuildArtifact versions to those specified with UseLastKnownGood
        for (artifactAlias, artifactName, buildId, definitionId, project, organization) in serverBDF.GetAllADOBuildArtifacts():
            artifactAliasLower = artifactAlias.lower()
            if artifactAliasLower in self.m_adoBuildArtifactSources:
                utils.showInfoMsg ('UseLastKnownGood [{0}:{1}] updating ADOBuildArtifactSource for {2} to (ArtifactName={3} BuildId={4} DefinitionId={5} Project={6} Organization={7})\n'.format(lkgSource.GetName(), '-'.join(versionList), artifactAlias, artifactName, buildId, definitionId, project, organization), utils.INFO_LEVEL_SomewhatInteresting)
                self.m_adoBuildArtifactSources[artifactAliasLower].m_artifactName = artifactName
                self.m_adoBuildArtifactSources[artifactAliasLower].m_buildId = buildId
                self.m_adoBuildArtifactSources[artifactAliasLower].m_definitionId = definitionId
                self.m_adoBuildArtifactSources[artifactAliasLower].m_project = project
                self.m_adoBuildArtifactSources[artifactAliasLower].m_organization = organization
            else:
                utils.showInfoMsg ('UseLastKnownGood [{0}:{1}] adding ADOBuildArtifactSource {2} with (ArtifactName={3} BuildId={4} DefinitionId={5} Project={6} Organization={7})\n'.format(lkgSource.GetName(), '-'.join(versionList), artifactAlias, artifactName, buildId, definitionId, project, organization), utils.INFO_LEVEL_SomewhatInteresting)
                self.m_adoBuildArtifactSources[artifactAliasLower] = ADOBuildArtifactSource(artifactAlias, artifactName, None, definitionId, project, organization, buildId)

        # For anything where the LKGs aren't saved, set them to be Build Once
        for unsavedLkg in serverBDF.GetAllUnsavedLkgs():
            splitList = serverBDF.GetUnsavedLkgs (unsavedLkg).split(':')
            if len(splitList) != 2:
                utils.showInfoMsg ('Incorrect format in UnsavedLKGs ({0})\n'.format(serverBDF.GetUnsavedLkgs (unsavedLkg)), utils.INFO_LEVEL_SomewhatInteresting)
                continue
            partFile = splitList[0]
            partName = splitList[1]

            # Find or make a part strategy
            partStrat = self.FindPartStrategy (partFile, partName)
            if partStrat.m_name[0].lower()==partFile.lower() and partStrat.m_name[1].lower()==partName.lower():
                partStrat.m_buildFromSource = globalvars.BUILD_OPTION_Once
                msg = "PartStrategy [Updated by UseLastKnownGood [{0}:{1}] (UnsavedLKGs)] <{2},{3}> Build=once".format (lkgSource.GetName(), '-'.join(versionList), partFile, partName)
                self.m_partStrategyOverlays.append (msg)  # store this for bb debug
            else:
                # Need to create one
                partStrat = partStrat.Duplicate()  # A copy of the default or some * strat
                partStrat.m_buildFromSource = globalvars.BUILD_OPTION_Once
                partStrat.m_name = (partFile, partName)

                stratLevel = self._getStratLevel (partStrat)
                # Put all wildcard strategies after all specific strategies
                iStrat = 0
                while (iStrat < len (self.m_partStrategies)):
                    if ( self._getStratLevel(self.m_partStrategies[iStrat]) >= stratLevel):
                        break
                    iStrat += 1
                self.m_partStrategies.insert (iStrat, partStrat)
                msg = "PartStrategy [Added by UseLastKnownGood [{0}:{1}] (UnsavedLKGs)] <{2},{3}> Build=once".format (lkgSource.GetName(), '-'.join(versionList), partFile, partName)
                self.m_partStrategyOverlays.append (msg)  # store this for bb debug

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def ProcessUseLastKnownGoods (self, dom):
        lkgSources = utils.getDomElementsByName(dom, "UseLastKnownGood")
        for lkgSource in lkgSources:
            lkgName = lkgSource.getAttribute("LastKnownGoodSource")
            if "" == lkgName:
                raise utils.StrategyError ("No LastKnownGoodSource attribute specified for UseLastKnownGood element")

            matchLkgs = None
            matchLkgsString = lkgSource.getAttribute("MatchLkgs")
            if "" != matchLkgsString:
                matchLkgs = matchLkgsString

            # Store it for debugging, but we are only going to use it now.
            self.m_useLastKnownGood.append ((lkgName, matchLkgs))
            self.UpdateChildLkgSources (lkgName, matchLkgs)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def ProcessRepoTags (self, dom):
        repoTags = utils.getDomElementsByName(dom, "RepositoryTag")
        for repoTag in repoTags:
            name = repoTag.getAttribute("Name")
            if "" == name:
                raise utils.StrategyError ("No Name given for RepositoryTag")

            tag = repoTag.getAttribute("Tag")
            if "" == tag:
                raise utils.StrategyError ("No Tag given for RepositoryTag " + name)

            if "head" == tag.lower():
                raise utils.StrategyError ("Tag 'HEAD' is illegal for RepositoryTag " + name)

            pullOnceVal = utils.getOptionalBoolAttr ("PullOnce", True, "PullOnce value of RepositoryTag", repoTag)
            forceBranch = utils.getOptionalBoolAttr ("ForceBranch", False, "ForceBranch value of RepositoryTag", repoTag)
            
            # --- Residual version stuff for NuGet ---
            if name.lower() in self.m_nugetSources:
                utils.showInfoMsg ('RepositoryTag for NuGet Repo {0} should be moved to NuGetSoures\n'.format(name), utils.INFO_LEVEL_SomewhatInteresting)
                self.m_nugetSources[name.lower()].Update (tag, None)
            
            else: # Normal case
                self.m_repoOptions[name.lower()] = RepositoryOptions ()
                self.m_repoOptions[name.lower()].m_legacyTag = tag
                self.m_repoOptions[name.lower()].SetLegacyPullOnce(pullOnceVal)
                self.m_repoOptions[name.lower()].m_forceCheckout = forceBranch
                self.m_repoOptionsChangeLog.append('RepositoryOptions Name={0}, Options={1}'.format(name, self.m_repoOptions[name.lower()].Dump()))

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:BuildStrategy
    #-------------------------------------------------------------------------------------------
    def ProcessRepositoryOptions(self, dom):
        repoOptionsElements = utils.getDomElementsByName(dom, "RepositoryOptions")
        for repoOption in repoOptionsElements:
            name = repoOption.getAttribute("Name")
            if "" == name:
                raise utils.StrategyError ("No Name given for RepositoryOptions")

            tag = repoOption.getAttribute ("Tag")
            branch = repoOption.getAttribute ("Branch")
            revision = repoOption.getAttribute("Revision")

            if tag and revision:
                raise utils.StrategyError ("Tag and Revision attributes cannot exist on the same RepositoryOptions")
            if tag and branch:
                raise utils.StrategyError ("Tag and Branch attributes cannot exist on the same RepositoryOptions")
            if branch and revision:
                raise utils.StrategyError ("Revision and Branch attributes cannot exist on the same RepositoryOptions")

            if name.lower() not in self.m_repoOptions:
                self.m_repoOptions[name.lower()] = RepositoryOptions()

            repoOptions = self.m_repoOptions[name.lower()]

            if repoOption.hasAttribute("Tag"):
                repoOptions.m_tag = tag
            if repoOption.hasAttribute("Branch"):
                repoOptions.m_branch = branch
            if repoOption.hasAttribute("Revision"):
                repoOptions.SetRevision(revision)
            if repoOption.hasAttribute("PullToTip"):
                repoOptions.m_pullToTip = utils.getOptionalBoolAttr ("PullToTip", False, "PullToTip value of RepositoryOptions", repoOption)
            if repoOption.hasAttribute("SkipPull"):
                repoOptions.m_skipPull = utils.getOptionalBoolAttr ("SkipPull", False, "SkipPull value of RepositoryOptions", repoOption)
            if repoOption.hasAttribute("ForceCheckout"):
                repoOptions.m_forceCheckout = utils.getOptionalBoolAttr ("ForceCheckout", False, "ForceCheckout value of RepositoryOptions", repoOption)
            repoOptions.m_legacyTag = None
            self.m_repoOptionsChangeLog.append('RepositoryOptions Name={0}, Options=({1})'.format(name, repoOptions.Dump()))

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def ProcessWixBundleStrategy (self, dom):
        wixBundlePartDoms = utils.getDomElementsByName(dom, "WixBundlePart")
        
        if len (wixBundlePartDoms) == 0 or wixBundlePartDoms == None:
            return

        wixBundlePartDom = wixBundlePartDoms [0]
        self.m_wixBundleStrategy = (wixBundlePartDom.getAttribute ("PartName"), wixBundlePartDom.getAttribute ("PartFile"), wixBundlePartDom.getAttribute ("Repository"))

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def ProcessLKGSources (self, dom):
        lkgName = utils.getDomElementsByName(dom, "DefaultLastKnownGoodSource")
        if len (lkgName) > 0:
            defLkgName = lkgName[0].getAttribute("Name")
            if defLkgName:
                self.m_defaultLKG = defLkgName
            bdfServer = lkgName[0].getAttribute("BDFServer")
            if bdfServer:
                self.m_defaultBdfServerName = bdfServer

        # Check for forced LKGs - used by PRG
        lkgForced = utils.getDomElementsByName(dom, "ForceLastKnownGood")
        for lkgForce in lkgForced:
            lkgTypeString = lkgForce.getAttribute("Type")
            if "" == lkgTypeString:
                raise utils.StrategyError ("No Type given for ForceLastKnownGood")

            self.m_forceLkgType = utils.resolveOption (lkgTypeString, lkgs.lkgSourceTypes, "ForceLastKnownGood Type ")
            lkgs.CheckLKGSourceType (self.m_forceLkgType)

        # Look for any servers
        lkgServers = utils.getDomElementsByName(dom, "LastKnownGoodServer")
        for lkgServer in lkgServers:
            name = lkgServer.getAttribute("Name")
            if "" == name:
                raise utils.StrategyError ("No Name given for LastKnownGoodServer")

            lkgTypeString = lkgServer.getAttribute("Type")
            if "" == lkgTypeString:
                raise utils.StrategyError ("No Type given for LastKnownGoodServer " + name)
            
            lkgType = utils.resolveOption (lkgTypeString, lkgs.lkgServerTypes, "LastKnownGoodServer Type for " + name)

            address = lkgServer.getAttribute("Address")
            if "" == address:
                raise utils.StrategyError ("No Address given for LastKnownGoodServer " + name)
                
            isVersioned = False
            isVersionedString = lkgServer.getAttribute("Versioned")
            if "" != isVersionedString:
                isVersioned = utils.resolveBoolean (isVersionedString, "LastKnownGoodServer Versioned flag for " + name)

            provider = utils.getOptionalAttr("CredentialProvider", globalvars.CREDENTIAL_PROVIDER_BASIC, globalvars.CredentialProviderTypes, "LastKnownGoodServer {0} attribute CredentialProvider".format (name), lkgServer)
            self.m_lkgServers[name.lower()] = lkgs.LKGServer (name, lkgType, address, isVersioned, provider)

        # Look for any sources
        lkgSources = utils.getDomElementsByName(dom, "LastKnownGoodSource")
        for lkgSource in lkgSources:
            name = lkgSource.getAttribute("Name")
            if "" == name:
                raise utils.StrategyError ("No Name given for LastKnownGoodSource")

            lkgTypeString = lkgSource.getAttribute("Type")
            if "" != lkgTypeString:
                lkgType = utils.resolveOption (lkgTypeString, lkgs.lkgSourceTypes, "LastKnownGoodSource Type for " + name)
            else:
                lkgType = lkgs.LKGSOURCE_dev

            lkgVersion = None
            lkgVersionString = lkgSource.getAttribute("Version")
            if "" != lkgVersionString:
                lkgVersion = lkgVersionString

            prgServer = lkgSource.getAttribute("PrgServer")
            devServer = lkgSource.getAttribute("DevServer")

            platformsList = None
            platformAttrs = lkgSource.getAttribute("Platforms")
            if len (platformAttrs) > 0:
                def handleNoPlatformMatch (csv, nonvalidMatch):
                    raise utils.StrategyError ("LastKnownGoodSource '{0}' Platforms='{1}' has platform or wildcard '{2}' that does not match any platform, must be one of {3}.".format(name, csv, nonvalidMatch, targetplatform.GetXmlOptions().__str__()))

                platformsList = []
                targetplatform.UpdatePlatformListByName (platformsList, platformAttrs, handleNoPlatformMatch)
            
            # This allows partial changs of the LKG Source.
            if name.lower() in self.m_lkgSources:
                self.m_lkgSources[name.lower()].Update (type=lkgType, prgServer=prgServer, devServer=devServer, version=lkgVersion, platforms=platformsList)
            else:
                self.m_lkgSources[name.lower()] = lkgs.LKGSource (name, lkgType, prgServer, devServer, lkgVersion, platformsList)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def ProcessSdkSources (self, dom):
        sdkSources = utils.getDomElementsByName(dom, "SdkSource")

        name = None
        def handleNoPlatformMatch (csv, nonvalidMatch):
            raise utils.StrategyError ("SdkSource '{0}' Platforms='{1}' has platform or wildcard '{2}' that does not match any platform, must be one of {3}.".format(name, csv, nonvalidMatch, targetplatform.GetXmlOptions().__str__()))

        for sdkSource in sdkSources:
            name = sdkSource.getAttribute("Name")
            if "" == name:
                raise utils.StrategyError ("No Name given for SdkSource")

            lkgTypeString = sdkSource.getAttribute("Type")
            if "" != lkgTypeString:
                lkgType = utils.resolveOption (lkgTypeString, lkgs.lkgSourceTypes, "LastKnownGoodSource Type for SdkSource " + name)
            else:
                lkgType = lkgs.LKGSOURCE_dev

            outputDir = sdkSource.getAttribute("PrgOutputDir")

            prodName = sdkSource.getAttribute("ProductName")

            lkgVersion = sdkSource.getAttribute("Version")

            lkgStreamName = sdkSource.getAttribute("Stream")

            prgServer = sdkSource.getAttribute("PrgServer")
            devServer = sdkSource.getAttribute("DevServer")

            platformsList = None
            platformAttrs = sdkSource.getAttribute("Platforms")
            if len (platformAttrs) > 0:
                platformsList = []
                targetplatform.UpdatePlatformListByName (platformsList, platformAttrs, handleNoPlatformMatch)
            
            # This allows partial changes of the LKG Source.
            if name.lower() in self.m_sdkSources:
                self.m_sdkSources[name.lower()].Update (type=lkgType, prgServer=prgServer, devServer=devServer, 
                        version=lkgVersion, platforms=platformsList, prgOutputDir=outputDir, productDir=prodName, streamName=lkgStreamName)
            else:
                if not outputDir:
                    raise utils.StrategyError ("No PrgOutputDir given for SdkSource {0}".format(name))
                if not prodName:
                    raise utils.StrategyError ("No ProductName given for SdkSource {0}".format(name))
                if not lkgVersion:
                    raise utils.StrategyError ("No Version given for SdkSource {0}".format(name))

                self.m_sdkSources[name.lower()] = SdkSource (name, lkgType, prgServer, devServer, lkgVersion, platformsList, outputDir, prodName, lkgStreamName)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def ProcessNuGetSources (self, dom):
        # First process any Feed items
        nugetFeeds = utils.getDomElementsByName(dom, "NuGetFeed")
        for feed in nugetFeeds:
            name = feed.getAttribute("Name")
            if "" == name:
                raise utils.StrategyError ("No Name given for NugetFeed")
            address = feed.getAttribute("Address")
            if "" == address:
                raise utils.StrategyError ("No Address given for NugetFeed {0}".format(name))
            provider = utils.getOptionalAttr("CredentialProvider", globalvars.CREDENTIAL_PROVIDER_BASIC, globalvars.CredentialProviderTypes, "NugetFeed {0} attribute CredentialProvider".format (name), feed)
            feedType = utils.getOptionalAttr("Type", nugetpkg.NUGETFEED_http, nugetpkg.NuGetFeedTypes, "NugetFeed {0} attribute Type".format (name), feed)
            self.m_nugetFeeds[name] = nugetpkg.NuGetFeed (name, address, provider, feedType)
            
        # Grab any Defaults - feed must be defined by now
        nugetDefaultList = utils.getDomElementsByName(dom, "DefaultNuGetFeed")
        for nugetDefault in nugetDefaultList:
            pullFeed = nugetDefault.getAttribute("Pull")
            if pullFeed:
                if not pullFeed in self.m_nugetFeeds:
                    raise utils.StrategyError ("Default Pull Nuget Feed {0} has not been defined".format(pullFeed))
                self.m_defaultNugetPullFeed = self.m_nugetFeeds[pullFeed]

            pushFeed = nugetDefault.getAttribute("Push")
            if pushFeed:
                if not pushFeed in self.m_nugetFeeds:
                    raise utils.StrategyError ("Default Push Nuget Feed {0} has not been defined".format(pushFeed))
                self.m_defaultNugetPushFeed = self.m_nugetFeeds[pushFeed]

        # Now process the packages
        nugetSources = utils.getDomElementsByName(dom, "NuGetSource")

        name = None
        def handleNoPlatformMatch (csv, nonvalidMatch):
            raise utils.StrategyError ("NuGetSource '{0}' Platforms='{1}' has platform or wildcard '{2}' that does not match any platform, must be one of {3}.".format(name, csv, nonvalidMatch, targetplatform.GetXmlOptions().__str__()))

        for nugetSource in nugetSources:
            name = nugetSource.getAttribute("Name")
            if "" == name:
                raise utils.StrategyError ("No Name given for NuGetSource")

            alias = nugetSource.getAttribute("Alias")

            if not alias:
                alias = name

            version = nugetSource.getAttribute("Version")
            if "" != nugetSource.getAttribute ('FromBB'):
                raise utils.StrategyError ("Please replace 'FromBB' with 'Multiplatform' in NuGetSource {0}. The attribute was renamed to better reflect its purpose.".format(name))

            multiplatform = utils.getOptionalBoolAttr ('Multiplatform', False, 'Multiplatform', nugetSource)
            feed = nugetSource.getAttribute("Feed")
            feed = feed if feed else None   # Switch from '' to None

            platformsList = None
            platformAttrs = nugetSource.getAttribute("Platforms")
            if len (platformAttrs) > 0:
                platformsList = []
                targetplatform.UpdatePlatformListByName (platformsList, platformAttrs, handleNoPlatformMatch)
            
            # This allows partial changes of the Nuget Source.
            if alias.lower() in self.m_nugetSources:
                self.m_nugetSources[alias.lower()].Update (version, platformsList, feed)
            else:
                if not version:
                    raise utils.StrategyError ("No Version given for NuGetSource {0}".format(name))

                self.m_nugetSources[alias.lower()] = nugetpkg.NuGetSource (name, version, platformsList, multiplatform, feed=feed, alias=alias)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:BuildStrategy
    #-------------------------------------------------------------------------------------------
    def ProcessADOBuildArtifactSources (self, dom):
        # Now process ADOBuildArtifactSources
        artifactSources = utils.getDomElementsByName(dom, "ADOBuildArtifactSource")
        for artifactSource in artifactSources:
            alias = artifactSource.getAttribute("Alias")
            if "" == alias:
                raise utils.StrategyError ("No Alias given for ADOBuildArtifactSource")
            artifactName = artifactSource.getAttribute("ArtifactName")
            if "" == artifactName:
                raise utils.StrategyError ("No ArtifactName given for ADOBuildArtifactSource {0}".format(alias))
            version = artifactSource.getAttribute("Version")
            if "" == version:
                raise utils.StrategyError ("No Version given for ADOBuildArtifactSource {0}".format(alias))
            definitionId = artifactSource.getAttribute("DefinitionId")
            if "" == definitionId:
                raise utils.StrategyError ("No DefinitionId given for ADOBuildArtifactSource {0}".format(alias))
            project = artifactSource.getAttribute("Project")
            if "" == project:
                raise utils.StrategyError ("No Project given for ADOBuildArtifactSource {0}".format(alias))
            organization = artifactSource.getAttribute("Organization")
            if "" == organization:
                raise utils.StrategyError ("No Organization given for ADOBuildArtifactSource {0}".format(alias))

            self.m_adoBuildArtifactSources[alias.lower()] = ADOBuildArtifactSource(alias, artifactName, version, definitionId, project, organization)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def ProcessUpackSources (self, dom):
        # First process any Feed items
        upkgFeeds = utils.getDomElementsByName(dom, "UpackFeed")
        for feed in upkgFeeds:
            name = feed.getAttribute("Name")
            if "" == name:
                raise utils.StrategyError ("No Name given for UpackFeed")
            address = feed.getAttribute("Address")
            if "" == address:
                raise utils.StrategyError ("No Address given for UpackFeed {0}".format(name))
            ufeed = feed.getAttribute("Feed")
            if "" == ufeed:
                raise utils.StrategyError ("No Feed given for UpackFeed {0}".format(name))
            feedType = utils.getOptionalAttr("Type", universalpkg.UPACKFEED_azurecli, universalpkg.UPackFeedTypes, "UpackFeed {0} attribute Type".format (name), feed)
            self.m_upkgFeeds[name] = UpackFeed(name, address, ufeed, feedType)
            
        # Grab any Defaults - feed must be defined by now
        defaultList = utils.getDomElementsByName(dom, "DefaultUpackFeed")
        for defaultItem in defaultList:
            pullFeed = defaultItem.getAttribute("Pull")
            if pullFeed:
                if not pullFeed in self.m_upkgFeeds:
                    raise utils.StrategyError ("Default Pull Upack Feed {0} has not been defined".format(pullFeed))
                self.m_defaultUpkgPullFeed = self.m_upkgFeeds[pullFeed]

            pushFeed = defaultItem.getAttribute("Push")
            if pushFeed:
                if not pushFeed in self.m_upkgFeeds:
                    raise utils.StrategyError ("Default Push Upack Feed {0} has not been defined".format(pushFeed))
                self.m_defaultUpkgPushFeed = self.m_upkgFeeds[pushFeed]

        # Now process Universal Packages
        upkgSources = utils.getDomElementsByName(dom, "UpackSource")

        name = None
        def handleNoPlatformMatchUpkg (csv, nonvalidMatch):
            raise utils.StrategyError ("UpackSource '{0}' Platforms='{1}' has platform or wildcard '{2}' that does not match any platform, must be one of {3}.".format(name, csv, nonvalidMatch, targetplatform.GetXmlOptions().__str__()))

        def GetAttributeOrNone (upkgSource, attrName):
            attr =  upkgSource.getAttribute(attrName)
            return attr if attr else None   # Switch from '' to None

        for upkgSource in upkgSources:
            name = upkgSource.getAttribute("Name")
            alias = upkgSource.getAttribute("Alias")

            if not alias:
                alias = name

            if "" == name:
                raise utils.StrategyError ("No Name given for UpackSource")

            version = GetAttributeOrNone(upkgSource, "Version")
            feed = GetAttributeOrNone(upkgSource, "Feed")
            partFile = GetAttributeOrNone(upkgSource, "PartFile")
            partName = GetAttributeOrNone(upkgSource, "PartName")
            multiPlatform = utils.getOptionalBoolAttr ('Multiplatform', False, 'Are multiple platforms available using bb naming conventions?', upkgSource)

            if (not partFile and partName) or (partFile and not partName):
                raise utils.StrategyError ("UpackSource {0} must have either both a PartFile and PartName or neither ({1}:{2})".format(name, partFile, partName))

            platformsList = None
            platformAttrs = upkgSource.getAttribute("Platforms")
            if len (platformAttrs) > 0:
                platformsList = []
                targetplatform.UpdatePlatformListByName (platformsList, platformAttrs, handleNoPlatformMatchUpkg)
            
            # This allows partial changes of the Package.
            if alias.lower() in self.m_upackSources:
                self.m_upackSources[alias.lower()].Update (version, platformsList, feed, partFile, partName, multiPlatform)
            else:
                if not version:
                    raise utils.StrategyError ("No Version given for UpackSource {0}".format(name))

                self.m_upackSources[alias.lower()] = UpackSource (name, version, platformsList, feed, partFile, partName, multiPlatform, alias)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def ProcessNpmFeeds (self, dom):
        # First process any Feed items
        NpmFeeds = utils.getDomElementsByName(dom, "NpmFeed")
        for feed in NpmFeeds:
            name = feed.getAttribute("Name")
            if "" == name:
                raise utils.StrategyError ("No Name given for NpmFeed")
            address = feed.getAttribute("Address")
            if "" == address:
                raise utils.StrategyError ("No Address given for NpmFeed {0}".format(name))
            provider = utils.getOptionalAttr("CredentialProvider", globalvars.CREDENTIAL_PROVIDER_AUTO, globalvars.CredentialProviderTypes, "NpmFeed {0} attribute CredentialProvider".format (name), feed)
            self.m_npmFeeds[name] = NpmFeed (name, address, provider)
            
        # Grab any Defaults - feed must be defined by now
        NpmDefaultList = utils.getDomElementsByName(dom, "DefaultNpmFeed")
        for NpmDefault in NpmDefaultList:
            pullFeed = NpmDefault.getAttribute("Pull")
            if pullFeed:
                if not pullFeed in self.m_npmFeeds:
                    raise utils.StrategyError ("Default Pull Npm Feed {0} has not been defined".format(pullFeed))
                self.m_defaultNpmPullFeed = self.m_npmFeeds[pullFeed]
                utils.showInfoMsg('NPM default pull feed name {0} address {1}\n'.format (self.m_defaultNpmPullFeed.m_name, self.m_defaultNpmPullFeed.m_address), utils.INFO_LEVEL_RarelyUseful)

            pushFeed = NpmDefault.getAttribute("Push")
            if pushFeed:
                if not pushFeed in self.m_npmFeeds:
                    raise utils.StrategyError ("Default Push Npm Feed {0} has not been defined".format(pushFeed))
                self.m_defaultNpmPushFeed = self.m_npmFeeds[pushFeed]
                utils.showInfoMsg('NPM default push feed name {0} address {1}\n'.format (self.m_defaultNpmPushFeed.m_name, self.m_defaultNpmPushFeed.m_address), utils.INFO_LEVEL_RarelyUseful)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def ProcessStrategyFile (self, strategyName):
        if strategyName in self.m_accessedFiles:    # never include strategy files more than once
            return
        self.AddAccessedFile (strategyName)

        try:
            bsDom = utils.parseXml (strategyName)
        except xml.parsers.expat.ExpatError as errIns:
            raise utils.StrategyError("Parse Error : " + errIns.__str__())

        bsNodes = utils.getDomElementsByName (bsDom, "BuildStrategy")
        strategyDom = bsNodes[0]
        self.ProcessIncludes (strategyDom)
        self.ProcessAliases (strategyDom)
        self.ProcessBuildControl (strategyDom)
        self.ProcessStrategyEnvVariables(strategyDom)   # Before anything else so that variables will be available for other "Process*" methods if required.
        self.ProcessRemoteRepositoryLists (strategyDom)
        self.ProcessLocalRepositories (strategyDom)
        self.ProcessDefaults (strategyDom)
        self.ProcessLKGSources (strategyDom)            # Before UseLastKnownGood so that a version can be set.
        self.ProcessUseLastKnownGoods (strategyDom)     # Before PartStrategiees so FromLKG can be overridden.
        self.ProcessPartStrategies (strategyDom)
        self.ProcessDeferStrategies (strategyDom)
        self.ProcessSdkSources (strategyDom)
        self.ProcessNuGetSources (strategyDom)
        self.ProcessUpackSources (strategyDom)
        self.ProcessADOBuildArtifactSources (strategyDom)
        self.ProcessNpmFeeds (strategyDom)
        self.ProcessPullBDF (strategyDom)
        self.ProcessRepoTags (strategyDom)
        self.ProcessRepositoryOptions (strategyDom)
        self.ProcessWixBundleStrategy (strategyDom)
        self.ProcessToolsets (strategyDom)
        self.ProcessToolParts (strategyDom)
        self.ProcessL10nProduct (strategyDom)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def DoStrategyFile (self, strategyName, root):
        strategyName = self.FindBuildStrategy(strategyName, root)
        if not os.path.isfile(strategyName):
            raise utils.StrategyError ("Can't open BuildStrategy File '{0}'\n".format (strategyName))

        saveCurr = globalvars.currentStrategyFile
        globalvars.currentStrategyFile = strategyName
        try:
            self.ProcessStrategyFile (strategyName)
            globalvars.currentStrategyFile = saveCurr

        except utils.StrategyError as err:
            if None == globalvars.currentStrategyFile:
                err.errmessage = err.errmessage + "\n   " + strategyName
            globalvars.currentStrategyFile = None
            raise utils.StrategyError (err.errmessage)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def FindBuildStrategy (self, strategyName, root):
        if os.path.isabs(strategyName):
            return strategyName
        repo, stratBase = self.SplitStratAndRepo(strategyName)
        if repo:
            self.GetAliases(True)  # Makes sure the LocalRepositories are loaded up.
            return self.DownloadStrategyFile (repo, stratBase)
        return symlinks.normalizePathName(findBuildStratFile(strategyName, root))

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetAliases (self, loadRepos):
        if not self.m_strategyAliases:
            if loadRepos and not translationkit.IsTranskitShell():
                # Read RepositoryList and Aliases
                stratFile = self.FindBuildStrategy('RepositoryLists', None)
                if not os.path.isfile(stratFile):
                    raise utils.StrategyError ("Can't open RepositoryList.BuildStrategy File for aliases '{0}'\n".format (stratFile))
                self.ProcessStrategyFile (stratFile)

                self.m_remoteRepoLists.AddEntriesFromConfig ()
# TODO: Find if this is needed
#                    if self.m_commandLineBdf:
#                        self.m_aliasTeamConfigRepoList.AddCmdLineBdf (utils.parseRevisionSource (self.m_commandLineBdf))

            stratFile = self.FindBuildStrategy('Aliases', None)
            if not os.path.isfile(stratFile):
                # It's a tough call - should the file be required? Certainly at first it won't be around.
                # raise utils.StrategyError ("Cannot open Aliases.BuildStrategy file for aliases '{0}'\n".format (stratFile))
                utils.showInfoMsg('Cannot open Aliases.BuildStrategy file for aliases\n', utils.INFO_LEVEL_RarelyUseful)
                self.m_strategyAliases = []  # It's ok for now.
            else:
                self.ProcessStrategyFile (stratFile)

        return self.m_strategyAliases

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def SplitStratAndRepo (self, stratPiece):
        if ':' in stratPiece and stratPiece[1] != ':':  # No 1-char repo names so full path to strat can be specified.
            pieceSplit = stratPiece.split(':')
            return pieceSplit[0], pieceSplit[1]
        return None, stratPiece

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def ExpandSingleStrat (self, stratName, aliasesUsed, levelList):
        aliases = self.GetAliases(False)
        semiSplit = stratName.split(';')
        newStratList = []
        newMultiStratList = []
        for stratPiece in semiSplit:
            resultStrat = stratPiece
            repo, stratBase = self.SplitStratAndRepo(stratPiece)
            utils.showInfoMsg('Evaluating strategy repo={0} base={1} from {2}\n'.format (repo, stratBase, stratPiece), utils.INFO_LEVEL_RarelyUseful)
            # If there's a repo specified then it's not an alias
            # If it has any path pieces then also isn't an alias. It won't match alias dict.
            if not repo and stratBase.lower() in aliases:
                origBase = stratBase # For error message 
                aliasesUsed.add(stratBase)  # Keeping track to look for files with the same name later
                stratBase = aliases[stratBase.lower()][1]
                utils.showInfoMsg('Expanded alias "{0}" to "{1}"\n'.format (origBase, stratBase), utils.INFO_LEVEL_RarelyUseful)
                aliasRepo, _ = self.SplitStratAndRepo(stratBase)

                # Can't have an interation (+) in the middle of an accumulation (;)
                if len (semiSplit) > 1 and '+' in stratBase:
                    raise utils.StrategyError ("Strategy expanded to have a '+' in an ';' clause in part '{0}' of '{1}'\nresult:\n{2} \nstack:\n{3}\n".format 
                                               (origBase, stratName, ' + '.join(newStratList), repr(levelList)))

                if '+' in stratBase or ';' in stratBase:
                    # Multiple strategies defined in the alias. Recurse and expand them.
                    expandedList = self.ExpandStratList (stratBase, aliasesUsed, levelList)
                elif aliasRepo:
                    # Single strategy in alias with repo, down't recurse. This is important so you can alias BentleyLib to Bentley:BentleyLib without infinite recursion.
                    expandedList = [stratBase]
                else:
                    # Single strategy in alias without repo. Recurse to evaluate.
                    expandedList = self.ExpandStratList (stratBase, aliasesUsed, levelList)

                if len(expandedList) == 1:
                    newStratList += expandedList
                else:
                    newMultiStratList += expandedList
            else:
                newStratList.append (resultStrat)
        
        returnList = [';'.join(newStratList)] if newStratList else newMultiStratList
        return returnList

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def ExpandStratList (self, fullStrat, aliasesUsed, levelList):
        # Prevent eternal recursion.

        if levelList == None:
            levelList = []
        levelList.append (fullStrat)
        if len(levelList) > 10:
            raise utils.StrategyError ("Strategy resolution depth exceeds 10\n{0}\n".format (repr(levelList)))

        separateStrats = fullStrat.split('+')
        expandedStrats = []
        if aliasesUsed == None:
            aliasesUsed = set()
        for oneStrat in separateStrats:
            expandedStrats.extend(self.ExpandSingleStrat (oneStrat, aliasesUsed, levelList))

        levelList.pop()

        # Look for files that match the aliases that were used; shouldn't have both or it will be too confusing.
        if not levelList:
            for alias in aliasesUsed:
                stratFile = self.FindBuildStrategy(alias, None)
                # The strat file may be found in the local directory, and we want to eliminate that. Also made sure that cwd isn't BuildStrategies.
                # Note that it will find it in the local directory first, so if the dev runs bb in the local repo dir that has the strategy 
                # it won't pick up on the other strategy in BuildStrategies.
                if os.path.exists(stratFile):
                    if stratFile.lower().startswith(GetDefaultStrategyDir().lower()) or not stratFile.lower().startswith(os.getcwd().lower()):
                        raise utils.StrategyError ("Strategy file and alias with the same name: {0}  {1}\n".format (alias, stratFile))

        return expandedStrats

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    @staticmethod
    def ExpandStrategyAliases (strategyName):
        strat = BuildStrategy()
        return strat.ExpandStratList(strategyName, None, None)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def FindLKGSource (self, name, partInfo):
        if None == name:
            name = self.m_defaultLKG
        if None == name:
            if partInfo is not None:
                raise utils.StrategyError("No LastKnownGoodSource provided for {0} in PartFile {1}\n".format(partInfo.m_name, partInfo.m_file))
            else:
                raise utils.StrategyError("No default LastKnownGoodSource set to retrieve\n")
        try:
            return self.m_lkgSources[name.lower()]
        except KeyError as _:
            if partInfo is not None:
                raise utils.StrategyError("Can't find LastKnownGoodSource {0} for Part {1} in Partfile {2}\n".format(name, partInfo.m_name, partInfo.m_file))
            else:
                raise utils.StrategyError("Can't find LastKnownGoodSource {0}\n".format(name))

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def FindLocalRepository (self, name):
        if name.lower() not in self.m_localRepos:
            if name.lower() in self.m_nugetSources:
                raise utils.StrategyError ("Can't find LocalRepository " + name + "\nPlease change your RequiredRepository element to a NuGetPackage element for " + name + "\n")
            else:
                raise utils.StrategyError ("Can't find LocalRepository " + name + "\n")
        return self.m_localRepos[name.lower()]

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def FindLocalRepositoryByDirname (self, dirname):
        dirnameLower = dirname.lower()
        for name in self.m_localRepos:
            if dirnameLower == self.m_localRepos[name].GetLocalDir().lower():
                return self.m_localRepos[name]
    
        raise utils.StrategyError ("Can't find LocalRepository for directory " + dirname + "\n")

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def AddLocalRepository (self, repo):
        name = repo.m_name.lower()
        if not name in self.m_localRepos:
            self.m_localRepos[name] = repo

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def ReplaceLocalRepository (self, repo):
        # Since path is immutable, you must use this if updating the path.
        self.m_localRepos[repo.m_name.lower()] = repo

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def HasLocalRepository (self, name):
        return name.lower() in self.m_localRepos

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def FindLocalL10NRepository (self, name, languageSetting):
        l10nRepo        = None
        l10nRepoName    = translationkit.getL10NRepoName (name, languageSetting)

        try:
            l10nRepo = self.FindLocalRepository (l10nRepoName)
        except:
            utils.ShowAndDeferMessage ("Cannot find repository {0}\n".format (l10nRepoName), utils.INFO_LEVEL_SomewhatInteresting)

        return l10nRepo

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:BuildStrategy
    #-------------------------------------------------------------------------------------------
    def FindRepositoryOptions (self, name):
        name = name.lower()
        if not name in self.m_repoOptions:
            return None

        repoOptions = self.m_repoOptions[name]
        if repoOptions:  # Expand here rather than on read because strategies can modify env. This handles ${}.
            if repoOptions.m_tag:
                repoOptions.m_tag = os.path.expandvars(repoOptions.m_tag)
            if repoOptions.m_branch:
                repoOptions.m_branch = os.path.expandvars(repoOptions.m_branch)
            if repoOptions.m_revision:
                repoOptions.m_revision = os.path.expandvars(repoOptions.m_revision)
            if repoOptions.m_legacyTag:
                repoOptions.m_legacyTag = os.path.expandvars(repoOptions.m_legacyTag)
        return repoOptions

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetRemoteRepositoryList (self):
        return self.m_remoteRepoLists

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def FindRemoteRepository (self, repoName, repoType=None):
        return self.GetRemoteRepositoryList().FindRepository (repoName, repoType)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def FindRemoteL10Repository (self, repoName, languageSetting):
        l10nRepo        = None
        l10nRepoName    = translationkit.getL10NRepoName (repoName, languageSetting)
        try:
            l10nRepo = self.FindRemoteRepository (l10nRepoName)
        except:
            utils.ShowAndDeferMessage ("Cannot find repository {0}\n".format (l10nRepoName), utils.INFO_LEVEL_SomewhatInteresting)

        return l10nRepo

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def FindPartStrategy (self, partFile, partName):
        if not self.m_usePartStrategyFindCache:
            # Notice - this is a linear search.
            for strategy in self.m_partStrategies:
                if strategy.DoesMatch (partFile, partName):
                    return strategy
        else:  
            # Use cached result of whatever was found previously.
            keyName = "{0}{1}".format (partFile, partName)
            entry = self.m_foundPartStrategyCache.get (keyName)
            if None != entry:
                entry[1] += 1 # Keep track of how many times the cached entry was used for debug analysis.
                return entry[0]
            # Notice - this is a linear search. Cache the result so we don't search again.
            for strategy in self.m_partStrategies:
                if strategy.DoesMatch (partFile, partName):
                    self.m_foundPartStrategyCache[keyName] = [strategy, 1]
                    return strategy
            self.m_foundPartStrategyCache[keyName] = [self.m_default, 1]

        return self.m_default

    #--------------------------------------------------------------------------------
    # @bsimethod
    #--------------------------------------------------------------------------------
    def DumpCachedCallCount (self):
        sl = []
        for k, v in self.m_foundPartStrategyCache.items():
            sl.append ([v[1],k])

        maxCount = 20
        msg = "Dumping top {0} entries from BuildStrategy FindPartStrategy cache.\n This indicates how many times the cached entry was reused.\n".format (repr(maxCount))
        utils.showInfoMsg(msg, utils.INFO_LEVEL_Essential)

        count = 0
        for item in sorted (sl, reverse=True):
            msg = "\t{0} : {1}\n".format (item[0], item[1])
            utils.showInfoMsg(msg, utils.INFO_LEVEL_Essential)

            count += 1
            if count >= maxCount:
                break

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def FindExactPartStrategy (self, partFile, partName):
        partFile = partFile.lower()
        partName = partName.lower()
        for strategy in self.m_partStrategies:
            if strategy.m_name[0].lower() == partFile and strategy.m_name[1].lower() == partName:
                return strategy 

        return None

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def FindProvenanceErrorStrategy (self, repoName):
        # Granted, this is not a "strategy" in the sense of BuildStrategy or PartStrategy,
        # it's just an integer that specifies what to do when we encounter a provenance error
        for errorStrategy in self.m_onProvenanceErrors:
            if utils.NameMatch (repoName, errorStrategy[0]):
                return errorStrategy[1]

        return self.m_defaultOnProvenanceErrors

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def Dump (self, options):

        partStrategyOverlays = []
        envVariablesChangelog = []
        repositoryOptionsChangelog = []
        if options.strategies or options.strategy_env:
            strat = BuildStrategy ()
            strat.RecreateStrategy (self.m_strategyName, self.m_additionalImports, partStrategyOverlays=partStrategyOverlays, envVariablesChangelog=envVariablesChangelog, repoOptionsChangeLog=repositoryOptionsChangelog)
        else:
            strat = self

        if options.strategies:
            if options.tabComplete != None and not options.tabComplete:
                utils.showInfoMsg( "Created from Strategy Files:\n", utils.INFO_LEVEL_Essential)
                for accfile in strat.m_accessedFiles:
                    utils.showInfoMsg( "    {0}\n".format (accfile), utils.INFO_LEVEL_Essential)

        if options.defaults:
            utils.showInfoMsg( "\nDefaults:\n", utils.INFO_LEVEL_Essential)
            utils.showInfoMsg( "   Target: Repository={0}, PartFile={1}, PartName={2}\n".format (strat.m_defaultRepo, strat.m_defaultPartFile, strat.m_defaultPartName), utils.INFO_LEVEL_Essential)
#            utils.showInfoMsg( "   StreamName={0}\n".format (repository.getStreamName()), utils.INFO_LEVEL_Essential) # TODO Change to Git repo branch
            utils.showInfoMsg( "   OutputRootDir={0} ({1})\n".format(strat.m_defaultOutputRootDir, self.ExpandEnvironmentVariables(strat.m_defaultOutputRootDir)), utils.INFO_LEVEL_Essential)
            if strat.m_forceLkgType:
                utils.showInfoMsg( "   ForcedLastKnownGood: type={0}\n".format(lkgs.lkgSourceTypes[strat.m_forceLkgType]), utils.INFO_LEVEL_Essential)
            utils.showInfoMsg( "   Provenance: OnError={0}\n".format(provenanceErrors[strat.m_defaultOnProvenanceErrors]), utils.INFO_LEVEL_Essential)
            for tsPlat in sorted(strat.m_toolsetForPlatform.keys()):
                tsName, tsLibType = strat.m_toolsetForPlatform[tsPlat]
                utils.showInfoMsg( "   Toolset={0} for Platform={1} libTypte={2}\n".format (tsName, tsPlat, tsLibType), utils.INFO_LEVEL_Essential)
                if tsPlat in self.m_toolVersionForPlatform:
                    ver = self.m_toolVersionForPlatform [tsPlat]
                    if ver:
                        utils.showInfoMsg( "       ToolVersion={0}\n".format (ver), utils.INFO_LEVEL_Essential)
                if tsPlat in self.m_dotNetRuntimeForPlatform:
                    dotNetRuntime = self.m_dotNetRuntimeForPlatform [tsPlat]
                    if dotNetRuntime:
                        utils.showInfoMsg( "       TargetDotNetRuntime={0}\n".format (dotNetRuntime), utils.INFO_LEVEL_Essential)
                if tsPlat in self.m_windowsSdkVersionForPlatform:
                    windowsSdkVersion = self.m_windowsSdkVersionForPlatform [tsPlat]
                    if windowsSdkVersion:
                        utils.showInfoMsg( "       TargetWindowsSdkVersion={0}\n".format (windowsSdkVersion), utils.INFO_LEVEL_Essential)

            utils.showInfoMsg( "\nLastKnownGoodSources:\n", utils.INFO_LEVEL_Essential)
            for name, lkgSource in sorted (strat.m_lkgSources.items()):
                lkgSource.Dump(4)

            utils.showInfoMsg( "\nLastKnownGoodServers:\n", utils.INFO_LEVEL_Essential)
            for name, lkgServer in sorted (strat.m_lkgServers.items()):
                lkgServer.Dump(4)

        if options.repos:
            utils.showInfoMsg( "\nResolved Repository Options:\n", utils.INFO_LEVEL_Essential)
            for line in repositoryOptionsChangelog:
                utils.showInfoMsg( '    ' + line + '\n', utils.INFO_LEVEL_Essential)

            if options.tabComplete:
                repos = []
                for name, repo in sorted (strat.m_localRepos.items()):
                    repos.append(name)
                repos = sorted(repos, key=lambda s: s.lower())
                for name in repos:
                    utils.showInfoMsg(name + '\n', utils.INFO_LEVEL_Essential)
            else:
                utils.showInfoMsg( "\nLocalRepositories:\n", utils.INFO_LEVEL_Essential)
                for name, repo in sorted (strat.m_localRepos.items()):
                    repo.Dump(4)

        if options.strategy_env:
            if not options.tabComplete:
                utils.showInfoMsg( "\nStrategyEnvVariables as they were added:\n", utils.INFO_LEVEL_Essential)
                for sev in envVariablesChangelog:
                    utils.showInfoMsg( '    ' + sev + '\n', utils.INFO_LEVEL_Essential)

                utils.showInfoMsg( "\nResolved StrategyEnvVariables:\n", utils.INFO_LEVEL_Essential)
                stratEnvDefVars = strat.GetStrategyDefinedEnvVariables()
                stratEnvDefVars.sort(key=(lambda e: e.GetName()))
                for sev in stratEnvDefVars:
                    sev.Dump (4)

                utils.showInfoMsg( "\nResolved Strategy Environment:\n", utils.INFO_LEVEL_Essential)
                stratEnvVars = strat.GetEnvVariables()
                stratEnvVars.sort(key=(lambda e: e.GetName()))
                for sev in stratEnvVars:
                    sev.Dump (4)

        if options.strategies:
            if options.tabComplete:
                strats = []
                for pstrat in strat.m_partStrategies:
                    strats.append(pstrat.m_name[0])
                strats = sorted(strats, key=lambda s: s.lower())
                for s in strats:
                    utils.showInfoMsg("{0}\n".format(s), utils.INFO_LEVEL_Essential)
            else:
                utils.showInfoMsg( "\nPart Strategies as they were added:\n", utils.INFO_LEVEL_Essential)
                for overlay in partStrategyOverlays:
                    utils.showInfoMsg( '    ' + overlay + '\n', utils.INFO_LEVEL_Essential)

                utils.showInfoMsg( "\nResolved Part Strategies (in order of precedence):\n", utils.INFO_LEVEL_Essential)
                for pstrat in strat.m_partStrategies:
#                   if pstrat == pstrat.FindPartStrategy (pstrat.m_name[0], pstrat.m_name[1]): # Check to make sure it's unique; should be only one now.
                    pstrat.Dump (4)

                strat.m_default.Dump(4)

    # The following methods are for cleaning up the output of properties that we want to expand
    # variables for. For example, we need a way to say "the last path component of m_defaultPartFile",
    # so there's a helper function here to return it.

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetDefaultPartFileBasename (self):
        baseName = self.GetDefaultPartFileName()
        if baseName and baseName.upper().endswith ('.PARTFILE.XML'):
            baseName = baseName[0:-13]
        return baseName

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetDefaultPartFileName (self):
        if globalvars.programOptions.partFile:
            return os.path.basename (globalvars.programOptions.partFile)
        elif self.m_defaultPartFile:
            return os.path.basename (self.m_defaultPartFile)
        else:
            return None


