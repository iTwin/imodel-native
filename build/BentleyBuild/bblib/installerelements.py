#---------------------------------------------------------------------------------------------
#  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
#  See LICENSE.md in the repository root for full copyright notice.
#---------------------------------------------------------------------------------------------

import codecs, filecmp, glob, os, re, uuid
from xml.dom import minidom

from . import builddescriptionfile, buildpaths, cmdutil, globalvars, symlinks, translationkit, utils, targetplatform


_wixFragmentXmlString               = "<Wix xmlns=\"http://schemas.microsoft.com/wix/2006/wi\"/>"
wixWxiXmlString                     = "<Include xmlns=\"http://schemas.microsoft.com/wix/2006/wi\"/>"
WIX_TOOLSET_DEFAULT_VERSION         = "3.8.1128.0"
WIX_TOOLSET_VERSION                 = None
WIX_TOOLSET_FROM_UPACK              = False
WIX_TOOLSET_REPOSITORIES            = []

buildInstallSetWxsAllPartFileName   = "AllPartFragments.wxs"
buildInstallSetWxsAllDirectoryNames = "AllDirectories.wxs"
buildInstallSetMkiFileName          = "installerBentleyBuildMacros.mki"
buildInstallSetMki2FileName         = "InstallSetDefintion.mki"
buildInstallSetWxiFileName          = "InstallSetDefintion.wxi"
buildInstallSetWxsFileName          = "MainWxsFile.wxs"
installSetContextAllowedExtensionList = [".msi", ".msm", ".msp", ".exe", ".log", ".xml", ".nupkg"]
packagePropertieFileName            = "packages.installers.xml"
defaultGuidSeed                     = "${REL_V}.${MAJ_V}.${MIN_V}.${SUBMIN_V}.${Language}.${PartName}.${Platform}"
defaultLangPackGuidSeed             = "${PartName}.${Platform}"

PACKAGE_TYPE_MSI = 0
PACKAGE_TYPE_NUGET = 1
PACKAGE_TYPE_CSPKG_NUGET = 2

# These types needs to be each class.
PACKAGE_TYPES = ["MSI", "NUGET", "CSPKG-NUGET"]

def SetWixVersion (part):
    installWixFile = part.m_installerFileSource
    if not installWixFile:
        return

    global WIX_TOOLSET_VERSION
    global WIX_TOOLSET_FROM_UPACK

    if installWixFile.IsWixFromUpackTool():
        WIX_TOOLSET_FROM_UPACK = True

    if installWixFile.IsWixVersion ():
        if WIX_TOOLSET_VERSION != None and  WIX_TOOLSET_VERSION != installWixFile.GetWixVersion ():
            raise utils.PartBuildError ("WixToolsetVersion version mismatch FirstVersion={0} and SecondVersion={1}".format (WIX_TOOLSET_VERSION, installWixFile.GetWixVersion ()), part)
        WIX_TOOLSET_VERSION = installWixFile.GetWixVersion ()

        if installWixFile.IsWixRepo () and not installWixFile.IsWixRepo () in WIX_TOOLSET_REPOSITORIES:
            WIX_TOOLSET_REPOSITORIES.append (installWixFile.GetWixRepo ())

def IsWixFromUpackTool ():
    return WIX_TOOLSET_FROM_UPACK

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def GetInstallerToolsetVersion ():
    if WIX_TOOLSET_VERSION != None:
        return WIX_TOOLSET_VERSION
    else:
        return WIX_TOOLSET_DEFAULT_VERSION

def GetInstallerToolsetRepositories ():
    return WIX_TOOLSET_REPOSITORIES


#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def GetNodeText (node):
    if not node:
        return None

    fc = node.firstChild
    if fc.nodeType != fc.TEXT_NODE:
        return None
    return fc.data

def SpliText (text):
    if not text :
        return None

    return text.split (";")

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def GetPackageTypeToBuildFromOption (option):
    if option.upper () in PACKAGE_TYPES:
        return PACKAGE_TYPES.index (option.upper ())
    else:
        return PACKAGE_TYPE_MSI

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def IsLanguagePackBuild ():
    if translationkit.IsTranskitShell ():
        return not translationkit.IsTranskitMultilingualBuild ()
    return False

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def GetStaticDirName (dirName, isStatic):
    return (dirName + "_staticinstalldirid") if isStatic else dirName

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def checkAndInitializeVersion (versionStr=None):
    if not versionStr and "INSTALLERBUILD_TESTVERSION" in os.environ:
        versionStr = os.environ ["INSTALLERBUILD_TESTVERSION"]

    # Check for Version in Environment. If not in place, put environment for not a PRG build.
    environKeys = [keyName.upper () for keyName in os.environ]

    versionToSet = {"REL_V" :"09", "MAJ_V" : "99", "MIN_V" : "99", "SUBMIN_V" : "99"}

    # Set Transkit version in transkit shell
    if translationkit.IsTranskitShell ():
        teamRevListFileName = translationkit.getL10NSourceRepoRevisionListFile()
        teamRevList = builddescriptionfile.BuildDescription (teamRevListFileName)
        teamRevList.ReadFromFile ()
        versionToSet = {"REL_V" : str (teamRevList.GetProductVersion () [0]), "MAJ_V" : str (teamRevList.GetProductVersion () [1]), "MIN_V" : str (teamRevList.GetProductVersion () [2]), "SUBMIN_V" : str (teamRevList.GetProductVersion () [3])}

    if versionStr:
        versionStrSplit = versionStr.split ('.')
        if len (versionStrSplit) == 4:
            versionToSet = {"REL_V" : versionStrSplit [0], "MAJ_V" : versionStrSplit [1], "MIN_V" : versionStrSplit [2], "SUBMIN_V" : versionStrSplit [3]}

    def RaiseBuildErrorForPRGBuild (versionName):
        if utils.isSubnetPrg() and not translationkit.IsTranskitShell ():
            raise utils.BuildError ("Version is not defined for a PRG build {0}".format (versionName))

    msg = "Initializing version to : "
    for name in versionToSet.keys ():
        if name not in environKeys:
            RaiseBuildErrorForPRGBuild (name)
            value = versionToSet[name]
            os.environ [name] = value
            msg = msg + "{0}={1};".format (name, value)

    utils.showInfoMsg (msg + "\n", utils.INFO_LEVEL_VeryInteresting)
    return ".".join ([os.environ ["REL_V"], os.environ ["MAJ_V"], os.environ ["MIN_V"], os.environ ["SUBMIN_V"]])

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def getExapandableName (attributeName, exptype):
    if exptype == 0:
        return "$(" + attributeName + ")"
    if exptype == 1:
        return "${" + attributeName + "}"
    if exptype == 2:
        return "[" + attributeName + "]"

    return "$(" + attributeName + ")"

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
if "nt" == os.name:
    import win32api
    def getExeVersion (exeFilePath):
        versionStr = None
        try:
            versionInfo = win32api.GetFileVersionInfo(exeFilePath, "\\")
            ms = versionInfo['FileVersionMS']
            ls = versionInfo['FileVersionLS']
            versionStr = "%d.%d.%d.%d" % (win32api.HIWORD(ms), win32api.LOWORD (ms), win32api.HIWORD (ls), win32api.LOWORD (ls))

        except:
            versionStr = "0.0.0.0"

        return versionStr

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def getMsiVersion (_msiFilePath):
    #NEEDSWORK Do we ever need to use MSI version, since, Wix-Bundle knows, how to take care of MSI upgrades\downgrades 
    return None

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def getProductBuildVersion (numberOfFields=4):
    versionQuad = [os.environ ["REL_V"], os.environ ["MAJ_V"], os.environ ["MIN_V"], os.environ ["SUBMIN_V"]]
    if numberOfFields > 4 or numberOfFields < 1:
        return ".".join (versionQuad)
    return ".".join ([versionQuad [i] for i in range (numberOfFields)])

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def getProductBuildVersionTrimZero (numberOfFields=4):
    versionQuadTrim = [str(int(os.environ ["REL_V"])), str(int(os.environ ["MAJ_V"])), str(int(os.environ ["MIN_V"])), str(int(os.environ ["SUBMIN_V"]))]
    if numberOfFields > 4 or numberOfFields < 1:
        return ".".join (versionQuadTrim)
    return ".".join ([versionQuadTrim [i] for i in range (numberOfFields)])

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def getPackagePropertiesFileName ():
    if IsLanguagePackBuild ():
        return "packages_{0}.installers.xml".format (GetCurrentLanguage ().m_culture)

    return "packages.installers.xml"

_gCurrentLanguageSetting = None
def SetCurrentLanguage (languageSetting):
    global _gCurrentLanguageSetting
    prevLang = _gCurrentLanguageSetting
    _gCurrentLanguageSetting = languageSetting
    return prevLang

def GetCurrentLanguage ():
    if  _gCurrentLanguageSetting:
        return _gCurrentLanguageSetting
    else:
        return translationkit.getLanguageSettings () [0]

def GetBaseLanguage ():
    return translationkit.getLanguageSettings () [0]
#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def getInstallSetContextRoot (platform):
    return os.path.join (buildpaths.getOutputRoot (platform, False), "InstallSetContext")

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def getInstallSetContextFolder (part):
    return os.path.join (getInstallSetContextRoot (part.GetPlatform()), part.m_info.m_buildContext)

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def getInstallerBuildDirForPlatform (platform): #pylint: disable=function-redefined
    if IsLanguagePackBuild ():
        return os.path.join (buildpaths.getOutputRoot (platform, False), "build", "Installer_" + GetCurrentLanguage ().m_culture)
    return os.path.join (buildpaths.getOutputRoot (platform, False), "build", "Installer")

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def getCSPKGBuildDir (part):
    return os.path.join (getInstallerBuildDirForPlatform (part.GetPlatform()), part.m_info.m_name)

#-------------------------------------------------------------------------------------------
# Installer dir functions
#-------------------------------------------------------------------------------------------

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def getOutputBuildDir (part):
    return os.path.join (getInstallerBuildDirForPlatform (part.GetPlatform()), part.m_info.m_name)

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def getInstallerBuildDir (part):
    return os.path.join (getOutputBuildDir (part), "build")

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def getInstallerGenSrcDir (part):
    return os.path.join (getOutputBuildDir (part), "gensrc")

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def getInstallerBuildSourceDir (part):
    return os.path.join (getOutputBuildDir (part), "InstallBuildSource")

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def getInstallerBuildBinDir (part):
    return os.path.join (getInstallerBuildSourceDir (part), "Bin")

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def getInstallerBuildMakeDir (part):
    return os.path.join (getInstallerBuildSourceDir (part), "Make")

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def getInstallerBuildWixSourceDir (part):
    return os.path.join (getInstallerBuildSourceDir (part), "WixSource")

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def getInstallerBuildWxsAllFragmentsGenSrc (part):
    return os.path.join (getInstallerGenSrcDir (part), buildInstallSetWxsAllPartFileName)

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def getComponentGroupName (partInfo):
    return (os.path.basename (partInfo.m_file.lower ()).replace (".partfile.xml", "") + "_" + partInfo.m_name.lower ()).replace ("-", "_")

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
#-------------------------------------------------------------------------------------------
__wixSourceTypeList = [".wxs", ".wxi", ".wxl"]
__makeTypeList = [".mke", ".mki", ".py", ".xslt"]
#-------------------------------------------------------------------------------------------
def getInstallerBuildDirForType (part, filePath):
    (_, ext) = os.path.splitext (filePath.lower ())

    if ext in __wixSourceTypeList:
        return getInstallerBuildWixSourceDir (part)

    if ext in __makeTypeList:
        return getInstallerBuildMakeDir (part)

    return getInstallerBuildBinDir (part) 

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def generateIdentifier (idString, stringToRemove, stringListToAppend=None, stringPrefix ="", stringSuffix=""):
    idString    = idString.lower ().replace (stringToRemove.lower (), "")
    if stringListToAppend == None:
        stringListToAppend = []

    stringListToAppend.insert (0, idString)
    idString = "|".join (stringListToAppend)

    return stringPrefix + "|".join (stringListToAppend) + stringSuffix

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def validateGuidOrExpandMacrosToSeed (partName, platformName, additionalStr, guidSeedStr="", seedGuid="FED455DF-DC42-4E8C-8ABD-03829FA7D284", expandLanguage=True):
    isValidGuid = True
    guidSeedStr = guidSeedStr.upper ()

    seedGuid = seedGuid.rstrip ('}').lstrip ('}')

    if guidSeedStr.find (";") > -1:
        if platformName.endswith ("86"):
            guidSeedStr = guidSeedStr.split (";") [0]
        else:
            guidSeedStr = guidSeedStr.split (";") [1]

    if len (guidSeedStr) == 0:
        utils.showInfoMsg ("#### No Guid or seed provided, setting seed : " + defaultGuidSeed, utils.INFO_LEVEL_VeryInteresting, utils.YELLOW)
        guidSeedStr = defaultGuidSeed
        isValidGuid = False

    try:
        uuid.UUID (guidSeedStr)
    except:
        isValidGuid = False

    if isValidGuid:
        return guidSeedStr

    versionNames = ["REL_V", "MAJ_V", "MIN_V", "SUBMIN_V"]

    for versioName in versionNames:
        guidSeedStr = guidSeedStr.replace ("${" + versioName + "}", str (int (os.environ [versioName])))

    guidSeedStr = guidSeedStr.replace ("${PARTNAME}", partName.lower ())
    guidSeedStr = guidSeedStr.replace ("${PLATFORM}", platformName.lower ())
    if expandLanguage:
        guidSeedStr = guidSeedStr.replace ("${LANGUAGE}", ".".join ([langSetting.m_culture for langSetting in translationkit.getLanguageSettings ()]))
    else:
        guidSeedStr = guidSeedStr.replace ("${LANGUAGE}", "en")

    utils.showInfoMsg ("\n#### Using seed to generate GUID : " + guidSeedStr + ";" + additionalStr+ " Base: " + seedGuid + "\n", utils.INFO_LEVEL_SomewhatInteresting)
    return utils.createSeededGuid (guidSeedStr + ";" + additionalStr.lower (), seedGuid)

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def createRandomGuid ():
    return "{" + str (uuid.uuid4()).upper() + "}"


#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def convertGuildToIdFormat (guidStr):
    return guidStr.replace ("{", "").replace ("}", "").replace ("-", "").strip ().upper ()


#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def ExtractRegistry (fileId, compId, filePath, part, isWinx64):
    extractorCmd = []
    singletonISExe = utils.GetBsiToolsFile ("SingletonIS.exe")
    if os.path.exists (singletonISExe):
        extractorCmd.append (singletonISExe)

    #noModalErrorDialog = utils.GetBsiToolsFile ("NoModalErrorDialog.exe")
    #if os.path.exists (noModalErrorDialog):
    #    extractorCmd.append (noModalErrorDialog)

    filePath = os.path.normcase (os.path.normpath (filePath))
    wxiFileFullPath = os.path.join (getInstallerGenSrcDir (part), os.path.basename (filePath) + ".wxi")
    extractor = os.path.join (getInstallerBuildBinDir (part), "RegistryExtractor.exe")
    if not os.path.exists (extractor):
        extractor = utils.GetBsiToolsFile ("RegistryExtractor_x64.exe" if isWinx64 else "RegistryExtractor_x86.exe")

    extractorCmd.append (extractor)
    extractorCmd.append (filePath)
    extractorCmd.extend (["/outfile",  wxiFileFullPath])
    if filePath.lower ().endswith (".exe"):
        extractorCmd.extend (["/extrType",  "tlbrgs"])

    def ArgsWithEqualTo (lhs, rhs):
        return lhs + "=" + rhs 

    dirPath = os.path.dirname (filePath).strip ("\\")
    extractorCmd.append (ArgsWithEqualTo (filePath,  "[#" + fileId + "]"))
    extractorCmd.append (ArgsWithEqualTo (dirPath + "\\",   "[$" + compId + "]"))
    extractorCmd.append (ArgsWithEqualTo (dirPath,   "[$" + compId + "]"))

    symlinks.makeSureBaseDirectoryExists (wxiFileFullPath)
    utils.showAndLogMsg ("Executing:\r\n{0}\r\n".format (" ".join (extractorCmd)), None, utils.INFO_LEVEL_VeryInteresting)
    status = cmdutil.runAndLog (extractorCmd, None, os.path.dirname (filePath))
    if status != 0:
        raise utils.BuildError ("Cannot extract registry for  " + filePath)
    return wxiFileFullPath

