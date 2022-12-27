#---------------------------------------------------------------------------------------------
#  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
#  See LICENSE.md in the repository root for full copyright notice.
#---------------------------------------------------------------------------------------------
import os, time, glob, re, threading, shutil, xml
from . import buildpaths, builddescriptionfile, cmdutil, compat, utils, symlinks, globalvars, versionutils, languagesettings, pseudolocalize
from . import internal
#  We should figure out how to split this up if it needs to be used here, like CreateSymLinks.
#   Alternatively we could make it a call (subprocess) 
import XliffToSqlangDb

from operator import itemgetter
from xml.dom import minidom

LOCALIZED_FOLDER_NAME = "Localized"
L10N_REPOSITORY_LISTS_BUILDSTRATEGY = "l10n-buildstrategies:l10n-RepositoryLists"
GIT_L10N_REPO_VERSION_BRANCH_PREFIX = 'version/'

g_vsVersionToVSVarsPath = {"VS2005": None, "VS2008": None, "VS2010": None, "VS2012": None, "VS2013": None, "VS2015": None, "VS2017": None, "VS2019": None, "VS2022": None,
                           "1400": None, "1500": None, "1600": None, "1700": None, "1800": None, "1900": None, "1914": None, "1920": None }
g_VsVarsPathInitialized = False
g_VsVarsPathInitializeLock = threading.Lock()
g_wordExportingToPdfLock = threading.Lock()
g_vsVersionFileCreationLocks = {}

gL10NServerAddress = internal.L10N_SERVER_ADDRESS

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def GetDefaultVSVersion(part):
    defaultVsVersion = globalvars.buildStrategy.GetToolsetForPlatform (part.GetPlatform())
    if defaultVsVersion:
        return defaultVsVersion
    else:
        return "VS2017"

#--------------------------------------------------------------------------------
# @bsimethod
# Transkit repository functions
#--------------------------------------------------------------------------------
def getMissingRemoteRepositoryList (languageSetting):
    if "TRANSKIT_ROOT" not in os.environ or not IsTranskitShell ():
        raise utils.BuildError ("TRANSKIT_ROOT is not defined or his is not a transkit shell")

    repoListFileName = getL10NBuildStrategyFilePath ()
    sourceFolder = symlinks.normalizePathName (os.path.join (os.getenv ("TRANSKIT_ROOT"), "Localized", languageSetting.m_culture))

    if not os.path.exists (sourceFolder):
        raise utils.BuildError ("Translated folder cannot be found {0}".format (sourceFolder))

    repoNames = [globalvars.buildStrategy.m_l10nProduct]

    repoListFile = None
    if not os.path.isfile (repoListFileName):
        repoListFile = minidom.parseString ("""<?xml version="1.0" encoding="utf-8"?>
<BuildStrategy  xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:noNamespaceSchemaLocation="../BentleyBuild/BuildStrategy.xsd"><RemoteRepositoryList Name="l10n-{0}"/></BuildStrategy>""".format (languageSetting.m_culture))
    else:
        repoListFile = utils.parseXml (repoListFileName)

    remoteRepoRoot =  repoListFile.getElementsByTagName ("RemoteRepositoryList") [0]
    localRepoRoot = repoListFile.documentElement

    remoteRepos = set ([remoteNode.getAttribute ("Name") for remoteNode in remoteRepoRoot.getElementsByTagName ("RemoteRepository")])
    localRepos  = set ([localNode.getAttribute ("Name") for localNode in localRepoRoot.getElementsByTagName ("LocalRepository")])

    if len (remoteRepos.symmetric_difference (localRepos)):
        raise utils.BuildError ("RemoteRepositoryList and LocalRepository are not same in numbers")

    returnRepoList = []
    for repoName in repoNames:
        l10nRepoName = getL10NRepoName (repoName, languageSetting)
        if l10nRepoName in remoteRepos:
            continue
        returnRepoList.append ((l10nRepoName, "{0}/l10n/{1}".format (gL10NServerAddress, l10nRepoName), "${SrcRoot}l10n\\" + l10nRepoName))

    return returnRepoList

def createMissingRepositories (missingRepositoryList, _languageSetting, developerDebugging=False):
    if len (missingRepositoryList) < 1:
        return

    repoListFileName = getL10NBuildStrategyFilePath ()
    repoListFile = None
    if not os.path.isfile (repoListFileName):
        repoListFile = minidom.parseString ("""<?xml version="1.0" encoding="utf-8"?><BuildStrategy  xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:noNamespaceSchemaLocation="../BentleyBuild/BuildStrategy.xsd"><RemoteRepositoryList Name="l10n"/></BuildStrategy>""")
    else:
        repoListFile = utils.parseXml (repoListFileName)

    remoteRepoRoot =  repoListFile.getElementsByTagName ("RemoteRepositoryList") [0]
    localRepoRoot = repoListFile.documentElement
    for missinRepo in missingRepositoryList:
        if not RemoteAddressExists (missinRepo [1]):
            if Confirml10nRepoCreation (missinRepo [1]):
                localRepoPath = symlinks.normalizePathName (missinRepo [2])
                symlinks.makeSureDirectoryExists (localRepoPath)
                if not developerDebugging:
                    status = CreateRemoteRepository (missinRepo [0], missinRepo [1])
                    if status != 0:
                        raise utils.BuildError ("Cannot create remote repository {0}".format (missinRepo [1]))
                    status = CloneCreatedRepository (localRepoPath, missinRepo [1])
                    if status != 0:
                        raise utils.BuildError ("Cannot create remote repository {0}".format (missinRepo [1]))
            else:
                continue

        remoteRepo = repoListFile.createElement ("RemoteRepository")
        remoteRepo.setAttribute ("Name", missinRepo [0])
        remoteRepo.setAttribute ("Address", missinRepo [1])
        remoteRepoRoot.appendChild (remoteRepo)

        localRepo = repoListFile.createElement ("LocalRepository")
        localRepo.setAttribute ("Name", missinRepo [0])
        localRepo.setAttribute ("Directory", missinRepo [2])
        localRepo.setAttribute ("Type", "Hg")
        localRepoRoot.appendChild (localRepo)

    utils.xml_removeWhiteSpaceNodes (repoListFile)

    with open (repoListFileName, 'wt') as rlFile:
        repoListFile.writexml (rlFile,  "", "  ", "\n", "UTF-8")

def RemoteAddressExists (address):
    hgPingCmd = ["hg.exe", "pingremote", address]

    utils.showRewritableLine ("Checking {0} existence".format (address), utils.INFO_LEVEL_VeryInteresting)
    remoteAddressExist = []
    def ParseHgAddOutput (outputLine):
        if outputLine.startswith ("pingremote:"):
            if outputLine.find ("repository exists") > -1:
                remoteAddressExist.append (outputLine)

    cmdutil.runAndWait (hgPingCmd, None, ParseHgAddOutput)
    return len (remoteAddressExist) > 0

def CreateRemoteRepository (repoName, repoUrl):
    def processOutput (output):
        if output.startswith ("abort:"):
            utils.exitOnError (1, output)
        elif output.startswith ("error:"):
            utils.exitOnError (1, output)

    cmd = ['hg', 'createremote', '-n', repoName, '-d','l10n copy of the localized items', "{0}/l10n/".format (gL10NServerAddress)]
    utils.showInfoMsg ("creating the new L10N repository {0}\n".format (repoUrl), utils.INFO_LEVEL_SomewhatInteresting)
    utils.showInfoMsg ("{0}\n".format (" ".join(cmd)), utils.INFO_LEVEL_RarelyUseful)
    return cmdutil.runAndWait (cmd, None, processOutput)

def CloneCreatedRepository (localPath, remoteAddress):
    def processOutput (output):
        if output.startswith ("abort:"):
            utils.exitOnError (1, output)
        elif output.startswith ("error:"):
            utils.exitOnError (1, output)

    utils.showInfoMsg ("Cloning the new repository {0}\n".format (remoteAddress), utils.INFO_LEVEL_VeryInteresting)
    return cmdutil.runAndWait (['hg', 'clone', remoteAddress, localPath], None, processOutput)

def Confirml10nRepoCreation (remoteAddress):
    while True:
        ans = compat.getInput("Are you sure you'd like to create " + remoteAddress + "? (y/n): ")
        if ans not in ['y', 'Y', 'n', 'N']:
            utils.showInfoMsg ("Please enter 'y' or 'n': ", utils.INFO_LEVEL_Essential)
            continue
        if ans == 'y' or ans == 'Y':
            return True
        if ans == 'n' or ans == 'N':
            return False


def getL10NBuildStrategyRepoName ():
    return "l10n-buildstrategies"

def getL10NLocalizedDescriptionRepoName ():
    return "l10n-localizeddescriptions"

def getL10NSourceRepoRevisionListFolder (transkitRootPath=None):
    return os.path.join (transkitRootPath if transkitRootPath else os.getenv ("TRANSKIT_ROOT"), "Manifests", "RepoManifest")

def getL10NSourceRepoRevisionListFile ():
    filePath = os.path.join (getL10NSourceRepoRevisionListFolder (), "RepositoryInfo.xml")
    if not os.path.isfile (filePath):
        raise utils.BuildError ("Cannot find L10N Manifest File {0}".format (filePath))
    return filePath

_transkitVersion = None
def getTranskitVersion ():
    global _transkitVersion
    if not _transkitVersion:
        teamRevListFileName = getL10NSourceRepoRevisionListFile()
        teamRevList = builddescriptionfile.BuildDescription (teamRevListFileName)
        teamRevList.ReadFromFile ()
        versionTuple = teamRevList.GetProductVersion()
        _transkitVersion = versionutils.Version.InitFromList([versionTuple[0], versionTuple[1], versionTuple[2], versionTuple[3]])
    return _transkitVersion

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def GetL10nRepoVersionBranchName (version):
    return GIT_L10N_REPO_VERSION_BRANCH_PREFIX + version.StringForFilename()

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def getL10nProductVersionBranchName (localRepo, currentVersion):
    allRefs = localRepo.GetRemoteBranches()
    versions = []
    for ref in allRefs:
        if ref.startswith(GIT_L10N_REPO_VERSION_BRANCH_PREFIX):
            versions.append(versionutils.Version(ref[len(GIT_L10N_REPO_VERSION_BRANCH_PREFIX):]))
    possibleVersions = []
    for version in versions:
        if version <= currentVersion:
            possibleVersions.append(version)
    if len(possibleVersions) == 0:
        return None

    possibleVersions.sort(reverse=True)
    return GetL10nRepoVersionBranchName (possibleVersions[0])

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def IsL10nRepoOnBuildVersionBranch (localRepo, version):
    # l10n repos are supposed to be only Git so should be safe to get the branch
    return localRepo.GetCurrentBranch() == GetL10nRepoVersionBranchName (version)

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def CreateAndCheckoutVersionBranch (localRepo, version):
    localRepo.CreateAndCheckoutBranch (GetL10nRepoVersionBranchName (version))

def getL10NLocalizedDescriptionFile (productName, version, languageSetting, allowSearch=True):
    l10nDescriptionDir = globalvars.buildStrategy.FindLocalRepository (getL10NLocalizedDescriptionRepoName ()).GetLocalDir()
    fileNameFormat =  "{0}_{1}_{2}.BuildStrategy.xml"

    def GetL10NDescFilePath (versionStr):
        return os.path.join (l10nDescriptionDir, fileNameFormat.format (productName, versionStr, languageSetting.m_culture))

    ldfFilePath = GetL10NDescFilePath (version)
    if os.path.isfile (ldfFilePath):
        return (ldfFilePath, True)

    if not allowSearch:
        return (ldfFilePath, False)

    ldfFilePath = GetL10NDescFilePath ("*")
    utils.showInfoMsg ("Searching for {0}\r\n".format (ldfFilePath), utils.INFO_LEVEL_Essential, utils.YELLOW)
    files = glob.glob (ldfFilePath)
    if len (files) > 0:
        currentVersion = versionutils.Version(version)
        versions = []
        for ldfFile in files:
            productVersionString = ldfFile.split('_')[-2]
            versions.append(versionutils.Version(productVersionString))
        possibleVersions = []
        for version in versions:
            if version <= currentVersion:
                possibleVersions.append(version)
        if len(possibleVersions) == 0:
            return (ldfFilePath, False)

        possibleVersions.sort(reverse=True)
        ldfFilePath = GetL10NDescFilePath(possibleVersions[0].StringForFilename())
        utils.showInfoMsg ("Using Description file {0}\r\n".format (ldfFilePath), utils.INFO_LEVEL_Essential, utils.YELLOW)
        return (ldfFilePath, True)

    return (ldfFilePath, False)

def getL10NBuildStrategyFilePath ():
    if IsTranskitShell():
        l10nStrategyFile = os.path.join (os.getenv ("TRANSKIT_ROOT"),"Manifests", "l10n", "l10n-buildstrategies", "l10n-RepositoryLists.BuildStrategy.xml")
        if os.path.isfile (l10nStrategyFile):
            return l10nStrategyFile
    else:
        if globalvars.buildStrategy:
            return os.path.join (globalvars.buildStrategy.FindLocalRepository ("l10n-buildstrategies").GetLocalDir(), "l10n-RepositoryLists.BuildStrategy.xml")
        else:
            filePath = os.path.join (os.path.expandvars("${SrcRoot}\\l10n\\l10n-buildstrategies\\l10n-RepositoryLists.BuildStrategy.xml"))
            if os.path.exists (filePath):
                return filePath

    return None

#--------------------------------------------------------------------------------
# @bsimethod
# Transkit Repo Names
#--------------------------------------------------------------------------------
def getL10NRepoName (repoName, languageSetting):
    return "l10n-{0}-{1}".format (repoName, languageSetting.m_culture)

#def getL10NRepoListName (languageSetting):
#    return "l10n-{0}".format (languageSetting.m_culture)

#--------------------------------------------------------------------------------
# @bsimethod
# Transkit ZIP folder functions
#--------------------------------------------------------------------------------
TRANSKIT_ZIP_FILE_NAME  = "Transkit.zip"

def TkTempDir (platform):
    # Using shorter name
    return os.path.join (symlinks.normalizePathName (buildpaths.getOutputRoot (platform, False)), "Build", "Tk")

def TranskitZipBuildDir (platform, subFolderName):
    return os.path.join (symlinks.normalizePathName (TkTempDir (platform)), subFolderName, "Transkit")

def TranskitZipTempPath (platform, subFoldername):
    return os.path.join (symlinks.normalizePathName (TkTempDir (platform)), subFoldername, "zip")

def TranskitTempFile (platform, reason=""):
    tmpFileName = utils.createSeededGuid (reason)
    tkTmpFilePath = os.path.join (TkTempDir (platform), tmpFileName)

    symlinks.makeSureBaseDirectoryExists (tkTmpFilePath)
    if not os.path.exists (tkTmpFilePath):
        tkTempFile = open (tkTmpFilePath, 'wt')
        tkTempFile.write ("THIS IS A TEMP FILE TO BE A PLACE HOLDER IN TRANSKIT ENVIRONMENT\r\n{0}".format (reason))
        tkTempFile.close ()

    return tkTmpFilePath

#--------------------------------------------------------------------------------
# @bsimethod
# Translated Resources folder function
#--------------------------------------------------------------------------------
def getLocalizedResourceRootDirectory ():
    if IsTranskitShell ():
        return symlinks.normalizePathName (os.path.join (os.getenv ("TRANSKIT_ROOT", ""), LOCALIZED_FOLDER_NAME))

    return os.path.join (symlinks.normalizePathName (buildpaths.getOutputRoot (globalvars.programOptions.platform, False)), "Build", LOCALIZED_FOLDER_NAME)

def getTKBindingRelativePath (partInfo, tkBinding):
    destPath = os.path.join (partInfo.m_buildContext, partInfo.m_name)
    if len (tkBinding.m_baseName) > 0 :
        return os.path.join (destPath, tkBinding.m_baseName)

    return destPath

def getLocalizedResourceRepoRoot (localRepo, languageSetting):
    return os.path.join (getLocalizedResourceRootDirectory (), languageSetting.m_culture, localRepo.m_name)

