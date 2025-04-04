#---------------------------------------------------------------------------------------------
#  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
#  See LICENSE.md in the repository root for full copyright notice.
#---------------------------------------------------------------------------------------------
import os, threading, xml

from . import azurecli, azuredevopsrestapi, bbutils, bdfcache, builddescriptionfile, buildpaths, cmdutil
from . import internal, globalvars, nugetpkg, symlinks, utils, versionutils

LKGTYPE_fileshare   = 0
LKGTYPE_rsync       = 1
LKGTYPE_local       = 2
LKGTYPE_nuget       = 3
LKGTYPE_azurebuildartifact = 4
lkgServerTypes   = ['fileshare', 'rsync', 'local', 'nuget', 'azurebuildartifact']

lkgSourceTypes   = ['dev', 'prg']
LKGSOURCE_dev       = lkgSourceTypes.index ('dev')
LKGSOURCE_prg       = lkgSourceTypes.index ('prg')

CHECK_COMPLETE_FLAGS = 'checkComplete.flag'
PRODUCT_FLAG = 'prod.{0}.flag' # format with current platform
NONPROD_FLAG = 'lkg.{0}.flag' # format with current platform

# These two speed up pulls.  There's an bit of a leak here, but the theory is that a get is an
#   atomic operation and then bb will exit.  If things change, create a context object in the get scripts
# Cache every directory listing from the server to reduce the number of times we have to hit it.
s_versionDirListingCache = {}
# Also hang onto any BDFs we need to load
s_bdfsDownloaded = {}

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def WriteCheckCompleteFlag(path):
    # Simply create this flag file to indicate that we are going to write completion flags, but only if it doesn't exist.
    # Either SaveLKGs or SaveProduct might have already added it -- no need to risk file contention
    symlinks.makeSureDirectoryExists(path)
    flagFilePath = os.path.join(path, CHECK_COMPLETE_FLAGS)
    if not os.path.exists(flagFilePath):
        open(flagFilePath, 'wt').close()

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def CheckLKGServerType (lkgType):
    typeArray = [LKGTYPE_fileshare, LKGTYPE_rsync, LKGTYPE_local, LKGTYPE_nuget, LKGTYPE_azurebuildartifact]
    if not lkgType in typeArray:
        raise utils.StrategyError ('LKG type {0} is not valid'.format(repr(lkgType)))

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def GetLKGServerTypeFromName (lkgTypeName):
    if lkgTypeName not in lkgServerTypes:
        raise utils.StrategyError ('LKG type {0} is not valid'.format(repr(lkgTypeName)))
    return lkgServerTypes.index(lkgTypeName)

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def CheckLKGSourceType (lkgType):
    typeArray = [LKGSOURCE_dev, LKGSOURCE_prg]
    if not lkgType in typeArray:
        raise utils.StrategyError ('LastKnownGoodSource source type {0} is not valid'.format(repr(lkgType)))

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def GetRsyncDirListing (pathToCheck):
    if not pathToCheck in s_versionDirListingCache:
        dirList = cmdutil.rsyncListDir (pathToCheck)
        s_versionDirListingCache[pathToCheck] = dirList
    return s_versionDirListingCache[pathToCheck]

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def GetLKGProvenanceFilePath (lkgName, platform, isStatic):
    staticDir = 'static' if isStatic else ''
    return os.path.join (buildpaths.getLKGRoot(platform), staticDir, lkgName + globalvars.provfileName)

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def ReadLKGProvenance(provFilePath):
    try:
        provDom = utils.parseXml (provFilePath)
    except xml.parsers.expat.ExpatError as errIns:
        raise utils.BuildError("Provenance parse error {0}: {1}".format(provFilePath, errIns.__str__()))

    provEl = utils.getDomElementsByName(provDom, "Provenance")
    lkgEntries = utils.getDomElementsByName(provEl[0], "LKG")
    if len(lkgEntries) != 1:
        raise utils.BuildError("Expected one LKG entry in provenance file {} but found {}".format(provFilePath, len(lkgEntries)))
    lkgSourceName = lkgEntries[0].getAttribute("LKGSourceName")
    if "" == lkgSourceName:
        raise utils.BuildError("LKG LKGSourceName not defined in provenance file {}".format(provFilePath))
    version = lkgEntries[0].getAttribute("Version")
    if "" == version:
        raise utils.BuildError("LKG Version not defined in provenance file {}".format(provFilePath))
    lkgType = lkgEntries[0].getAttribute("Type")
    if "" == lkgType:
        raise utils.BuildError("LKG Type not defined in provenance file {}".format(provFilePath))
    try: 
        lkgTypeAsInt = int(lkgType)
    except ValueError:
        raise utils.BuildError("LKG Type defined not as an integer in provenance file {}".format(provFilePath))
    serverUrl = lkgEntries[0].getAttribute("Url")
    if "" == serverUrl:
        raise utils.BuildError("LKG Url not defined in provenance file {}".format(provFilePath))
    return lkgSourceName, version, lkgTypeAsInt, serverUrl

