#---------------------------------------------------------------------------------------------
#  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
#  See LICENSE.md in the repository root for full copyright notice.
#---------------------------------------------------------------------------------------------

import glob, os, datetime
from . import builddescriptionfile, buildpaths, symlinks, utils, versionutils, azurecli, azuredevopsrestapi
import xml.etree.cElementTree as ElementTree

BDF_CACHE        = 'BdfCache'

#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
def GetServerBdfCacheDir ():
    return os.path.join (buildpaths.GetBBCacheDir(), BDF_CACHE)

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def GetFixedVersionCachedBdf (productName, productVersion, cacheDir, outputFunc):
    # For fixed version, look in the cache first and then hit the server if we fail.
    bdfFileName = builddescriptionfile.VersionedFileName (productName, productVersion.AsList(), None, False)
    cacheFileName = os.path.join (cacheDir, bdfFileName)
    if os.path.exists (cacheFileName):
        serverBDF = builddescriptionfile.BuildDescription ()
        if outputFunc:
            outputFunc ('Using BDF {0} {1} from bdf cache\n'.format (productName, productVersion.StringForFilename()))
        status, message = serverBDF.ReadFromFile (cacheFileName)
    else:
        try:
            serverBDF, status = builddescriptionfile.BuildDescription.QueryDatabaseForBuildDescription (productName, productVersion.AsList())
            if 0 == status:
                # Resolve actual version if there were *'s
                versionList = [str(x) for x in serverBDF.GetProductVersion()]
                cacheFileName = os.path.join (cacheDir, builddescriptionfile.VersionedFileName (productName, versionList, None, False))
                if not os.path.exists (cacheDir):
                    os.makedirs (cacheDir)
                serverBDF.WriteToFile (cacheFileName, True, None, False, None)
        except builddescriptionfile.BuildDescriptionError as _:
            message="Error: could not get versioned BDF from server: [{0}:{1}]\n".format (productName, productVersion.StringForFilename())
            return None, 1, message
    return serverBDF, status, None

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def GetWildcardCachedBdf (productName, productVersion, cacheDir, outputFunc):
    message = ''
    # For wildcard, try to hit the server and then look in the cache if we fail.
    try:
        serverBDF, status = builddescriptionfile.BuildDescription.QueryDatabaseForBuildDescription (productName, productVersion.AsList())
        if 0 == status:
            # Resolve actual version if there were *'s
            versionList = [str(x) for x in serverBDF.GetProductVersion()]
            cacheFileName = os.path.join (cacheDir, builddescriptionfile.VersionedFileName (productName, versionList, None, False))
            symlinks.makeSureDirectoryExists (cacheDir)
            serverBDF.WriteToFile (cacheFileName, True, None, False, None)
            return serverBDF, 0, None
    except builddescriptionfile.BuildDescriptionError as _:
        message = 'Error: could not get wildcard BDF from server: [{0}:{1}] \n'.format (productName, productVersion)

    # Server failed; try local directory.
    bdfFileNameGlob = builddescriptionfile.VersionedFileName (productName, productVersion.AsList(), None, False).replace ('.xml', '.*')
    cacheFileGlob = os.path.join (cacheDir, bdfFileNameGlob)
    cacheFiles = glob.glob (cacheFileGlob)
    if not cacheFiles:
        message += 'Error: could not get wildcard BDF from local cache: [{0}:{1}]\n'.format (productName, productVersion.StringForFilename())
        return None, 1, message

    splitL = lambda f: f[:f.rfind('.')].split('_')[-1]
    cacheVersions = [versionutils.Version (splitL(fn)) for fn in cacheFiles]
    cacheVersions.sort (reverse=True)
    
    resolvedVersionList = [str(x) for x in cacheVersions[-1].m_verQuad]
    cacheFileName = os.path.join (cacheDir, builddescriptionfile.VersionedFileName (productName, resolvedVersionList, None, False))
    serverBDF = builddescriptionfile.BuildDescription ()
    if outputFunc:
        outputFunc ('Using BDF {0} {1} [originally {2}] from BDF cache\n'.format (productName, '-'.join(resolvedVersionList), productVersion))
    status, message = serverBDF.ReadFromFile (cacheFileName)
    return serverBDF, status, message

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def GetCachedBdf (productName, productVersion, outputFunc=None):
    # Check cache first; if it's there then use it. If not pull from BDF server and cache it.
    cacheDir = GetServerBdfCacheDir()

    if '*' in str(productVersion):
        serverBDF, status, message = GetWildcardCachedBdf (productName, productVersion, cacheDir, outputFunc)
    else:
        serverBDF, status, message = GetFixedVersionCachedBdf (productName, productVersion, cacheDir, outputFunc)

    if 0 == status:
        bdfCache = BdfCacheUsage(cacheDir)
        bdfCache.UpdateUsage (productName, versionutils.Version.InitFromList(serverBDF.GetProductVersion()))
        bdfCache.WriteUsage()

    return serverBDF, status, message

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def GetCachedBdfWithStatusCheck (name, versionList):
    def outputLines (line):
        utils.showInfoMsg (line, utils.INFO_LEVEL_SomewhatInteresting)

    serverBdf, status, message = GetCachedBdf (name, versionutils.Version.InitFromList(versionList), outputFunc=outputLines)
    if 0 != status:
        if not message:
            message="Error: could not get BDF from server: [{0}:{1}] status = {2}\n".format (name, '-'.join(versionList), status)
        raise utils.StrategyError (message)
    return serverBdf

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def GetCachedBdfWithoutStatusCheck (name, versionList):
    def outputLines (line):
        utils.showInfoMsg (line, utils.INFO_LEVEL_SomewhatInteresting)

    serverBdf, _, _ = GetCachedBdf (name, versionutils.Version.InitFromList(versionList), outputFunc=outputLines)
    return serverBdf

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def GetMatchingLkg (productName, matchBdf, outputFunc=None):
    cacheDir = GetServerBdfCacheDir()
    matchList = BdfCacheMatches (cacheDir)
    matchVersion = matchList.FindMatch (productName, matchBdf.GetPrgBuildName(), versionutils.Version.InitFromList(matchBdf.GetProductVersion()))
    if not matchVersion:
        return None, 0, None
    if outputFunc:
        outputFunc ('Found cached BDF match {0} [{1}] which matches {2} [{3}]\n'.format (productName, matchVersion, matchBdf.GetPrgBuildName(), '-'.join([str(x) for x in matchBdf.GetProductVersion()])))
    return GetCachedBdf (productName, versionutils.Version(matchVersion), outputFunc)
    