def getLocalizedResourceDirectory (part, tkBinding, languageSetting, childFileInFolder=None):
    # How a localized resource is picked.
    # On a Developer shell
    # If Neutral -> Always use from BuildContext.
    # If TranskitBuild is using Repo and Directory exist, then use it from there.
    # if TranskitBuild is using Repo and Directory do not exist, then
    #       Check if it is martian, then return Pseudo-localized directory.
    #       Otherwise, return English file path.
    if languageSetting.IsNeutral () and not IsTranskitShell ():
        return os.path.join (part.GetMyBuildContextDir (), tkBinding.m_sourceDir)

    if IsTranskitBuildUsingRepo ():
        pathInRepo = getRepoPathForBinding (part, tkBinding, languageSetting, childFileInFolder)
        if pathInRepo:
            return pathInRepo
        elif IsTranskitShell ():
            englishZippedFolder = symlinks.normalizePathName (os.path.join (os.getenv ("TRANSKIT_ROOT"), "Localized", "en", part.m_info.m_repo.m_name, getTKBindingRelativePath (part.m_info, tkBinding)))
            utils.ShowAndDeferMessage ("Using english for {0}\n".format (part), utils.INFO_LEVEL_VeryInteresting, utils.YELLOW)
            return englishZippedFolder
        else: # This is a developer shell. Hence, use English folder path in BuildContext
            return os.path.join (part.GetMyBuildContextDir (), tkBinding.m_sourceDir)

    # Now build is invoked with instruction not to use repo path.
    if IsTranskitShell ():
        return symlinks.normalizePathName (os.path.join (os.getenv ("TRANSKIT_ROOT"), "Localized", languageSetting.m_culture, part.m_info.m_repo.m_name, getTKBindingRelativePath (part.m_info, tkBinding)))
    else: # This is developer shel and developer have told to use not-repository
        if languageSetting.IsPseudoLocalized ():
            return os.path.join (getLocalizedResourceRepoRoot (part.m_info.m_repo, languageSetting), getTKBindingRelativePath (part.m_info, tkBinding))
        else:
            raise utils.PartBuildError ("Use repository for languages other than mr", part)

def getRepoPathForBinding (part, tkBinding, languageSetting, childFileInFolder=None):
    pathInRepo      = None
    l10nRepo = globalvars.buildStrategy.FindLocalL10NRepository (globalvars.buildStrategy.m_l10nProduct, languageSetting)
    if not l10nRepo:
        return None

    pathInRepo = os.path.join (l10nRepo.GetLocalDir(), part.m_info.m_repo.m_name, getTKBindingRelativePath (part.m_info, tkBinding))
    # Needs to be fixed.
    # if not os.path.isdir (pathInRepo):
        # utils.ShowAndDeferMessage ("Cannot find path {0}\n".format (pathInRepo), utils.INFO_LEVEL_VeryInteresting)
        # return None

    if not childFileInFolder:
        return pathInRepo

    if  childFileInFolder.find ("*") > -1:
        fileList = glob.glob (os.path.join (pathInRepo, childFileInFolder))
        if len (fileList) > 0:
            return pathInRepo
        else:
            utils.ShowAndDeferMessage ("Cannot find file {0}\\{1}\n".format (pathInRepo, childFileInFolder), utils.INFO_LEVEL_VeryInteresting)
            return None
    else:
        fileFullPath = os.path.join (pathInRepo, childFileInFolder)
        if os.path.isfile (fileFullPath):
            return pathInRepo
        else:
            utils.ShowAndDeferMessage ("Cannot find file {0}\\{1}\n".format (pathInRepo, childFileInFolder), utils.INFO_LEVEL_VeryInteresting)
            return None

#--------------------------------------------------------------------------------
# @bsimethod
#--------------------------------------------------------------------------------
_gReadTranskitOutFilesOnly = True
def ReadOutFilesOnly ():
    # For installer builds, we do not care about existence of localizable resources.
    return _gReadTranskitOutFilesOnly

def SetReadOutFilesOnly (readOutFilesOnly=True):
    global _gReadTranskitOutFilesOnly
    _gReadTranskitOutFilesOnly = readOutFilesOnly

#--------------------------------------------------------------------------------
# @bsimethod
#--------------------------------------------------------------------------------
_gCreateTempMissingTranskitSources = False
def CreateTempMissingTranskitSources ():
    return _gCreateTempMissingTranskitSources

def SetCreateTempMissingTranskitSources (createTmpSource = False):
    global _gCreateTempMissingTranskitSources
    _gCreateTempMissingTranskitSources = createTmpSource

#--------------------------------------------------------------------------------
# @bsimethod
#--------------------------------------------------------------------------------
def InitializeVsVarsPathFromRegistry ():
    global g_VsVarsPathInitialized

    with g_VsVarsPathInitializeLock:
        if g_VsVarsPathInitialized:
            return

        g_vsVersionToVSVarsPath ["VS2022"] = cmdutil.getVSVarsFromRegistryDir (["17.0"])
        g_vsVersionToVSVarsPath ["VS2019"] = cmdutil.getVSVarsFromRegistryDir (["16.0"])
        g_vsVersionToVSVarsPath ["VS2017"] = cmdutil.getVSVarsFromRegistryDir (["15.0"])
        g_vsVersionToVSVarsPath ["VS2015"] = cmdutil.getVSVarsFromRegistryDir (["14.0"])
        g_vsVersionToVSVarsPath ["VS2013"] = cmdutil.getVSVarsFromRegistryDir (["12.0"])
        g_vsVersionToVSVarsPath ["VS2012"] = cmdutil.getVSVarsFromRegistryDir (["11.0"])
        g_vsVersionToVSVarsPath ["VS2010"] = cmdutil.getVSVarsFromRegistryDir (["10.0"])
        g_vsVersionToVSVarsPath ["VS2008"] = cmdutil.getVSVarsFromRegistryDir (["9.0"])
        g_vsVersionToVSVarsPath ["VS2005"] = cmdutil.getVSVarsFromRegistryDir (["8.0"])

        g_vsVersionToVSVarsPath ["1920"] =  g_vsVersionToVSVarsPath ["VS2019"]
        g_vsVersionToVSVarsPath ["1914"] =  g_vsVersionToVSVarsPath ["VS2017"]
        g_vsVersionToVSVarsPath ["1900"] =  g_vsVersionToVSVarsPath ["VS2015"]
        g_vsVersionToVSVarsPath ["1800"] =  g_vsVersionToVSVarsPath ["VS2013"]
        g_vsVersionToVSVarsPath ["1700"] =  g_vsVersionToVSVarsPath ["VS2012"]
        g_vsVersionToVSVarsPath ["1600"] =  g_vsVersionToVSVarsPath ["VS2010"]
        g_vsVersionToVSVarsPath ["1500"] =  g_vsVersionToVSVarsPath ["VS2008"]
        g_vsVersionToVSVarsPath ["1400"] =  g_vsVersionToVSVarsPath ["VS2005"]

        g_VsVarsPathInitialized = True

        for vsVersion in g_vsVersionToVSVarsPath:
            g_vsVersionFileCreationLocks [vsVersion] = threading.Lock ()

def TranskitToolsDir (platform, fromBentleyBuild=True):
    transkitRelativePath = os.path.join ("Transkit", "Tools")
    if fromBentleyBuild:
        if IsTKBuildOnly () and "TRANSKIT_ROOT" in os.environ:
            return os.path.join (symlinks.normalizePathName (os.getenv ("TRANSKIT_ROOT")), "Tools")
        return symlinks.normalizePathName (os.path.join (buildpaths.getOutputRoot (platform, False), transkitRelativePath))

    outputNonStaticRoot = symlinks.normalizePathName ("${OutputRootDir}").lower ().rstrip ("\\")
    if outputNonStaticRoot.endswith ('\\static'):
        outputNonStaticRoot = outputNonStaticRoot[:-6]
    return symlinks.normalizePathName (os.path.join (outputNonStaticRoot, transkitRelativePath))


def TranskitBuildDir (platform):
    if platform == None:
        platform = globalvars.programOptions.platform

    transkitRelativePath = os.path.join ("Build", "LocalizedBuilds")
    return os.path.join (buildpaths.getOutputRoot (platform, False), transkitRelativePath)

def TranskitBuildDirLanguage (languageSetting, platform, isStatic):
    staticString = 'static' if isStatic else ''
    return os.path.join (TranskitBuildDir (platform), staticString, languageSetting.m_culture)

def getRcompPath ():
    raise utils.BuildError ("RCOMP Path is not specified")

def getCommandLineOptions ():
    langSpecFile = languagesettings.getLanguageSpecificationsFile ()
    dom = utils.parseXml (langSpecFile)

    return "[" + ", ".join ([langNode.getAttribute ("Name").lower () for langNode in dom.getElementsByTagName ("LanguageSpecification")]) +  "]"
    
def getLanguagesFromCSV (languages, useDefault=True):
    langs = []
    if useDefault:
        langs.append (u'en')

    languages = languages.strip()

    langsTemp = []
    if "" != languages:
        langsTemp = langsTemp + [language.strip().lower() for language in languages.split (",")]

    for lang in langsTemp:
        if lang not in langs:
            langs.append (lang)
    return langs

_gLanguageSettings = []
def isLanguageSettingsInitialized ():
    return len(_gLanguageSettings) > 0

def setLanguagesCSV (languagesList, forceClearAndUpdate=False):
    if not languagesList:
        return True

    global _gLanguageSettings
    if forceClearAndUpdate:
        _gLanguageSettings = []

    if isLanguageSettingsInitialized():
        return True

    langSpecFile = languagesettings.getLanguageSpecificationsFile ()
    dom = utils.parseXml (langSpecFile)
    languagesList = languagesList.lower ().strip ("\"")
    # Use en language in list, if this is not a transkit shell.
    useNeutralLang = not IsTranskitShell ()

    # Do not use en langauge, if language is set to clear and load only provided languages.
    if forceClearAndUpdate:
        useNeutralLang = False

    langaugesOrdered = getLanguagesFromCSV (languagesList, useNeutralLang)
    languages = set (langaugesOrdered)
    if languages.issubset (set ([langNode.getAttribute ("Name").lower () for langNode in dom.getElementsByTagName ("LanguageSpecification")])):
        addedLanguages = [langSetting.m_culture.lower () for langSetting in _gLanguageSettings]
        for language in langaugesOrdered:
            if language not in addedLanguages:
                _gLanguageSettings.append (languagesettings.LanguageSettings (language))
        return True
    else:
        return False

def languagesToCompileFor ():
    if len (_gLanguageSettings) > 0:
        return _gLanguageSettings
    return [languagesettings.LanguageSettings ("en")]

def getLanguageSettings ():
    return languagesToCompileFor ()

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
_gIsTranskitMultiLingual = False
def SetTranskitMultilingual ():
    global _gIsTranskitMultiLingual
    if IsTranskitShell():
        _gIsTranskitMultiLingual = True

def IsTranskitMultilingualBuild ():
    if not IsTranskitShell ():
        return False

    if len (getLanguageSettings ()) <= 1:
        return False

    return _gIsTranskitMultiLingual

def IsMultilingualBuild ():
    if len (getLanguageSettings ()) <= 1:
        return False

    if IsTranskitShell ():
        return False

    return True

def getLanguageSettingsWithoutNeutral ():
    languageSettings = []
    getLanguageSettings ()
    for languageSetting in getLanguageSettings ():
        if not languageSetting.IsNeutral ():
            languageSettings.append (languageSetting)

    return languageSettings

def getMartianLanguageSetting ():
    return languagesettings.LanguageSettings ("mr")

def getFirstNonEnglishLanguageSetting ():
    getLanguageSettings ()
    if len (getLanguageSettings ()) > 0:
        for languageSetting in getLanguageSettings ():
            if not languageSetting.IsNeutral ():
                return languageSetting

    return None

def getTKBindingBuildContextRelativePath (_partInfo, tkBinding):
    destPath = os.path.join (tkBinding.m_sourceDir)

    if len (tkBinding.m_baseName) > 0 and False:
        return os.path.join (destPath, tkBinding.m_baseName)
    return destPath

def getStringValueTrueFalse (value, defaultValue=True):
    if not value or len (value) == 0:
        return defaultValue

    trueValues  = ["true", "1"]
    falseValues = ["false", "0"]

    if value.lower() in trueValues:
        return True
    if value.lower() in falseValues:
        return False
    return defaultValue

#--------------------------------------------------------------------------------
# @bsimethod
#--------------------------------------------------------------------------------
def IsClobberBuild ():
    if hasattr (globalvars.currentAction.m_actionOptions, "clobberBuild"):
        return globalvars.currentAction.m_actionOptions.clobberBuild
    else:
        return False

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def IsDebug ():
    for key in os.environ:
        if key.upper() == "DEBUG":
            return True
    return False

#--------------------------------------------------------------------------------
# @bsimethod
# Function to find Transkit build environment
#--------------------------------------------------------------------------------
_gIsTranskitShell           = False
_gCheckedForTranskitShell   = False
def IsTranskitShell ():
    global _gCheckedForTranskitShell, _gIsTranskitShell
    if not _gCheckedForTranskitShell:
        for key in os.environ:
            if key.upper() == "TRANSKIT_SHELL":
                _gIsTranskitShell = True
                break
        _gCheckedForTranskitShell = True

    return _gIsTranskitShell

def IsDeveloperShell ():
    return IsTranskitShell () == False

#--------------------------------------------------------------------------------
# @bsimethod
#--------------------------------------------------------------------------------
_gUseRepositoryForLocalizedResources = IsTranskitShell ()
def SetUseL10NRepos (useRepos=True):
    global _gUseRepositoryForLocalizedResources
    _gUseRepositoryForLocalizedResources = useRepos

def IsTranskitBuildUsingRepo ():
    return _gUseRepositoryForLocalizedResources

#--------------------------------------------------------------------------------
# @bsimethod
#--------------------------------------------------------------------------------
gTkOnlyBuild    = False
def IsTKBuildOnly ():
    if IsTranskitShell ():
        return True

    elif "TRANSKIT_ONLY_BUILD" in os.environ:
        return True

    return gTkOnlyBuild
#--------------------------------------------------------------------------------
# @bsimethod
#--------------------------------------------------------------------------------
def IsL10nRepoCheck (crossStreamRepos):
    for (localRepo, _, _) in crossStreamRepos:
        lowerName = localRepo.m_name.lower()
        if "l10n-" in lowerName:
            continue
        else:
            return False
    return True