#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
class LKGSource (object):
    def __init__(self, name, lkgType, prgServer, devServer, version, platforms):
        self.m_name = name
        self.m_type = lkgType
        self.m_prgServer = prgServer
        self.m_devServer = devServer
        self.m_server = None
        self.m_version = version
        self.m_versionSource = ''
        self.m_platforms = platforms
        self.m_threadLock = None   # Some of the BDF operations push and pop from a stack, so lock during search.  See bdfSearchedPath
        self.m_strategyVersion = None # If we're changing (locking) the LKG version due to UseLastKnownGood, store the original here for printing.
        
        CheckLKGSourceType (self.m_type)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def Update (self, **kwargs): # type, prgServer, devServer, version, platforms

        def GetValue (name, default):
            val = kwargs.get (name, None)
            return val if val != None and val != '' else default # Written this way because Type can be 0

        self.m_type =           GetValue ('type', self.m_type)
        self.m_prgServer =      GetValue ('prgServer', self.m_prgServer)
        self.m_devServer =      GetValue ('devServer', self.m_devServer)
        self.m_version =        GetValue ('version', self.m_version)
        self.m_platforms =      GetValue ('platforms', self.m_platforms)

        CheckLKGSourceType (self.m_type)

    #---------------------------------------------------------------------------------------
    # bsimethod
    #---------------------------------------------------------------------------------------
    def GetAuthenticationHeader (self, serverAddress=None):
        addressToUse = serverAddress if serverAddress else self.m_server.m_address
        return GetAuthenticationHeader(self.m_server, self.m_server.m_name, addressToUse)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetPrgOutputDir (self):
        return self.m_name

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def IsLkgType (self, typeToCheck):
        return self.m_server.m_type == typeToCheck

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def IsFromNuGet (self):
        return self.m_server.IsNuGet()

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def IsServerVersioned (self):
        return  self.m_server.IsVersioned ()

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def IsPlatformAvailable (self, platform):
        if None == self.m_platforms:   # If nothing is set, assume everything is available
            return True
            
        return platform in self.m_platforms

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetServerName (self, typeToCheck=None):
        server = None
        if typeToCheck is None:
            typeToCheck = self.m_type
        if LKGSOURCE_dev == typeToCheck:
            server = self.m_devServer
        elif LKGSOURCE_prg == typeToCheck:
            server = self.m_prgServer
        if not server:
            raise utils.StrategyError ('Missing Server Name for LastKnownGoodSource "{0}" type: {1}'.format (self.m_name, lkgSourceTypes[typeToCheck]))
        return server

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetServerTypeName (self):
        return self.m_server.GetTypeName()

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetServerUrl (self):
        return self.m_server.m_address

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetAzureOrganization (self):
        return self.m_server.GetAzureOrganization()

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetAzureProject (self):
        return self.m_server.GetAzureProject()

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def SetServer (self, lkgServer):
        self.m_server = lkgServer

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def AddThreadLock (self):
        self.m_threadLock = threading.Lock()   # Some of the BDF operations push and pop from a stack, so lock during search.  See bdfSearchedPath

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetName (self):
        return self.m_name

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def ResolveVersion (self):
        # Set up the version information.  Env Var will override strategy (print this)
        versionEnv = 'LKGVersion_'+self.GetPrgOutputDir()
        version = os.getenv (versionEnv, "")
        if version:
            self.m_version = version
            self.m_versionSource = versionEnv
            utils.showInfoMsg ("Geting LKG version from environment {0}={1}\n".format (self.m_versionSource, self.m_version), utils.INFO_LEVEL_SomewhatInteresting)
        elif self.m_version:  # Came from Strategy
            self.m_versionSource = 'LastKnownGoodSource Version'
            stratVerString = ' (original strategy version {0})'.format(self.m_strategyVersion) if self.m_strategyVersion else ''
            utils.showInfoMsg ("Using LKG version from strategy {0}={1}{2}\n".format (self.m_name, self.m_version, stratVerString), utils.INFO_LEVEL_SomewhatInteresting)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetVersionEnv (self):
        return self.m_versionSource

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetVersion (self):
        return self.m_version if self.m_version else ''

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def UpdateVersion (self, newVersion):
        if newVersion == self.m_version:
            return
        self.m_strategyVersion = self.m_version
        self.m_version = newVersion

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetNormalizedPathName (self, platform, partInfo, revisionList, isProductLkg, parent=None):
        lkgDownloadInfo = self.GetLKGDownloadInfo(platform, partInfo, revisionList, isProductLkg, parent)
        return lkgDownloadInfo.m_normalizedServerUrl if lkgDownloadInfo is not None else None

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetLKGDownloadInfo (self, platform, partInfo, revisionList, isProductLkg, parent=None):
        with self.m_threadLock:
            self.m_server.ValidateServer (self.GetPrgOutputDir(), platform, None, isProductLkg)
            return self.GetLKGDownloadInfoUnsafe (platform, partInfo.m_buildContext, partInfo.m_name, revisionList, isProductLkg, partInfo.GetPartStrategy().m_LKGSource, parent, partInfo.m_static)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetLKGDownloadInfoUnsafe (self, platform, contextName, partName, revisionList, isProductLkg, lkgSource, parent, isStatic=False):
        # Better to call GetNormalizedPathName; this was for LkgComponents
        # isStatic is used only for NuGet servers since we store LKGs into NuGet separately for dynamic and static contexts not like rsync where everything is in the same path
        # For versioned LKGs we need to possibly recurse through BDF's looking for a specific part.
        if self.IsServerVersioned ():
            return self.SearchForLkgs (platform, contextName, partName, lkgSource, revisionList, isProductLkg, parent, isStatic)
        else:
            normalizedPath = self.m_server.GetNormalizedPathName (self.m_name, platform, isProductLkg)
            return LKGDownloadInfo (contextName, platform, isStatic, normalizedPath, self.m_name, None, self.m_server.m_type, self.m_server.m_address)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetNormalizedPathNameFromServer (self, sourceName, platform, streamOrVersion, isProductLkg):
        return self.m_server.GetNormalizedPathName (sourceName, platform, streamOrVersion, isProductLkg)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def Dump (self, indent):
        utils.showInfoMsg (" "*indent + "Name='{0}', Type={1}, prgServer='{2}', devServer='{3}'\n".format (self.m_name, lkgSourceTypes[self.m_type], self.m_prgServer, self.m_devServer), utils.INFO_LEVEL_Essential)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def SearchForLkgs (self, platform, contextName, partName, prgOutputDir, revisionList, isProductLkg, parent, isStatic=False):
        # These should be reset each time we are searching for a new LKG.
        self.bdfSearchedPath = []
        self.bdfSearchedPrgDirToVersion = {}

        # Check revision list first if provided. This is used if pulling to a BDF
        if revisionList is not None and (len(revisionList.GetAllBuiltContexts()) > 0 or len(revisionList.m_lkgMapEntries) > 0):
            lkgDownloadInfo = self.SearchForLkgsViaBdf (revisionList, platform, contextName, partName, parent, isStatic, isProductLkg)
            if lkgDownloadInfo is not None:
                return lkgDownloadInfo

        if None == prgOutputDir:
            prgOutputDir = globalvars.buildStrategy.m_defaultLKG

        if None == prgOutputDir:
            raise utils.StrategyError ("No LastKnownGoodSource or DefaultLastKnownGoodSource provided for partfile {0}.\n".format (contextName))

        version = "*"
        if self.GetVersion():
            versionEnv = self.GetVersionEnv()
            version = self.GetVersion().replace('.', '-')
            if versionEnv:
                utils.showInfoMsg ("'{}' env var set to specific version '{}'.\n".format (versionEnv, version), utils.INFO_LEVEL_SomewhatInteresting)

        # Auto-correct version (there are cases where people specify 10-02-01-12 which is not a valid folder name in LKGs and should be 10-2-1-12)
        version = versionutils.Version(version).StringForFilename()

        bdf = self.m_server.GetBDF(prgOutputDir, platform, version, isProductLkg)
        if bdf:
            utils.showInfoMsg ("Using BDF {} ({}) for LKG {} resolution.\n".format (bdf.GetPrgBuildName(), builddescriptionfile.getDirStrForVersion(bdf.m_version), contextName), utils.INFO_LEVEL_RarelyUseful)
            if not isProductLkg:
                return self.SearchForLkgsViaBdf(bdf, platform, contextName, partName, parent, isStatic, isProductLkg)
            else:
                return self.SearchForLkgsInLKGSource(platform, contextName, partName, prgOutputDir, builddescriptionfile.getDirStrForVersion(bdf.m_version), isProductLkg, isStatic)

        utils.showInfoMsg ("BDF not found for LKGSource {} ({}). Searching for LKG {} on the server directly.\n".format (prgOutputDir, version, contextName), utils.INFO_LEVEL_RarelyUseful)

        if '*' in version:
            nameToSearch = contextName if self.IsFromNuGet() else prgOutputDir # NuGet server has no PrgOutputDirs only LKG names
            if self.m_server.m_type == LKGTYPE_local and '$(version)' not in self.m_server.m_address:
                utils.showInfoMsg ("Skipping version resolution for LKG {} since version doesn't play any part in the used server address.\n".format (nameToSearch), utils.INFO_LEVEL_RarelyUseful)
            else:
                versionList = self.m_server.GetVersionListing (nameToSearch, platform, False)
                path = self.m_server.GetServerPathWithoutVersion (nameToSearch, platform)
                version = getNewestRebuildOfVersion (versionList, version, self.m_name, path, isProductLkg, platform.GetXmlName(), self.IsFromNuGet())

        return self.SearchForLkgsInLKGSource(platform, contextName, partName, prgOutputDir, version, isProductLkg, isStatic)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def SearchForLkgsViaBdf (self, bdf, platform, contextName, partName, parent, isStatic, isProductLkg, serverOverride=None):
        serverToUse = self.m_server if serverOverride is None else serverOverride

        utils.showInfoMsg ("Looking for LKG {0} in BDF {1} ({2}).\n".format (contextName, bdf.GetPrgBuildName(), builddescriptionfile.getDirStrForVersion(bdf.m_version)), utils.INFO_LEVEL_SomewhatInteresting)

        self.bdfSearchedPath.append ((bdf.GetPrgBuildName(), tuple(bdf.m_version)))

        utils.showInfoMsg ("Checking BuildContexts for LKG {0} in BDF {1} ({2}).\n".format (contextName, bdf.GetPrgBuildName(), builddescriptionfile.getDirStrForVersion(bdf.m_version)), utils.INFO_LEVEL_RarelyUseful)

        # Check BuildContexts if this LKG was built with this LKGSource.
        for context in bdf.GetAllBuiltContexts():
            if context.lower() == contextName.lower():
                lkgDownloadInfo = self.SearchForLkgsInLKGSource(platform, contextName, partName, bdf.GetPrgBuildName(), builddescriptionfile.getDirStrForVersion(bdf.m_version), False, isStatic, serverToUse)
                if lkgDownloadInfo is not None:
                    self.bdfSearchedPath.pop()
                    return lkgDownloadInfo

        # Old BDFs don't have BuildContexts so we just need to check blindly
        if len(bdf.GetAllBuiltContexts()) == 0:
            utils.showInfoMsg ("No BuildContexts defined in BDF. Checking blindly.\n", utils.INFO_LEVEL_RarelyUseful)
            lkgDownloadInfo = self.SearchForLkgsInLKGSource(platform, contextName, partName, bdf.GetPrgBuildName(), builddescriptionfile.getDirStrForVersion(bdf.m_version), False, isStatic, serverToUse)
            if lkgDownloadInfo is not None:
                self.bdfSearchedPath.pop()
                return lkgDownloadInfo

        utils.showInfoMsg ("Checking LkgMap entries for LKG {0} in BDF {1} ({2}).\n".format (contextName, bdf.GetPrgBuildName(), builddescriptionfile.getDirStrForVersion(bdf.m_version)), utils.INFO_LEVEL_RarelyUseful)

        # Check the LkgPartMap. That should specify where the bdf got its SubLkgs from that aren't saved to the same LKGSource.
        if len (bdf.m_lkgMapEntries) > 0:
            shortName = buildpaths.getShortPartDescriptor(contextName+'.PartFile.xml', partName)
            accepted = []
            for key in bdf.m_lkgMapEntries:
                shortNameEntry, prgOutputDir = bdf.GetLkgMapEntry (key)

                if None != shortNameEntry and None != prgOutputDir and utils.NameMatch (shortName, shortNameEntry):
                    accepted.append ((shortNameEntry, prgOutputDir))

            usePair = None
            if len (accepted) > 1:
                usePair = accepted[0][0], accepted[0][1]
                for key, prgDir in accepted:
                    if key.lower() == shortName.lower(): # if it is fully resolved use it
                        usePair = key, prgDir

            elif len (accepted) == 1:
                usePair = accepted[0][0], accepted[0][1]

            if usePair is not None:
                prgDir = usePair[1]

                newServer = serverToUse.CreateNewServerFromBDF(bdf, prgDir)
                newServer.ValidateServer(contextName, platform, None, False)

                lkgBdf = newServer.GetBDF (prgDir, platform, builddescriptionfile.getDirStrForVersion(bdf.GetLKGVersion(prgDir)), isProductLkg)

                # Check if it's a sub-LKG
                lkgDownloadInfo = self.SearchForLkgsViaBdf (lkgBdf, platform, contextName, partName, parent, isStatic, isProductLkg, newServer)

                self.bdfSearchedPath.pop()
                return lkgDownloadInfo

            utils.showInfoMsg ("LKG {0} not found in LkgMap entries of BDF {1} ({2}).\n".format (contextName, bdf.GetPrgBuildName(), builddescriptionfile.getDirStrForVersion(bdf.m_version)), utils.INFO_LEVEL_RarelyUseful)

            # Check if it's source. This handles the condition where we're consuming the LKG (i.e. APR)
            # from a subpart (LoggingSDK) of the BDF (ebecplugin) where APR:* is not listed in ebecplugin BDF
            # because ebecplugin doesn't use any native parts that would consume it, but it is coming from 
            # LoggingSdk as source.
            if parent is None:
                self.bdfSearchedPath.pop()
                return None

            utils.showInfoMsg ("Searching for parent part LKG {0} in BDF {1} ({2}).\n".format (parent.m_info.m_buildContext, bdf.GetPrgBuildName(), builddescriptionfile.getDirStrForVersion(bdf.m_version)), utils.INFO_LEVEL_RarelyUseful)

            lkgDownloadInfo = self.SearchForLkgsViaBdf(bdf, platform, parent.m_info.m_buildContext, partName, None, parent.IsStatic(), isProductLkg, serverToUse)
            if lkgDownloadInfo is None:
                self.bdfSearchedPath.pop()
                return None

            utils.showInfoMsg ("Looking for LKG {0} in parent part BDF {1} ({2}).\n".format (contextName, lkgDownloadInfo.m_lkgSourceName, lkgDownloadInfo.m_version), utils.INFO_LEVEL_RarelyUseful)

            newServer = serverToUse.CreateNewServer(lkgDownloadInfo.m_lkgType, lkgDownloadInfo.m_serverUrl)
            newServer.ValidateServer(contextName, platform, None, False)

            lkgBdf = newServer.GetBDF (lkgDownloadInfo.m_lkgSourceName, platform, lkgDownloadInfo.m_version, isProductLkg)

            lkgDownloadInfo = self.SearchForLkgsViaBdf (lkgBdf, platform, contextName, partName, None, isStatic, isProductLkg, newServer)

            self.bdfSearchedPath.pop()
            return lkgDownloadInfo

        self.bdfSearchedPath.pop()
        return None

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def SearchForLkgsInLKGSource (self, platform, contextName, partName, prgOutputDir, version, isProductLkg, isStatic=False, serverOverride=None):
        serverToUse = self.m_server if serverOverride is None else serverOverride
        if serverToUse.m_type == LKGTYPE_azurebuildartifact:
            if not self.m_version:
                self.m_version = "*"

            authHeader = azurecli.GetAuthenticationHeader()
            organization = self.GetAzureOrganization()
            project = self.GetAzureProject()
            version = versionutils.Version(self.m_version).StringForAzureBuildArtifact()

            buildDefinitionId = azuredevopsrestapi.GetBuildDefinitionId(authHeader, organization, project, self.m_name)
            buildId = azuredevopsrestapi.GetBuildId(authHeader, organization, project, buildDefinitionId, version, None)
            resolvedVersion = str(versionutils.Version(azuredevopsrestapi.GetBuildVersion(authHeader, organization, project, buildDefinitionId, version, None)))
            containerId = azuredevopsrestapi.GetContainerID(authHeader, organization, project, buildId)
            downloadUrl = azuredevopsrestapi.GetArtifactDirectoryDownloadUrl(organization, containerId, azuredevopsrestapi.GetBuildArtifactLkgPath(platform.GetDirName(), contextName, isStatic))
            return LKGDownloadInfo(contextName, platform, isStatic, downloadUrl, self.m_name, resolvedVersion, serverToUse.m_type, serverToUse.m_address)

        lkgAddress = serverToUse.GetNormalizedPathName (contextName if serverToUse.IsNuGet() else prgOutputDir, platform, version, isProductLkg)
        lkgNameBeingSearched = '{} [{}]'.format(contextName, 'static' if isStatic else 'dynamic')
        if serverToUse.IsNuGet():
            lkgNameBeingSearched = nugetpkg.TransformNugetName(contextName, platform, isStatic, False)

        # Show bdf path that got us where we are.
        bdfPath = []
        for tup in self.bdfSearchedPath:
            bdfPath.append ("{0}({1})".format (tup[0], builddescriptionfile.getDirStrForVersion (tup[1])))

        if 0 != len (bdfPath):
            utils.showInfoMsg ("    BDF Path: {}\n".format (':'.join(bdfPath)), utils.INFO_LEVEL_SomewhatInteresting)
        utils.showInfoMsg ("    Searching in {} for LKG {}\n".format (lkgAddress, lkgNameBeingSearched), utils.INFO_LEVEL_SomewhatInteresting)

        if serverToUse.DoesLKGExist (prgOutputDir, platform, version, contextName, partName, isStatic, isProductLkg):
            utils.showInfoMsg ("        Found.\n", utils.INFO_LEVEL_SomewhatInteresting)
            return LKGDownloadInfo (contextName, platform, isStatic, lkgAddress, prgOutputDir, version, serverToUse.m_type, serverToUse.m_address)
        utils.showInfoMsg ("        Not Found.\n", utils.INFO_LEVEL_SomewhatInteresting)
        return None