#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
class WixFragment (object):
    def __init__(self, buildInstallSetPartUpgradeCode=""):
        self.m_componentGroups      = {}
        self.m_pathComponentGroups = {}

        self.m_buildInstallSetPartUpgradeCode = buildInstallSetPartUpgradeCode

        self.m_directories          = {}
        self.m_directoriesWithoutParent = {}
        self.m_mergeModules         = {}
        self.m_componentAttributes  = {}
        self.m_componentGroupNamesForNoDefaultReference = []
        self.m_bundleModules        = set ()
        self._m_isDirectoryDefinitionsIgnored = False
        self.m_directoryConfigurationData = []

        self.m_partsWithConfigurableMergeModule = []
        self.m_allDirectoriesAdded =    False
        self.m_componentElemUniquenessConstraint = []

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # bsiclass
    #-------------------------------------------------------------------------------------------
    def AddConfigurableModule (self, part):
        self.m_partsWithConfigurableMergeModule.append (part)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # bsiclass
    #-------------------------------------------------------------------------------------------
    def GetComponentGroup (self, part):
        compGroupName = getComponentGroupName (part.m_info)
        if compGroupName not in self.m_componentGroups:
            self.m_componentGroups [compGroupName] = WixComponentGroup (compGroupName)
            self.AddPartToParentComponentGroup (part)

        return self.m_componentGroups [compGroupName]

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # bsiclass
    #-------------------------------------------------------------------------------------------
    def AddComponentGroupNamesForNoDefaultReference (self, componentGroupNames):
        if len (self.m_componentGroupNamesForNoDefaultReference) > 0:
            return

        ignoreList = []
        for name in componentGroupNames.keys ():
            if name not in self.m_componentGroups:
                cmpGrp = WixComponentGroup (name)
                cmpGrp.m_userDefinedGrp = True
                for value in componentGroupNames[name]:
                    cmpGrp.AddComponentGroup (WixComponentGroup (value))
                    ignoreList.append (value)

                self.m_componentGroups [name] = cmpGrp

        self.m_componentGroupNamesForNoDefaultReference = ignoreList

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # bsiclass
    #-------------------------------------------------------------------------------------------
    def AddBundleModuleComponentGroup (self, cmpGrpNames):
        if len (cmpGrpNames) == 0 or cmpGrpNames == None:
            return

        for name in cmpGrpNames:
            self.m_bundleModules.add (name)
    #-------------------------------------------------------------------------------------------
    # bsimethod
    # bsiclass
    #-------------------------------------------------------------------------------------------
    def AddComponentGroupAttributes (self, componentAttributes):
        if not componentAttributes:
            return

        for grpName in componentAttributes.keys ():
            if grpName not in self.m_componentAttributes:
                self.m_componentAttributes [grpName] = componentAttributes[grpName]

    def GetComponentGroupAttribute (self, cmpGrpName):
        if not self.m_componentAttributes:
            return None

        if cmpGrpName not in self.m_componentAttributes:
            return None

        return self.m_componentAttributes [cmpGrpName]

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # bsiclass
    #-------------------------------------------------------------------------------------------
    def AddPathComponentGroups (self, filePaths):
        if len (self.m_pathComponentGroups.keys ()) > 0:
            return

        for filePath in filePaths:
            normalizedFilePath = os.path.normcase (os.path.normpath (filePath.lower ()).strip ('\\'))
            self.m_pathComponentGroups [normalizedFilePath] = WixComponentGroup (normalizedFilePath.replace ("\\", "_"))

    def GetPathComponentGroup (self, filePath):
        normalizedFilePath = os.path.normcase (os.path.normpath (filePath.lower ()).strip ('\\'))
        for (cmpId, cmpGrp) in self.m_pathComponentGroups:
            if str(cmpId).find (normalizedFilePath) > 0:
                return cmpGrp

        return None

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # bsiclass
    #-------------------------------------------------------------------------------------------
    def IsComponentGroupInNoDefaultReference (self, componentGroupName):
        if componentGroupName in self.m_componentGroupNamesForNoDefaultReference:
            return True

        return False

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # bsiclass
    #-------------------------------------------------------------------------------------------
    def AddDirectoryRefNames (self, directoryNames):
        if self._m_isDirectoryDefinitionsIgnored:
            return

        for directoryName in directoryNames:
            productDirectoryTuple = (True, directoryName [0], "", True)
            self.AddDirectory (directoryName [1], productDirectoryTuple, fromDirectoryRef=True)

        self._m_isDirectoryDefinitionsIgnored = True

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # bsiclass
    #-------------------------------------------------------------------------------------------
    def AddMergeModule (self, filePath, binding, part):
        wixMerge = WixMergeModule (filePath, part, binding)
        self.m_mergeModules [wixMerge.Id ()] = wixMerge


    #-------------------------------------------------------------------------------------------
    # bsimethod
    # bsiclass
    #-------------------------------------------------------------------------------------------
    def AddFile (self, filePath, dirTuple, part, extractReg="", languageSetting=None):
        dirTuple = (GetStaticDirName (dirTuple[0], part.m_currentLibType == globalvars.LIB_TYPE_Static), dirTuple[1], dirTuple[2])
        wixComp = WixComponent (filePath, part.GetPlatform (), dirTuple[0], dirTuple[1], extractReg, self.m_buildInstallSetPartUpgradeCode)
        self.GetComponentGroup (part).AddComponent (wixComp)

        if languageSetting:
            wixComp.SetLanguageSetting (languageSetting)

        if None != dirTuple[1] and len (dirTuple[1]) > 0:
            subDirs = os.path.normpath (dirTuple[1]).strip ("\\").split ("\\")
            subDirName = dirTuple[0] + "_" + subDirs [0].lower ()
            subDirTuple = (dirTuple[2], dirTuple[0], "\\".join (subDirs), True)
            self.AddDirectory (subDirName, subDirTuple, createdTuple=True)


    #-------------------------------------------------------------------------------------------
    # bsimethod
    # bsiclass
    #-------------------------------------------------------------------------------------------
    def AddFreeFile (self, filePath, part, dirId):
        dirId = GetStaticDirName (dirId, part.m_currentLibType == globalvars.LIB_TYPE_Static)
        wixComp = WixComponent (filePath, part.GetPlatform (), dirId, "", extractReg="", buildInstallSetPartUpgradeCode=self.m_buildInstallSetPartUpgradeCode)
        self.GetComponentGroup (part).AddComponent (wixComp)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # bsiclass
    #-------------------------------------------------------------------------------------------
    def AddTranskitFile (self, filePath, dirTuple, part, resourceObj, productDir=None):
        subDir = (dirTuple[1].strip ("\\") + os.sep if dirTuple[1] and len (dirTuple[1]) > 0 else "") + resourceObj.LanguageFolderName ()
        if productDir:
            subDir = os.path.join (subDir, productDir)
        dirTuple = (GetStaticDirName (dirTuple[0], part.m_currentLibType == globalvars.LIB_TYPE_Static), dirTuple[1], dirTuple[2])
        wixComp = WixComponent (filePath, part.GetPlatform (), dirTuple[0], subDir, extractReg="", buildInstallSetPartUpgradeCode=self.m_buildInstallSetPartUpgradeCode)
        wixComp.SetLanguageSetting (resourceObj.m_languageSetting)

        self.GetComponentGroup (part).AddComponent (wixComp)

        subDirs = os.path.normpath (subDir).strip ("\\").split ("\\")
        subDirName = dirTuple[0] + "_" + subDirs [0].lower ()
        subDirTuple = (dirTuple[2], dirTuple[0], "\\".join (subDirs), True)
        self.AddDirectory (subDirName, subDirTuple, createdTuple=True)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # bsiclass
    #-------------------------------------------------------------------------------------------
    def AddTranskitDir (self, dirPath, dirTuple, part, resourceObj, productDir=None):
        thisDirName = os.path.basename (dirPath)
        if productDir:
            thisDirName = os.path.join (productDir, thisDirName)

        for tfile in os.listdir (dirPath):
            fullFilePath = os.path.join (dirPath, tfile)
            if os.path.isfile (fullFilePath):
                self.AddTranskitFile (fullFilePath, dirTuple, part, resourceObj, thisDirName)
            elif os.path.isdir (fullFilePath):
                self.AddTranskitDir (fullFilePath, dirTuple, part, resourceObj, thisDirName)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # bsiclass
    #-------------------------------------------------------------------------------------------
    def AddBindingDir (self, dirPath, dirTuple, part, allowEmptyDir=False, languageSetting=None):
        dirItems  = os.listdir (dirPath)
        if allowEmptyDir:
            self.AddFile (dirPath, dirTuple, part, languageSetting=languageSetting)

        for item in dirItems:
            if item.lower () == "cvs" or item.lower () == ".hg" or item.lower () == ".git":
                continue

            fullPath = os.path.join (dirPath, item)
            if os.path.isdir (fullPath):
                newdirTuple = (dirTuple[0], ((dirTuple[1] + "\\") if dirTuple[1] and len (dirTuple[1]) > 0 else "") + item, dirTuple[2])
                self.AddBindingDir (fullPath, tuple(newdirTuple), part, allowEmptyDir, languageSetting)
            else:
                self.AddFile (fullPath, dirTuple, part, languageSetting=languageSetting)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # bsiclass
    #-------------------------------------------------------------------------------------------
    def AddPartToParentComponentGroup (self, part):
        if part.m_installSetPart != None and not part.m_isBuildInstallSet:
            self.GetComponentGroup (part.m_installSetPart).AddComponentGroup (self.GetComponentGroup (part))

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # bsiclass
    #-------------------------------------------------------------------------------------------
    def AddProductRootDir (self, rootDir):
        dirName = "installdir"
        wixDir = WixDirectory (dirName, rootDir, "installdirparent")
        self.m_directories [wixDir.Id ()]  = wixDir

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # bsiclass
    #-------------------------------------------------------------------------------------------
    def AddDirectory (self, dirName, productDirTuple, fromDirectoryRef=False, createdTuple=False):
        dirName         = dirName.lower ()
        #isDirInclude    = productDirTuple [0]
        relativeTo      = productDirTuple [1].lower () if not fromDirectoryRef else productDirTuple [1]
        path            = os.path.normpath (productDirTuple [2]).strip ("\\")

        #if isDirInclude == False:
        #    return

        if "installdir" not in self.m_directories:
            return

        if len (dirName) == 0:
            return

        if len (relativeTo) == 0:
            relativeTo = "installdir"

        tDirName            = dirName
        tRelativeFolders    = path.split ("\\")
        tRelativeFolders.reverse ()

        thisFolderName = "."
        if tRelativeFolders != None and len (tRelativeFolders) > 0:
            thisFolderName = tRelativeFolders.pop ()

        if len (tRelativeFolders) > 0 and not createdTuple:
            tDirName = "_".join ([x.lower () for x in tRelativeFolders]) + "_" + tDirName


        if WixDirectory.DirectoryId (tDirName) in self.m_directories:
            wixDir = self.m_directories [WixDirectory.DirectoryId (tDirName)]
            if wixDir.m_isAddedFromDirectoryRef:
                wixDir.SetFolderName (path)
                if WixDirectory.DirectoryId (relativeTo) in self.m_directories:
                    parentDir = self.m_directories [WixDirectory.DirectoryId (relativeTo)]
                    wixDir.m_OriginalDirElemIfCreated = WixDirectory (tDirName, os.path.join (parentDir.m_folderCompletePath, thisFolderName), parentDir.Id ())
        else:
            wixDir = WixDirectory (tDirName, thisFolderName, WixDirectory.DirectoryId (relativeTo, False))
            wixDir.m_isAddedFromDirectoryRef    = fromDirectoryRef
            self.m_directories [WixDirectory.DirectoryId (tDirName)]       = wixDir

        if len (tRelativeFolders) > 0:
            tRelativeFolders.reverse ()
            childDirTuple = (productDirTuple [0], tDirName, "\\".join (tRelativeFolders), productDirTuple [3])
            if createdTuple:
                self.AddDirectory (dirName + "_" + tRelativeFolders [0], childDirTuple, fromDirectoryRef=fromDirectoryRef, createdTuple=createdTuple)
            else:
                self.AddDirectory (dirName, childDirTuple, fromDirectoryRef=fromDirectoryRef, createdTuple=createdTuple)



    #-------------------------------------------------------------------------------------------
    # bsimethod
    # bsiclass
    #-------------------------------------------------------------------------------------------
    def _ResolveDirectoryHeirarchy (self):
        for dirId in self.m_directories.keys ():
            dirObj = self.m_directories[dirId]
            if dirObj.m_parentName not in self.m_directories:
                self.m_directoriesWithoutParent [dirId] = dirObj
                continue

            self.m_directories [dirObj.m_parentName].AddChild (dirObj)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # bsiclass
    #-------------------------------------------------------------------------------------------
    def GetAllConfigurationData (self):
        configurationData = []
        for cmpGrp in self.m_componentGroups.values ():
            for cmpItem in cmpGrp.m_componentList.values ():
                configurationData.append (cmpItem.ConfigDataElement())

        return configurationData


    #-------------------------------------------------------------------------------------------
    # bsimethod
    # bsiclass
    #-------------------------------------------------------------------------------------------
    def SaveCSPKG_NUGETFormat (self, saveNuGetFilePath, sourceNuGetFilePath):
        saveXmlFilePath = os.path.expandvars (saveNuGetFilePath)
        symlinks.makeSureBaseDirectoryExists (saveNuGetFilePath)

        sourceNuSpec = minidom.parse (sourceNuGetFilePath)

        idElements = sourceNuSpec.getElementsByTagName ("id")
        if len (idElements) == 0:
            raise utils.BuildError ("{0} NuSpec file do not have Id attribute".format (sourceNuGetFilePath))

        fc = idElements [0].firstChild
        if fc and fc.nodeType == fc.TEXT_NODE:
            self.m_nuGetPackageId = fc.data

        filesElems = sourceNuSpec.getElementsByTagName ("files")
        if len (filesElems) == 0:
            return

        filesElem = filesElems [0]
        csPkgPath = getCSPKGBuildDir (globalvars.currentAction.m_buildInstallSetPart)

        files = glob.glob (os.path.join (csPkgPath, "*.cspkg"))
        files.extend (glob.glob (os.path.join (csPkgPath, "*.cscfg")))

        for nfile in files:
            fileElement = sourceNuSpec.createElement ("file")
            fileElement.setAttribute ("src", nfile)
            fileElement.setAttribute ("target", "$CSPKG_NUGET_PATH$\\" + os.path.basename (nfile))
            filesElem.appendChild (fileElement)

        fileListFragmentFile    = FileWithModificationCheck (saveXmlFilePath)
        fileListFragmentFile.WriteXml (sourceNuSpec)
        fileListFragmentFile.Close ()


    #-------------------------------------------------------------------------------------------
    # bsimethod
    # bsiclass
    #-------------------------------------------------------------------------------------------
    def SaveNuGetFormat (self, saveNuGetFilePath, sourceNuGetFilePath):
        saveXmlFilePath = os.path.expandvars (saveNuGetFilePath)
        symlinks.makeSureBaseDirectoryExists (saveNuGetFilePath)

        sourceNuSpec = minidom.parse (sourceNuGetFilePath)

        idElements = sourceNuSpec.getElementsByTagName ("id")
        if len (idElements) == 0:
            raise utils.BuildError ("{0} NuSpec file do not have Id attribute".format (sourceNuGetFilePath))

        fc = idElements [0].firstChild
        if fc and fc.nodeType == fc.TEXT_NODE:
            self.m_nuGetPackageId = fc.data

        filesElems = sourceNuSpec.getElementsByTagName ("files")
        if len (filesElems) == 0:
            return

        filesElem = filesElems [0]

        self._ResolveDirectoryHeirarchy ()
        self.m_allDirectoriesAdded = True

        installDirPath = symlinks.normalizePathName (self.m_directories ["installdir"].GetCompleteFolderPath ())

        for cmpGrp in self.m_componentGroups.values ():
            for cmpItem in cmpGrp.m_componentList.values ():
                if cmpItem.m_isEmptyDirComponent:
                    continue
                sourceFile = cmpItem.m_filePath
                targetDir = self.m_directories [cmpItem.m_directoryIdRef.lower ()]
                targetPath = os.path.join (symlinks.normalizePathName (targetDir.GetCompleteFolderPath ()), os.path.basename (sourceFile)).replace (installDirPath, "").strip ("\\")
                fileElement = sourceNuSpec.createElement ("file")
                fileElement.setAttribute ("src", sourceFile)
                fileElement.setAttribute ("target", targetPath)
                filesElem.appendChild (fileElement)


        fileListFragmentFile    = FileWithModificationCheck (saveXmlFilePath)
        fileListFragmentFile.WriteXml (sourceNuSpec)
        fileListFragmentFile.Close ()


    #-------------------------------------------------------------------------------------------
    # bsimethod
    # bsiclass
    #-------------------------------------------------------------------------------------------
    def SaveBundleModuleDirs (self, bundleModule, genSrcDir):
        def GetDirNode (wixDirFrag):
            dirs = wixDirFrag.getElementsByTagName ("Directory")
            if len (dirs) > 0:
                return dirs [0]
            return None


        allDirWxsFile = os.path.join (os.path.expandvars (genSrcDir), bundleModule.GetDirWxsName ())
        directoryListFragment   = minidom.parseString (_wixFragmentXmlString)
        dirsWixElement          = directoryListFragment.documentElement

        for rootDir in self.m_directoriesWithoutParent.values ():
            allDirectories = rootDir.AllDirectoriesToXml ()
            for wixDir in allDirectories:
                dirNode = GetDirNode (wixDir)
                if dirNode:
                    dirId = dirNode.getAttribute ("Id")
                    if dirId != bundleModule.m_rootDirectory and dirId != "installdir":
                        dirsWixElement.appendChild (wixDir)

        dirsListFragmentFile    = FileWithModificationCheck (allDirWxsFile, encoding=True)
        dirsListFragmentFile.WriteXml (directoryListFragment)
        dirsListFragmentFile.Close ()

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # bsiclass
    #-------------------------------------------------------------------------------------------
    def Save (self, buildInstallSetPart, saveAllParts, saveAllDirectories, isProduct=False):

        saveAllParts         = os.path.expandvars (saveAllParts)
        saveAllDirectories   = os.path.expandvars (saveAllDirectories)

        fileListFragment        = minidom.parseString (_wixFragmentXmlString)
        wixElement              = fileListFragment.documentElement

        directoryListFragment   = minidom.parseString (_wixFragmentXmlString)
        dirsWixElement          = directoryListFragment.documentElement

        if isProduct and not self.m_allDirectoriesAdded:
            self._ResolveDirectoryHeirarchy ()
            self.m_allDirectoriesAdded = True

            def GetDirNode (wixDirFrag):
                dirs = wixDirFrag.getElementsByTagName ("Directory")
                if len (dirs) > 0:
                    return dirs [0]
                return None

            for rootDir in self.m_directoriesWithoutParent.values ():
                allDirectories = rootDir.AllDirectoriesToXml ()
                for wixDir in allDirectories:
                    if wixDir.getAttribute ("Id") != "Dir_installdir":  # installdir is not defined by wxs file with file list. It is suppposed to be defined by product element only
                        dirsWixElement.appendChild (wixDir)
                    dirNode = GetDirNode (wixDir)
                    if dirNode:
                        bundleModules = buildInstallSetPart.m_buildInstallSetDescription.GetBundleModulesByDir (dirNode.getAttribute ("Id"))
                        if bundleModules:
                            for bundleModule in bundleModules:
                                dirNode.appendChild (bundleModule.GetCmpXml ())

            for configurableModulePart in self.m_partsWithConfigurableMergeModule:
                wixElement.appendChild (getMergeElementFromPart (configurableModulePart))

            # All Directories are obtained now. We need to find all components belongs to particular PathComponenGroup
            for wixDir in self.m_directories.values ():
                dirPath = os.path.normcase (os.path.normpath (wixDir.GetCompleteFolderPath ().lower ()))
                for pathKey in self.m_pathComponentGroups.keys ():
                    if dirPath.find (pathKey) > -1:
                        for cmpGrp in self.m_componentGroups.values ():
                            components = cmpGrp.GetAndDeleteComponentsWithDirectoryId (wixDir.Id ())
                            for wixComponent in components:
                                self.m_pathComponentGroups[pathKey].AddComponent (wixComponent)

        else:
            directoryIdUniqueness = []
            for cmpGrp in self.m_componentGroups.values ():
                for cmpItem in cmpGrp.m_componentList.values ():
                    if cmpItem.m_directoryIdRef in directoryIdUniqueness:
                        continue

                    wixElement.appendChild (cmpItem.ConfigDirectoryElement ())
                    directoryIdUniqueness.append (cmpItem.m_directoryIdRef)
                    self.m_directoryConfigurationData.append (ConfigurationElement (cmpItem.m_directoryIdRef))
                    self.m_directoryConfigurationData.append (SubstitutionElement (cmpItem.m_directoryIdRef))

        for cmpGrp in self.m_componentGroups.values ():
            wixElement.appendChild (cmpGrp.ToXml (self))

        for cmpGrp in self.m_pathComponentGroups.values ():
            wixElement.appendChild (cmpGrp.ToXml (self))

        (saveDirectory, _) = os.path.split (saveAllParts)
        symlinks.makeSureDirectoryExists (saveDirectory)


        fileListFragmentFile    = FileWithModificationCheck (saveAllParts, encoding=True)
        fileListFragmentFile.WriteXml (fileListFragment)
        fileListFragmentFile.Close ()

        dirsListFragmentFile    = FileWithModificationCheck (saveAllDirectories, encoding=True)
        dirsListFragmentFile.WriteXml (directoryListFragment)
        dirsListFragmentFile.Close ()