#--------------------------------------------------------------------------------
# @bsimethod
#--------------------------------------------------------------------------------
def IsLanguageBuild ():
    if len (getLanguageSettingsWithoutNeutral ()) > 0:
        return True
    return False

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def GetVersion (matchingDllName):
    # Try to get version from environment (PRG)
    relV = majV = minV = subminV = -1

    if 'REL_V' in os.environ:
        relV = os.environ['REL_V']
    if 'MAJ_V' in os.environ:
        majV = os.environ['MAJ_V']
    if 'MIN_V' in os.environ:
        minV = os.environ['MIN_V']
    if 'SUBMIN_V' in os.environ:
        subminV = os.environ['SUBMIN_V']

    if relV != -1 and majV != -1 and minV != -1 and subminV != -1:
        return (relV, majV, minV, subminV)

    # Next try to use the version from the DLL
    if matchingDllName:
        listverExe = utils.GetBsiToolsFile ("listver.exe")

        resultLines = []
        def processOutput (outputLine):
            resultLines.append (outputLine)

        cmd = [listverExe, matchingDllName]

        verRe = re.compile (r"\s*([0-9]+)\.([0-9]+)\.([0-9]+)\.([0-9]+) .*")

        stat = cmdutil.runAndWait (cmd, None, processOutput)
        if 0 == stat:
            for line in resultLines:
                matchGrp = verRe.match (line)
                if matchGrp:
                    relV = matchGrp.group(1)
                    majV = matchGrp.group(2)
                    minV = matchGrp.group(3)
                    subminV = matchGrp.group(4)
                    return (relV, majV, minV, subminV)


    # Finally, dump in a bogus number.
    return globalvars.defaultVersion

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def GetVersionMkiArg (outputFileName, matchingSourceDll, productName):
    # This doesn't play nice with policy files.
    isDebug = IsDebug ()

    (relV, majV, minV, subminV) =  GetVersion (matchingSourceDll)

    baseFileName = os.path.split (outputFileName)[1]

    cmdFileName = os.path.join (os.path.dirname (outputFileName), outputFileName+".version")
    cmdFile = open (cmdFileName, "wt")

    cmdFile.write ('-n{0}.{1}.{2}.{3}\n'.format (relV, majV, minV, subminV))
    cmdFile.write ('-k"InternalName={0}"\n'.format (baseFileName))
    cmdFile.write ('-k"FileDescription={0} for Windows"\n'.format (baseFileName))
    cmdFile.write ('-k"OriginalFilename={0}"\n'.format (baseFileName))
    cmdFile.write ('-k"CompanyName=Bentley Systems, Incorporated"\n')
    cmdFile.write ('-k"LegalCopyright=Copyright \xA9 \x25Y Bentley Systems, Incorporated. All rights reserved."\n')

    if productName:
        cmdFile.write ('-k"ProductName={0}"\n'.format (productName))

    if isDebug:
        cmdFile.write ('-fVS_FF_DEBUG=1\n')

    if not utils.isPrg():
        cmdFile.write ('-fVS_FF_PRERELEASE=1\n')
        cmdFile.write ('-fVS_FF_PRIVATEBUILD=1\n')
        cmdFile.write ('-k"PrivateBuild=Built by {0} on {1} FOR INTERNAL USE ONLY"\n'.format (os.environ['USERNAME'], os.environ['COMPUTERNAME']))

    cmdFile.flush ()
    cmdFile.close ()
    return cmdFileName

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def PutFilesToTranskitTools (binding, part):
    sourceDir = part.GetMyBuildContextDir ()

    transkitToolsDir = TranskitToolsDir (part.GetPlatform ())
    symlinks.makeSureDirectoryExists (transkitToolsDir)

    sourceNameList = part.SplitBindings (binding.m_val)
    for sourceName in sourceNameList:
        sourceName = os.path.normpath (os.path.join (sourceDir, sourceName))
        sourceFiles = glob.glob (os.path.expandvars (sourceName))

        for sourceFile in sourceFiles:
            fileName = os.path.basename (sourceFile)
            targetFile =  os.path.join (transkitToolsDir, fileName)
            if os.path.isdir (sourceFile):
                symlinks.createDirSymLink (targetFile, sourceFile, True, True, True)
            else:
                symlinks.createFileSymLink (targetFile, sourceFile, True, True, True)

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def deleteTranskitFile (fileName):
    if os.path.exists (fileName):
        utils.showInfoMsg ("Deleting file {0}\r\n".format(fileName), utils.INFO_LEVEL_VeryInteresting)
        os.remove (fileName)

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def deleteTranskitFolder (folderName):
    if os.path.exists (folderName):
        utils.showInfoMsg ("Deleting folder {0}\r\n".format(folderName), utils.INFO_LEVEL_VeryInteresting)
        shutil.rmtree (folderName, True)

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def getIncludePaths (includePaths):
    allPaths = []
    for path in includePaths:
        if None != path:
            allPaths.append ("-i{0}".format (path))
    return allPaths


#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
class ResourceBase (object): 
    def __init__ (self, builtFileName, tkFilePath, languageSetting, part, pseudoLocalizeCommand):
        if True != os.access (tkFilePath, os.R_OK):
            raise utils.PartBuildError ("Transkit file {0} does not exist".format(tkFilePath), part)

        if not languageSetting:
            languageSetting = languagesettings.LanguageSettings ("en")

        self.m_incPaths         = []
        self.m_languageSetting  = languageSetting
        self.m_supportingFileList = []
        self.m_logFile          = None

        self.m_builtFileName    = builtFileName.replace ("$(culture)", self.m_languageSetting.m_culture).replace ("$(language)", self.m_languageSetting.m_name)
        self.m_builtFileName    = self.m_builtFileName.replace ("$(threeLetterWindowsName)", self.m_languageSetting.m_threeLetterWindowsName)
        self.m_tkFilePath       = tkFilePath

        self.m_part = part
        self.m_tkOutFilePath    = None
        self.m_readTkSource     = False
        self.m_excludeLanguages = []
        self.m_includeLanguages = []
        self.m_useLanguageSubFolder = True
        self.m_isDir = False
        self.m_isExcludeBuildLocalizedDir = False
        self.m_pseudoLocalizeCommand = pseudoLocalizeCommand
        self.m_useCustomPseudolocalization = pseudoLocalizeCommand != None and len(pseudoLocalizeCommand) > 0

        self.__UpdateExcludeIncludeLanguages ()

    def SetLogFile (self, logFile):
        self.m_logFile = logFile

    def IsDir (self):
        return self.m_isDir

    def TranskitXmlDir (self):
        return os.path.dirname (self.m_tkFilePath)

    def LanguageFolderName (self):
        return self.m_languageSetting.m_culture if self.m_useLanguageSubFolder else "."

    def __UpdateExcludeIncludeLanguages (self):
        currentDom = self.GetTkDom ()  # pylint: disable=assignment-from-none

        if currentDom == None:
            return

        self.m_includeLanguages = self.m_includeLanguages + getLanguagesFromCSV (currentDom.getAttribute ("IncludeLanguages"), False)
        self.m_excludeLanguages = self.m_excludeLanguages + getLanguagesFromCSV (currentDom.getAttribute ("ExcludeLanguages"), False)
        self.m_useLanguageSubFolder = currentDom.getAttribute ("UseLanguageSubFolder").lower () != "false"
        self.m_isExcludeBuildLocalizedDir = currentDom.getAttribute ("BuildForLocalizedDir").lower () == "false"

    def IsTrueForBuild (self):
        if self.m_isExcludeBuildLocalizedDir and CreateTempMissingTranskitSources ():
            return False

        if self.m_languageSetting.m_culture.lower() in self.m_excludeLanguages:
            return False

        # If Include languages are provided, then current language has to be one of the include language. 
        if len (self.m_includeLanguages) > 0:
            if self.m_languageSetting.m_culture.lower() in self.m_includeLanguages:
                return True
            else:
                return False

        return True

    def GetTranslatableResources (self):
        if not  self.m_readTkSource:
            raise utils.PartBuildError ("ReadSourceFiles needs to be called", self.m_part)

        raise utils.PartBuildError ("Resource class needs to implement GetTkTranslatableResources", self.m_part)

    def GetNonTranslatableResources (self):
        if not  self.m_readTkSource:
            raise utils.PartBuildError ("ReadSourceFiles needs to be called", self.m_part)
        raise utils.PartBuildError ("Resource class needs to implement GetNonTranslatableResources", self.m_part)

    def GetExtraBuildContextItems (self):
        return []

    def BeforeCompile (self, obj):
        pass
        #NEEDSWORK: Update this function for language compiles

    def GetTkDom (self):
        return None

    def RemoveBuiltFile (self, tkBinding):
        self.ReadSourceFiles (tkBinding)
        deleteTranskitFile (self.BuiltFilePath ())

    def Compile (self, tkBinding): # pylint: disable=unused-argument
        raise utils.PartBuildError ("Build function needs to be implemented", self.m_part)

    def ReadSourceFiles (self, tkBinding): # pylint: disable=unused-argument
        self.ReadSupportingFiles ()
        self.m_readTkSource = True

    def ReadSupportingFiles (self):
        tkDom = self.GetTkDom ()   # pylint: disable=assignment-from-none

        if not tkDom:
            return

        supportingDoms = tkDom.getElementsByTagName ("SupportingResources")
        if len (supportingDoms) > 0:
            supportingFiles = utils.getDomElementValue (supportingDoms [0].parentNode, "SupportingResources")
            supportingFilesSplit = []
            if supportingFiles.find (";") > -1:
                supportingFilesSplit = supportingFiles.split (";")
            else:
                supportingFilesSplit = [supportingFiles]
            for supportingFile in supportingFilesSplit:
                if supportingFile.find (".dll") > -1 and 'BentleyBuild_TranskitDontSearchSupportingDlls' not in os.environ:
                    self.FindSupportingDLL(supportingFile)
                else:
                    if supportingFile.find ("*") > -1:
                        self.m_supportingFileList.extend(glob.glob (os.path.join (os.path.dirname (self.m_tkFilePath), supportingFile)))
                    else:
                        self.m_supportingFileList.append(os.path.join (os.path.dirname (self.m_tkFilePath), supportingFile))

    def FindSupportingDLL(self, supportingFile):
        transkitbuildContext = os.path.join(os.path.expandvars("${SrcRoot}TranskitBuildContext"), self.m_part.GetPlatform().GetDirName()) + "\\"
        supportingFile = supportingFile.replace("$(TranskitBuildContext)", transkitbuildContext)
        if os.path.isfile (supportingFile):
            self.m_supportingFileList.append(supportingFile)
            return

    def SubPathForProduct (self):
        if not self.m_readTkSource:
            raise utils.PartBuildError ("Call ReadSourceFiles function first", self.m_part)

        return os.path.join (self.LanguageFolderName (), os.path.basename (self.m_tkOutFilePath))

    def BuiltFilePath (self):
        return self.m_tkOutFilePath

    def _CheckIfAlreadyAdded (self, filePath):
        allResources = self.GetTranslatableResources () + self.GetNonTranslatableResources ()
        for rfile in allResources:
            if utils.IsSameFile (filePath, rfile):
                return True

        return False

    def AddIncPath (self, includePath):
        if not os.path.exists (includePath):
            return

        if includePath in self.m_incPaths:
            return

        self.m_incPaths.append (includePath)
        for fileItem in os.listdir (includePath):
            fullPath = os.path.join (includePath, fileItem)
            if os.path.isdir (fullPath):
                self.AddIncPath (fullPath)
            else:
                if self._CheckIfAlreadyAdded (fullPath):
                    continue
                self.m_supportingFileList.append (fullPath)

    def CustomPseudolocalization (self, sourceFile, destFile, destinationDir):
        pseudoLocalizeFinalCommand = self.m_pseudoLocalizeCommand.replace ("[SourceFile]", sourceFile)
        pseudoLocalizeFinalCommand = pseudoLocalizeFinalCommand.replace ("[OutputFile]", destFile).replace ("${TranskitTools}", TranskitToolsDir (self.m_part.GetPlatform ()))
        utils.showAndLogMsg ("Pseudolocalization command {0}\r\n".format(pseudoLocalizeFinalCommand), self.m_logFile, utils.INFO_LEVEL_VeryInteresting)
        stat = cmdutil.runAndWait (pseudoLocalizeFinalCommand, destinationDir)
        if stat != 0:
            raise utils.PartBuildError ("Failed to build localized file {0} \r\n Failed for command {1}".format (self.m_tkOutFilePath, pseudoLocalizeFinalCommand), self.m_part)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def PseudoLocalize (self, tkBinding, pseudolocalizeOptions):
        martianizerToolDir = os.path.join (TranskitToolsDir (self.m_part.GetPlatform ()), "martiantool")
        if not os.path.exists (martianizerToolDir):
            martianizerToolDir = os.path.join (symlinks.normalizePathName (buildpaths.getOutputRoot (self.m_part.GetPlatform (), False)), "BuildContexts" ,"TranskitTools", "Delivery", "martiantool")

        self.ReadSourceFiles (tkBinding)
        for transFile in self.GetTranslatableResources ():
            sourceFile = symlinks.normalizePathName (transFile)
            destinationDir = getLocalizedResourceDirectory (self.m_part, tkBinding, getMartianLanguageSetting ())
            sourceDir = os.path.dirname (sourceFile).lower ().rstrip ('\\/')
            relativePath = sourceDir.replace (self.TranskitXmlDir ().lower (), "").strip ('\\/')

            if len (relativePath) > 0 and not sourceDir == relativePath:
                destinationDir = os.path.join (destinationDir, relativePath)

            symlinks.makeSureDirectoryExists (destinationDir)
            if os.path.isdir (sourceFile):
                destFile = os.path.join (destinationDir, os.path.basename (sourceFile))
                symlinks.makeSureDirectoryExists (destFile)
                if self.m_useCustomPseudolocalization:
                    self.CustomPseudolocalization(sourceFile, destFile, destinationDir)
                else:
                    pseudolocalize.processDirectory (sourceFile, destFile, pseudolocalizeOptions, martianizerToolDir=martianizerToolDir)
                return

            sourceFiles = []
            if sourceFile.find ("*"):
                sourceFiles = glob.glob (sourceFile)
            else:
                sourceFiles.append (sourceFile)

            for sfile in sourceFiles:
                symlinks.makeSureBaseDirectoryExists (sfile)
                destFile = os.path.join (destinationDir, os.path.basename (sfile))
                symlinks.makeSureBaseDirectoryExists (destFile)
                if self.m_useCustomPseudolocalization:
                    self.CustomPseudolocalization(sfile, destFile, destinationDir)
                else:
                    pseudolocalize.processFile (sfile, destFile, pseudolocalizeOptions, martianizerToolDir=martianizerToolDir)