#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
class LKGDownloadInfo (object):
    def __init__(self, lkgName, platform, isStatic, normalizedServerUrl, lkgSourceName, version, lkgType, serverUrl):
        self.m_lkgName = lkgName
        self.m_platform = platform
        self.m_isStatic = isStatic
        self.m_normalizedServerUrl = normalizedServerUrl
        self.m_lkgSourceName = lkgSourceName
        self.m_version = version
        self.m_lkgType = lkgType
        self.m_serverUrl = serverUrl.replace("$(name)", lkgSourceName).replace("$(version)", version)

    def WriteProvenance(self):
        provFilePath = GetLKGProvenanceFilePath(self.m_lkgName, self.m_platform, self.m_isStatic)
        with open (provFilePath, "wt") as provFile:
            provFile.write ("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n")
            provFile.write ("<Provenance>\n")
            provFile.write ("    <LKG LKGSourceName=\"{}\" Version=\"{}\" Type=\"{}\" Url=\"{}\"/>\n".format (self.m_lkgSourceName, self.m_version, self.m_lkgType, self.m_serverUrl))
            provFile.write ("</Provenance>\n")
        provFile.close()

#---------------------------------------------------------------------------------------
# bsimethod
#---------------------------------------------------------------------------------------
def GetAuthenticationHeader (serverToUse, serverName, serverAddress):
    provider = serverToUse.m_provider
    # No authentication token required
    if provider == globalvars.CREDENTIAL_PROVIDER_BASIC:
        return None

    # Check if Azure CLI is forced globally
    if globalvars.useAzureCliAuth:
        return azurecli.GetAuthenticationHeader()

    # Check what provider is requested by build strategies
    if provider == globalvars.CREDENTIAL_PROVIDER_TOKEN:
        return globalvars.buildStrategy.GetAuthenticationToken(serverName)
    if provider == globalvars.CREDENTIAL_PROVIDER_MCP:
        return cmdutil.GetMicrosoftProviderToken(serverAddress.replace ('feeds', 'nuget'))
    if provider == globalvars.CREDENTIAL_PROVIDER_AZ:
        return azurecli.GetAuthenticationHeader()
    if provider == globalvars.CREDENTIAL_PROVIDER_AUTO:
        return bbutils.ChooseAuthenticationHeader (serverToUse, serverName, serverAddress)
        
    return None