#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
class WixComponentGroup (object):
    def __init__ (self, groupName):
        self.m_name = groupName
        self.m_componentList = {}
        self._m_componentGroupList = {}
        self.m_userDefinedGrp  = False

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # bsiclass
    #-------------------------------------------------------------------------------------------
    def Id (self):
        name =  self.m_name.replace ("-", "_")
        if (name [:1]).isdigit ():
            name = "CG_" + name

        return name

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # bsiclass
    #-------------------------------------------------------------------------------------------
    def AddComponent (self, wixComp):
        self.m_componentList [wixComp.Id ()] = wixComp

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # bsiclass
    #-------------------------------------------------------------------------------------------
    def AddComponentGroup (self, wixCompGroup):
        self._m_componentGroupList [wixCompGroup.Id ()] = wixCompGroup

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # bsiclass
    #-------------------------------------------------------------------------------------------
    def GetAndDeleteComponentsWithDirectoryId (self, dirId):
        components = []
        for cmpItem in self.m_componentList.values ():
            if cmpItem.m_directoryIdRef.lower () == dirId.lower ():
                components.append (cmpItem)
                self.m_componentList.pop (cmpItem.Id ())

        for cmpGrp in self._m_componentGroupList.values ():
            components.extend (cmpGrp.GetAndDeleteComponentsWithDirectoryId (dirId))

        return components

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # bsiclass
    #-------------------------------------------------------------------------------------------
    def Length (self):
        count = len (self.Components ())
        for cmpGrp in self.ComponentGroups ():
            count = count + cmpGrp.Length ()

        return count


    #-------------------------------------------------------------------------------------------
    # bsimethod
    # bsiclass
    #-------------------------------------------------------------------------------------------
    def ToRefXml (self):
        cmpGrpElem = minidom.Document ().createElement ("ComponentGroupRef")
        cmpGrpElem.setAttribute ("Id", self.Id ())

        return cmpGrpElem

    def Components (self):
        return list(self.m_componentList.values ())

    def ComponentGroups (self):
        return list(self._m_componentGroupList.values ())

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # bsiclass
    #-------------------------------------------------------------------------------------------
    def ToXml (self, wixParentFragment):
        cmpGrpElem = minidom.Document ().createElement ("ComponentGroup")
        cmpGrpElem.setAttribute ("Id", self.Id ())
        cmpGrpFragElem = minidom.Document ().createElement ("Fragment")
        cmpGrpFragElem.setAttribute ("Id", self.Id ())


        currentCmpAttrs = wixParentFragment.GetComponentGroupAttribute (self.m_name)
        
        #isCurrentDefault = True
        #if wixParentFragment.IsComponentGroupInNoDefaultReference (self.m_name):
        #    isCurrentDefault = False
        #if self.m_name in wixParentFragment.m_bundleModules:
        #    isCurrentDefault = False
        for cmpGrp in self.ComponentGroups ():
            isIncludeChildCmpGrp = False if (wixParentFragment.IsComponentGroupInNoDefaultReference (cmpGrp.m_name)) else True
            if not isIncludeChildCmpGrp and self.m_userDefinedGrp:
                isIncludeChildCmpGrp = True

            isIncludeChildCmpGrp =  isIncludeChildCmpGrp and cmpGrp.m_name not in wixParentFragment.m_bundleModules
            if isIncludeChildCmpGrp and not globalvars.currentAction.IsOptionalComponentGroup (cmpGrp.m_name, globalvars.currentAction.m_buildInstallSetPart):
                cmpGrpElem.appendChild (cmpGrp.ToRefXml ())
                continue

        for cmpItem in self.Components ():
            if not cmpItem.IsValidForBuild ():
                continue

            completefilePath = os.path.normpath(os.path.join(wixParentFragment.m_directories.get(cmpItem.m_directoryIdRef).GetCompleteFolderPath(), os.path.basename (cmpItem.m_filePath)))
            if globalvars.currentAction.IsInExclusionList (completefilePath, globalvars.currentAction.m_buildInstallSetPart):
                cmpItem.ToXml (None) # NEEDSWORK: Call this function anyway to extract the registry. Fix this by separating the extract registry operation from ToXml ()
                continue

            if currentCmpAttrs:
                for attrName in currentCmpAttrs.m_attributes.keys ():
                    cmpItem.SetCmpAttribute (attrName, currentCmpAttrs.m_attributes[attrName])

            cmpRef = cmpItem.ToRefXml ()
            cmpGrpElem.appendChild (cmpRef)

            cmpId = cmpRef.getAttribute ("Id").upper ()

            if cmpId not in wixParentFragment.m_componentElemUniquenessConstraint:
                cmpGrpFragElem.appendChild (cmpItem.ToXml (cmpGrpFragElem))
                wixParentFragment.m_componentElemUniquenessConstraint.append (cmpId)
            else:
                utils.showInfoMsg ("#### Warning: Dulplicate binding for Component {0}:{1}\n".format (self.m_name, cmpId), utils.INFO_LEVEL_VeryInteresting, utils.YELLOW)

        cmpGrpFragElem.appendChild (cmpGrpElem)

        return cmpGrpFragElem