#-------------------------------------------------------------------------------------------
# bsiclass
# http://msdn.microsoft.com/en-us/library/8fkteez0.aspx MFC GuideLine
#-------------------------------------------------------------------------------------------
class WindowsRCDelivery(ResourceBase):
    def __init__(self, winRCDom, tkFilePath, languageSetting, part):
        self.m_rcDom       = winRCDom
        dllName = winRCDom.getAttribute ("ResourceDLLBase")
        relatedAssemblyExtension = ".dll"

        exeName = winRCDom.getAttribute ("ResourceExeBase")
        relatedAssemblyExtension = ".exe"

        name = dllName if dllName and len (dllName) > 0 else exeName
        relatedAssemblyExtension = ".dll" if dllName and len (dllName) > 0 else ".exe"

        if "" == name:
            raise utils.PartBuildError ("No ResourceDLLBase or ResourceExeBase given for WindowsRc", part)

        rclangDll = name + languageSetting.m_threeLetterWindowsName +".dll"

        self.m_relatedAssemblyFileName = name + relatedAssemblyExtension

        rcFiles =  utils.getDomElementsByName (winRCDom, "ResourceSource")
        for rcFile in rcFiles:
            resourceFileName = rcFile.getAttribute ("Name")
            if "" == resourceFileName:
                raise utils.PartBuildError ("No Name for WindowsRc Transkit", part)

        ResourceBase.__init__(self, rclangDll, tkFilePath, languageSetting, part, None)
        # Default use if language subfolder for WindowsRC is false.
        if 'BBPW' in os.environ: # DMS speciality code until they update
            useLanguageSubFolder = self.GetTkDom ().getAttribute ("UseLanguageSubFolder").lower ()
            self.m_useLanguageSubFolder = useLanguageSubFolder == "true" or len(useLanguageSubFolder) == 0
        else:
            self.m_useLanguageSubFolder = self.GetTkDom ().getAttribute ("UseLanguageSubFolder").lower () == "true"

    #-------------------------------------------------------------------------------------------
    # bsiclass
    #-------------------------------------------------------------------------------------------
    def ReadSourceFiles (self, tkBinding):

        self.m_tkOutFilePath    = os.path.join (TranskitBuildDirLanguage (self.m_languageSetting, self.m_part.GetPlatform (), self.m_part.m_info.m_static), getTKBindingRelativePath (self.m_part.m_info, tkBinding), self.m_builtFileName)
        self.m_tkOutFilePath = utils.resolveBindingMacros (self.m_tkOutFilePath, self.m_part.GetPlatform (), self.m_part, forceUseApiNumber=True)
        if ReadOutFilesOnly ():
            return

        self.m_relatedAssemblyFileName = utils.resolveBindingMacros (self.m_relatedAssemblyFileName, self.m_part.GetPlatform (), self.m_part, forceUseApiNumber=True)

        self.m_resFiles = []
        self.m_incPaths.append (self.TranskitXmlDir ())

        rcFiles =  utils.getDomElementsByName (self.m_rcDom, "ResourceSource")
        for rcFile in rcFiles:
            resourceFileName = utils.resolveBindingMacros (rcFile.getAttribute ("Name"), self.m_part.GetPlatform (), self.m_part, forceUseApiNumber=True) 
            fileName, _ = os.path.splitext(resourceFileName)
            resname = "{0}{1}.res".format (fileName, self.m_languageSetting.m_culture)
            self.m_resFiles.append ((resname, os.path.join (getLocalizedResourceDirectory (self.m_part, tkBinding, self.m_languageSetting, childFileInFolder=resourceFileName), resourceFileName)))

        self.m_relatedAssemblyFilePath = self._FindRelatedAssembly (tkBinding)
        if self.m_relatedAssemblyFilePath:
            self.m_relatedAssemblyFilePath = utils.resolveBindingMacros (self.m_relatedAssemblyFilePath, self.m_part.GetPlatform (), self.m_part, forceUseApiNumber=True)

        if 'BBPW' in os.environ: # DMS speciality code until they update
            self.AddIncPath (os.path.join (self.m_part.GetMyBuildContextDir (), tkBinding.m_includeDir))
        self.AddIncPath (os.path.join (os.path.dirname (self.TranskitXmlDir ()), "include"))
        self.AddIncPath (os.path.join (os.path.dirname (self.TranskitXmlDir ()), "inc"))

        ResourceBase.ReadSourceFiles (self, tkBinding)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetTranslatableResources(self):
        return [m_resFiles [1] for m_resFiles in self.m_resFiles]

    def GetNonTranslatableResources(self):
        return [self.m_tkFilePath] + self.m_supportingFileList

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def _FindRelatedAssembly (self, tkbinding):
        bindings = utils.splitSources(tkbinding.m_parentBinding.m_val)
        for binding in bindings:
            sourceFiles = glob.glob (os.path.join (symlinks.normalizePathName (self.m_part.GetMyBuildContextDir ()), binding))
            for sourceFile in sourceFiles:
                if self.m_relatedAssemblyFileName.lower () == os.path.basename (sourceFile).lower ():
                    return sourceFile

        msg =  "Base assembly with name {0} cannot be found for transkit {1}".format (self.m_relatedAssemblyFileName, self.m_tkFilePath)
        msg =  msg + "\nSearching in {0}\n".format (self.m_part.GetMyBuildContextDir ())
        utils.showAndLogMsg (msg, self.m_logFile, utils.INFO_LEVEL_VeryInteresting)

        def SearchFileInFolder (fileName, folderPath):
            folderPath = symlinks.normalizePathName (folderPath)
            for fileItem in os.listdir (folderPath):
                itemPath = os.path.join (folderPath, fileItem)
                if os.path.isfile (itemPath):
                    if os.path.basename (itemPath).lower () == fileName.lower ():
                        return itemPath
                elif os.path.isdir (itemPath):
                    return SearchFileInFolder (fileName, itemPath)

            return None

        searchedFile = SearchFileInFolder (self.m_relatedAssemblyFileName, self.m_part.GetMyBuildContextDir ())
        if searchedFile:
            return searchedFile

        return None

   #-------------------------------------------------------------------------------------------
   # bsimethod
   #-------------------------------------------------------------------------------------------
    def Compile(self, tkBinding):
        SetReadOutFilesOnly (False)
        self.ReadSourceFiles (tkBinding)

        if not utils.IsSourceModified ([self.m_tkOutFilePath], self.GetTranslatableResources () + self.GetNonTranslatableResources ()):
            return

        buildDir = os.path.dirname (self.m_tkOutFilePath)

        symlinks.makeSureDirectoryExists (buildDir)

        includes = []
        for filepath in self.m_supportingFileList:
            self.AddIncPath (os.path.dirname (filepath))

        for incPath  in self.m_incPaths:
            includes = includes + ["-I" + incPath]

        platform = self.m_part.GetPlatform ()
        if platform.m_option == "x86":
            machineOption = "-d_X86_"
        else:
            machineOption = "-d_X64_"

        vsVersion = GetDefaultVSVersion(self.m_part)
        _, vsEnvironment = getVsEnvironmentAndCheckOverrides (None, self.m_logFile, self.m_part) # since we no longer use the version from the transkit file, pass in None for it not to complain about it not being defined
        #runAndLog doesn't seem to work with passed-in env unless you path it out.
        rcExe = getVSExePath ("rc.exe", vsVersion, vsEnvironment)
        for resfiles in self.m_resFiles:
            outResFile = os.path.join(buildDir,resfiles[0])
            cmd = [rcExe, machineOption, '-dwinNT', '-fo{0}'.format(outResFile)]
            cmd.extend (includes)
            cmd.append (resfiles[1])

            utils.showAndLogMsg (" RC Compiling  {0}\r\n".format(resfiles[1]), self.m_logFile, utils.INFO_LEVEL_VeryInteresting)
            utils.showAndLogMsg ("{0}\r\n".format(cmd), self.m_logFile, utils.INFO_LEVEL_Interesting)

            stat = cmdutil.runAndLogWithEnv (cmd, vsEnvironment, self.m_logFile, None)
            if 0 != stat:
                raise utils.PartBuildError ("Error compiling {0}".format (os.path.split(resfiles[1])[0]), self.m_part)

            self._LinkResources (buildDir, vsVersion, vsEnvironment, self.m_part)
            self._InjectVersion (self.m_relatedAssemblyFilePath, buildDir, os.path.splitext(self.m_relatedAssemblyFileName)[0])

            signtoolExe = getVSExePath ("signtool.exe", vsVersion, vsEnvironment)
            signstat = cmdutil.signFileCommand(self.m_tkOutFilePath,signtoolExe, self.m_logFile)
            if 0 !=signstat:
                raise utils.PartBuildError ("Error signing {0}".format (self.m_tkOutFilePath), self.m_part)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def _LinkResources (self, buildDir, vsVersion, vsEnvironment, part):

        if os.path.exists(self.m_tkOutFilePath):
            return

        for resFile in self.m_resFiles:
            resFileName = resFile[0]

            resFilePath = os.path.join(buildDir, resFileName)

            linkExe = getVSExePath ("link.exe", vsVersion, vsEnvironment)
            stdOptions = "-WX -Release -incremental:no -fixed:no -dll -noentry".split()
            debugOption = ""

            platform = part.GetPlatform ()
            if platform.m_option == "x86":
                machineOption = "-machine:x86"
            else:
                machineOption = "-machine:x64"

            pdbFileFileLoc = ""
            filesToLink = []
            
            filesToLink.append(resFilePath)
            cmd = [linkExe]
            if (debugOption): cmd.append(debugOption)
            cmd.extend(stdOptions)
            cmd.append(machineOption)
            if pdbFileFileLoc: cmd.append(pdbFileFileLoc)
            cmd.append ('-out:'+self.m_tkOutFilePath)
            cmd.extend (filesToLink)

            utils.showAndLogMsg ("Linking ResourceDLL {0}\r\n".format(self.m_tkOutFilePath), self.m_logFile, utils.INFO_LEVEL_VeryInteresting)
            utils.showAndLogMsg ("{0}\r\n".format(cmd), self.m_logFile, utils.INFO_LEVEL_Interesting)

            stat = cmdutil.runAndLogWithEnv (cmd, vsEnvironment, self.m_logFile, buildDir)
            if 0 != stat:
                raise utils.PartBuildError ("Error linking {0}".format (self.m_tkOutFilePath), self.m_part)
            utils.showAndLogMsg ("\r\n", self.m_logFile, utils.INFO_LEVEL_VeryInteresting)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def _InjectVersion (self, relatedAssembly, buildDir, productName):
        versionCmdFile = GetVersionMkiArg (self.m_tkOutFilePath, relatedAssembly, productName)

        injectVersionExe = utils.GetBsiToolsFile ("injectversion.exe")

        cmd  = [injectVersionExe, "@" + versionCmdFile, self.m_tkOutFilePath]
        utils.showAndLogMsg ("Versioning Assembly {0}\r\n".format(self.m_tkOutFilePath), self.m_logFile, utils.INFO_LEVEL_VeryInteresting)
        stat = cmdutil.runAndLog (cmd, self.m_logFile, buildDir)
        if 0 != stat:
            raise utils.PartBuildError ("Error versioning {0}".format (buildDir), self.m_part)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetTkDom(self):
        return self.m_rcDom

#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
class BentleyResourceFile (ResourceBase):
    def __init__(self, bentleyRscDom, tkFilePath, languageSetting, part):
        self.m_rscDom       = bentleyRscDom
        ResourceBase.__init__(self, bentleyRscDom.getAttribute ("Name"), tkFilePath, languageSetting, part, None)

        deliveryEnglish = bentleyRscDom.getAttribute ("DeliverEnglish")
        if deliveryEnglish.lower () == "false":
            self.m_excludeLanguages.append ("en")

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetTkDom(self):
        return self.m_rscDom

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def ReadSourceFiles (self, tkBinding):
        self.m_builtFileName = utils.resolveBindingMacros (self.m_builtFileName, self.m_part.GetPlatform (), self.m_part, forceUseApiNumber=True)
        self.m_tkOutFilePath    = os.path.join (TranskitBuildDirLanguage (self.m_languageSetting, self.m_part.GetPlatform (), self.m_part.m_info.m_static), getTKBindingRelativePath (self.m_part.m_info, tkBinding), self.m_builtFileName)
        self.m_tkOutFilePath = utils.resolveBindingMacros (self.m_tkOutFilePath, self.m_part.GetPlatform (), self.m_part, forceUseApiNumber=True)
        if ReadOutFilesOnly ():
            return

        self.m_xliffPath = os.path.join (getLocalizedResourceDirectory (self.m_part, tkBinding, self.m_languageSetting, childFileInFolder=self.m_builtFileName + ".xliff"), self.m_builtFileName + ".xliff")
        self.m_xliffPath = utils.resolveBindingMacros (self.m_xliffPath, self.m_part.GetPlatform (), self.m_part, forceUseApiNumber=True)

        self.m_inputMui  = os.path.join (self.m_part.GetMyBuildContextDir (), tkBinding.m_sourceDir, self.m_builtFileName)
        self.m_inputMui = utils.resolveBindingMacros (self.m_inputMui, self.m_part.GetPlatform (), self.m_part, forceUseApiNumber=True)

        ResourceBase.ReadSourceFiles (self, tkBinding)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetTranslatableResources(self):
        return [self.m_xliffPath]

    def GetNonTranslatableResources(self):
        return [self.m_tkFilePath, self.m_inputMui] + self.m_supportingFileList

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def Compile (self, tkBinding):
        SetReadOutFilesOnly (False)
        self.ReadSourceFiles (tkBinding)

        if not utils.IsSourceModified ([self.m_tkOutFilePath], self.GetTranslatableResources () + self.GetNonTranslatableResources ()):
            return

        self.BeforeCompile (self)

        outputMui   = self.BuiltFilePath ()

        symlinks.makeSureBaseDirectoryExists (outputMui)
        
        generateLangPackPath = os.path.join (TranskitToolsDir (self.m_part.GetPlatform ()), "GenerateLangPack.exe")
        if not os.path.exists (generateLangPackPath):
            raise utils.PartBuildError ("{0} was not found and is required for native MUI compilation.".format (generateLangPackPath), self.m_part)

        cmd = [generateLangPackPath, self.m_inputMui, self.m_xliffPath, outputMui]
        utils.showAndLogMsg ("{0}\r\n".format(" ".join (cmd)), self.m_logFile, utils.INFO_LEVEL_VeryInteresting)

        stat = cmdutil.runAndLog (cmd, self.m_logFile, None)
        if 0 != stat:
            raise utils.PartBuildError ("Error creating localized MUI file \"{0}\"".format (outputMui), self.m_part)
        utils.showAndLogMsg ("\r\n", self.m_logFile, utils.INFO_LEVEL_VeryInteresting)

#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
class SupplementalSchemaDelivery(ResourceBase):
    def __init__(self, schemaResourceDom, tkFilePath, languageSetting, part):
        self.schemaResourceDom = schemaResourceDom
        ResourceBase.__init__(self, schemaResourceDom.getAttribute ("Name"), tkFilePath, languageSetting, part, None)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetTkDom(self):
        return self.schemaResourceDom

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def Compile (self, tkBinding):
        SetReadOutFilesOnly (False)
        self.ReadSourceFiles (tkBinding)

        if not utils.IsSourceModified ([self.BuiltFilePath ()], self.GetTranslatableResources () + self.GetNonTranslatableResources ()):
            return

        schemaResXGenerator = utils.GetBsiToolsFile ("SchemaResXGenerator.exe")
        symlinks.makeSureBaseDirectoryExists (self.BuiltFilePath ())

        inputfile = ["-i", self.m_translatedFile]
        output  = ["-o", os.path.dirname (self.BuiltFilePath ())]
        culture = ["-c", self.m_languageSetting.m_culture.lower ()]
        cmd = [schemaResXGenerator] + inputfile + output + culture
        utils.showAndLogMsg ("{0}\r\n".format(" ".join (cmd)), self.m_logFile, utils.INFO_LEVEL_VeryInteresting)

        stat = cmdutil.runAndLog (cmd, self.m_logFile, None)
        if 0 != stat:
            raise utils.PartBuildError ("Error creating localized supplemental schema file or files \"{0}\"".format (self.m_builtFileName), self.m_part)
        if not os.path.isfile (self.BuiltFilePath ()):
            raise utils.PartBuildError ("Failed to create {0}".format (self.BuiltFilePath ()), self.m_part)

        utils.showAndLogMsg ("\r\n", self.m_logFile, utils.INFO_LEVEL_VeryInteresting)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def ReadSourceFiles (self, tkBinding):
        #Any culture with a hyphen(zh-CN) is converted to one with underscore(zh_CN) when
        #appended as part of a supplemental schema file name,so updating the m_builtFileName below.
        if "-" in self.m_languageSetting.m_culture:
            cultrWithHyphen = self.m_languageSetting.m_culture
            cultrWithUnderscore = cultrWithHyphen.replace("-","_")
            self.m_builtFileName = self.m_builtFileName.replace(cultrWithHyphen,cultrWithUnderscore)

        self.m_tkOutFilePath    = os.path.join (TranskitBuildDirLanguage (self.m_languageSetting, self.m_part.GetPlatform (), self.m_part.m_info.m_static), getTKBindingRelativePath (self.m_part.m_info, tkBinding), self.m_builtFileName)
        if ReadOutFilesOnly ():
            return

        resxName     = utils.resolveBindingMacros (utils.getDomElementValue (self.schemaResourceDom, "ResourceSource"), self.m_part.GetPlatform (), self.m_part, forceUseApiNumber=True)
        self.m_translatedFile = None
        sourcePath = symlinks.normalizePathName (getLocalizedResourceDirectory (self.m_part, tkBinding, self.m_languageSetting, childFileInFolder=resxName))
        if resxName.find ("*") > -1:
            resxFiles = glob.glob (os.path.join (sourcePath, resxName))
            if len (resxFiles) == 0:
                raise utils.PartBuildError ("No RESX file found at {0}".format (sourcePath), self.m_part)
            self.m_translatedFile = resxFiles [0]
        else:
            self.m_translatedFile = os.path.join (sourcePath, resxName)

        ResourceBase.ReadSourceFiles (self, tkBinding)
    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetTranslatableResources(self):
        return [self.m_translatedFile]

    def GetNonTranslatableResources(self):
        return [self.m_tkFilePath] + self.m_supportingFileList

#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
class BentleyRibbonResourceFile (BentleyResourceFile):
    def __init__(self, bentleyRscDom, tkFilePath, languageSetting, part):
        self.m_bentleyRscDom = bentleyRscDom
        BentleyResourceFile.__init__(self, bentleyRscDom, tkFilePath, languageSetting, part)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetTkDom(self):
        return self.m_bentleyRscDom

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def Compile(self, tkBinding):
        SetReadOutFilesOnly (False)

        self.ReadSourceFiles (tkBinding)
        outputMui       = self.BuiltFilePath ()

        if not utils.IsSourceModified([outputMui], [self.m_xliffPath, self.m_tkFilePath]):
            return

        symlinks.makeSureBaseDirectoryExists (outputMui)
        utils.showAndLogMsg ("XliffToSqlangDb.ImportXliffIntoDb ({0}, {1})\r\n".format  (self.m_xliffPath, outputMui), self.m_logFile, utils.INFO_LEVEL_VeryInteresting)
        XliffToSqlangDb.ImportXliffIntoDb (self.m_xliffPath, outputMui)