#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def SetMatchingLkg (productName, productVersion, matchBdf):
    matchVersion = versionutils.Version.InitFromList(matchBdf.GetProductVersion())

    matchList = BdfCacheMatches (GetServerBdfCacheDir())
    matchList.UpdateUsage (productName, productVersion, matchBdf.GetPrgBuildName(), matchVersion)
    matchList.WriteUsage ()

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def GetBdfFromAzureArtifact (lkgServer, sourceName, sourceVersion):
    authHeader = azurecli.GetAuthenticationHeader()
    organization = lkgServer.GetAzureOrganization()
    project = lkgServer.GetAzureProject()
    version = versionutils.Version(sourceVersion).StringForAzureBuildArtifact()

    buildDefinitionId = azuredevopsrestapi.GetBuildDefinitionId(authHeader, organization, project, sourceName)
    buildInfo = azuredevopsrestapi.GetBuildInfo(authHeader, organization, project, buildDefinitionId, version, None)
    buildNumber = versionutils.Version(buildInfo["buildNumber"])
    cacheFileName = os.path.join (GetServerBdfCacheDir(), "{0}_{1}.xml".format(sourceName, buildNumber.StringForFilename()))

    if not os.path.exists(cacheFileName):
        containerId = azuredevopsrestapi.GetContainerID(authHeader, organization, project, buildInfo["id"])
        downloadUrl = azuredevopsrestapi.GetArtifactFileDownloadUrl(organization, containerId, azuredevopsrestapi.GetBuildArtifactBdfFilePath(sourceName))
        utils.showInfoMsg ("Downloading BDF to {0} from Azure Build artifacts {1}\n".format(cacheFileName, downloadUrl), utils.INFO_LEVEL_SomewhatInteresting)
        azuredevopsrestapi.DownloadBuildArtifactFile(authHeader, downloadUrl, cacheFileName)

    serverBdf = builddescriptionfile.BuildDescription ()
    status, message = serverBdf.ReadFromFile (cacheFileName)
    if status != 0:
        if not message:
            message="Failed to load BDF file {0} with error {1}\n".format (cacheFileName, status)
        raise utils.StrategyError (message)

    bdfCache = BdfCacheUsage(GetServerBdfCacheDir())
    bdfCache.UpdateUsage (sourceName, buildNumber)
    bdfCache.WriteUsage()
    return serverBdf