#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
class WixComponent (object):
    #-------------------------------------------------------------------------------------------
    # bsimethod
    # bsiclass
    #-------------------------------------------------------------------------------------------
    def __init__(self, filePath, platform, dirIdRef, subdir, extractReg="", buildInstallSetPartUpgradeCode=""):
        self.m_filePath                 = filePath
        self._m_subDir                  = os.path.normpath (subdir)
        self.m_directoryIdRef           = WixDirectory.DirectoryId ((dirIdRef.lower () + ("_" + self._m_subDir.replace ("\\", "_") if (len (self._m_subDir) > 0 and self._m_subDir != ".") else "")))

        identifiers = [self.m_directoryIdRef.lower (), os.path.basename (filePath.lower ()), platform.GetDirName ().lower ()]
        if buildInstallSetPartUpgradeCode:
            identifiers.insert(0, buildInstallSetPartUpgradeCode)

        self._m_componentKeyIdentifier  = "|".join (identifiers)
        self._m_componentId             = utils.createSeededGuid (self._m_componentKeyIdentifier)
        self.m_extractRegistry          = extractReg.lower().strip ()
        self.m_languageSetting          = None
        self.m_cmpAttributes            = {}
        self.m_isWin64                  = platform.IsType (targetplatform.winX64)
        self.m_isEmptyDirComponent      = False
        if os.path.isdir (filePath):
            self.m_isEmptyDirComponent = True


    #-------------------------------------------------------------------------------------------
    # bsimethod
    # bsiclass
    #-------------------------------------------------------------------------------------------
    def SetLanguageSetting (self, languageSetting):
        self.m_languageSetting = languageSetting

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # bsiclass
    #-------------------------------------------------------------------------------------------
    def Id (self):
        return self._m_componentKeyIdentifier

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # bsiclass
    #-------------------------------------------------------------------------------------------
    def SetCmpAttribute (self, name, value):
        if name not in self.m_cmpAttributes:
            self.m_cmpAttributes [name] = value

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # bsiclass
    #-------------------------------------------------------------------------------------------
    def ToXml (self, cmpGrpFragElem):
        cmpElem = minidom.Document ().createElement ("Component")

        cmpId       = "C" + convertGuildToIdFormat (self._m_componentId)
        fileId      = "F" + convertGuildToIdFormat (self._m_componentId)

        cmpElem.setAttribute ("Id", cmpId)
        cmpElem.setAttribute ("Guid", self._m_componentId)


        commentNode = minidom.Document ().createComment ("ID Seeded by: " + CleanStringForComment (self._m_componentKeyIdentifier))
        cmpElem.appendChild (commentNode)

        if self.m_isWin64:
            cmpElem.setAttribute ("Win64", "yes")
        else:
            cmpElem.setAttribute ("Win64", "no")

        for attrName in self.m_cmpAttributes.keys ():
            cmpElem.setAttribute (attrName, self.m_cmpAttributes[attrName])

        if not self.m_isEmptyDirComponent:
            fileElem = minidom.Document ().createElement ("File")
            fileElem.setAttribute ("Source", os.path.normpath (self.m_filePath))
            fileElem.setAttribute ("Id", fileId)
            fileElem.setAttribute ("KeyPath", "yes")
            if len (self.m_extractRegistry) > 0:
                wixDir = globalvars.currentAction.m_buildInstallSetPart.m_installFragment.m_directories [self.m_directoryIdRef]
                wxiFilePath = None
                if self.m_extractRegistry == "extract" or self.m_extractRegistry == "true" or self.m_extractRegistry == "yes":
                    wxiFilePath = ExtractRegistry (fileId, cmpId, os.path.join (wixDir.GetCompleteFolderPath (), os.path.basename (self.m_filePath)), globalvars.currentAction.m_buildInstallSetPart, self.m_isWin64)
                elif self.m_extractRegistry == "include":
                    wxiFilePath = os.path.basename (self.m_filePath) + ".wxi"
                elif self.m_extractRegistry == "includechild":
                    wxiFilePath = os.path.basename (self.m_filePath) + ".wxi"

                if not wxiFilePath:
                    raise utils.BuildError ("Uknown option is provided ExtractReg={0}".format (self.m_extractRegistry))

                includeWxi = minidom.Document ().createProcessingInstruction ("include", wxiFilePath)
                
                if self.m_extractRegistry == "includechild":
                    fileElem.appendChild (includeWxi)
                else:
                    cmpElem.appendChild (includeWxi)


            cmpElem.appendChild (fileElem)
        else:
            cmpElem.appendChild (minidom.Document ().createElement ("CreateFolder"))
        if not cmpGrpFragElem:
            return None

        dirElem = getElementById (cmpGrpFragElem, "DirectoryRef", self.m_directoryIdRef.lower ())
        if dirElem == None:
            dirElem = minidom.Document ().createElement ("DirectoryRef")
            dirElem.setAttribute ("Id", self.m_directoryIdRef.lower ())

        dirElem.appendChild (cmpElem)

        return dirElem

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # bsiclass
    #-------------------------------------------------------------------------------------------
    def ToRefXml (self):
        cmpElem = minidom.Document ().createElement ("ComponentRef")
        cmpElem.setAttribute ("Id", "C" + convertGuildToIdFormat (self._m_componentId))

        return cmpElem

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # bsiclass
    #-------------------------------------------------------------------------------------------
    def IsValidForBuild (self):
        if not IsLanguagePackBuild ():
            return True

        if not self.m_languageSetting:
            return False

        return self.m_languageSetting.m_culture.lower () == GetCurrentLanguage ().m_culture.lower ()
    #-------------------------------------------------------------------------------------------
    # bsimethod
    # bsiclass
    #-------------------------------------------------------------------------------------------
    def ConfigDirectoryElement(self):
        dirElem = minidom.Document ().createElement ("Directory")
        dirElem.setAttribute ("Id", self.m_directoryIdRef)
        dirRefElem = minidom.Document ().createElement ("DirectoryRef")
        dirRefElem.setAttribute ("Id", "INSTALLDIR")
        dirRefElem.appendChild (dirElem)
        fragElem = minidom.Document ().createElement ("Fragment")
        fragElem.setAttribute ("Id", "INSTALLDIR_" + self.m_directoryIdRef)
        fragElem.appendChild (dirRefElem)

        return fragElem

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # bsiclass
    #-------------------------------------------------------------------------------------------
    def ConfigDataElement (self):
        return Configurationdata (self.m_directoryIdRef)


#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
class WixMergeModule (object):
    #-------------------------------------------------------------------------------------------
    # bsimethod
    # bsiclass
    #-------------------------------------------------------------------------------------------
    def __init__(self, filePath, part, binding):
        self._m_filePath                = filePath
        self._m_moduleKeyIdentifier     = WixMergeModule.BuildId (part) + "_" + os.path.basename (filePath).lower ()
        self._m_directoryIdRef          = binding.m_productDirectory
        if not self._m_directoryIdRef or len (self._m_directoryIdRef) == 0 or self._m_directoryIdRef.lower () == "mergemodules":
            self._m_directoryIdRef = "TARGETDIR"

        self._m_configurationData       = {}
        self.m_childDirectories         = []

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # bsiclass
    #-------------------------------------------------------------------------------------------
    def AddConfigurationData (self, moduleConfigurationData):
        self._m_configurationData [moduleConfigurationData.m_name] = moduleConfigurationData

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # bsiclass
    #-------------------------------------------------------------------------------------------
    def Id (self):
        return self._m_moduleKeyIdentifier

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # bsiclass
    #-------------------------------------------------------------------------------------------
    def ToXml (self):
        mergeElem = minidom.Document ().createElement ("Merge")
        mergeElem.setAttribute ("Id", self.Id())
        mergeElem.setAttribute ("Language", "0")
        mergeElem.setAttribute ("SourceFile", self._m_filePath)
        mergeElem.setAttribute ("DiskId", "1")

        for configData in self._m_configurationData.values ():
            mergeElem.appendChild (configData.ToXml ())

        dirElem = minidom.Document ().createElement ("DirectoryRef")
        dirElem.setAttribute ("Id", self._m_directoryIdRef)
        dirElem.appendChild (mergeElem)

        fragElem = minidom.Document ().createElement ("Fragment")
        fragElem.setAttribute ("Id", "Merge_" + self.Id ())
        fragElem.appendChild (dirElem)

        return fragElem

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # bsiclass
    #-------------------------------------------------------------------------------------------
    def ToRefXml (self):
        mergeElem = minidom.Document ().createElement ("MergeRef")
        mergeElem.setAttribute ("Id", self.Id ())

        return mergeElem

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # bsiclass
    #-------------------------------------------------------------------------------------------
    @staticmethod
    def BuildId (part):
        return os.path.basename (part.m_info.m_file.lower ()).replace (".partfile.xml", "") + "_" + part.m_info.m_name.lower ()


#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
class WixDirectory (object):
    def __init__ (self, productDirName, folderName, parentName):
        self._m_productDirName              = WixDirectory.DirectoryId (productDirName)
        self._m_folderName                  = folderName
        self.m_parentName                   = parentName
        self.m_folderCompletePath           = folderName
        self.m_OriginalDirElemIfCreated     = None
        self.m_falseDirectory               = False
        self.m_realParentId                 = ""
        self.m_isAddedFromDirectoryRef      = False

        if self._m_productDirName == "installdir":
            self.m_folderCompletePath = self._m_folderName
            self._m_folderName = ""

        self._m_childDirectories = []

    @staticmethod
    def DirectoryId (dirName, convertToLowerCase=True):
        #dirName = dirName.replace ("-", "_").replace (" ", "_")
        dirName = re.sub ('[^0-9a-zA-Z_]+', '_', dirName)
        if convertToLowerCase:
            return dirName.lower ()
        else:
            return dirName

    def ToXml (self):
        dirElem = minidom.Document ().createElement ("Directory")
        dirElem.setAttribute ("Id", self.Id ())
        if len (self._m_folderName) > 0:
            dirElem.setAttribute ("Name", self._m_folderName)

        commentElem = minidom.Document ().createComment ("Directory files folder: " + CleanStringForComment (self.GetCompleteFolderPath ()))
        dirElem.appendChild (commentElem)

        parentDirRef   = minidom.Document ().createElement ("DirectoryRef")
        parentDirRef.setAttribute ("Id", self.m_parentName)
        if self.m_OriginalDirElemIfCreated:
            parentDirRef.appendChild (minidom.Document ().createComment ("REAL_ID:" + CleanStringForComment (self.m_OriginalDirElemIfCreated.m_parentName)))

        parentDirRef.appendChild (dirElem)

        fragElem    = minidom.Document ().createElement ("Fragment")
        fragElem.setAttribute ("Id", "Dir_" + self.Id ())
        fragElem.appendChild (parentDirRef)

        return fragElem

    def AllDirectoriesToXml (self):
        toXmlDirList = []

        toXmlDirList.append (self.ToXml ())
        for child in self._m_childDirectories:
            child.m_folderCompletePath = os.path.join (self.GetCompleteFolderPath (), child.GetCompleteFolderPath ())
            toXmlDirList.extend (child.AllDirectoriesToXml ())

        return toXmlDirList

    def AddChild (self, childDir):
        self._m_childDirectories.append (childDir)

    def Id (self):
        return self._m_productDirName

    def SetFolderName (self, folderName):
        if (len (self._m_folderName) == 0 or self._m_folderName == ".")and len (folderName) != 0:
            self._m_folderName = folderName

    def GetCompleteFolderPath (self):
        return self.m_folderCompletePath if not self.m_OriginalDirElemIfCreated else self.m_OriginalDirElemIfCreated.m_folderCompletePath


#-------------------------------------------------------------------------------------------
# bsiclass
# Class just to save file, if changed
#-------------------------------------------------------------------------------------------
class  FileWithModificationCheck:
    def __init__ (self, filePath, encoding=None):
        self._m_filePath    = filePath
        self._m_filePathTmp = filePath + ".tmp"
        if encoding:
            self._m_fileObj = codecs.open (self._m_filePathTmp, "w", 'utf-8')
        else:
            self._m_fileObj = open (self._m_filePathTmp, 'wt')

    def Write (self, strToWrite):
        self._m_fileObj.write (strToWrite)

    def WriteLine (self, lineToWrite):
        self.Write (lineToWrite +  "\n")

    def WritePath (self, lineToWrite):
        self.WriteLine (lineToWrite +  "/")

    def WriteBmakeMacro (self, name, value):
        self.WriteLine (name + "=" + value)

    def WritePathMacro (self, name, value):
        self.WritePath (name + "=" + value)

    def WriteXml (self, xmlStream):
        xmlStream.writexml (self._m_fileObj, "", "  ", "\n")

    def WriteXmlNoIndent (self, xmlStream):
        xmlStream.writexml (self._m_fileObj)

    def Close (self):
        self._m_fileObj.close ()
        if os.path.exists (self._m_filePath):
            # if both the files are same, then just remove temp and go back
            if filecmp.cmp (self._m_filePath, self._m_filePathTmp):
                os.remove (self._m_filePathTmp)
                return
            else:
                os.remove (self._m_filePath)
        os.rename (self._m_filePathTmp, self._m_filePath)

#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
class ModuleConfigurationData (object):
    def __init__ (self, name, value, Type="name"):

        self.m_name     = name.upper () + "_" + Type.upper ()
        self.m_value    = value

    def ToXml (self):
        configElem = minidom.Document ().createElement ("ConfigurationData")
        configElem.setAttribute ("Name", self.m_name)
        configElem.setAttribute ("Value", self.m_value)

        return configElem

#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
class ComponentAttributes (object):
    def __init__ (self, cmpGrpName):
        self.m_cmpGroupName  = cmpGrpName
        self.m_attributes = {}

    def AddAttribute (self, name, value):
        if name not in self.m_attributes:
            self.m_attributes [name] = value

    def Name (self):
        return self.m_cmpGroupName.lower ()