#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
class LKGServer (object):
    def __init__(self, name, lkgType, address, isVersioned, provider = globalvars.CREDENTIAL_PROVIDER_BASIC, createdFromParent=False):
        self.m_name = name
        self.m_type = lkgType
        self.m_address = symlinks.normalizePathName(address) if lkgType == LKGTYPE_local else address
        self.m_isVersioned = isVersioned
        self.m_streamIsValid = None
        self.m_provider = provider
        self.m_createdFromParent = createdFromParent

        typeArray = [LKGTYPE_fileshare, LKGTYPE_rsync, LKGTYPE_local, LKGTYPE_nuget, LKGTYPE_azurebuildartifact]
        if not lkgType in typeArray:
            raise utils.StrategyError ('LKG type ' + repr(lkgType) + ' is not valid for LastKnownGoodServer ' + self.m_name)

    def IsNuGet(self):
        return self.m_type == LKGTYPE_nuget

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetName (self):
        return self.m_name

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetTypeName (self):
        return lkgServerTypes[self.m_type]

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def IsVersioned (self):
        return self.m_isVersioned

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetAzureOrganization (self):
        if self.m_type == LKGTYPE_azurebuildartifact:
            return (self.m_address.replace(internal.AZURE_DEVOPS_SERVER + "/", "").split("/"))[0]
        return ""

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetAzureProject (self):
        if self.m_type == LKGTYPE_azurebuildartifact:
            return (self.m_address.replace(internal.AZURE_DEVOPS_SERVER + "/", "").split("/"))[1]
        return ""

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def ValidateServer (self, sourceName, platform, streamOrVersion, isProductLkg):
        streamForValidation = None if self.IsVersioned () else streamOrVersion
        if None == self.m_streamIsValid:
            if self.m_type != LKGTYPE_rsync:
                self.m_streamIsValid = True
                return
                
            serverPath = self.GetServerPathWithoutVersion (sourceName, platform)
            try:
                listOfDirs = GetRsyncDirListing (serverPath)
            except utils.PartBuildError as err:
                raise utils.StrategyError ('LKGServer {} is unreachable ({})'.format(self.m_name, serverPath), trace=err.stackTrace)
                
            if not streamForValidation:  # Good, as far as we know
                return
                
            if streamForValidation.lower() in [name.lower() for name in listOfDirs]:
                return
            
            serverPath = self.GetNormalizedPathName (sourceName, platform, streamForValidation, isProductLkg)
            try:
                listOfDirs = GetRsyncDirListing (serverPath)
            except utils.PartBuildError as err:
                msg = 'LKGServer {} is unreachable. \nThis may simply be due to a network problem.\n'.format(serverPath) +\
                      'If this is a team (rather than a branch) then there will not be LKGs available. \n' +\
                      'In this case you should use "bb stream --lkgsource=BranchWithLKGs" to set the LKG source\n' +\
                      'for this stream.  See "bb stream --help" or the wiki for more information'
                raise utils.StrategyError (msg, trace=err.stackTrace)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def CreateNewServerFromBDF (self, bdf, prgDir):
        serverType = GetLKGServerTypeFromName(bdf.GetLKGType(prgDir))
        address = bdf.GetLKGUrl(prgDir)
        return self.CreateNewServer(serverType, address)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def CreateNewServer (self, serverType, address):
        newServer = LKGServer (self.m_name, serverType, address, self.m_isVersioned, self.m_provider, True)
        utils.showInfoMsg ("Created new LKGServer with type {} and url {}\n".format(lkgServerTypes[serverType], address), utils.INFO_LEVEL_SomewhatInteresting)
        return newServer

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetServerPathWithoutVersion (self, sourceName, platform, serverOverride=None):
        serverToUse = self if serverOverride is None else serverOverride
        if serverToUse.IsVersioned():
            # This does make the assumption that versioned paths follow the pattern $(name)\$(version)\$(platform)
            badVerPath = serverToUse.GetNormalizedPathNameByStrings (sourceName, 'REPLACE_ME', 'REPLACE_ME')
        elif serverToUse.m_address.lower().endswith('$(platform)'):
            # This case and the next one will go away once the LKGs are restructured. This change is to get a 
            #  working bb that won't have to be updated on every branch.
            badVerPath = serverToUse.GetNormalizedPathNameByStrings (sourceName, 'REPLACE_ME', 'REPLACE_ME')
        else:
            badVerPath = serverToUse.GetNormalizedPathName (sourceName, platform, 'REPLACE_ME', False)
            if not 'REPLACE_ME' in badVerPath:
                # Case where version is part of the string: lkgs\$(platform)\$(name)\wsg02-6
                lastSlashIndex = badVerPath.rfind ('\\')
                badVerPath = badVerPath[0:lastSlashIndex]
        return badVerPath.replace ('\\REPLACE_ME', '')

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetNormalizedPathName (self, sourceName, platform, streamOrVersion, isProductLkg, serverOverride=None):
        serverToUse = self if serverOverride is None else serverOverride
        if serverToUse.m_type == LKGTYPE_nuget:
            platformName = platform.m_option if platform else ''
        else:
            platformName = platform.GetDirName() if platform else ''

        if isProductLkg and serverToUse.m_type != LKGTYPE_nuget:
            platformName = 'Products/'+platformName
        return serverToUse.GetNormalizedPathNameByStrings (sourceName, platformName, streamOrVersion)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetNormalizedPathNameByStrings (self, sourceName, platformName, streamOrVersion, serverOverride=None):
        serverToUse = self if serverOverride is None else serverOverride
        if not serverToUse.m_address:
            raise utils.StrategyError ('Bad server LKG path specified for server "{0}" type {1} '.format (serverToUse.m_address, serverToUse.m_type))

        address = serverToUse.m_address
        address = address.replace ('$(name)', sourceName)
        address = address.replace ('$(platform)', platformName)
        address = address.replace ('$(version)', streamOrVersion)