#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
class DgnlibResourceFile (BentleyResourceFile):
    def __init__(self, bentleyDgnlibDom, tkFilePath, languageSetting, part):
        self.m_bentleyDgnlibDom = bentleyDgnlibDom
        BentleyResourceFile.__init__(self, bentleyDgnlibDom, tkFilePath, languageSetting, part)
        #ResourceBase.__init__(self, bentleyDgnlibDom.getAttribute ("Name"), tkFilePath, languageSetting)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetTkDom(self):
        return self.m_bentleyDgnlibDom

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def Compile(self, tkBinding):
        SetReadOutFilesOnly (False)

        self.ReadSourceFiles (tkBinding)
        outputMui       = self.BuiltFilePath () #This incorrectly returns D:\Builds\Lithium\out\Winx64\Build\LocalizedBuilds\en\MstnAppData\LocalizableDgnlibs-DrawComp\DrawingSeed.dgnlib instead of DrawingSeed.dgnlib.mui

        if not utils.IsSourceModified([outputMui], [self.m_xliffPath, self.m_tkFilePath]):
            return

        symlinks.makeSureBaseDirectoryExists (outputMui)
        utils.showAndLogMsg ("XliffToSqlangDb.ImportXliffIntoDb ({0}, {1})\r\n".format  (self.m_xliffPath, outputMui), self.m_logFile, utils.INFO_LEVEL_VeryInteresting)
        XliffToSqlangDb.ImportXliffIntoDb (self.m_xliffPath, outputMui)


#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
class TranslatedDelivery (ResourceBase):
    def __init__(self, tkDeliveryDom, tkFilePath, languageSetting, part, fileName=None):
        # Set up a little weird because RawResxDelivery only has a filename to pass in, no dom.
        if tkDeliveryDom:
            self.m_tkDeliveryDom = tkDeliveryDom
            self.m_l10nRepository = False if str (tkDeliveryDom.getAttribute ("L10NRepository")).lower () == "false" else True
            ResourceBase.__init__(self, tkDeliveryDom.getAttribute ("Name"), tkFilePath, languageSetting, part, None)
        else:
            self.m_tkDeliveryDom = None
            ResourceBase.__init__(self, fileName, tkFilePath, languageSetting, part, None)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetTkDom(self):
        return self.m_tkDeliveryDom

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def ReadSourceFiles (self, tkBinding):
        self.m_builtFileName = utils.resolveBindingMacros (self.m_builtFileName, self.m_part.GetPlatform (), self.m_part, forceUseApiNumber=True)

        self.m_tkOutFilePath = os.path.join (TranskitBuildDirLanguage (self.m_languageSetting, self.m_part.GetPlatform(), self.m_part.m_info.m_static), getTKBindingRelativePath (self.m_part.m_info, tkBinding), self.m_builtFileName)
        self.m_tkOutFilePath = utils.resolveBindingMacros (self.m_tkOutFilePath, self.m_part.GetPlatform (), self.m_part, forceUseApiNumber=True)

        if ReadOutFilesOnly ():
            return

        if not self.m_l10nRepository:
            self.m_translatedFile = os.path.join (os.path.dirname (self.m_tkFilePath), self.m_builtFileName)
        else:
            childInFolder = None if self.m_isDir else self.m_builtFileName
            self.m_translatedFile = os.path.join (getLocalizedResourceDirectory (self.m_part, tkBinding, self.m_languageSetting, childFileInFolder=childInFolder), self.m_builtFileName)
        ResourceBase.ReadSourceFiles (self, tkBinding)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetTranslatableResources(self):
        if self.IsTrueForBuild ():
            return [self.m_translatedFile]
        return []

    def GetNonTranslatableResources(self):
        return [self.m_tkFilePath] + self.m_supportingFileList

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def Compile (self, tkBinding):
        SetReadOutFilesOnly (False)
        self.ReadSourceFiles (tkBinding)

        if not os.path.exists (self.m_translatedFile) and CreateTempMissingTranskitSources () and not self.m_languageSetting.IsNeutral ():
            msg = "Using temp, file is not found {0}.".format (self.m_translatedFile)
            self.m_translatedFile = TranskitTempFile (self.m_part.GetPlatform (), msg)
            msg = msg + "TempFilePath={0}\r\n".format (self.m_translatedFile)
            utils.showInfoMsg (msg, utils.INFO_LEVEL_VeryInteresting)


        if not utils.IsSourceModified ([self.m_tkOutFilePath], [self.m_translatedFile, self.m_tkFilePath]):
            return

        utils.showAndLogMsg ("Copy ({0}, {1})\r\n".format  (self.m_translatedFile,  self.m_tkOutFilePath), self.m_logFile, utils.INFO_LEVEL_VeryInteresting)
        symlinks.makeSureBaseDirectoryExists (self.m_tkOutFilePath)
        shutil.copy (self.m_translatedFile, self.m_tkOutFilePath)
        if not os.path.exists(self.m_tkOutFilePath):
            utils.copyFileWithRetries (self.m_translatedFile, self.m_tkOutFilePath)

#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
class TranslatedDirDelivery (TranslatedDelivery):
    def __init__(self, tkDeliveryDom, tkFilePath, languageSetting, part):
        TranslatedDelivery.__init__(self, tkDeliveryDom, tkFilePath, languageSetting, part)
        self.m_isDir = True

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def Compile (self, tkBinding):
        SetReadOutFilesOnly (False)
        self.ReadSourceFiles (tkBinding)

        if not os.path.exists (self.m_translatedFile) and CreateTempMissingTranskitSources ():
            msg = "Using temp, file is not found {0}.".format (self.m_translatedFile)
            self.m_translatedFile = TranskitTempFile (self.m_part.GetPlatform (), msg)
            msg = msg + "TempFilePath={0}\r\n".format (self.m_translatedFile)
            utils.showInfoMsg (msg, utils.INFO_LEVEL_VeryInteresting, utils.YELLOW)

        utils.showAndLogMsg ("symlinks.createFileSymLink ({0}, {1})\r\n".format  (self.m_tkOutFilePath, self.m_translatedFile), self.m_logFile, utils.INFO_LEVEL_VeryInteresting)
        symlinks.createDirSymLink (self.m_tkOutFilePath, self.m_translatedFile)

#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
class RawResxDelivery (TranslatedDelivery):
    def __init__(self, fileName, tkFilePath, languageSetting, part, outputDirectory, rawResxDom):
        self.m_outputDirectory = outputDirectory
        self.m_rawResxDom = rawResxDom
        self.m_l10nRepository = True
        TranslatedDelivery.__init__(self, None, tkFilePath, languageSetting, part, os.path.basename (fileName))

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def SubPathForProduct(self):
        if not self.m_readTkSource:
            raise utils.PartBuildError ("Call ReadSourceFiles function first", self.m_part)

        return os.path.join (self.LanguageFolderName (), self.m_outputDirectory, os.path.basename (self.m_tkOutFilePath))

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetTkDom(self):
        return self.m_rawResxDom

#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
class WordToPdf(ResourceBase):
    def __init__(self, wordToPdfDom, tkFilePath, languageSetting, part):
        if os.name != 'nt':
            raise utils.BuildError ("Unsupported OS: Word to PDF is currently only supported on Windows")
        self.m_wordToPdfDom = wordToPdfDom
        rscSrc = utils.getDomElementsByName (wordToPdfDom, "ResourceSource")[0]

        replaceStringDoms = utils.getDomElementsByName (wordToPdfDom, "ReplaceText")
        self.m_replaceTexts = []
        for dom in replaceStringDoms:
            token = dom.getAttribute ("Token")
            value = dom.firstChild.nodeValue
            self.m_replaceTexts.append((token, value))

        self.m_inputFile = None
        self.m_fileName = rscSrc.firstChild.nodeValue

        ResourceBase.__init__(self, os.path.splitext(self.m_fileName)[0] + ".pdf", tkFilePath, languageSetting, part, None)
        self.m_useCustomPseudolocalization = True

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetTkDom(self):
        return self.m_wordToPdfDom

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def Compile (self, tkBinding):
        SetReadOutFilesOnly (False)
        self.ReadSourceFiles (tkBinding)

        if not os.path.exists(os.path.dirname(self.m_tkOutFilePath)):
            os.makedirs(os.path.dirname(self.m_tkOutFilePath))

        self._SaveWordToPdf()

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def _SaveWordToPdf (self):
        import pythoncom
        from win32com import client

        with g_wordExportingToPdfLock:
            pythoncom.CoInitialize()  # pylint: disable=no-member
            word = client.Dispatch('Word.Application')
            doc = word.Documents.Open(self.m_inputFile)
            doc.AcceptAllRevisions()
            if doc.Comments.Count:
                doc.DeleteAllComments()

            for replacement_pair in self.m_replaceTexts:
                word.Selection.Find.Execute(replacement_pair[0], True, True, False, False, False, True, 1, False, os.path.expandvars(replacement_pair[1]), 2)

            doc.SaveAs2(self.m_tkOutFilePath, FileFormat=17) #File format to save a word document as a pdf
            doc.Close(False) # False closes without saving
            word.Quit()
            pythoncom.CoUninitialize() # pylint: disable=no-member

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def ReadSourceFiles(self, tkBinding):
        self.m_tkOutFilePath = symlinks.normalizePathName (os.path.join (TranskitBuildDirLanguage (self.m_languageSetting, self.m_part.GetPlatform (), self.m_part.m_info.m_static), getTKBindingRelativePath (self.m_part.m_info, tkBinding), self.m_builtFileName))
        self.m_tkOutFilePath = utils.resolveBindingMacros (self.m_tkOutFilePath, self.m_part.GetPlatform (), self.m_part, forceUseApiNumber=True)
        self.m_inputFile = os.path.join (getLocalizedResourceDirectory (self.m_part, tkBinding, self.m_languageSetting, childFileInFolder=self.m_fileName), self.m_fileName)
        ResourceBase.ReadSourceFiles(self, tkBinding)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetTranslatableResources(self):
        return [self.m_inputFile]

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetNonTranslatableResources(self):
        return [self.m_tkFilePath] + self.m_supportingFileList

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def CustomPseudolocalization (self, sourceFile, destFile, destinationDir):
        utils.showInfoMsg ("WordToPdf Pseudolocalization only copies files. Removing destination file if already exists and copying over the original one.\n", utils.INFO_LEVEL_SomewhatInteresting)
        if os.path.exists(destFile):
            os.remove(destFile)
        stat = utils.copyFile(sourceFile, destFile)
        if stat != 0:
            raise utils.PartBuildError ("Failed to build localized file {0} \r\n Failed to copy {1} to {2}".format (self.m_tkOutFilePath, sourceFile, destFile), self.m_part)

#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
class MSBuildLocalizationProject(ResourceBase):
    def __init__(self, msbuildProjectRCDom, tkFilePath, languageSetting, part):
        if os.name != 'nt':
            raise utils.BuildError ("Unsupported OS: MSBuildProject localization is currently only supported on Windows")
        self.m_msbuildProjectRCDom = msbuildProjectRCDom
        self.m_projectFolder = utils.getDomElementsByName (msbuildProjectRCDom, "ProjectFolder")[0].firstChild.nodeValue
        self.m_projectPath = utils.getDomElementsByName (msbuildProjectRCDom, "ProjectPath")[0].firstChild.nodeValue
        configFileElems = utils.getDomElementsByName (msbuildProjectRCDom, "ConfigFile")

        self.m_configFile = None if len(configFileElems) < 1 else configFileElems[0].firstChild.nodeValue

        self.m_languageSpecificProjectFolder = None

        outputFileName = msbuildProjectRCDom.getAttribute("OutputFileName")
        ResourceBase.__init__(self, outputFileName, tkFilePath, languageSetting, part, None)
        self.m_useCustomPseudolocalization = True

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetTkDom(self):
        return self.m_msbuildProjectRCDom

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def Compile (self, tkBinding):
        SetReadOutFilesOnly (False)
        self.ReadSourceFiles (tkBinding)

        if not os.path.exists(os.path.dirname(self.m_tkOutFilePath)):
            os.makedirs(os.path.dirname(self.m_tkOutFilePath))

        self._CompileMSBuildProject()

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def _CompileMSBuildProject (self):
        fullProjectPath = os.path.join(self.m_languageSpecificProjectFolder, self.m_projectPath)
        projectBuildDir = os.path.join(os.path.dirname(self.m_tkOutFilePath), "bin")+os.sep
        configFilePath = os.path.join(self.TranskitXmlDir(), self.m_configFile) if self.m_configFile else None

        buildCmd = ["bmake"]
        buildCmd.append ('+dMSBUILD_PROJECT_PATH={}'.format(fullProjectPath))
        buildCmd.append ('+dMSBUILD_PROPERTIES_PATH={}'.format(os.path.join(os.path.dirname(self.m_tkOutFilePath), "build.properties")))
        buildCmd.append ('+dDEFAULT_TARGET_PROCESSOR_ARCHITECTURE={}'.format(self.m_part.GetPlatform().GetArchitecture()))
        buildCmd.append ('+dMSBUILD_EARLY_EVALUATE_PROPERTIES=1')
        buildCmd.append (os.path.abspath(utils.GetSharedMkiFile("projbuild.mke")))

        localEnv = globalvars.buildStrategy.GetEnv()
        utils.appendEnvVar (localEnv, "BuildToolCache", buildpaths.GetToolsOutputRoot() + os.sep)
        utils.appendEnvVar (localEnv, "OutputRootDir", buildpaths.getOutputRoot(self.m_part.GetPlatform(), self.m_part.IsStatic())+os.sep)
        utils.appendEnvVar (localEnv, "PartBuildDir", projectBuildDir)
        if globalvars.buildStrategy.GetToolsetForPlatform (self.m_part.GetPlatform()):
            utils.appendEnvVar (localEnv, "BB_DEFAULT_TOOLSET", globalvars.buildStrategy.GetToolsetForPlatform (self.m_part.GetPlatform()))
        if globalvars.buildStrategy.GetToolVersionForPlatform (self.m_part.GetPlatform()):
            utils.appendEnvVar (localEnv, "TOOL_VERSION", globalvars.buildStrategy.GetToolVersionForPlatform (self.m_part.GetPlatform()))
        if configFilePath:
            utils.appendEnvVar (localEnv, "BB_DEFAULT_CONFIGURATION_FILE", configFilePath)
        utils.appendEnvVar (localEnv, "TRANSKIT_TOOLS_DIR", TranskitToolsDir (self.m_part.GetPlatform())+os.sep)
        utils.appendEnvVar (localEnv, "COMPILED_LOCALIZED_FILE_PATH", self.m_tkOutFilePath)
        utils.appendEnvVar (localEnv, "TRANSKIT_XML_DIR", self.TranskitXmlDir()+os.sep)
        utils.appendEnvVar (localEnv, "BB_TRANSKIT_COMPILE_LANGUAGE", self.m_languageSetting.m_culture.lower())

        versionutils.pushPartStrategyVersionToEnv (localEnv, self.m_part)

        cmdutil.runAndLogWithEnv (buildCmd, localEnv, self.m_logFile, None)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def ReadSourceFiles(self, tkBinding):
        self.m_tkOutFilePath = symlinks.normalizePathName (os.path.join (TranskitBuildDirLanguage (self.m_languageSetting, self.m_part.GetPlatform (), self.m_part.m_info.m_static), getTKBindingRelativePath (self.m_part.m_info, tkBinding), self.m_builtFileName))
        self.m_tkOutFilePath = utils.resolveBindingMacros (self.m_tkOutFilePath, self.m_part.GetPlatform (), self.m_part, forceUseApiNumber=True)
        self.m_languageSpecificProjectFolder = os.path.join (getLocalizedResourceDirectory (self.m_part, tkBinding, self.m_languageSetting), self.m_projectFolder)
        ResourceBase.ReadSourceFiles(self, tkBinding)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetTranslatableResources(self):
        return [self.m_languageSpecificProjectFolder]

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetNonTranslatableResources(self):
        return [self.m_tkFilePath] + self.m_supportingFileList

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def CustomPseudolocalization (self, sourceFile, destFile, destinationDir):
        sourceFolder = sourceFile
        destFolder = destFile
        utils.showInfoMsg ("MSBuildProject Pseudolocalization only copies files. Removes destination directory if already exists and copies over the original one.\n", utils.INFO_LEVEL_SomewhatInteresting)
        if os.path.exists(destFolder):
            utils.cleanDirectory (destFolder, deleteFiles=True)
            os.rmdir(destFolder)
        stat = cmdutil.roboCopyDir(sourceFolder, destFolder)
        if stat != 0:
            raise utils.PartBuildError ("Failed to build localized file {0} \r\n Failed to copy {1} to {2}".format (self.m_tkOutFilePath, sourceFolder, destFolder), self.m_part)