#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
class BundleModule (object):
    def __init__ (self, dom, part):
        self.m_name                     = self.GetValue (dom, "Name").lower ()
        if globalvars.currentAction.m_sideBySide:
            self.m_productCode = validateGuidOrExpandMacrosToSeed (part.m_info.m_name + self.m_name, part.GetPlatform ().GetXmlName ().lower (), "bundle-module-creating-guid-sideBySide", defaultGuidSeed, "68231720-EC35-4F9A-9D19-0B6D2AB5EAA7", False)
            self.m_upgradeCode = validateGuidOrExpandMacrosToSeed (part.m_info.m_name + self.m_name, part.GetPlatform ().GetXmlName ().lower (), "bundle-module-creating-guid-sideBySide", defaultGuidSeed, "68231720-EC35-4F9A-9D19-0B6D2AB5EAA7", False)
        if not globalvars.currentAction.m_betaBuild:
            self.m_productCode              = validateGuidOrExpandMacrosToSeed (part.m_info.m_name, part.GetPlatform ().GetXmlName ().lower (), "bundle-module-creating-guid", self.GetValue (dom, "ProductCode"), "5161D406-C50E-4ACA-8158-16A4696C9C00", False)
            self.m_upgradeCode              = validateGuidOrExpandMacrosToSeed (part.m_info.m_name, part.GetPlatform ().GetXmlName ().lower (), "bundle-module-creating-guid", self.GetValue (dom, "UpgradeCode"), "5161D406-C50E-4ACA-8158-16A4696C9C00", False)        
        else:
            self.m_productCode = validateGuidOrExpandMacrosToSeed (part.m_info.m_name + self.m_name, part.GetPlatform ().GetXmlName ().lower (), "bundle-module-creating-guid-beta", defaultGuidSeed, "076ED856-9D09-44CE-9C15-61958BD9AFA5", False)
            self.m_upgradeCode = validateGuidOrExpandMacrosToSeed (part.m_info.m_name + self.m_name, part.GetPlatform ().GetXmlName ().lower (), "bundle-module-creating-guid-beta", defaultGuidSeed, "076ED856-9D09-44CE-9C15-61958BD9AFA5", False)

        self.m_isWin64                  = part.GetPlatform ().IsType (targetplatform.winX64)
        self.m_platform                 = part.GetPlatform ()
        self.m_componentGrpName         = self.GetValue (dom, "ComponentGroupName")
        self.m_rootDirectory            = self.GetValue (dom, "RootDirectory")
        self.m_productWxsFileName       = self.GetValue (dom, "ProductWxsFileName")
        self.m_productName              = self.GetValue (dom, "ProductName")
        self.m_isLanguagePack           = str (dom.getAttribute ("IsLanguagePackBuild")).lower () == "true"

        if globalvars.currentAction.m_sideBySide:
            self._m_cmpGuidSeedStr          = "|".join ([self.m_name, self.m_rootDirectory.lower (), self.m_componentGrpName.lower (), self.m_upgradeCode.lower (), "SideBySide"])
        else:
            self._m_cmpGuidSeedStr          = "|".join ([self.m_name, self.m_rootDirectory.lower (), self.m_componentGrpName.lower (), self.m_upgradeCode.lower (), "Beta" if globalvars.currentAction.m_betaBuild else "Release"])
        self.m_componenGuid             = utils.createSeededGuid (self._m_cmpGuidSeedStr, self.m_upgradeCode)
        self.m_propertyRefs             = self.GetItemSet (dom, "ModulePropertyRef")
        self.m_customActionRefs         = set ()

        self.m_langpackProductCode      = utils.createSeededGuid ("|".join ([self.m_name, self.m_platform.GetDirName (), self.m_productCode, "BundleModule"]), "BD89AD79-A02C-4182-875B-2908237E1D77")
        self.m_langpackUpgradeCode      = utils.createSeededGuid ("|".join ([self.m_name, self.m_platform.GetDirName (), self.m_upgradeCode, "BundleModule"]), "BD89AD79-A02C-4182-875B-2908237E1D77")

    def GetValue (self, dom, attributeName):
        value = dom.getAttribute (attributeName)
        self.CheckAndError (attributeName, value)
        return value

    def GetItemSet (self, dom, itemName):
        values = set ()
        itemSources = dom.getElementsByTagName (itemName)
        if not itemSources:
            return values
        for node in itemSources:
            languages = node.getAttribute ("Languages")
            if not IsValidForCurrentLanguage (languages):
                continue
            texts = SpliText (GetNodeText (node))
            if texts:
                for text in texts:
                    if text not in values:
                        values.add (text)
        return values

    def IsBuildAllowed (self):
        # If current build is langauge pack and language pack is allowed.
        if IsLanguagePackBuild () and not self.m_isLanguagePack:
            return False
        return True

    def GetFeatureIdName (self, isLangPackBuild):
        if translationkit.IsTranskitShell ():
            if isLangPackBuild:
                return "{0}_{1}".format (self.m_name, "langpack")
        return self.m_name

    def GetDirWxsName (self):
        return self.m_name + "_dir.wxs"

    def GetMainWxsName (self):
        return self.m_name + "_main.wxs"

    def GetWxiName (self):
        return self.m_name + "_inc.wxi"

    def GetMkiName (self):
        return self.m_name + "_inc.mki"

    def GetMsiName (self):
        if IsLanguagePackBuild ():
            return self.m_productName + self.m_platform.GetXmlName () + "_" + GetCurrentLanguage ().m_culture
        return self.m_productName + self.m_platform.GetXmlName ()

    def CheckAndError (self, attributeName, value):
        if len (value) == 0 or value == None:
            raise utils.BuildError ("{0} is a required attribute for BundleModule.".format (attributeName))

    def SaveWxiFile (self, genSrcDir):
        wxiFilePath         = os.path.join (genSrcDir, self.GetWxiName ())
        productProperties   = minidom.parseString (wixWxiXmlString)
        includeElem         = productProperties.documentElement

        def getProcessingInstructions (attribute):
            return productProperties.createProcessingInstruction ("define", attribute [0] + "=" + attribute [1])

        attributes = [
                    ("bbBundleModuleBaseCmpGuid", self.m_componenGuid),
                    ("bbBundleModuleRootDir", self.m_rootDirectory),
                     ]

        if IsLanguagePackBuild ():
            attributes.append (("bbBundleModuleProductCode", self.m_langpackProductCode))
            attributes.append (("bbBundleModuleUpgradeCode", self.m_langpackUpgradeCode))
            attributes.append (("bbBundleModuleProductCodeCore", self.m_productCode))
            attributes.append (("bbBundleModuleUpgradeCodeCore", self.m_upgradeCode))
            attributes.append (("bbBundleModuleShortName", "{0}-{1}".format (self.m_name, GetCurrentLanguage ().m_culture)))
            attributes.append (("bbBundleModuleFullName", "{0} - {1}".format (self.m_productName, GetCurrentLanguage ().m_languageName)))
        else:
            attributes.append (("bbBundleModuleProductCode", self.m_productCode))
            attributes.append (("bbBundleModuleUpgradeCode", self.m_upgradeCode))
            attributes.append (("bbBundleModuleShortName", "{0}".format (self.m_name)))
            attributes.append (("bbBundleModuleFullName", "{0}".format (self.m_productName)))

        for attribute in attributes:
            includeElem.appendChild (getProcessingInstructions (attribute))

        symlinks.makeSureBaseDirectoryExists (wxiFilePath)
        wxiFile    = FileWithModificationCheck (wxiFilePath)
        wxiFile.WriteXml (productProperties)
        wxiFile.Close ()

    def SaveMkiFile (self, genSrcDir):
        mkiFilePath         = os.path.join (genSrcDir, self.GetMkiName ())
        symlinks.makeSureBaseDirectoryExists (mkiFilePath)

        mkiFile    = FileWithModificationCheck (mkiFilePath)

        mkiFile.WriteBmakeMacro ("BundleModuleProductCode", self.m_productCode)
        mkiFile.WriteBmakeMacro ("bbBundleModuleUpgradeCode", self.m_upgradeCode)
        mkiFile.WriteBmakeMacro ("bbBundleModuleShortName", self.m_name)
        mkiFile.WriteBmakeMacro ("bbBundleModuleFullName", self.m_productName)
        mkiFile.WriteBmakeMacro ("bbBundleModuleBaseCmpGuid", self.m_componenGuid)
        mkiFile.WriteBmakeMacro ("bbBundleModuleRootDir", self.m_rootDirectory)
        mkiFile.WriteBmakeMacro ("INSTALLER_OUTPUT_FILENAME", self.GetMsiName ())
        mkiFile.WriteLine       ("INSTALLER_COMPILER_OPT    + -dBundleModuleWxi=" + self.GetWxiName ())
        mkiFile.WriteBmakeMacro ("bbBundleModuleDirWxs", self.GetDirWxsName ())
        mkiFile.Close ()

    def GetCmpXml (self):
        cmpElem = minidom.Document ().createElement ("Component")

        cmpId       = "C" + convertGuildToIdFormat (self.m_componenGuid)
        cmpElem.setAttribute ("Id", cmpId)
        cmpElem.setAttribute ("Guid", self.m_componenGuid)

        commentNode = minidom.Document ().createComment ("ID Seeded by: " + CleanStringForComment (self._m_cmpGuidSeedStr))
        cmpElem.appendChild (commentNode)

        if self.m_isWin64:
            cmpElem.setAttribute ("Win64", "yes")
        else:
            cmpElem.setAttribute ("Win64", "no")

        cmpElem.appendChild (minidom.Document ().createElement ("CreateFolder"))
        return cmpElem

    def GetCmpRefXml (self):
        cmpElem = minidom.Document ().createElement ("ComponentRef")
        cmpElem.setAttribute ("Id", "C" + convertGuildToIdFormat (self.m_componenGuid))
        return cmpElem

    def GetCmpGrpRefXml (self):
        cmpGrpElem = minidom.Document ().createElement ("ComponentGroupRef")
        cmpGrpElem.setAttribute ("Id", self.m_componentGrpName)
        return cmpGrpElem

    def __str__ (self):
        return "Name={0}, ProductCode={1}, UpgradeCode={2}, ComponentGroup={3}, RootDir={4}, WxsFile={5}, CompId={6}".format (
                    self.m_name,
                    self.m_productCode,
                    self.m_upgradeCode,
                    self.m_componentGrpName,
                    self.m_rootDirectory,
                    self.m_productWxsFileName,
                    self.m_componenGuid)