#        address = address.replace ('$(stream)', globalvars.streamName if serverToUse.m_type == LKGTYPE_nuget else streamOrVersion) TODO - what is the right stream?

        resolvedServerName = '[Generated from BDF; Parent Server name is {}]'.format(serverToUse.m_name) if serverToUse.m_createdFromParent else serverToUse.m_name

        if '$' in address or '(' in address or ')' in address:
            raise utils.StrategyError ('LKGServer {} has an address that does not fully resolve: \n'.format(resolvedServerName) \
                + '  original: {}\n  resolved: {}'.format(serverToUse.m_address, address))

        if serverToUse.m_type == LKGTYPE_fileshare or serverToUse.m_type == LKGTYPE_local:
            pathToReturn = symlinks.normalizePathName(address)
            if not pathToReturn:
                raise utils.StrategyError ('Unable to find LKG path: {}'.format(address))
            return pathToReturn
        elif serverToUse.m_type == LKGTYPE_rsync:
            return address
        elif serverToUse.m_type == LKGTYPE_nuget:
            address = address.replace('/feeds/', '/nuget/')
            return address
        elif serverToUse.m_type == LKGTYPE_azurebuildartifact:
            return address
        else:
            raise utils.StrategyError ('Unknown LKG type for server: {}'.format(resolvedServerName))
        return serverToUse.GetNormalizedPathNameByStrings (sourceName, platformName, streamOrVersion)

    #---------------------------------------------------------------------------------------
    # bsimethod
    #---------------------------------------------------------------------------------------
    def GetAuthenticationHeader (self, serverOverride=None):
        serverToUse = self if serverOverride is None else serverOverride
        return GetAuthenticationHeader(serverToUse, serverToUse.m_name, serverToUse.m_address)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetVersionListing (self, sourceName, platform, isStatic=False, serverOverride=None):
        serverToUse = self if serverOverride is None else serverOverride
        pathToCheck = serverToUse.GetServerPathWithoutVersion (sourceName, platform)

        if serverToUse.m_type == LKGTYPE_fileshare or serverToUse.m_type == LKGTYPE_local:
            if not os.path.exists (pathToCheck):
                # Because we're seeing this failing when copying from LKGOutput in PRG we have added this extra logging info for Mike circa 2018-10. It probably won't be useful for long.
                if os.name == 'nt':
                    import time, win32api
                    gle = win32api.GetLastError ()
                    exceptStr = 'no exception'
                    try:
                        time.sleep (1)
                        os.stat(pathToCheck)
                        return os.listdir (pathToCheck)
                    except Exception as e:
                        exceptStr = str(e)
                    raise utils.BuildError ("Searching for LKG could not find directory {0}. GLE is {1}, stat GLE is {2}. Exception is {3}".format(pathToCheck, gle, win32api.GetLastError (), exceptStr))
                else:
                    raise utils.BuildError ("Searching for LKG could not find directory {0}.".format(pathToCheck))
            return os.listdir (pathToCheck)

        elif serverToUse.m_type == LKGTYPE_rsync:
            return GetRsyncDirListing(pathToCheck)

        elif serverToUse.m_type == LKGTYPE_nuget:
            address = serverToUse.m_address
            address = address.replace ('/feeds/', '/nuget/')
            nugetName = nugetpkg.TransformNugetName(sourceName, platform, isStatic, False)
            nugetSource = nugetpkg.NuGetSource (nugetName, '*')
            return nugetSource.GetPkg().SearchVersionsFromServer(address, False)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def DoesLKGExist (self, sourceName, platform, streamOrVersion, contextDir, partName, isStatic, isProductLkg, serverOverride=None):
        serverToUse = self if serverOverride is None else serverOverride
        if serverToUse.m_type == LKGTYPE_nuget:
            nugetName = nugetpkg.TransformNugetName(contextDir, platform, isStatic, False)
            version = streamOrVersion.replace('-', '.')
            nugetSource = nugetpkg.NuGetSource (nugetName, version)
            lkgDownloadUrl = nugetSource.GetPkg().GetDownloadUrl(serverToUse.m_address, nugetName, version, False, httpAuth=serverToUse.GetAuthenticationHeader())
            if lkgDownloadUrl is not None:
                return True
            return False

        basePath = serverToUse.GetNormalizedPathName (sourceName, platform, streamOrVersion, isProductLkg)

        # ProductLKGs use the part name
        dirNameToCheck = partName if isProductLkg else contextDir
        pathToCheck = os.path.join (basePath, 'static' if isStatic else '')

        if serverToUse.m_type == LKGTYPE_fileshare or serverToUse.m_type == LKGTYPE_local:
            return os.path.exists (os.path.join(pathToCheck, dirNameToCheck))
            
        elif serverToUse.m_type == LKGTYPE_rsync:

            def ItemInListOfDirs (contextDir, listOfDirs):
                contextDirLower = contextDir.lower()
                for item in listOfDirs:
                    if item.lower() == contextDirLower:
                        return True
                return False

            if isStatic: # Make sure 'static' directory appears before looking in it
                listOfDirs = GetRsyncDirListing(basePath)
                if not ItemInListOfDirs ('static', listOfDirs):
                    return False
        
            # Check the directory for the part context dir.
            listOfDirs = GetRsyncDirListing(pathToCheck)
            return ItemInListOfDirs (dirNameToCheck, listOfDirs)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetBDF (self, sourceName, platform, streamOrVersion, isProductLkg, serverOverride=None):
        serverToUse = self if serverOverride is None else serverOverride
        # First try the normal approach
        bdf = serverToUse.GetBDFFromLKGOutput (sourceName, platform, streamOrVersion, isProductLkg)
        if bdf:
            return bdf

        # now try the bdf server
        if serverToUse.IsVersioned():
            lkgAddress = 'bdfserver:{0}:{1}'.format (sourceName, streamOrVersion)
            if lkgAddress in s_bdfsDownloaded:
                bdf = s_bdfsDownloaded[lkgAddress]
            elif serverToUse.m_type == LKGTYPE_azurebuildartifact:
                bdf = bdfcache.GetBdfFromAzureArtifact (serverToUse, sourceName, streamOrVersion)
                s_bdfsDownloaded[lkgAddress] = bdf
            else:
                bdf = bdfcache.GetCachedBdfWithoutStatusCheck (sourceName, versionutils.Version(streamOrVersion).AsList())
                s_bdfsDownloaded[lkgAddress] = bdf
        return bdf

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetBDFFromLKGOutput (self, sourceName, platform, streamOrVersion, isProductLkg, serverOverride=None):
        serverToUse = self if serverOverride is None else serverOverride
        if serverToUse.m_type == LKGTYPE_nuget or serverToUse.m_type == LKGTYPE_azurebuildartifact:
            return None

        version = streamOrVersion
        if '*' in version:
            if serverToUse.m_type == LKGTYPE_local and '$(version)' not in serverToUse.m_address:
                utils.showInfoMsg ("Skipping version resolution for LKG {} BDF since version doesn't play any part in the used server address.\n".format (sourceName), utils.INFO_LEVEL_RarelyUseful)
            else:
                versionList = serverToUse.GetVersionListing (sourceName, platform, False)
                path = serverToUse.GetServerPathWithoutVersion (sourceName, platform)
                version = getNewestRebuildOfVersion (versionList, version, sourceName, path, isProductLkg, platform.GetXmlName(), False)

        lkgAddress = serverToUse.GetNormalizedPathName (sourceName, platform, version, False)

        if serverToUse.m_type == LKGTYPE_fileshare or serverToUse.m_type == LKGTYPE_local:
            bdfSource = os.path.join (lkgAddress, 'buildinfo', 'guidList.xml')
            if not os.path.exists (bdfSource):
                return None

            bdf = builddescriptionfile.BuildDescription (bdfSource)
            result, msg = bdf.ReadFromFile()
            if 0 != result:
                utils.showInfoMsg ("Searching for LKG for part found a bad Build Description File before finding LKG.\n Part: {0}\n BDF: {1}\n BDF Error: {2}\n".format (sourceName, bdfSource, msg), utils.INFO_LEVEL_SomewhatInteresting)
                return None

        elif serverToUse.m_type == LKGTYPE_rsync:
            localDir = os.path.join (buildpaths.GetLastKnownGoodLocation(), platform.GetDirName(), '.rsyncBuildDefinitions', sourceName)
            # Download if we haven't already
            if not lkgAddress in s_bdfsDownloaded:
                utils.showInfoMsg ("Downloading BDF via rsync from {0}\n".format (lkgAddress), utils.INFO_LEVEL_SomewhatInteresting)
                
                try:
                    listOfDirs = GetRsyncDirListing (lkgAddress)
                except utils.PartPullError as ppe:
                    if not isProductLkg:
                        raise ppe
                    listOfDirs = []

                if not 'buildinfo' in listOfDirs:
                    utils.showInfoMsg (lkgAddress+"\\buildinfo does not exist.\n PRG might have removed the version. \n This path was determined while traversing the BDF LKGS.", utils.INFO_LEVEL_SomewhatInteresting)
                    return None

                # Bring down the buildinfo directory
                bdfDir = lkgAddress + '\\buildinfo'  # Dealing with rsync; think we want fixed seperators
                cmdutil.rsyncDir (bdfDir, localDir)

                bdfSource = os.path.join (localDir, 'buildinfo', 'guidList.xml')
                if not os.path.exists (bdfSource):
                    utils.showInfoMsg (bdfSource+" does not exist.\n This should have been rsyncd from " +bdfDir+ "\n This path was determined while traversing the BDF LKGS.", utils.INFO_LEVEL_SomewhatInteresting)
                    return None

                readbdf = builddescriptionfile.BuildDescription (bdfSource)
                result, msg = readbdf.ReadFromFile()
                if 0 != result:
                    utils.showInfoMsg ("Searching for LKG for part found a bad Build Description File before finding LKG.\n Part: {0}\n BDF: {1}\n BDF Error: {2}\n".format (sourceName, bdfSource, msg), utils.INFO_LEVEL_SomewhatInteresting)
                    return None

                s_bdfsDownloaded[lkgAddress] = readbdf # Don't reload
                
            bdf = s_bdfsDownloaded[lkgAddress]

        return bdf

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def Dump (self, indent):
        utils.showInfoMsg (" "*indent + "Name='{0}', Type={1}, Versioned={2} address='{3}'\n".format (self.m_name, lkgServerTypes[self.m_type], self.m_isVersioned, self.m_address), utils.INFO_LEVEL_Essential)

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def getNewestRebuildOfVersion (listOfItems, version, sourceName, prgOutputPath, isProductLkg, platformXmlName, ignoreFlagCheck):
    dirNameDict = {}

    versionRequirement = versionutils.Version (version)
    utils.showInfoMsg ("Looking for newest version of LKG Source {2} that matches version {0} based on specified version {1}\n".format (versionRequirement.StringForFilename(), version, sourceName), utils.INFO_LEVEL_SomewhatInteresting)

    for item in listOfItems:
        if not versionRequirement.MatchString (item):
            continue
        d = item.replace ('.', '-')  # also convert periods (nuget version format) to dash
        t = tuple (map(int, d.split ('-')))
        dirNameDict[t] = item

    keys = list(dirNameDict.keys())
    keys.sort(reverse = True)

    if len (keys) < 1:
        #raise utils.BuildError ("Searching for specific LKG version '{0}' could not be found at {1}.".format (version, path))
        return version # Check here if you want to change the behavior of this method to not necesarily fail out but to just return the version 
        # that was asked. We will find out in another method if the path doesn't exist - and what to do. (stop searching etc.)

    remoteVer = keys[0]
    if prgOutputPath and not ignoreFlagCheck:
        for remoteVer in keys:
            path = os.path.join(prgOutputPath, dirNameDict[remoteVer])
            dirContents = cmdutil.rsyncListDir(path)
            if CHECK_COMPLETE_FLAGS not in dirContents:
                # without a flag to check for completion flags, break and resume with old behavior
                break
            flagName = PRODUCT_FLAG.format(platformXmlName) if isProductLkg else NONPROD_FLAG.format(platformXmlName)
            if flagName in dirContents:
                break
            utils.showInfoMsg("Skipping incomplete {0} {1} version {2} for platform {3}".format('Product' if isProductLkg else 'LKG Source', sourceName, dirNameDict[remoteVer], platformXmlName), utils.INFO_LEVEL_Interesting)

    utils.showInfoMsg ("LKG Source {3} found version {2} that matches version {0} based on specified version {1}\n".format (versionRequirement.StringForFilename(), version, dirNameDict[remoteVer], sourceName), utils.INFO_LEVEL_SomewhatInteresting)
    return dirNameDict[remoteVer]

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
# get LKG package via HTTP or rsync and unzip contents
def DownloadLKGFromNuGet(lkgDownloadInfo, localDir, httpAuth = None):
    nugetName = nugetpkg.TransformNugetName(lkgDownloadInfo.m_lkgName, lkgDownloadInfo.m_platform, lkgDownloadInfo.m_isStatic, False)
    nugetVersion = lkgDownloadInfo.m_version.replace('-', '.')
    pkgBaseName = '{0}.{1}'.format(nugetName, nugetVersion)
    localPkgPath = os.path.join(localDir, '{0}.nupkg'.format(pkgBaseName))
    nugetSource = nugetpkg.NuGetSource (nugetName, nugetVersion)

    # Have we already downloaded and unzipped this package -- "package" folder is the last to unzip
    if os.path.exists(localPkgPath) and os.path.exists(os.path.join(localDir, "package")):
        utils.showInfoMsg("Package already downloaded: {0}\n".format(localPkgPath), utils.INFO_LEVEL_SomewhatInteresting)
        return None
    if bbutils.IsRemoteAddress(lkgDownloadInfo.m_normalizedServerUrl):
        pkgGetUrl = nugetSource.GetPkg().GetDownloadUrl(lkgDownloadInfo.m_normalizedServerUrl, nugetName, nugetVersion, True, httpAuth)
    else:
        pkgGetUrl = os.path.join(lkgDownloadInfo.m_normalizedServerUrl, '{0}.nupkg'.format(pkgBaseName.lower()))

    # Clean current LKG and its provenance
    if os.path.exists(localDir):
        utils.cleanDirectory(localDir, True)
        os.rmdir(localDir)
    provFilePath = GetLKGProvenanceFilePath(lkgDownloadInfo.m_lkgName, lkgDownloadInfo.m_platform, lkgDownloadInfo.m_isStatic)
    utils.deleteFileWithRetries(provFilePath, level=utils.INFO_LEVEL_SomewhatInteresting)

    symlinks.makeSureDirectoryExists(localDir)

    nugetSource.GetPkg().GetUnzipPackage(pkgGetUrl, nugetName, localPkgPath, nugetVersion, localDir, localDir, httpAuth)
    return localPkgPath

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def DownloadLKGFromAzureBuildArtifact(lkgDownloadInfo, targetDir, platformDir):
    if not azuredevopsrestapi.DownloadBuildArtifactDirectory(azurecli.GetAuthenticationHeader(), lkgDownloadInfo.m_normalizedServerUrl, targetDir, azuredevopsrestapi.GetBuildArtifactLkgPath(platformDir, lkgDownloadInfo.m_lkgName, lkgDownloadInfo.m_isStatic), versionutils.Version(lkgDownloadInfo.m_version).StringForFilename()):
        utils.showInfoMsg("Azure artifact LKG already downloaded: {0}\n".format(targetDir), utils.INFO_LEVEL_SomewhatInteresting)
        # Return if the extracted contents have already been preprocessed
        if len(os.listdir(targetDir)) > 2:
            return False

    nugetName = nugetpkg.TransformNugetName(lkgDownloadInfo.m_lkgName, lkgDownloadInfo.m_platform, lkgDownloadInfo.m_isStatic, False)
    nugetVersion = lkgDownloadInfo.m_version.replace('-', '.')
    pkgBaseName = '{0}.{1}'.format(nugetName, nugetVersion)
    localPkgPath = os.path.join(targetDir, '{0}.nupkg'.format(pkgBaseName))
    nugetSource = nugetpkg.NuGetSource (nugetName, nugetVersion)

    if os.path.exists(localPkgPath):
        nugetSource.GetPkg().UnzipPackage(targetDir, localPkgPath)

    return True