#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
class SatelliteAssembly(ResourceBase):
    def __init__(self, satelliteDom, tkFilePath, languageSetting, part):
        self.m_satelliteDom = satelliteDom

        dllName = satelliteDom.getAttribute ("DllName")
        exeName = satelliteDom.getAttribute ("ExeName")
        name = dllName if dllName and len (dllName) > 0 else exeName
        relatedAssemblyExtension = ".dll" if dllName and len (dllName) > 0 else ".exe"
        if len (name) == 0:
            raise utils.PartBuildError ("Either define DllName or ExeName attribute for satellite assembly in file {0}".format (tkFilePath), self.m_part)

        self.m_targetFramework = None
        targetFramework = satelliteDom.getAttribute ("TargetFramework")
        if targetFramework and len (targetFramework) > 0:
            self.m_targetFramework = targetFramework

        self.m_deliveryEnglish = getStringValueTrueFalse (satelliteDom.getAttribute ("DeliverEnglish").lower (), True)
        ResourceBase.__init__(self, name + ".resources.dll", tkFilePath, languageSetting, part, None)

        self.m_relatedAssemblyFileName = name + relatedAssemblyExtension

        deliveryEnglish = satelliteDom.getAttribute ("DeliverEnglish")
        if deliveryEnglish.lower () == "false":
            self.m_excludeLanguages.append ("en")

        self.m_resxFiles = []

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetTkDom(self):
        return self.m_satelliteDom

    #-------------------------------------------------------------------------------------------
    # bsiclass
    #-------------------------------------------------------------------------------------------
    def ReadSourceFiles (self, tkBinding):
        self.m_tkOutFilePath = os.path.join (TranskitBuildDirLanguage (self.m_languageSetting, self.m_part.GetPlatform (), self.m_part.m_info.m_static), getTKBindingRelativePath (self.m_part.m_info, tkBinding), self.m_builtFileName)
        self.m_tkOutFilePath = utils.resolveBindingMacros (self.m_tkOutFilePath, self.m_part.GetPlatform (), self.m_part, forceUseApiNumber=True)

        if ReadOutFilesOnly ():
            return

        self.m_relatedAssemblyFileName = utils.resolveBindingMacros (self.m_relatedAssemblyFileName, self.m_part.GetPlatform (), self.m_part, forceUseApiNumber=True)

        resxFiles = self.m_satelliteDom.parentNode.getElementsByTagName ("ResxFile")
        for resxFile in resxFiles:
            resourceName = utils.resolveBindingMacros (resxFile.getAttribute ("ResourceName"), self.m_part.GetPlatform (), self.m_part, forceUseApiNumber=True) 
            resourceFile = utils.resolveBindingMacros (resxFile.getAttribute ("FileName"), self.m_part.GetPlatform (), self.m_part, forceUseApiNumber=True)

            if not resourceName or len (resourceName) == 0:
                resourceName, _ = os.path.split (resourceFile)
            # Store the resourceFile as well to use it for compilation when linking to resxTemp directory. This avoids path replacement failures.
            self.m_resxFiles.append ((resourceName, os.path.join (getLocalizedResourceDirectory (self.m_part, tkBinding, self.m_languageSetting, childFileInFolder=resourceFile), resourceFile), resourceFile))

        self.m_relatedAssemblyFilePath = self._FindRelatedAssembly (tkBinding)
        self.m_relatedAssemblyFilePath = utils.resolveBindingMacros (self.m_relatedAssemblyFilePath, self.m_part.GetPlatform (), self.m_part, forceUseApiNumber=True)

        ResourceBase.ReadSourceFiles (self, tkBinding)


    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetTranslatableResources(self):
        return [resxFile [1] for resxFile in self.m_resxFiles]

    def GetNonTranslatableResources(self):
        return [self.m_tkFilePath] + self.m_supportingFileList

    def GetExtraBuildContextItems (self):
        return [self.m_relatedAssemblyFilePath]
    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def Compile (self, tkBinding):
        SetReadOutFilesOnly (False)
        self.ReadSourceFiles (tkBinding)

        if not utils.IsSourceModified ( [self.BuiltFilePath ()], self.GetTranslatableResources () + self.GetNonTranslatableResources ()):
            return

        vsVersion = GetDefaultVSVersion(self.m_part)
        _, vsEnvironment = self.GetVsEnvironment ()
        buildDir = os.path.dirname (self.m_tkOutFilePath)

        # Use the same signature type as related assembly
        signatureType = getSignatureType(self.m_relatedAssemblyFilePath, vsVersion, vsEnvironment)
        relatedAssemblyKey = os.path.join (buildDir, self.m_relatedAssemblyFileName + "1.pubkey")
        extractPublicKeyFile (self.m_relatedAssemblyFilePath, buildDir, relatedAssemblyKey, vsVersion, vsEnvironment, self.m_logFile)

        # Compile and link
        resourceFiles = self._CompileResx (vsVersion, vsEnvironment)
        self._LinkResources (resourceFiles, relatedAssemblyKey, vsVersion, vsEnvironment, signatureType)
        self._InjectVersion (self.m_relatedAssemblyFilePath, buildDir, os.path.splitext(self.m_relatedAssemblyFileName)[0])
        
        # Sign resource dll
        if os.path.exists (relatedAssemblyKey):
            signAssembly (self.m_tkOutFilePath, buildDir, signatureType, vsVersion, vsEnvironment, self.m_logFile)
            self._ValidateSignatureWithBaseAssembly(relatedAssemblyKey, buildDir, vsVersion, vsEnvironment)

        signtoolExe = getVSExePath ("signtool.exe", vsVersion, vsEnvironment)
        signstat = cmdutil.signFileCommand(self.m_tkOutFilePath, signtoolExe, self.m_logFile)
        if 0 !=signstat:
            raise utils.PartBuildError ("Error signing {0}".format (self.m_tkOutFilePath), self.m_part)


    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def _FindRelatedAssembly (self, tkbinding):
        bindings = utils.splitSources(tkbinding.m_parentBinding.m_val)
        for binding in bindings:
            sourceFiles = glob.glob (os.path.join (symlinks.normalizePathName (self.m_part.GetMyBuildContextDir ()), binding))
            for sourceFile in sourceFiles:
                if self.m_relatedAssemblyFileName.lower () == os.path.basename (sourceFile).lower ():
                    return sourceFile

        msg =  "Base assembly with name {0} cannot be found for transkit {1}".format (self.m_relatedAssemblyFileName, self.m_tkFilePath)
        msg =  msg + "\nSearching in {0}\n".format (self.m_part.GetMyBuildContextDir ())
        utils.showAndLogMsg (msg, self.m_logFile, utils.INFO_LEVEL_VeryInteresting)

        def SearchFileInFolder (fileName, folderPath):
            folderPath = symlinks.normalizePathName (folderPath)
            for fileItem in os.listdir (folderPath):
                itemPath = os.path.join (folderPath, fileItem)
                if os.path.isfile (itemPath):
                    if os.path.basename (itemPath).lower () == fileName.lower ():
                        return itemPath
                elif os.path.isdir (itemPath):
                    return SearchFileInFolder (fileName, itemPath)

            return None

        searchedFile = SearchFileInFolder (self.m_relatedAssemblyFileName, self.m_part.GetMyBuildContextDir ())
        if searchedFile:
            return searchedFile

        raise utils.PartBuildError ("Base assembly with name {0} cannot be found for transkit {1}".format (self.m_relatedAssemblyFileName, self.m_tkFilePath), self.m_part)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def _InjectVersion (self, relatedAssembly, buildDir, productName):
        versionCmdFile = GetVersionMkiArg (self.m_tkOutFilePath, relatedAssembly, productName)

        injectVersionExe = utils.GetBsiToolsFile ("injectversion.exe")

        cmd  = [injectVersionExe, "@" + versionCmdFile, self.m_tkOutFilePath]
        utils.showAndLogMsg ("Versioning Assembly {0}\r\n".format(self.m_tkOutFilePath), self.m_logFile, utils.INFO_LEVEL_VeryInteresting)
        stat = cmdutil.runAndLog (cmd, self.m_logFile, buildDir)
        if 0 != stat:
            raise utils.PartBuildError ("Error versioning {0}".format (relatedAssembly), self.m_part)


    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def _ValidateSignatureWithBaseAssembly (self, relatedAssemblyKey, buildDir, vsVersion, vsEnvironment):
        satAssemblyKey  = os.path.join (buildDir, self.m_relatedAssemblyFileName + "2.pubkey")
        extractPublicKeyFile (self.m_tkOutFilePath, buildDir, satAssemblyKey, vsVersion, vsEnvironment, self.m_logFile)

        if not os.path.exists (satAssemblyKey):
            msg = "WARNING: Public keys for satellite assembly is missing. {0}\n".format (self.m_tkOutFilePath)
            utils.showInfoMsg (msg, utils.INFO_LEVEL_Essential, utils.YELLOW)
            return

        satAssemblyPublicKey = None
        relatedAssemblyPublicKey = None
        with open (satAssemblyKey, "rb") as satAssemblyKeyFile:
            satAssemblyPublicKey = satAssemblyKeyFile.read ()
        with open (relatedAssemblyKey, "rb") as relatedAssemblyKeyFile:
            relatedAssemblyPublicKey = relatedAssemblyKeyFile.read ()

        if satAssemblyPublicKey != relatedAssemblyPublicKey:
            msg = "public keys for assembly and satellite assembly are different. This means that the satellite assemblies will not be loadable.\n"
            utils.showInfoMsg (msg, utils.INFO_LEVEL_Essential, utils.YELLOW)
            return

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def _LinkResources (self, resourceFiles, relatedAssemblyKey, vsVersion, vsEnvironment, signatureType):
        alExe = None

        alExe = getAssemblyLinkerExePath (vsVersion, vsEnvironment, self.m_targetFramework, self.m_part.GetPlatform().GetXmlName())

        if not alExe:
            raise utils.PartBuildError ("Cannot find al.exe with parameters VSVersion={0} and WinSDKVersion={1}".format (vsVersion, self.m_targetFramework), self.m_part)

        cmdFilePath = os.path.join (os.path.dirname (self.m_tkOutFilePath), "al")

        with open (cmdFilePath, "wt") as cmdFile:
            if os.path.exists (relatedAssemblyKey):
                cmdFile.write ("/out:\"{0}\" /c:{1} /delay+ /template:{2} /keyfile:{3}\r\n".format (self.m_tkOutFilePath, self.m_languageSetting.m_culture, self.m_relatedAssemblyFilePath, utils.getPublicKeyFilePath(signatureType)))
            else:
                cmdFile.write ("/out:\"{0}\" /c:{1} /template:{2}\r\n".format (self.m_tkOutFilePath, self.m_languageSetting.m_culture, self.m_relatedAssemblyFilePath))
                msg =  "INFO: {0} did not generate a public keyfile. This might indicate the file was not strong named. Skipping strong naming of the Satellite Assembly.\n".format (self.m_relatedAssemblyFilePath)
                utils.showAndLogMsg (msg, self.m_logFile, utils.INFO_LEVEL_VeryInteresting)

            for resourceFile in resourceFiles:
                cmdFile.write (" /embed:\"{0}\",{1},Private\r\n".format (resourceFile, os.path.basename (resourceFile)))
            cmdFile.flush ()
            cmdFile.close ()

        cmdNewFilePath = cmdFilePath + ".cmd"
        if os.path.exists (cmdNewFilePath):  # remove existing for rebuild
            os.remove (cmdNewFilePath)

        # Wait till 2 seconds with check in every 1 milliseconds.
        count = 2000
        renamed = False
        while count > 0:
            count -= 1
            try:
                os.rename (cmdFilePath, cmdNewFilePath)
                renamed = True
                break
            except:
                time.sleep (0.001)
                continue
                
        if not renamed:
            raise utils.PartBuildError ("Error closing / renaming  {0}\n{1}".format (cmdFilePath, cmdNewFilePath), self.m_part)

        cmdFilePath = cmdNewFilePath

        cmd = [alExe, "@" + cmdFilePath]

        utils.showAndLogMsg ("Running resource linking command: {0}\n".format(cmd), self.m_logFile, utils.INFO_LEVEL_RarelyUseful)

        stat = cmdutil.runAndLogWithEnv (cmd, vsEnvironment, self.m_logFile, None)
        if 0 != stat:
            explanation = ""
            if not os.path.exists (self.m_relatedAssemblyFilePath):
                explanation = "Missing Neutral dll '{0}' which is required when compiling satellite assemblies.".format (self.m_relatedAssemblyFilePath)
            raise utils.PartBuildError ("Error linking {0}\n{1}".format (self.m_tkOutFilePath, explanation), self.m_part)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def _CompileResx (self, vsVersion, vsEnvironment):
        resgenExe = getVSExePath ("resgen.exe", vsVersion, vsEnvironment)
        buildDir = os.path.dirname (self.m_tkOutFilePath)
        symlinks.makeSureDirectoryExists (buildDir)
        resourceFiles = []
        resxFiles = []

        resxBuildDir = os.path.join (buildDir, "resxTemp")
        symlinks.makeSureDirectoryExists (resxBuildDir)

        for resxFile in self.m_resxFiles:
            resxTempFile = os.path.join (resxBuildDir, resxFile[2])
            symlinks.createFileSymLink (resxTempFile, resxFile [1], checkSame=False)
            resxTempFileTuple = (resxFile [0], resxTempFile)
            resxFiles.append (resxTempFileTuple)

        for supportingFile in self.m_supportingFileList:
            tempFileLocation = os.path.join (resxBuildDir, supportingFile.replace(os.path.dirname (self.m_tkFilePath), "").strip('\\'))
            symlinks.createFileSymLink (tempFileLocation, supportingFile)

        def symlinkFilesInDir (sourceDir, targetDir):
            if not os.path.exists (sourceDir):
                return

            symlinks.makeSureDirectoryExists (targetDir)
            for item in os.listdir (sourceDir):
                fullPath = os.path.join (sourceDir, item)
                if os.path.isfile (fullPath):
                    symlinks.createFileSymLink (os.path.join (targetDir, item), fullPath)
                else:
                    symlinkFilesInDir (fullPath, os.path.join (targetDir, item))

        #symlinkFilesInDir (os.path.dirname (self.m_tkFilePath), resxBuildDir)
        
        devexpress=False
        sub = "DevExpress"
        if len(self.m_supportingFileList) and any(sub in string for string in self.m_supportingFileList):                        
            devexpress=True
                
        for resxFile in resxFiles:
            resourceFile    = os.path.join (buildDir, resxFile [0] + "." + self.m_languageSetting.m_culture  + ".resources")
            cmd             =   [resgenExe]
            if devexpress:
                for sfile in self.m_supportingFileList:
                    supportFile ="-r:"+sfile
                    cmd.append(supportFile)
            cmd.append ("/useSourcePath")
            cmd.append (resxFile [1])
            cmd.append (resourceFile)
            utils.showAndLogMsg ("Resgen {0}\r\n".format(' '.join (cmd)), self.m_logFile, utils.INFO_LEVEL_VeryInteresting)
            stat = cmdutil.runAndLogWithEnv (cmd, vsEnvironment, self.m_logFile, None)
            if 0 != stat:
                raise utils.PartBuildError ("Error compiling {0}".format (resourceFile), self.m_part)

            resourceFiles.append (resourceFile)

        return resourceFiles

    #--------------------------------------------------------------------------------
    # Return MSVC_Version string if it was found. Otherwise returns empty string "".)
    # @bsimethod
    #--------------------------------------------------------------------------------
    def getVsVersionFromTranskitFile (self, filename):
        fi = open (filename, "rt")
        versionNumberStr = "" 
        for line in fi.readlines():
            if -1 != line.upper().find ("MSVC_VERSION"):
                versionNumberStr = line.split ('=')[1].strip()
                break
        fi.close()
        return versionNumberStr
    
    #--------------------------------------------------------------------------------
    # bsimethod
    #--------------------------------------------------------------------------------
    def GetVsEnvironment (self):
        vsvarsFile, vsEnvironment = getVsEnvironmentAndCheckOverrides (None, self.m_logFile, self.m_part) # since we no longer use the version from the transkit file, pass in None for it not to complain about it not being defined
        # Figure out which version the non resource assembly was created with and use those instead of the transkit.xml manifest.
        # That can be a validation step to check .NET framework version used. But determining VS version, at time of build by looking at DLL is not possible.
        return vsvarsFile, vsEnvironment