#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
class BuildInstallSetDescription (object):
    PRODUCT_WXS_NAME = "IFProductWxsFile.wxs"
    MODULE_WXS_NAME = "IFModuleWxsFile.wxs"
    BUNDLE_WXS_NAME = "IFBundleWxsFile.wxs"

                                    # (default, ElementName, IsRequired, WixMacroName or Element name
    _productIdElem                  = ("",      "ProductID",            False,  "bbVarProductID")
    _productCodeElem                = ("*",     "ProductCode",          False,  "bbVarProductCode")
    _upgradeCodeElem                = ("*",     "UpgradeCode",          False,  "bbVarUpgradeCode")
    _dependencyKeyNameCodeElem      = ("",      "DependencyKeyName",    False,  "bbVarDependencyKeyName")
    _productNameElem                = ("",      "ProductName",          False,  "bbVarProductName")
    _productShortNameElem           = ("",      "ProductShortName",     False,  "bbVarProductShortName")
    _propertyRefsElem               = ({},      "PropertyRef",          False,  "PropertyRef")
    _customActionRefsElem           = ({},      "CustomActionRef",      False,  "CustomActionRef")
    _wxsElementRefs                 = ({},      "WxsElementRefs",       False,  "WxsElementRefs")
    _directoryRefsElem              = ({},      "DirectoryRef",         False,  "DirectoryRef")
    _componentGroupsElem            = ({},      "ComponentGroup",       False,  "ComponentGroup")
    _pathComponentGroupsElem        = ({},      "PathComponentGroup",   False,  "PathComponentGroup")
    _uiTypeElem                     = ("",      "UIType",               False,  "bbVarUIType")
    _moduleNameElem                 = ("",      "ModuleName",           False,  "bbVarModuleName")
    _moduleCodeElem                 = ("*",     "ModuleCode",          False,  "bbVarModuleCode")
    _bundleNameElem                 = ("",      "BundleName",           False,  "bbVarBundleName")
    _bundleUpgradeCodeElem          = ("*",     "BundleUpgradeCode",   True,   "bbVarBundleUpgradeCode")
    _chainFragmentNameElem          = ("",      "ChainFragment",        False,  "bbVarChangeFragmentName")
    _productTargetPlatform          = ("x64",    "ProductTargetPlatform",     False,  "bbVarProductTargetPlatform")

    _productCodeElemCore            = ("*",     "ProductCode",          False,  "bbVarProductCodeCore")
    _upgradeCodeElemCore            = ("*",     "UpgradeCode",          False,  "bbVarUpgradeCodeCore")

    _exclusionTokensElem            = ({},      "ExclusionTokens",      False,  None)
    _omitExclusionTokensElem        = ({},      "OmitExclusionTokens",      False,  None)
    _optionalComponentGroups        = ({},      "OptionalComponentGroups",      False,  None)

    _productWxsFileName             = (PRODUCT_WXS_NAME,  "ProductWxsFileName",     False, None)
    _moduleWxsFileName              = (MODULE_WXS_NAME,   "ModuleWxsFileName",      False, None)
    _bundleWxsFileName              = (BUNDLE_WXS_NAME,   "BundleWxsFileName",      False, None)
    _bundleMkiFileName              = ("",                "BundleMkiFileName",      False, None)
    _bundleMkiAfterLinking          = ("",                "BundleMkiAfterLinking",  False, None)

    _mkiBeforeCompile               = ("",      "MkiBeforeCompile",     False,  "")
    _mkiAfterCompile                = ("",      "MkiAfterCompile",      False,  "")
    _mkiAfterLinking                = ("",      "MkiAfterLinking",      False,  "")
    _useDefaultFeature              = ("",      "UseDefaultFeature",    False,  "")
    _productBrandName               = ("",      "BrandName",            False,  "bbVarBrandName")

    def __init__ (self, part, ignoreMissingDom=False):
        self._m_part = part
        self.m_wixXmlFilePath = part.GetInstallerFilePath ()[1]
        if ignoreMissingDom: # This is passed in during Taglist where the files may not exist in the output tree. Otherwise use previous behavior.
            dom = minidom.parse (self.m_wixXmlFilePath) if self.m_wixXmlFilePath != None and os.path.exists(self.m_wixXmlFilePath) else None
        else:
            dom = minidom.parse (self.m_wixXmlFilePath) if self.m_wixXmlFilePath != None else None

        # Deafult values are being used, if on definition file is provided. Need to know this, so that, binding can be created from bsicommon\sharedwix\
        self.m_defaultValuesAreUsed  = True if dom == None else False
        self.m_requiredProductResources = {}

        def defaultGuid (_guidSeed):
            return defaultGuidSeed

        def directoryTupleCreator (dirElem):
            dirRefTuples = []
            if len (dirElem.childNodes) == 0:
                return dirRefTuples

            parentDirName = dirElem.getAttribute ("Parent")
            if parentDirName == None or len (parentDirName) == 0:
                return dirRefTuples

            for childDirName in dirElem.childNodes [0].nodeValue.split (";"):
                dirRefTuples.append ((parentDirName, childDirName))

            return dirRefTuples

        def exclusionFileParser(fileElem):
            values = []
            if len (fileElem.childNodes) > 0:
                exclusions = fileElem.childNodes [0].nodeValue.split (";")
                if exclusions:
                    for item in exclusions:
                        if item[0] == "@":
                            # a reference to a simple text file that contains the exclusion tokens
                            exclusionFileName = os.path.join (self._m_part.m_info.m_partFileDir, os.path.expandvars(item[1:]))
                            utils.showInfoMsg ("{0}: reading exclusion values from file '{1}'".format(part.GetInstallerFilePath()[1], exclusionFileName), utils.INFO_LEVEL_RarelyUseful)
                            f = open(exclusionFileName)
                            for line in f:
                                s = line.strip().strip('\r').strip('\n')
                                if len(s):
                                    values.append (s)
                        else:
                            # the item to be excluded
                            values.append (item)
            return values


        # Wxs file to be updated
        self.m_productWxsFileName   = self._getPropertyValue (dom, BuildInstallSetDescription._productWxsFileName) 
        self.m_moduleWxsFileName    = self._getPropertyValue (dom, BuildInstallSetDescription._moduleWxsFileName) 
        self.m_bundleWxsFileName    = self._getPropertyValue (dom, BuildInstallSetDescription._bundleWxsFileName)

        # product properties
        self.m_productID            = self._getPropertyValue (dom, BuildInstallSetDescription._productIdElem, defaultValue=None)
        self.m_productCode          = self._getPropertyValue (dom, BuildInstallSetDescription._productCodeElem, defaultValue=defaultGuid (BuildInstallSetDescription._productCodeElem [1]))
        self.m_upgradeCode          = self._getPropertyValue (dom, BuildInstallSetDescription._upgradeCodeElem, defaultValue=defaultGuid (BuildInstallSetDescription._upgradeCodeElem [1]))
        self.m_dependencyKeyName    = self._getPropertyValue (dom, BuildInstallSetDescription._dependencyKeyNameCodeElem)
        self.m_productName          = self._getPropertyValue (dom, BuildInstallSetDescription._productNameElem, defaultValue=self._m_part.m_info.m_name) 
        self.m_languagePackProductName = None
        self.m_productShortName     = self._getPropertyValue (dom, BuildInstallSetDescription._productShortNameElem, defaultValue=self._m_part.m_info.m_name) 
        self.m_propertyRefs         = self._getPropertyValue (dom, BuildInstallSetDescription._propertyRefsElem, isDict=True)
        self.m_customActionRefs     = self._getPropertyValue (dom, BuildInstallSetDescription._customActionRefsElem, isDict=True)
        self.m_wxsElementRefs       = self._getPropertyValue (dom, BuildInstallSetDescription._wxsElementRefs, isDict=True)
        self.m_directoryRefs        = self._getPropertyValue (dom, BuildInstallSetDescription._directoryRefsElem, isDict=True, childNodeParser=directoryTupleCreator)
        self.m_componentGroups      = self._getPropertyValue (dom, BuildInstallSetDescription._componentGroupsElem, isDict=True)
        self.m_pathComponentGroups  = self._getPropertyValue (dom, BuildInstallSetDescription._pathComponentGroupsElem, isDict=True)
        self.m_uiType               = self._getPropertyValue (dom, BuildInstallSetDescription._uiTypeElem)
        self.m_exclusionTokens      = self._getPropertyValue (dom, BuildInstallSetDescription._exclusionTokensElem, isDict=True, childNodeParser=exclusionFileParser)
        self.m_omitExclusionTokens  = self._getPropertyValue (dom, BuildInstallSetDescription._omitExclusionTokensElem, isDict=True, childNodeParser=exclusionFileParser)
        self.m_optionalComponentGroups = self._getPropertyValue (dom, BuildInstallSetDescription._optionalComponentGroups, isDict=True)
        self.m_productTargetPlatform = self._getPropertyValue (dom, BuildInstallSetDescription._productTargetPlatform, defaultValue=part.GetPlatform ().GetXmlName ())
        self.m_productBrandName     = self._getPropertyValue (dom, BuildInstallSetDescription._productBrandName) 

        #Merge module properties
        self.m_moduleName           = self._getPropertyValue (dom, BuildInstallSetDescription._moduleNameElem, defaultValue=self._m_part.m_info.m_name) 
        self.m_moduleCode           = self._getPropertyValue (dom, BuildInstallSetDescription._moduleCodeElem, defaultValue=defaultGuid (BuildInstallSetDescription._moduleCodeElem [1]))

        #Budle properties
        self.m_bundleName           = self._getPropertyValue (dom, BuildInstallSetDescription._bundleNameElem, defaultValue=self._m_part.m_info.m_name) 
        self.m_bundleUpgradeCode    = self._getPropertyValue (dom, BuildInstallSetDescription._bundleUpgradeCodeElem, defaultValue=defaultGuid (BuildInstallSetDescription._bundleUpgradeCodeElem [1]))
        self.m_chainFragmentName    = self._getPropertyValue (dom, BuildInstallSetDescription._chainFragmentNameElem)

        #Build instruction mki files
        self.m_beforeCompile        = self._getPropertyValue (dom, BuildInstallSetDescription._mkiBeforeCompile)
        self.m_afterCompile         = self._getPropertyValue (dom, BuildInstallSetDescription._mkiAfterCompile)
        self.m_afterLinking         = self._getPropertyValue (dom, BuildInstallSetDescription._mkiAfterLinking)
        self.m_useDefaultFeature    = False if (self._getPropertyValue (dom, BuildInstallSetDescription._useDefaultFeature, (BuildInstallSetDescription._useDefaultFeature [0]).lower () == "false")) else True
        self.m_bundleMkiFile        = self._getPropertyValue (dom, BuildInstallSetDescription._bundleMkiFileName)
        self.m_bundleMkiAfterLinking = self._getPropertyValue (dom, BuildInstallSetDescription._bundleMkiAfterLinking)

        #Lang pack properties
        self.m_productCodeCore = None
        self.m_upgradeCodeCore = None

        #Component additional attributes
        self.m_componentAttributes = {}
        self.UpdateComponentAttributes (dom)

        #Load bundle modules
        self.m_bundleModules = {}
        self.LoadBundleModules (dom)

        self.UpdateGuidCodesForBetaBuild ()
        self.UpdateGuidCodesForSideBySideBuild ()
        self.UpdateGuidWithExpandedValidValues ()

    #-------------------------------------------------------------------------------------------
    # bsiclass
    #                        BuildInstallSetDescription
    #-------------------------------------------------------------------------------------------
    def ExpandVariables (self, attribute):
        attribute = str (attribute).replace (getExapandableName (self.m_bundleUpgradeCode [1][3], 0), getFirstOrSecondFromSplit (self.m_bundleUpgradeCode, (self._m_part.GetPlatform().GetXmlName ().lower () == "x86")) [0])
        return attribute

    #-------------------------------------------------------------------------------------------
    # bsiclass
    #                        BuildInstallSetDescription
    #-------------------------------------------------------------------------------------------
    def MainWxsFileName (self):
        if self._m_part.IsProduct ():
            return self.m_productWxsFileName [0]
        else:
            return self.m_moduleWxsFileName [0]

    #-------------------------------------------------------------------------------------------
    # bsiclass
    #                        BuildInstallSetDescription
    #-------------------------------------------------------------------------------------------
    def LoadBundleModules (self, dom):
        if not dom:
            return

        bundleModuleElems = dom.getElementsByTagName ("BundleModule")
        for bm in bundleModuleElems:
            module = BundleModule (bm, self._m_part)
            if module.m_name not in self.m_bundleModules:
                self.m_bundleModules [module.m_name] = module

    #-------------------------------------------------------------------------------------------
    # bsiclass
    #                        BuildInstallSetDescription
    #-------------------------------------------------------------------------------------------
    def GetBundleModulesByDir (self, dirId):
        if dirId == None or len (dirId) == 0:
            return None

        bundleModules = []
        for bundleModule in self.m_bundleModules.values ():
            if dirId.lower () == bundleModule.m_rootDirectory:
                bundleModules.append (bundleModule)

        return bundleModules

    #-------------------------------------------------------------------------------------------
    # bsiclass
    #                        BuildInstallSetDescription
    #-------------------------------------------------------------------------------------------
    def GetBundleModuleCmpGrpNames (self):
        return [bundleModule.m_componentGrpName for bundleModule in self.m_bundleModules.values ()]

    #-------------------------------------------------------------------------------------------
    # bsiclass
    #                        BuildInstallSetDescription
    #-------------------------------------------------------------------------------------------
    def UpdateComponentAttributes (self, dom):
        if not dom:
            return

        compAttrs = dom.getElementsByTagName ("ComponentAttributes")
        if len (compAttrs) == 0:
            return

        for compAttr in compAttrs:
            name = compAttr.getAttribute ("ComponentGroupName")

            if len (name) == 0 or not name:
                continue

            cmpGrpAttributes = ComponentAttributes (name)

            if cmpGrpAttributes.Name () in self.m_componentAttributes:
                continue

            self.m_componentAttributes [cmpGrpAttributes.Name ()] = cmpGrpAttributes

            attributes = compAttr.getElementsByTagName ("Attribute")
            if not attributes or len (attributes) == 0:
                continue


            for attribute in attributes:
                attrName = attribute.getAttribute ("Name")
                if not attrName or len (attrName) == 0:
                    continue

                attrValue = attribute.getAttribute ("Value")
                cmpGrpAttributes.AddAttribute (attrName, attrValue)


    #-------------------------------------------------------------------------------------------
    # bsiclass
    #                        BuildInstallSetDescription
    #-------------------------------------------------------------------------------------------
    def UpdateGuidWithExpandedValidValues (self):
        self.m_productCode              = (validateGuidOrExpandMacrosToSeed (self._m_part.m_info.m_name, "x86", "productcode", self.m_productCode [0]) + ";" + validateGuidOrExpandMacrosToSeed (self._m_part.m_info.m_name, "x64", "productcode", self.m_productCode [0]), BuildInstallSetDescription._productCodeElem)
        self.m_upgradeCode              = (validateGuidOrExpandMacrosToSeed (self._m_part.m_info.m_name, "x86", "upgradecode", self.m_upgradeCode [0]) + ";" + validateGuidOrExpandMacrosToSeed (self._m_part.m_info.m_name, "x64", "upgradecode", self.m_upgradeCode [0]), BuildInstallSetDescription._upgradeCodeElem)
        self.m_moduleCode               = (validateGuidOrExpandMacrosToSeed (self._m_part.m_info.m_name, "x86", "modulecode", self.m_moduleCode [0]) + ";" + validateGuidOrExpandMacrosToSeed (self._m_part.m_info.m_name, "x64", "modulecode", self.m_moduleCode [0]), BuildInstallSetDescription._moduleCodeElem)
        self.m_bundleUpgradeCode        = (validateGuidOrExpandMacrosToSeed (self._m_part.m_info.m_name, "x86", "bundleupgradecode", self.m_bundleUpgradeCode [0], expandLanguage=False) + ";" + validateGuidOrExpandMacrosToSeed (self._m_part.m_info.m_name, "x64", "bundleupgradecode", self.m_bundleUpgradeCode [0], expandLanguage=False), BuildInstallSetDescription._bundleUpgradeCodeElem)

        if len (self.m_dependencyKeyName [0]) > 0 and self.m_dependencyKeyName [0] != None:
            self.m_dependencyKeyName        = (validateGuidOrExpandMacrosToSeed (self._m_part.m_info.m_name, "x86", "dependencykeyname", self.m_dependencyKeyName [0]), BuildInstallSetDescription._dependencyKeyNameCodeElem)

    #-------------------------------------------------------------------------------------------
    # bsiclass
    #                        BuildInstallSetDescription
    #-------------------------------------------------------------------------------------------
    def UpdateGuidCodesForBetaBuild (self):
        if not globalvars.currentAction.m_betaBuild:
            return

        self.m_productCode              = (defaultGuidSeed, BuildInstallSetDescription._productCodeElem)  
        self.m_upgradeCode              = (defaultGuidSeed, BuildInstallSetDescription._upgradeCodeElem)  
        self.m_moduleCode               = (defaultGuidSeed, BuildInstallSetDescription._moduleCodeElem)  
        self.m_bundleUpgradeCode        = (defaultGuidSeed, BuildInstallSetDescription._bundleUpgradeCodeElem)
           
    def UpdateGuidCodesForSideBySideBuild (self):
        if not globalvars.currentAction.m_sideBySide:
            return

        self.m_productCode              = (defaultGuidSeed, BuildInstallSetDescription._productCodeElem)  
        self.m_upgradeCode              = (defaultGuidSeed, BuildInstallSetDescription._upgradeCodeElem)  
        self.m_moduleCode               = (defaultGuidSeed, BuildInstallSetDescription._moduleCodeElem)  
        self.m_bundleUpgradeCode        = (defaultGuidSeed, BuildInstallSetDescription._bundleUpgradeCodeElem)
    
    #-------------------------------------------------------------------------------------------
    # bsiclass
    #                        BuildInstallSetDescription
    #-------------------------------------------------------------------------------------------
    def _getPropertyValue (self, dom, curProperty, defaultValue=None, isDict = False, childNodeParser=None):
        defaultValue = curProperty [0] if defaultValue == None else defaultValue
        # Initialize with new dict
        if isinstance (defaultValue, dict):
            defaultValue = {}

        if dom == None:
            return (defaultValue, curProperty)

        elems = dom.getElementsByTagName (curProperty [1])

        if None == elems or len (elems) == 0:
            return (defaultValue, curProperty)

        if not isDict:
            if len (elems [0].childNodes) == 0:
                return (defaultValue, curProperty)
            return (elems [0].childNodes [0].nodeValue, curProperty)

        valueDict = {}

        for elem in elems:
            name = elem.getAttribute ("Name")
            if name == "":
                name = elem.getAttribute ("Id")
            if name == "":
                name = createRandomGuid ()

            languages = elem.getAttribute ("Languages")
            if not IsValidForCurrentLanguage (languages):
                continue
            if childNodeParser != None:
                values = childNodeParser (elem)
            else:
                if len (elem.childNodes) > 0:
                    values = elem.childNodes [0].nodeValue.split (";")
                else:
                    values = []

            valueDict [name] = values

        return (valueDict, curProperty)

    #-------------------------------------------------------------------------------------------
    # bsiclass
    #                        BuildInstallSetDescription
    #-------------------------------------------------------------------------------------------
    def GetListTypePropertyValues (self, propertyName):
        attributeValue = getattr (self, propertyName)
        allValues = []
        if attributeValue == None:
            return allValues

        if isinstance (attributeValue [0], dict):
            for valueList in attributeValue [0].values ():
                for value in valueList:
                    allValues.append (value)

        return allValues

    #-------------------------------------------------------------------------------------------
    # bsiclass
    #                        BuildInstallSetDescription
    #-------------------------------------------------------------------------------------------
    def GetDictTypePropertyValue (self, propertyName):
        attributeValue = getattr (self, propertyName)
        allValues = {}
        if attributeValue == None:
            return allValues

        if isinstance (attributeValue [0], dict):
            return attributeValue [0]

        return allValues


    #-------------------------------------------------------------------------------------------
    # bsiclass
    #                        BuildInstallSetDescription
    #-------------------------------------------------------------------------------------------
    def CheckForFeatureAspect (self):
        if self._m_part.IsProduct () and self._m_part.m_featureAspects != None:
            return True

        return False

    #-------------------------------------------------------------------------------------------
    # bsiclass
    #                        BuildInstallSetDescription
    #-------------------------------------------------------------------------------------------
    def UpdateCodesForLanguagePackBuild (self):
        if not IsLanguagePackBuild ():
            return

        if not _gIsUpgradeCodeLanguageDependant and self.m_productCodeCore and self.m_upgradeCodeCore:
            return

        self.m_productCodeCore = (self.m_productCode [0], BuildInstallSetDescription._productCodeElemCore)
        self.m_upgradeCodeCore = (self.m_upgradeCode [0], BuildInstallSetDescription._upgradeCodeElemCore)

        productCode = getFirstOrSecondFromSplit (self.m_productCode, (self._m_part.GetPlatform().GetXmlName ().lower () == "x86")) [0]
        upgradeCode = getFirstOrSecondFromSplit (self.m_upgradeCode, (self._m_part.GetPlatform().GetXmlName ().lower () == "x86")) [0]
        self.m_productCode              = (validateGuidOrExpandMacrosToSeed (self._m_part.m_info.m_name, "x86", "productcode_langpack" + GetCurrentLanguage ().m_culture, defaultLangPackGuidSeed, productCode ) + ";" + validateGuidOrExpandMacrosToSeed (self._m_part.m_info.m_name, "x64", "productcode_langpack" + GetCurrentLanguage ().m_culture, defaultLangPackGuidSeed, productCode), BuildInstallSetDescription._productCodeElem)
        self.m_upgradeCode              = (validateGuidOrExpandMacrosToSeed (self._m_part.m_info.m_name, "x86", "upgradecode_langpack" + GetCurrentLanguage ().m_culture, defaultLangPackGuidSeed, upgradeCode ) + ";" + validateGuidOrExpandMacrosToSeed (self._m_part.m_info.m_name, "x64", "upgradecode_langpack" + GetCurrentLanguage ().m_culture, defaultLangPackGuidSeed, upgradeCode), BuildInstallSetDescription._upgradeCodeElem)
        self.m_languagePackProductName = (self.m_productName [0] + " " + GetCurrentLanguage ().m_languageName + "-Language Pack", self.m_productName [1])

    #-------------------------------------------------------------------------------------------
    # bsiclass
    #                        BuildInstallSetDescription
    #-------------------------------------------------------------------------------------------
    def SaveWxi (self, wxiPath, useFeatureAspects = True):
        productProperties   = minidom.parseString (wixWxiXmlString)
        includeElem         = productProperties.documentElement

        def getProcessingInstructions (attribute):
            return productProperties.createProcessingInstruction ("define", (attribute [1]) [3] + "=" + attribute [0])

        if IsLanguagePackBuild ():
            bundleCode = getFirstOrSecondFromSplit (self.m_bundleUpgradeCode, (self._m_part.GetPlatform ().GetXmlName ().lower () == "x86")) [0]
            includeElem.appendChild (productProperties.createProcessingInstruction ("define", ("BundleUpgradeCode_LanguagePack=" + validateGuidOrExpandMacrosToSeed (self._m_part.m_info.m_name, self._m_part.GetPlatform ().GetXmlName ().lower (), "bundle_languagePack" + bundleCode, defaultGuidSeed))))
            includeElem.appendChild (getProcessingInstructions (getFirstOrSecondFromSplit (self.m_productCodeCore, (self._m_part.GetPlatform ().GetXmlName ().lower () == "x86"))))
            includeElem.appendChild (getProcessingInstructions (getFirstOrSecondFromSplit (self.m_upgradeCodeCore, (self._m_part.GetPlatform ().GetXmlName ().lower () == "x86"))))

        includeElem.appendChild (getProcessingInstructions (getFirstOrSecondFromSplit (self.m_productCode, (self._m_part.GetPlatform ().GetXmlName ().lower () == "x86"))))
        includeElem.appendChild (getProcessingInstructions (getFirstOrSecondFromSplit (self.m_upgradeCode, (self._m_part.GetPlatform ().GetXmlName ().lower () == "x86"))))
        includeElem.appendChild (getProcessingInstructions (getFirstOrSecondFromSplit (self.m_moduleCode, (self._m_part.GetPlatform ().GetXmlName ().lower () == "x86"))))

        includeElem.appendChild (getProcessingInstructions (self.m_productID))
        if IsLanguagePackBuild ():
            includeElem.appendChild (getProcessingInstructions (self.m_languagePackProductName))
        else:
            includeElem.appendChild (getProcessingInstructions (self.m_productName))
        includeElem.appendChild (getProcessingInstructions (self.m_productShortName))
        includeElem.appendChild (getProcessingInstructions (self.m_productBrandName))
        includeElem.appendChild (getProcessingInstructions (self.m_uiType))
        includeElem.appendChild (getProcessingInstructions (self.m_moduleName))
        includeElem.appendChild (getProcessingInstructions (self.m_bundleName))
        includeElem.appendChild (getProcessingInstructions (getFirstOrSecondFromSplit (self.m_bundleUpgradeCode, (self._m_part.GetPlatform ().GetXmlName ().lower () == "x86"))))
        includeElem.appendChild (getProcessingInstructions (self.m_chainFragmentName))
        includeElem.appendChild (getProcessingInstructions (self.m_productTargetPlatform))
        includeElem.appendChild (productProperties.createProcessingInstruction ("define", "BBLanguageName" + "=" + GetCurrentLanguage ().m_name))
        includeElem.appendChild (productProperties.createProcessingInstruction ("define", "PRGLanguageShortName" + "=" + GetCurrentLanguage ().m_prgLanguageShortName))
        if translationkit.IsMultilingualBuild () or translationkit.IsTranskitMultilingualBuild ():
            includeElem.appendChild (productProperties.createProcessingInstruction ("define", "BB_MULTILINGUAL_BUILD" + "=" + "1"))
            includeElem.appendChild (productProperties.createProcessingInstruction ("define", "BB_MULTILINGUAL_LANGUAGES" + "=" + ",".join ( [lang.m_name for lang in translationkit.getLanguageSettings ()])))
            includeElem.appendChild (productProperties.createProcessingInstruction ("define", "BB_MULTILINGUAL_LANGUAGES_SEMICOLON" + "=" + ";".join ( [lang.m_name for lang in translationkit.getLanguageSettingsWithoutNeutral ()])))
        if translationkit.IsTranskitMultilingualBuild ():
            includeElem.appendChild (productProperties.createProcessingInstruction ("define", "BB_TRANSKIT_MULTILINGUAL_LANGUAGES_SEMICOLON" + "=" + ";".join ( [lang.m_name.replace('-', '_') for lang in translationkit.getLanguageSettingsWithoutNeutral ()])))

        if len (self.m_dependencyKeyName [0]) > 0 and self.m_dependencyKeyName [0] != None:
            includeElem.appendChild (getProcessingInstructions (self.m_dependencyKeyName))

        if globalvars.currentAction.m_sideBySide:
            includeElem.appendChild (productProperties.createProcessingInstruction ("define", "BBInstallerBuildTypeSideBySide" + "=1"))
        else:
            includeElem.appendChild (productProperties.createProcessingInstruction ("define", ("BBInstallerBuildTypeBeta" if globalvars.currentAction.m_betaBuild else  "BBinstallerBuildTypeRelease") + "=1"))
            
        includeElem.appendChild (productProperties.createProcessingInstruction ("define", "BBInstallSetRootComponentGroupName=" + getComponentGroupName (self._m_part.m_info)))

        if self.m_useDefaultFeature:
            includeElem.appendChild (productProperties.createProcessingInstruction ("define", "BBInstallerBuildUseDefaultFeature" + "=1"))

        if useFeatureAspects and self.CheckForFeatureAspect () and not IsLanguagePackBuild ():
            includeElem.appendChild (productProperties.createProcessingInstruction ("include", "PowerProductWixVars.wxi"))

        if IsLanguagePackBuild ():
            includeElem.appendChild (productProperties.createProcessingInstruction ("define", "bbVarLanguagePackBuild=1"))

        includeElem.appendChild (productProperties.createProcessingInstruction ("define", "bbVarProductVersionFirstThreeFields=" + getProductBuildVersionTrimZero (3)))

        (saveDirectory, _) = os.path.split (wxiPath)
        symlinks.makeSureDirectoryExists (saveDirectory)


        wxiFile    = FileWithModificationCheck (wxiPath)
        wxiFile.WriteXml (productProperties)
        wxiFile.Close ()

    #-------------------------------------------------------------------------------------------
    # bsiclass
    #                        BuildInstallSetDescription
    #-------------------------------------------------------------------------------------------
    def SaveMki (self, mkiPath, useFeatureAspects=True):

        (saveDirectory, _) = os.path.split (mkiPath)
        symlinks.makeSureDirectoryExists (saveDirectory)

        mkiFile    = FileWithModificationCheck (mkiPath)

        def writeDefintionInBmakeMacro (attribute):
            if len (attribute [0]) > 0 and attribute [0] != "false":
                mkiFile.WriteBmakeMacro ((attribute [1]) [1], attribute [0])

        writeDefintionInBmakeMacro (getFirstOrSecondFromSplit (self.m_productCode, (self._m_part.GetPlatform ().GetXmlName ().lower () == "x86")))
        writeDefintionInBmakeMacro (getFirstOrSecondFromSplit (self.m_upgradeCode, (self._m_part.GetPlatform ().GetXmlName ().lower () == "x86")))
        writeDefintionInBmakeMacro (self.m_productID)
        writeDefintionInBmakeMacro (self.m_productName)
        writeDefintionInBmakeMacro (self.m_productShortName)
        writeDefintionInBmakeMacro (self.m_productBrandName)
        writeDefintionInBmakeMacro (self.m_productWxsFileName)
        writeDefintionInBmakeMacro (self.m_bundleName)
        writeDefintionInBmakeMacro (getFirstOrSecondFromSplit (self.m_bundleUpgradeCode, (self._m_part.GetPlatform ().GetXmlName ().lower () == "x86")))
        writeDefintionInBmakeMacro (self.m_bundleWxsFileName)
        writeDefintionInBmakeMacro (self.m_bundleMkiFile)
        writeDefintionInBmakeMacro (self.m_bundleMkiAfterLinking)
        writeDefintionInBmakeMacro (self.m_chainFragmentName)
        writeDefintionInBmakeMacro (getFirstOrSecondFromSplit (self.m_moduleCode, (self._m_part.GetPlatform ().GetXmlName ().lower () == "x86")))
        writeDefintionInBmakeMacro (self.m_moduleName)
        writeDefintionInBmakeMacro (self.m_moduleWxsFileName)
        writeDefintionInBmakeMacro (self.m_uiType)
    
        if translationkit.IsMultilingualBuild () or translationkit.IsTranskitMultilingualBuild ():
            mkiFile.WriteBmakeMacro ("BB_MULTILINGUAL_BUILD", "1")
            mkiFile.WriteBmakeMacro ("BB_MULTILINGUAL_LANGUAGES", ",".join ( [lang.m_name for lang in translationkit.getLanguageSettings ()]))

        if hasattr (self._m_part, "m_installFragment"):
            if hasattr (self._m_part.m_installFragment, "m_nuGetPackageId"):
                mkiFile.WriteBmakeMacro ("BB_NUGET_PACKAGE_ID", self._m_part.m_installFragment.m_nuGetPackageId)

        if len (self.m_dependencyKeyName [0]) > 0 and self.m_dependencyKeyName [0] != None:
            writeDefintionInBmakeMacro (self.m_dependencyKeyName)

        writeDefintionInBmakeMacro (self.m_beforeCompile)
        writeDefintionInBmakeMacro (self.m_afterCompile)
        writeDefintionInBmakeMacro (self.m_afterLinking)

        mkiFile.WritePathMacro ("wixPackageOutFolder", getInstallerBuildDir (self._m_part))
        if self.m_useDefaultFeature:
            mkiFile.WriteBmakeMacro ("BBInstallerBuildUseDefaultFeature", "1")

        if IsLanguagePackBuild ():
            mkiFile.WriteBmakeMacro ("BBLanguagePackBuild", "1")

        if useFeatureAspects and self.CheckForFeatureAspect () and not IsLanguagePackBuild ():
            mkiFile.WriteLine ("%include $(BBInstallSetFeatureAspectPropertiesFolder)PowerProductProperties.mki")

        if self._m_part.IsProduct ():
            if self._m_part.m_nuSpecFile:
                mkiFile.WriteBmakeMacro ("BBNuSpecFile", self._m_part.m_nuSpecFile)

        mkiFile.Close ()

    #-------------------------------------------------------------------------------------------
    # bsiclass
    #                        BuildInstallSetDescription
    #-------------------------------------------------------------------------------------------
    def MergeBuildInstallSetDescription (self, buildInstallSetDecription):
        if buildInstallSetDecription == None:
            return

        def mergeAttributeValues (attribute1, attribute2):
            for name in attribute2 [0].keys ():
                if name not in attribute1 [0]:
                    (attribute1 [0])[name] = attribute2 [0][name]

        mergeAttributeValues (self.m_propertyRefs, buildInstallSetDecription.m_propertyRefs)
        mergeAttributeValues (self.m_customActionRefs, buildInstallSetDecription.m_customActionRefs)
        mergeAttributeValues (self.m_wxsElementRefs, buildInstallSetDecription.m_wxsElementRefs)
        mergeAttributeValues (self.m_directoryRefs, buildInstallSetDecription.m_directoryRefs)
        mergeAttributeValues (self.m_componentGroups, buildInstallSetDecription.m_componentGroups)
        mergeAttributeValues (self.m_pathComponentGroups, buildInstallSetDecription.m_pathComponentGroups)
        mergeAttributeValues (self.m_exclusionTokens, buildInstallSetDecription.m_exclusionTokens)
        mergeAttributeValues (self.m_omitExclusionTokens, buildInstallSetDecription.m_omitExclusionTokens)
        mergeAttributeValues (self.m_optionalComponentGroups, buildInstallSetDecription.m_optionalComponentGroups)

        for cmpGrpName in buildInstallSetDecription.m_componentAttributes.keys ():
            if cmpGrpName in self.m_componentAttributes:
                continue
            self.m_componentAttributes [cmpGrpName] = buildInstallSetDecription.m_componentAttributes[cmpGrpName]

        def getNonListAttributes (attribute1, attribute2):
            (name, ext) = os.path.splitext (attribute1 [0].lower ())
            if name == "false":
                return ("", attribute1 [1])

            if ext != ".mki":
                return attribute2

            return attribute1

        self.m_beforeCompile    = getNonListAttributes (self.m_beforeCompile,   buildInstallSetDecription.m_beforeCompile)
        self.m_afterCompile     = getNonListAttributes (self.m_afterCompile ,   buildInstallSetDecription.m_afterCompile)
        self.m_afterLinking     = getNonListAttributes (self.m_afterLinking ,   buildInstallSetDecription.m_afterLinking)
        self.m_bundleMkiFile    = getNonListAttributes (self.m_bundleMkiFile ,  buildInstallSetDecription.m_bundleMkiFile)
        self.m_bundleMkiAfterLinking = getNonListAttributes (self.m_bundleMkiAfterLinking ,  buildInstallSetDecription.m_bundleMkiAfterLinking)

    #-------------------------------------------------------------------------------------------
    # bsiclass
    #                        BuildInstallSetDescription
    #-------------------------------------------------------------------------------------------
    def SaveBundleModuleWxs (self, bundleModule, genSrcDir):
        mainWxsFileName = bundleModule.m_productWxsFileName
        mainWxsFilePath = os.path.join (getInstallerBuildDirForType (self._m_part, mainWxsFileName), mainWxsFileName)
        if not os.path.exists (mainWxsFilePath):
            raise utils.BuildError ("WXS file is missing  " + mainWxsFilePath)
    
        mainWxsDom = minidom.parse (mainWxsFilePath)
        def getFirstElement (tagName, errorOnZero=True):
            elems = mainWxsDom.getElementsByTagName (tagName)
            if None == elems or len (elems) == 0:
                if errorOnZero:
                    raise utils.BuildError (tagName + " tag name cannot be found in " + mainWxsFilePath)
                else:
                    return

            return elems [0]

        # Get and Check first directory element in Main Product.wxs file
        referenceSibling = getFirstElement ("Directory")
        if referenceSibling == None:
            raise utils.BuildError ("no sibling to reference is found")

        # Get and Check first Feature element in Main Product.wxs file. First Feature element is treated as Default Feature.
        defaultFeature = getFirstElement ("Feature")
        defaultFeature.appendChild (bundleModule.GetCmpGrpRefXml ())

        # NEEDSWORK: Currently there is no support for including of merge-modules in bundle modules.
        productElem = getFirstElement ("Product", True)
        for curProperty in bundleModule.m_propertyRefs:
            propertyRef = minidom.Document ().createElement ("PropertyRef")
            propertyRef.setAttribute ("Id", curProperty)
            productElem.insertBefore (propertyRef, referenceSibling)

        wxsFilePath    = os.path.join (genSrcDir, bundleModule.GetMainWxsName ())
        mainWxsFile    = FileWithModificationCheck (wxsFilePath)
        mainWxsFile.WriteXmlNoIndent (mainWxsDom)
        mainWxsFile.Close ()

    #-------------------------------------------------------------------------------------------
    # bsiclass
    #                        BuildInstallSetDescription
    #-------------------------------------------------------------------------------------------
    def SaveMainWxs (self, wxsFilePath):
        mainWxsFileName = self.MainWxsFileName ()
        mainWxsFilePath = os.path.join (getInstallerBuildDirForType (self._m_part, mainWxsFileName), mainWxsFileName)

        if not os.path.exists (mainWxsFilePath):
            raise utils.BuildError ("WXS file is missing  " + mainWxsFilePath)

        mainWxsDom = minidom.parse (mainWxsFilePath)
        def getFirstElement (tagName, errorOnZero=True):
            elems = mainWxsDom.getElementsByTagName (tagName)
            if None == elems or len (elems) == 0:
                if errorOnZero:
                    raise utils.BuildError (tagName + " tag name cannot be found in " + mainWxsFilePath)
                else:
                    return

            return elems [0]

        def createChildIdElem (curId, childName):
            elem = mainWxsDom.createElement (childName)
            elem.setAttribute ("Id", curId)
            return elem

        referenceSibling = getFirstElement ("Directory")

        if referenceSibling == None:
            raise utils.BuildError ("no sibling to reference is found")


        defaultFeature = getFirstElement ("Feature")
        for bundleModule in self.m_bundleModules.values ():
            defaultFeature.appendChild (bundleModule.GetCmpRefXml ())

        mainElem = getFirstElement ("Module", False)
        if self._m_part.IsProduct ():
            mainElem = getFirstElement ("Product")

            defaultFeature = getFirstElement ("Feature")
            for mergeModule in self._m_part.m_installFragment.m_mergeModules.values ():
                defaultFeature.appendChild (mergeModule.ToRefXml ())
                mainWxsDom.documentElement.appendChild (mergeModule.ToXml ())

        def appendChildrenIdElem (attribute):
            childName = (attribute [1]) [3]
            for childValues in attribute [0].values ():
                for childValue in childValues:
                    mainElem.insertBefore (createChildIdElem (childValue, childName), referenceSibling)

        appendChildrenIdElem (self.m_propertyRefs)
        appendChildrenIdElem (self.m_customActionRefs)

        for childValues in self.m_wxsElementRefs [0].values ():
            for childValue in childValues:
                nameValuePair = childValue.split ("=")
                if len (nameValuePair) < 2:
                    continue
                mainElem.insertBefore (createChildIdElem (nameValuePair [1], nameValuePair [0]), referenceSibling)
        #appendChildrenIdElem (self.m_directoryRefs)

        if not self._m_part.IsProduct ():
            for configSubsElem in self._m_part.m_installFragment.m_directoryConfigurationData:
                mainElem.appendChild (configSubsElem)
        else:
            featureElem = mainElem.getElementsByTagName ("Feature") [0]
            for configurableParts in self._m_part.m_installFragment.m_partsWithConfigurableMergeModule:
                featureElem.appendChild (getMergeRefElementFromPart (configurableParts))

        mainWxsFile    = FileWithModificationCheck (wxsFilePath)
        mainWxsFile.WriteXmlNoIndent (mainWxsDom)
        mainWxsFile.Close ()

    #-------------------------------------------------------------------------------------------
    # bsiclass
    #                        BuildInstallSetDescription
    #-------------------------------------------------------------------------------------------
    def GetBuiltPackagePath (self):
        return self.GetBuildPackageFileNameNoExtension () + "." + self.GetPackageExtension ()

    #-------------------------------------------------------------------------------------------
    # bsiclass
    #                        BuildInstallSetDescription
    #-------------------------------------------------------------------------------------------
    def GetPackageExtension(self):
        if self._m_part.m_packageBuildType == PACKAGE_TYPE_NUGET:
            return "nupkg"
        return "msi" if self._m_part.IsProduct () else "msm"

    #-------------------------------------------------------------------------------------------
    # bsiclass
    #                        BuildInstallSetDescription
    #-------------------------------------------------------------------------------------------
    def GetPackageFileName (self):
        if IsLanguagePackBuild ():
            return self.m_productShortName [0] + self._m_part.GetPlatform ().GetXmlName () + "_" + GetCurrentLanguage ().m_culture

        return self.m_productShortName [0] + self._m_part.GetPlatform ().GetXmlName ()

    #-------------------------------------------------------------------------------------------
    # bsiclass
    #                        BuildInstallSetDescription
    #-------------------------------------------------------------------------------------------
    def GetBuildPackageFileNameNoExtension (self):
        return os.path.join( getInstallerBuildDir (self._m_part), self.GetPackageFileName ())