#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
class BdfCacheUsage (object):

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def __init__ (self, path):
        self.m_rootName     = 'BdfCache'
        self.m_xmlFileName  = 'BdfCacheUsage.xml'
        self.m_path         = path
        self.m_dom          = None

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def Load (self):
        self.m_fullFileName = os.path.join (self.m_path, self.m_xmlFileName)
        self.m_dom = self.ReadFromPath (self.m_fullFileName)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def UpdateUsage (self, productName, prodVersion):
        name = '{0}_{1}.xml'.format(productName, prodVersion.StringForFilename())
        now = datetime.datetime.now().replace(microsecond=0).isoformat()
        
        if not self.m_dom:
            self.Load()

        updatedExisting = False
        root = self.m_dom.getroot()
        for item in root:
            if item.attrib['Name'] == name:
                item.attrib['LastUsed'] = now
                updatedExisting = True
                
        if not updatedExisting:
            ElementTree.SubElement(root, "Usage", Name=name, LastUsed=now)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def WriteUsage (self):
        IndentElementTree (self.m_dom.getroot())
        xmlStr = ElementTree.tostring(self.m_dom.getroot(), encoding="UTF-8", method="xml")
        with open (self.m_fullFileName, 'wb') as xmlFile:
            xmlFile.write(xmlStr)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def ReadFromPath(self, fullFileName):
        if os.path.exists(fullFileName):
            return ElementTree.parse(fullFileName)
        else:
            return ElementTree.ElementTree(ElementTree.Element(self.m_rootName))    # create a new tree with root node


#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
class BdfCacheMatches (object):

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def __init__ (self, path):
        self.m_rootName     = 'BdfLkgMatch'
        self.m_xmlFileName  = 'BdfLkgMatch.xml'
        self.m_path         = path
        self.m_dom          = None

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def Load (self):
        self.m_fullFileName = os.path.join (self.m_path, self.m_xmlFileName)
        self.m_dom = self.ReadFromPath (self.m_fullFileName)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def UpdateUsage (self, productName, productVersion, matchingProduct, matchingVersion):
        if not self.m_dom:
            self.Load()

        updatedExisting = False
        root = self.m_dom.getroot()
        for item in root:
            if item.attrib['ProductName'] == productName and item.attrib['ProductVersion'] == productVersion.StringForFilename():
                item.attrib['MatchingProduct'] = matchingProduct
                item.attrib['MatchingVersion'] = matchingVersion.StringForFilename()
                updatedExisting = True
                
        if not updatedExisting:
            ElementTree.SubElement(root, "Match", ProductName=productName, ProductVersion=productVersion.StringForFilename(), MatchingProduct=matchingProduct, MatchingVersion=matchingVersion.StringForFilename())

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def FindMatch (self, productName, matchingProduct, matchingVersion):
        if not self.m_dom:
            self.Load()

        root = self.m_dom.getroot()
        for item in root:
            if item.attrib['ProductName'] == productName and item.attrib['MatchingProduct']==matchingProduct and item.attrib['MatchingVersion']==matchingVersion.StringForFilename():
                return item.attrib['ProductVersion']

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def WriteUsage (self):
        IndentElementTree (self.m_dom.getroot())
        xmlStr = ElementTree.tostring(self.m_dom.getroot(), encoding="UTF-8", method="xml")
        with open (self.m_fullFileName, 'wb') as xmlFile:
            xmlFile.write(xmlStr)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def ReadFromPath(self, fullFileName):
        if os.path.exists(fullFileName):
            return ElementTree.parse(fullFileName)
        else:
            return ElementTree.ElementTree(ElementTree.Element(self.m_rootName))    # create a new tree with root node


#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
#helper function to "pretty print" the raw xml
def IndentElementTree(elem, level=0):
    i = "\n" + level*"  "
    if len(elem):
        if not elem.text or not elem.text.strip():
            elem.text = i + "  "
        if not elem.tail or not elem.tail.strip():
            elem.tail = i
        for elem in elem:
            IndentElementTree (elem, level+1)
        if not elem.tail or not elem.tail.strip():
            elem.tail = i
    else:
        if level and (not elem.tail or not elem.tail.strip()):
            elem.tail = i