#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
class CustomTranskitFileFormat (ResourceBase):
    def __init__(self, resDom, tkFilePath, languageSetting, part):
        outFileName = resDom.getAttribute ("Name")
        self.m_resDom = resDom

        pseudoLocalizeCommand = utils.getDomElementValue (resDom, "PseudoLocalizeCommand") if 0 != len(utils.getDomElementsByName(resDom, "PseudoLocalizeCommand")) else None
        ResourceBase.__init__(self, outFileName, tkFilePath, languageSetting, part, pseudoLocalizeCommand)

        self.m_localizerCommand = utils.getDomElementValue (resDom, "LocalizedBuildCommand")
        if len (self.m_localizerCommand) == 0:
            raise utils.PartBuildError ("LocalizerCommand is required element in {0}".format (tkFilePath), self.m_part)

        deliveryEnglish = resDom.getAttribute ("DeliverEnglish")
        if deliveryEnglish.lower () == "false":
            self.m_excludeLanguages.append ("en")

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetTkDom(self):
        return self.m_resDom

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def ReadSourceFiles(self, tkBinding):
        self.m_tkOutFilePath = symlinks.normalizePathName (os.path.join (TranskitBuildDirLanguage (self.m_languageSetting, self.m_part.GetPlatform (), self.m_part.m_info.m_static), getTKBindingRelativePath (self.m_part.m_info, tkBinding), self.m_builtFileName))
        self.m_tkOutFilePath = utils.resolveBindingMacros (self.m_tkOutFilePath, self.m_part.GetPlatform (), self.m_part, forceUseApiNumber=True)

        if ReadOutFilesOnly ():
            return

        checkBuiltFileInSource = utils.getOptionalBoolAttr('CheckBuiltFileInSource', True, 'CheckBuiltFileInSource', utils.getDomElementsByName (self.m_resDom, "ResourceSource")[0])
        sourceFile = utils.resolveBindingMacros (utils.getDomElementValue (self.m_resDom, "ResourceSource"), self.m_part.GetPlatform (), self.m_part, forceUseApiNumber=True)
        sourceDir =  symlinks.normalizePathName (os.path.join (getLocalizedResourceDirectory (self.m_part, tkBinding, self.m_languageSetting, childFileInFolder=sourceFile)))
        filenames = glob.glob (os.path.join (sourceDir, sourceFile))
        self.m_sourceFileList = []

        for sfile in filenames:
            if checkBuiltFileInSource:
                if self.m_builtFileName in sfile:
                    self.m_sourceFileList = glob.glob (os.path.join (sourceDir, sfile))
            else:
                self.m_sourceFileList = glob.glob (os.path.join (sourceDir, sfile))

        if len (self.m_sourceFileList) == 0:
            raise utils.PartBuildError ("Cannot find resource files using path {0}".format (os.path.join (sourceDir, sourceFile)), self.m_part)
        
        ResourceBase.ReadSourceFiles(self, tkBinding)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetNonTranslatableResources(self):
        return [self.m_tkFilePath] + self.m_supportingFileList

    def GetTranslatableResources(self):
        return self.m_sourceFileList

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def Compile(self, tkBinding):
        SetReadOutFilesOnly (False)
        self.ReadSourceFiles (tkBinding)

        if not utils.IsSourceModified ( [self.BuiltFilePath ()], self.GetTranslatableResources () + self.GetNonTranslatableResources ()):
            return

        buildDir = os.path.dirname (self.m_tkOutFilePath)
        symlinks.makeSureDirectoryExists (buildDir)
        self.m_localizerCommand = self.m_localizerCommand.replace ("[SourceFiles]", ";".join (self.m_sourceFileList))
        self.m_localizerCommand = self.m_localizerCommand.replace ("[OutputFile]", self.m_tkOutFilePath).replace ("${TranskitTools}", TranskitToolsDir (self.m_part.GetPlatform ()))
        self.m_localizerCommand = self.m_localizerCommand.replace ("[SupportingResources]", ";".join (self.m_supportingFileList))
        utils.showAndLogMsg ("Localizer command {0}\r\n".format(self.m_localizerCommand), self.m_logFile, utils.INFO_LEVEL_VeryInteresting)
        stat = cmdutil.runAndLogWithEnv (self.m_localizerCommand, None, self.m_logFile, buildDir)
        if stat != 0:
            raise utils.PartBuildError ("Failed to build localized file {0} \r\n Failed for command {1}".format (self.m_tkOutFilePath, self.m_localizerCommand), self.m_part)

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def getCustomTranskitItems (tkDom, tkFilePath, part):
    resObjs = []
    resFiles = tkDom.getElementsByTagName ("CustomResourceFormat")
    if len (resFiles) < 1:
        return resObjs

    for languageSetting in getLanguageSettings ():
        for resFile in resFiles:
            resObjs.append (CustomTranskitFileFormat (resFile, tkFilePath, languageSetting, part))

    return resObjs

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def getAllResxItems (tkDom, tkFilePath, part):
    resxObjs = []

    resxFiles = utils.getDomElementsByName (tkDom, "SatelliteAssembly")

    if len (resxFiles) < 1:
        return resxObjs

    for languageSetting in getLanguageSettings ():
        resxObjs.append (SatelliteAssembly (resxFiles [0], tkFilePath, languageSetting, part))

    return resxObjs

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def getAllRawResxItems (tkDom, tkFilePath, part):
    resxObjs = []

    resxFiles = utils.getDomElementsByName (tkDom, "RawResx")

    if len (resxFiles) < 1:
        return resxObjs

    # Store these outside the loop so they can be used in the submethod; otherwise python's closure rules will lock them at the first value.
    neutralLanguage = None
    curlanguageSetting = None
    def getNeutraAdjustedFileName (fileName):
        if neutralLanguage.lower () == curlanguageSetting.m_culture.lower ():
            return fileName
        (filebase, ext) = os.path.splitext (fileName)
        return "{0}.{1}{2}".format (filebase, curlanguageSetting.m_culture, ext)

    for languageSetting in getLanguageSettings ():
        curlanguageSetting = languageSetting
        for rawResx in resxFiles:
            fileName = rawResx.getAttribute ("Name")
            neutralLanguage = rawResx.getAttribute ("NeutralLanguage")
            outputDirectory = rawResx.getAttribute ("Directory")

            if "" == neutralLanguage:
                raise utils.PartBuildError ("RawResx must provide 'NeutralLanguage' attribute which is the neutral language. It will be used  to specify which language name does not get inserted into the output filename.language.resx ", part)
            if "" == outputDirectory:
                raise utils.PartBuildError ("RawResx must provide 'Directory' attribute which is the directory to place the file into.", part)


            if -1 == outputDirectory.find ("App_LocalResources") and -1 == outputDirectory.find ("App_GlobalResources"):
                raise utils.PartBuildError ("RawResx must provide 'Directory' attribute with 'App_LocalResources' or 'App_GlobalResources' in it.", part)

            if fileName.find ("*") > -1:
                fileNames = glob.glob (os.path.join (os.path.dirname (tkFilePath), fileName))
                for curfile in fileNames:
                    resxObjs.append (RawResxDelivery  (getNeutraAdjustedFileName (curfile), tkFilePath, languageSetting, part, outputDirectory, rawResx))
            else:
                resxObjs.append (RawResxDelivery (getNeutraAdjustedFileName (fileName), tkFilePath, languageSetting, part, outputDirectory, rawResx))

    return resxObjs

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def getAllBentleyRibbon (tkDom, tkFilePath, part):
    ribbonObjs = []
    ribbonFiles = utils.getDomElementsByName (tkDom, "BentleyRibbonRsc")

    if len (ribbonFiles) < 1:
        return ribbonObjs

    for languageSetting in getLanguageSettings ():
        ribbonObjs.append (BentleyRibbonResourceFile (ribbonFiles [0], tkFilePath, languageSetting, part))

    return ribbonObjs

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def getDgnlibResources (tkDom, tkFilePath, part):
    dgnlibObjs = []
    dgnlibFiles = utils.getDomElementsByName (tkDom, "BentleyDgnlibMui")

    if len (dgnlibFiles) < 1:
        return dgnlibObjs

    for languageSetting in getLanguageSettings ():
        dgnlibObjs.append (DgnlibResourceFile(dgnlibFiles [0], tkFilePath, languageSetting, part))

    return dgnlibObjs

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def getTranslatedFilesObjs (tkDom, tkFilePath, part):
    translatedObjs = []
    trFiles = utils.getDomElementsByName (tkDom, "TranslatedDelivery")

    if len (trFiles) < 1:
        return translatedObjs

    for languageSetting in getLanguageSettings ():
        for trFile in trFiles:
            translatedObjs.append (TranslatedDelivery (trFile, tkFilePath, languageSetting, part))

    return translatedObjs

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def getTranslatedDirObjs (tkDom, tkFilePath, part):
    translatedObjs = []
    trFiles = utils.getDomElementsByName (tkDom, "TranslatedDirDelivery")

    if len (trFiles) < 1:
        return translatedObjs

    for languageSetting in getLanguageSettings ():
        for trFile in trFiles:
            translatedObjs.append (TranslatedDirDelivery (trFile, tkFilePath, languageSetting, part))

    return translatedObjs

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------

def getSchemaResxItems (tkDom, tkFilePath, part):
    supSchemaObjs = []

    supSchemaFiles = utils.getDomElementsByName (tkDom, "SchemaResource")

    for supSchemaFile in supSchemaFiles:
        for languageSetting in getLanguageSettings ():
            supSchemaObjs.append (SupplementalSchemaDelivery (supSchemaFile, tkFilePath, languageSetting, part))

    return supSchemaObjs

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def getWindowResourceItems (tkDom, tkFilePath, part):
    winRCObjs = []

    winRCFiles = utils.getDomElementsByName (tkDom, "WindowsRc")

    for winRCFile in winRCFiles:
        for languageSetting in getLanguageSettings ():
            winRCObjs.append (WindowsRCDelivery (winRCFile, tkFilePath, languageSetting, part))

    return winRCObjs

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def getWordResourceItems (tkDom, tkFilePath, part):
    wordRCObjs = []

    wordRCFiles = utils.getDomElementsByName (tkDom, "WordToPdf")

    for wordRCFile in wordRCFiles:
        for languageSetting in getLanguageSettings ():
            wordRCObjs.append (WordToPdf(wordRCFile, tkFilePath, languageSetting, part))

    return wordRCObjs

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def getMSBuildLocalizationProjectResourceItems (tkDom, tkFilePath, part):
    msbuildProjectRCObjs = []

    msbuildProjectRCFiles = utils.getDomElementsByName (tkDom, "MSBuildLocalizationProject")

    for msbuildProjectRCFile in msbuildProjectRCFiles:
        for languageSetting in getLanguageSettings ():
            msbuildProjectRCObjs.append (MSBuildLocalizationProject(msbuildProjectRCFile, tkFilePath, languageSetting, part))

    return msbuildProjectRCObjs

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def getResourceObjs (transkitFile, part, apiNumber):
   
    transkitFile = symlinks.normalizePathName (transkitFile)
    resourceObjs = []

    # During a clobber build the symlink may remain but the referenced file is gone.
    if IsClobberBuild():
        if not symlinks.symlinkTargetExists (transkitFile):
            return resourceObjs
        try:
            transkitFile = symlinks.getSymlinkTarget (transkitFile)
        except symlinks.SymLinkError:
            return resourceObjs

    if not os.path.exists (transkitFile):
        if IsClobberBuild ():
            return resourceObjs
        raise utils.PartBuildError("Cannot find transkit file : {0}".format(transkitFile), part)

    try:
        transkitXmlRoot = utils.parseXml (transkitFile)
    except xml.parsers.expat.ExpatError as errIns:
        raise utils.PartBuildError("Error parsing {0}: ".format(transkitFile) + errIns.__str__(), part)

    tkDom = utils.getDomElementsByName (transkitXmlRoot, "Transkit")
    if 1 != len (tkDom):
        raise utils.PartBuildError("{0} has no root Transkit element".format(transkitFile), part)

    # Get All resource files
    rscFiles = utils.getDomElementsByName (tkDom[0], "BentleyRsc")
    for rscFile in rscFiles:
        name = rscFile.getAttribute ("Name")
        if "" == name:
            raise utils.PartBuildError ("No Name given for BentleyRsc", part)
        name = name.replace ('$(ApiNumber)', apiNumber)
        for languageSetting in getLanguageSettings ():
            bentleyRsc = BentleyResourceFile (rscFile, transkitFile, languageSetting, part)
            resourceObjs.append (bentleyRsc)
    
    resourceObjs.extend (getDgnlibResources (tkDom[0], transkitFile, part)) 
    resourceObjs.extend (getAllBentleyRibbon (tkDom[0], transkitFile, part))
    resourceObjs.extend (getAllResxItems (tkDom[0], transkitFile, part))
    resourceObjs.extend (getAllRawResxItems (tkDom[0], transkitFile, part))
    resourceObjs.extend (getCustomTranskitItems (tkDom[0], transkitFile, part))
    resourceObjs.extend (getTranslatedFilesObjs (tkDom [0], transkitFile, part))
    resourceObjs.extend (getTranslatedDirObjs (tkDom [0], transkitFile, part))
    resourceObjs.extend (getSchemaResxItems (tkDom [0], transkitFile, part))
    resourceObjs.extend (getWindowResourceItems (tkDom [0], transkitFile, part))
    resourceObjs.extend (getWordResourceItems (tkDom [0], transkitFile, part))
    resourceObjs.extend (getMSBuildLocalizationProjectResourceItems (tkDom [0], transkitFile, part))

    return resourceObjs

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def symlinkResourceToDir (tlResourceObj, deliveryDir, tkBinding):
    tlResourceObj.ReadSourceFiles (tkBinding)

    targetPath = os.path.join (deliveryDir, tlResourceObj.SubPathForProduct ())

    if tlResourceObj.IsDir ():
        symlinks.createDirSymLink (targetPath, tlResourceObj.m_tkOutFilePath, checkSame=True, checkTargetExists=True, skipIntermediateLinks=True)
    else:
        symlinks.createFileSymLink (targetPath, tlResourceObj.m_tkOutFilePath, checkSame=True, checkTargetExists=True, skipIntermediateLinks=True)

#--------------------------------------------------------------------------------
# bsimethod
#--------------------------------------------------------------------------------
def getSignatureType(assemblyPath, vsVersion, vsEnvironment):
    snExe   = getVSExePath ("sn.exe", vsVersion, vsEnvironment)

    cmd  = [snExe, "-q", "-T", assemblyPath]
    status, stdoutLines, stderrLines = cmdutil.runAndWaitSeparateErrors(cmd)

    if status == 0:
        if (utils.StrongNameSignatureType.PRG_RIGHTS_COMPLIANT in stdoutLines[0]):
            return utils.StrongNameSignatureType.PRG_RIGHTS_COMPLIANT

        if (utils.StrongNameSignatureType.PRG_NON_RIGHTS_COMPLIANT in stdoutLines[0]):
            return utils.StrongNameSignatureType.PRG_NON_RIGHTS_COMPLIANT

        if (utils.StrongNameSignatureType.NORMAL_RIGHTS_COMPLIANT in stdoutLines[0]):
            return utils.StrongNameSignatureType.NORMAL_RIGHTS_COMPLIANT

        if (utils.StrongNameSignatureType.NORMAL_NON_RIGHTS_COMPLIANT in stdoutLines[0]):
            return utils.StrongNameSignatureType.NORMAL_NON_RIGHTS_COMPLIANT

        if (utils.StrongNameSignatureType.TEST in stdoutLines[0]):
            return utils.StrongNameSignatureType.TEST

        utils.showInfoMsg ("Did not find a supported public token in {0}\n".format(assemblyPath), utils.INFO_LEVEL_SomewhatInteresting)
    else:
        for line in stderrLines:
            utils.showInfoMsg (line + '\n', utils.INFO_LEVEL_SomewhatInteresting)
    
    return utils.StrongNameSignatureType.NONE