#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
class InstallerProductDeliveryDirectory (object):
    def __init__ (self, thisDirectory, fileList, directoryList):
        self.m_thisDirectory = thisDirectory
        self.m_fileList      = fileList
        self.m_directoryList = directoryList

    #-------------------------------------------------------------------------------------------
    # bsiclass
    #                        InstallerProductDeliveryDirectory
    #-------------------------------------------------------------------------------------------
    @staticmethod
    def IsBlackListedFile (filePath):
        endsWiths = [".msm"]
        containsItems = ["\\cvs\\"]

        for endsWith in endsWiths:
            if filePath.lower ().endswith (endsWith):
                return True

        for contains in containsItems:
            if filePath.lower ().find (contains) > -1:
                return True

        return False

    #-------------------------------------------------------------------------------------------
    # bsiclass
    #                        InstallerProductDeliveryDirectory
    #-------------------------------------------------------------------------------------------
    @classmethod
    def FromProductDirectory (cls, rootDirectory):
        "Initialize InstallerProductDeliveryDirectory from product root directory"

        itemList = os.listdir (rootDirectory)
        files                = []
        childDirectories     = []

        for item in itemList:
            itemFullPath = os.path.join (rootDirectory, item)
            if os.path.isfile (itemFullPath):
                if globalvars.currentAction.IsInExclusionList (itemFullPath):
                    continue
                if InstallerProductDeliveryDirectory.IsBlackListedFile (itemFullPath):
                    continue
                files.append (item)
            else:
                childDirectories.append (InstallerProductDeliveryDirectory.FromProductDirectory (itemFullPath))

        return cls (rootDirectory, files, childDirectories)

    #-------------------------------------------------------------------------------------------
    # bsiclass
    #                        InstallerProductDeliveryDirectory
    #-------------------------------------------------------------------------------------------
    @classmethod
    def FromWxsFile (cls, wxsFilePath, buildInstallSetPart):
        productPartName = buildInstallSetPart.m_info.m_name
        wxsFileDom = minidom.parse (wxsFilePath)
        componentList = wxsFileDom.getElementsByTagName ("Component")
        if componentList == None:
            raise utils.BuildError ("Component cannot be found in " + wxsFilePath)

        directoryRefList = wxsFileDom.getElementsByTagName ("DirectoryRef")
        if directoryRefList == None:
            raise utils.BuildError ("Directory cannot be found in " + wxsFilePath)

        def getChildDirectory (directoryRefNode):
            dirList = []
            dirNodes = directoryRefNode.getElementsByTagName ("Directory")
            if None != dirNodes:
                for dirNode in dirNodes:
                    dirList.append ((dirNode.getAttribute ("Id"), dirNode.getAttribute ("Name")))

            return dirList


        def getDirectoryRefRealId (dirRefElem):
            realId = ""
            for childNode in dirRefElem.childNodes:
                if isinstance (childNode, minidom.Comment):
                    childNodeValue = childNode.nodeValue
                    if childNodeValue.startswith ("REAL_ID:"):
                        realId = childNodeValue.replace ("REAL_ID:", "")

            if len (realId) == 0:
                realId = dirRefElem.getAttribute ("Id")

            return realId

        directoryParentChildMapping = {}
        for directoryRef in directoryRefList:
            dirRefId = getDirectoryRefRealId (directoryRef)
            if (dirRefId in directoryParentChildMapping):
                directoryParentChildMapping [dirRefId].extend (getChildDirectory (directoryRef))
            else:
                directoryParentChildMapping [dirRefId] = getChildDirectory (directoryRef)

        directoryFileList = {}
        def getFilesFromComponentNode (componentNode):
            files = []
            fileNodes = componentNode.getElementsByTagName ("File")
            if fileNodes != None:
                for fileNode in fileNodes:
                    files.append (os.path.basename (fileNode.getAttribute ("Source")))

            return files


        for component in componentList:
            fileList = getFilesFromComponentNode (component)
            directoryId = component.parentNode.getAttribute ("Id")
            if directoryId in directoryFileList:
                directoryFileList [directoryId].extend (fileList)
            else:
                directoryFileList [directoryId] = fileList

        def getFileAndChildDirList (dirTuple, parentName):
            directorTuples = (directoryParentChildMapping [dirTuple [0]]) if dirTuple [0] in directoryParentChildMapping else []
            files = directoryFileList [dirTuple [0]] if dirTuple [0] in directoryFileList else []
            directoryList = []
            for directoryTuple in directorTuples:
                directoryList.append (getFileAndChildDirList (directoryTuple, parentName + "\\" + dirTuple [1]))

            return cls (parentName + "\\" + dirTuple [1], files, directoryList)

        return getFileAndChildDirList (("installdir", productPartName), buildInstallSetPart.GetProductRoot ())

    #-------------------------------------------------------------------------------------------
    # bsiclass
    #                        InstallerProductDeliveryDirectory
    #-------------------------------------------------------------------------------------------
    @staticmethod
    def CompareProductDeliveryDirectory (delivery1, delivery2):

        def getAllFiles (deliveryRoot, filesInDeliveryMap, previousSet):
            for curfile in deliveryRoot.m_fileList:
                path = os.path.normcase (os.path.normpath (os.path.join (deliveryRoot.m_thisDirectory, curfile))).lower ()
                if previousSet != None and path in previousSet:
                    previousSet.pop (path)
                    continue

                if path not in filesInDeliveryMap:
                    filesInDeliveryMap [path] = True

            for directory in deliveryRoot.m_directoryList:
                getAllFiles (directory, filesInDeliveryMap, previousSet)


        filesInDelivery1 = {}
        filesInDelivery2 = {}

        getAllFiles (delivery1, filesInDelivery1, {})
        getAllFiles (delivery2, filesInDelivery2, filesInDelivery1)

        return (list(filesInDelivery1.keys ()), list(filesInDelivery2.keys ()))