#--------------------------------------------------------------------------------
# @bsimethod
#--------------------------------------------------------------------------------
gVSEnvironmentAsPerVersion  = {}

def getVsEnvironmentAndCheckOverrides (vsVersion, logFile, part):
    """Lookup VSVersion path to where vsvars32.bat is for a specific version of the compiler and use that if provided. 
       Otherwise use the default one from options.vsvarsPath."""
    InitializeVsVarsPathFromRegistry()

    platform = part.GetPlatform ()

    versionToCheck = vsVersion if vsVersion else GetDefaultVSVersion(part)
    if versionToCheck.upper () in gVSEnvironmentAsPerVersion:
        return gVSEnvironmentAsPerVersion [versionToCheck.upper ()]

    if versionToCheck not in g_vsVersionToVSVarsPath:
        raise utils.PartBuildError ("VSVersion='{0}' is not among available VS Versions. '{0}'".format (" ".join (g_vsVersionToVSVarsPath)), part)

    vsvarsFile = ""
    vsEnvironment = ""
    if vsVersion:
        vsvarsFile, vsEnvironment = getVsEnvironmentForVersion (vsVersion, logFile, platform)
        if None == vsEnvironment:
            msg = "VSVersion='{0}' is depended on for resource but was not set during the BentleyBuild operation.\n".format (vsVersion)
            msg += "Make sure it is set before building.\n"
            raise utils.PartBuildError (msg, part)
    else:
        # vsvarsPath = options.vsvarsPath 
        # value of options.vsvarsPath is hardcoded below.
        # vsvarsPath = "\\out\\Winx64\\TransKitBuild\\TkWinTools"
        utils.ShowAndDeferMessage ("No VSVersion attribute is provided. Using default version {0}. {1}\n".format (versionToCheck, part), utils.INFO_LEVEL_VeryInteresting, utils.YELLOW)
        vsvarsFile, vsEnvironment = getVsEnvironment (versionToCheck, g_vsVersionToVSVarsPath [versionToCheck], logFile, platform)

    gVSEnvironmentAsPerVersion [versionToCheck.upper ()] = (vsvarsFile, vsEnvironment)
    return gVSEnvironmentAsPerVersion [versionToCheck.upper ()]

#--------------------------------------------------------------------------------
# @bsimethod
#--------------------------------------------------------------------------------
def getVsEnvironmentForVersion (vsVersionString, logFile, platform):
    InitializeVsVarsPathFromRegistry()
    if vsVersionString not in g_vsVersionToVSVarsPath.keys():
        return None, None
    if None != g_vsVersionToVSVarsPath[vsVersionString]:
        return getVsEnvironment (vsVersionString, g_vsVersionToVSVarsPath[vsVersionString], logFile, platform)
    return None, None

#-------------------------------------------------------------------------------------------
# @bsimethod
#-------------------------------------------------------------------------------------------
def getVsEnvironment (vsVersionDir, vsvarsLocation, logFile, platform):
    if vsVersionDir not in g_vsVersionFileCreationLocks:
        return

    with g_vsVersionFileCreationLocks [vsVersionDir]: 
        # NEEDSWORK: By the time you get this lock, VSVersion may already be poplucated by other thread. So check for it again here.
        if vsVersionDir == "VS2017" or vsVersionDir == "VS2019" or vsVersionDir == "VS2022":
            vsvarsFile = os.path.join (vsvarsLocation, "VsDevCmd.bat")
        else:
            vsvarsFile = os.path.join (vsvarsLocation, "vsvars32.bat")

        if not os.path.exists (vsvarsFile):
            raise utils.BuildError("Transkit: Cannot find vsvars at:  {0}: ".format(vsvarsFile))
        
        envMarker = "--------Environment-Start------------"

        buildDir = os.path.join (TranskitBuildDir (platform), "vsvarsenv", vsVersionDir)
        symlinks.makeSureDirectoryExists (buildDir)
        
        # Need the environment to be free of quoted bits of path with parenthesis ;"C:\Program Files (x86)\DXSDKOCT2006\Utilities\Bin\x86";
        localEnv = os.environ.copy ()
        pathVal = localEnv["PATH"]
        pathVal = pathVal.replace ('"', '')
        localEnv["PATH"] = pathVal
        tempName = os.path.join (buildDir, 'envt.bat')

        if not os.path.exists (tempName):
            msg = "Generating {0} to invoke {1}\n".format (tempName, vsvarsFile)
            utils.showAndLogMsg (msg, logFile, utils.INFO_LEVEL_VeryInteresting)
            tmpBatFile = open (tempName, 'wt')
            tmpBatFile.write ("@ECHO off\n")
            if vsVersionDir == "VS2017" or vsVersionDir == "VS2019" or vsVersionDir == "VS2022":
                defToolSetVer = globalvars.buildStrategy.GetToolVersionForPlatform (platform)
                if vsVersionDir == "VS2017":
                    vsinstalldir, _ = utils.queryForRegistryEntry ("SOFTWARE\\WOW6432Node\\Microsoft\\VisualStudio\\SxS\\VS7", "15.0", force32bitRegistry=True)
                elif vsVersionDir == "VS2019":
                    vsinstalldir = cmdutil.getVSInstallationPaths ("16.0")[0]
                elif vsVersionDir == "VS2022":
                    vsinstalldir = cmdutil.getVSInstallationPaths ("17.0")[0]
                toolsetLocation =  os.path.join (vsinstalldir, "VC\\Tools\\MSVC\\")
                toolversionpath = None
                for toolsetDir in os.listdir(toolsetLocation):
                    if os.path.isdir (toolsetLocation + toolsetDir) and toolsetDir.startswith (defToolSetVer):
                        toolversionpath = toolsetLocation + toolsetDir
                if(not(toolversionpath)):
                    utils.ShowAndDeferMessage ("Toolset version {0} is required \n\n".format(defToolSetVer), utils.INFO_LEVEL_VeryInteresting, utils.YELLOW)
                tmpBatFile.write ( '@call "'+ vsvarsFile + '" -vcvars_ver='+defToolSetVer+' \n')
            else:
                tmpBatFile.write ( '@call "'+ vsvarsFile + '"\n')
            tmpBatFile.write ("ECHO "+envMarker+"\n")
            tmpBatFile.write ("set\n")
            tmpBatFile.close ()

    #--------------------------------------------------------------------
    # Run the batch file and create and environment
    vsvarsAdditions = []
    
    def processOutput (outputLine):
        vsvarsAdditions.append (outputLine)

    cmd = ['cmd', '/c', tempName]  # VS2017 has commands which aren't compatible with TCC (trigger a bug) so run batch in CMD.
    cmdutil.runAndWait (cmd, buildDir, processOutput, utils.INFO_LEVEL_SomewhatInteresting, localEnv)

    newEnviron = {}
    readingEnv = False
    for line in vsvarsAdditions:
        line = line.strip()
        if readingEnv:
            re_match = re.search (r"^([A-Za-z0-9_]+)=+(.+)", line)
            if (re_match != None ):
                newEnviron[re_match.group(1)] = re_match.group(2)

        # Use this flag to eliminate any noise above this in the output
        if (line.startswith(envMarker)):
            readingEnv = True
    return vsvarsFile, newEnviron


#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def FindExeInPath (vsEnvironment, exeName):
    path = None
    for envItem in vsEnvironment:
        if envItem.upper() == "PATH":
            path = vsEnvironment[envItem]
            break

    if not path:
        return exeName

    pathList = path.split (';')
    
    for item in pathList:
        nameToFind = os.path.join (item, exeName)
        if os.path.exists (nameToFind):
            return nameToFind

    return exeName

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def SearchExeInPath (rootPath, exeName, searchInSubFolders=True):
    if not os.path.isdir (rootPath):
        return None

    exeFilePath = os.path.join (rootPath, exeName)
    if os.path.isfile (exeFilePath):
        return exeFilePath

    if not searchInSubFolders:
        return None

    childItems = os.listdir (rootPath)
    for childItem in childItems:
        fullPath = os.path.join (rootPath, childItem)
        if os.path.isdir (fullPath):
            exeFilePath = SearchExeInPath (fullPath, exeName, searchInSubFolders=searchInSubFolders)
            if exeFilePath:
                return exeFilePath

    return None

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def getAssemblyLinkerExePath (vsVersion, vsEnvironment, targetFramework, platform):
    AL_EXE = "al.exe"
    if not targetFramework:
        return getVSExePath (AL_EXE, vsVersion, vsEnvironment)

    exeKey = (targetFramework + platform + AL_EXE).lower ()
    if exeKey not in gVsExePaths:
        gVsExePaths [exeKey] = getWinSDKExePath (AL_EXE, targetFramework, platform)
    return gVsExePaths [exeKey]

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
gVsExePaths = {}
def getVSExePath (exeName, vsVersion, vsEnvironment):
    exeKey = (vsVersion + exeName).lower ()
    if exeKey not in gVsExePaths:
        gVsExePaths [exeKey] = FindExeInPath (vsEnvironment, exeName)

    return gVsExePaths [exeKey]

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def getWinSDKExePath (exeName, targetFramework, platform):
    registryDir = getNetFXSDKRegistryDir (targetFramework, platform)
    if not registryDir:
        return None

    return SearchExeInPath (registryDir, exeName)

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def getNetFXSDKRegistryDir (targetFramework, platform):
    """
    Try to find Windows SDK in Microsoft SDK registry, as per specified version
    """

    toolsVersion = None
    if targetFramework == 'v3.5':
        toolsVersion = 'WinSDK-NetFx35Tools-{0}'.format (platform)
    else:
        toolsVersion = 'WinSDK-NetFx40Tools-{0}'.format (platform)
    sdkRegRoot = 'SOFTWARE\\Microsoft\\Microsoft SDKs'

    sdkWindowsRegRoot = os.path.join(sdkRegRoot, 'Windows')
    # Expected to find something like this: HKEY_LOCAL_MACHINE\SOFTWARE\WOW6432Node\Microsoft\Microsoft SDKs\Windows\v8.0A\WinSDK-NetFx35Tools-x64
    for windowsSdkVersion in compat.getRegistrySubKeys(sdkWindowsRegRoot, force32bitRegistry=True):
        sdkReg = os.path.join (sdkWindowsRegRoot, windowsSdkVersion, toolsVersion)
        installDir, _ = utils.queryForRegistryEntry (sdkReg, 'InstallationFolder', force32bitRegistry=True)
        if installDir and os.path.isdir (installDir):
            return installDir

    sdkNetFXSDKRegRoot = os.path.join(sdkRegRoot, 'NETFXSDK')
    # Expected to find something like this: HKEY_LOCAL_MACHINE\SOFTWARE\WOW6432Node\Microsoft\Microsoft SDKs\NETFXSDK\4.8\WinSDK-NetFx40Tools-x64
    for dotNetVersion in compat.getRegistrySubKeys(sdkNetFXSDKRegRoot, force32bitRegistry=True):
        sdkReg = os.path.join (sdkNetFXSDKRegRoot, dotNetVersion, toolsVersion)
        installDir, _ = utils.queryForRegistryEntry (sdkReg, 'InstallationFolder', force32bitRegistry=True)
        if installDir and os.path.isdir (installDir):
            return installDir

    return None

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def extractPublicKeyFile (assemblyPath, buildDir, pubKeyFilePath, vsVersion, vsEnvironment, logFile):
    symlinks.makeSureBaseDirectoryExists (pubKeyFilePath)
    snExe   = getVSExePath ("sn.exe", vsVersion, vsEnvironment)
    cmd     = [snExe, "-e", assemblyPath, pubKeyFilePath]
    stat = cmdutil.runAndLogWithEnv (cmd, vsEnvironment, logFile, buildDir) 
    return stat

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def signAssembly (assemblyPath, buildDir, signatureType, vsVersion, vsEnvironment, logFile):
    if signatureType == utils.StrongNameSignatureType.NONE:
        utils.showInfoMsg ("Signature type is NONE. {assemblyPath} was not signed.\n"
            .format(assemblyPath=assemblyPath), utils.INFO_LEVEL_SomewhatInteresting)
        return 0

    # switch to test type if not on prg machine
    if signatureType == utils.StrongNameSignatureType.PRG_RIGHTS_COMPLIANT or \
        signatureType == utils.StrongNameSignatureType.PRG_NON_RIGHTS_COMPLIANT:
        if not utils.hasPrgSigningKeys():
            signatureType = utils.StrongNameSignatureType.TEST

    if signatureType == utils.StrongNameSignatureType.TEST:
        signingOptions = ("-TS", utils.getPrivateKeyFilePath(signatureType))
    else:
        signingOptions = ("-R", utils.getPrivateKeyFilePath(signatureType))

    snExe   = getVSExePath ("sn.exe", vsVersion, vsEnvironment)
    cmd  = [snExe, signingOptions [0], assemblyPath, signingOptions [1]]
    utils.showAndLogMsg (' '.join (cmd), logFile, utils.INFO_LEVEL_VeryInteresting)
    stat = cmdutil.runAndLogWithEnv (cmd, vsEnvironment, logFile, buildDir)
    return stat 

#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
TRANSKIT_REVISION_FILE_NAME = "TranskitPartRepoRevisionList.xml"
class TranskitPartRepoRevisionList (object):
    def __init__ (self):
        self.m_transkitRevisionFile = None
        self.m_partDescriptions       = []

        self.m_repoGUIDs = {}

    def WriteToXml (self, filePath):
        self.m_transkitRevisionFile = os.path.join (symlinks.normalizePathName (filePath), TRANSKIT_REVISION_FILE_NAME)
        symlinks.makeSureBaseDirectoryExists (self.m_transkitRevisionFile)

        tkdescDom = minidom.Document()
        tkdescNode = tkdescDom.createElement ("TranskitBuildDescription")
        tkdescDom.appendChild (tkdescNode)

        partsNode = tkdescDom.createElement ("BuildContexts")
        tkdescNode.appendChild (partsNode)

        self.WriteParts (partsNode, tkdescDom)

        tkDescFile = open (self.m_transkitRevisionFile, "wt")
        tkdescDom.writexml (tkDescFile, "", "  ", "\n", "UTF-8")
        tkDescFile.close ()

    def WriteParts (self, partsNode, tkdescDom):
        sortedDesc = sorted (self.m_partDescriptions, key=itemgetter ('Repository', 'Name', 'PartName'))
        for partDesc in sortedDesc:
            partNode = tkdescDom.createElement ("BuildContext")
            for (name, value) in partDesc.items ():
                partNode.setAttribute (name, value)
            partsNode.appendChild (partNode)

    def AddPart (self, part, isFromLKG):
        partDesc = {}
        partDesc ["Name"]       = part.m_info.m_buildContext.lower ()
        partDesc ["PartName"]   = part.m_info.m_name.lower ()
        repoName                = part.m_info.m_repo.m_name.lower ()
        partDesc ["Repository"] = repoName
        provFile = None
        if isFromLKG:
            staticDir = 'static' if part.IsStatic() else ''
            provFile = os.path.join (symlinks.normalizePathName(buildpaths.getLKGRoot(part.GetPlatform())), staticDir, part.m_info.m_buildContext, "Logs", part.m_info.m_name + globalvars.provfileName)
        else:
            provFile = symlinks.normalizePathName(part.GetProvenanceFileName ())

        guidFromProvFile = ""
        if not os.path.exists (provFile):
            return

        provDom = minidom.parse (provFile)
        repoNodes = provDom.getElementsByTagName ("Repository")
        for repoNode in repoNodes:
            if repoName == repoNode.getAttribute ("Name"):
                guidFromProvFile = repoNode.getAttribute ("Identifier")

            partDesc ["Guid"] = guidFromProvFile.rstrip ("+")

        self.m_partDescriptions.append (partDesc)