#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def getElementById (sourceNode, tagName, elid):
    elements = sourceNode.getElementsByTagName (tagName)
    for elem in elements:
        thisId = elem.getAttribute ("Id")
        if thisId != None and thisId == elid:
            return elem

    return None

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
_configElementStr = "<Configuration DefaultValue=\"INSTALLDIR\" Format=\"Key\"  Type=\"Directory\" ContextData=\"IsolationDir\" Name=\"\"/>"
def ConfigurationElement (dirId):
    configElem = minidom.parseString (_configElementStr)
    configElem.documentElement.setAttribute ("Name", "CONFIGURABLE_" + dirId.upper ())
    return configElem.documentElement

_substitutionElementStr = "<Substitution Column=\"Directory_Parent\" Row=\"\" Table=\"Directory\" Value=\"[=LANG_DIR_RETARGET_TARGETDIR3]\"/>"
def SubstitutionElement (dirId):
    substElem = minidom.parseString (_substitutionElementStr)
    substElem.documentElement.setAttribute ("Row", dirId)
    substElem.documentElement.setAttribute ("Value", "[=" + "CONFIGURABLE_" + dirId.upper () + "]")
    return substElem.documentElement

_configuraionData = "<ConfigurationData Name=\"\" Value=\"\"/>"
def Configurationdata (dirId):
    configElem = minidom.parseString (_configuraionData)
    configElem.documentElement.setAttribute ("Name", "CONFIGURABLE_" + dirId.upper ())
    configElem.documentElement.setAttribute ("Value", dirId)
    return configElem.documentElement

_mergeElementStr =   "<Fragment> <DirectoryRef Id=\"INSTALLDIR\"> <Merge Id=\"\" SourceFile=\"\" Language=\"0\" DiskId=\"1\" /> </DirectoryRef> </Fragment>"
def getMergeElementFromPart (part):
    merge = minidom.parseString (_mergeElementStr)
    fragElem = merge.documentElement
    mergeElem= fragElem.getElementsByTagName ("Merge") [0]
    mergeElem.setAttribute ("Id", getComponentGroupName (part.m_info) + ".msm")
    mergeElem.setAttribute ("SourceFile", part.m_buildInstallSetDescription.GetBuiltPackagePath ())
    uniquenessforDirRef = []
    for confiDataElem in part.m_installFragment.GetAllConfigurationData ():
        mergeElem.appendChild (confiDataElem)
        dirId = confiDataElem.getAttribute ("Value")
        if dirId in uniquenessforDirRef:
            continue

        dirRef = minidom.Document ().createElement ("DirectoryRef")
        dirRef.setAttribute ("Id", dirId)
        fragElem.appendChild (dirRef)
        uniquenessforDirRef.append (dirId)

    return fragElem

def getMergeRefElementFromPart (part):
    merge = minidom.Document ().createElement ("MergeRef")
    merge.setAttribute ("Id", getComponentGroupName (part.m_info) + ".msm")
    return merge

_gIsUpgradeCodeLanguageDependant = False
def SetLanguageDependantUpgradeCode ():
    global _gIsUpgradeCodeLanguageDependant
    if translationkit.IsTranskitShell():
        _gIsUpgradeCodeLanguageDependant = True

def IsValidForCurrentLanguage (languages):
    if not IsLanguagePackBuild ():
        if len (languages) < 1:
            return True
        else:
            if languages.lower () == "core":
                return True
            else:
                return False

    if len (languages) > 0:
        if languages.lower () == "all":
            return True
        langs = [lang.strip () for lang in languages.lower ().split (";")]
        if GetCurrentLanguage ().m_culture.lower () in langs:
            return True
        return False

    return False

def getFirstOrSecondFromSplit (element, IsFirst=True):
    value = element [0]
    if value.find (";") == -1:
        return element

    values = value.split (";")
    if IsFirst:
        return (values [0], element [1])
    else:
        return (values [1], element [1])

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def CleanStringForComment (inStr):
    inStr = inStr.replace ("--", "-")
    return inStr


#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def GlobPath (sourcePath):
    sourcePath = symlinks.normalizePathName  (sourcePath)
    if sourcePath.find ("*") < 0:
        return sourcePath

    splitAtSeparator = sourcePath.split (os.path.sep)
    composePath = splitAtSeparator [0]

    for splitItem in splitAtSeparator [1:]:
        if splitItem.find ("*") > -1:
            tempPath = composePath + os.path.sep + splitItem
            globPaths = glob.glob (tempPath)
            if len (globPaths) == 0 or globPaths == None:
                continue

            selectedPath = globPaths [0]
            prevLength = 0
            for globPath in globPaths [1:]:
                if len (globPath) > prevLength:
                    selectedPath = globPath
                elif globPath > selectedPath:
                    selectedPath = globPath
                else:
                    continue

            composePath = selectedPath
        else:
            composePath = composePath + os.path.sep + splitItem


    return composePath


#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def CheckUNCPath (path):
    path = os.path.join (path, "*")
    files = glob.glob (path)
    if len (files) > 0:
        return True
    return False

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def ExpandLanguageSourcePath (sourcePath, useLanguageNeutral):
    params = {}
    if useLanguageNeutral:
        langName = GetCurrentLanguage ().m_name
        if langName.find ("-"):
            params ["$(BBLanguageName)"] = GetCurrentLanguage ().m_name.split ("-") [0]
    else:
        params ["$(BBLanguageName)"] = GetCurrentLanguage ().m_name

    params ["$(PRGLanguageShortName)"] = GetCurrentLanguage ().m_prgLanguageShortName

    sourcePath = str (sourcePath).lower ()
    splitStr = str (sourcePath).split (";")
    sourcePath = splitStr [0]

    if len (splitStr) > 1:
        for splitItem in splitStr [1:]:
            if splitItem.find ("="):
                name = splitItem.split ("=") [0].strip ()
                if len (name) == 0:
                    continue
                value = os.environ [name] if  name in os.environ else  splitItem.split ("=") [1].strip ()
                params ["$(" + name + ")"] = value

    for name in params.keys ():
        sourcePath = sourcePath.replace (name.lower (), params[name].lower ())

    return GlobPath (sourcePath)

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def ExpandSourcePath (sourcePath):
    foundPath = ExpandLanguageSourcePath (sourcePath, False)
    utils.showInfoMsg ("\nFound {0}, SourcePath={1}.\n".format (foundPath, sourcePath), utils.INFO_LEVEL_Essential, utils.YELLOW)
    if not CheckUNCPath (foundPath):
        utils.showInfoMsg ("\nCannot find {0}, SourcePath={1}.\n".format (foundPath, sourcePath), utils.INFO_LEVEL_Essential, utils.YELLOW)
        foundPath = ExpandLanguageSourcePath (sourcePath, True)
        utils.showInfoMsg ("\nFound {0}, SourcePath={1}.\n".format (foundPath, sourcePath), utils.INFO_LEVEL_Essential, utils.YELLOW)

    return foundPath


#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def CreateIdFromName (name):
    # NEEDSWORK: This needs a better fix. File name need to be predictable and has to committed
    # to LKGOutput while PRG is posting.
    name = name.lower ().replace ("_" + GetCurrentLanguage ().m_name.lower () + "_", "")
    (outStr, _) = re.subn (r'[^a-zA-Z]', r'', name)
    return outStr

_uniqueDirMacro = []
#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def ScanDirectoryForFiles (parentDirName, dirToScan):
    macroList = []

    files = os.listdir (dirToScan)
    for curfile in files:
        fullPath = os.path.join (dirToScan, curfile)
        if os.path.isfile (fullPath):
            name = CreateIdFromName (curfile)
            if len (name) > 0:
                completeName = parentDirName + "_" + name
                if completeName not in _uniqueDirMacro:
                    macroList.append ("{0}={1}".format (completeName, curfile))
                    _uniqueDirMacro.append (completeName)

        elif os.path.isdir (fullPath):
            ScanDirectoryForFiles (parentDirName + "_" + curfile, fullPath)

    return macroList

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def GenerateMacrosForFilesInPath (localPath):
    macroList = []
    if not os.path.isdir (localPath):
        return macroList

    parentFolderName = os.path.basename (localPath)
    macroList.extend (ScanDirectoryForFiles (parentFolderName, localPath))
    return macroList
