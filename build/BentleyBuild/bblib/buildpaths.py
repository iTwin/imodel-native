#---------------------------------------------------------------------------------------------
#  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
#  See LICENSE.md in the repository root for full copyright notice.
#---------------------------------------------------------------------------------------------

import os
from . import globalvars, symlinks

DIRNAME_COMBINED            = "Combined"
DIRNAME_LOGFILES            = "LogFiles"
DIRNAME_PRODUCT             = "Product"
DIRNAME_INSTALLER           = "Installer"
DIRNAME_BUILDCONTEXTS       = "BuildContexts"
DIRNAME_TRANSKIT            = "TransKit"
DIRNAME_TRANSKIT_BUILD      = "TransKitBuild"
DIRNAME_TRANSKIT_LOCALIZED  = "Localized"
DIRNAME_TRANSKIT_INCLUDE    = "Include"
DIRNAME_NUGETPKG            = "Nugetpkg"
DIRNAME_TOOLCACHE           = "Tools"
DIRNAME_SRC_TOOLCACHE       = "toolcache"
DIRNAME_BBCACHE             = "bbcache"

FILENAME_BOOTSTRAPINFO      = 'BootstrapInfo.json'
FILENAME_BBCONFIG           = 'bbconfig.json'

s_toolCacheSrc = None

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def getOutputRoot (platform, isStatic):
    outPath = os.path.join(globalvars.programOptions.outRootDirectory, platform.GetDirName())
    if isStatic:
        outPath = os.path.join (outPath, 'static')
    return outPath

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def getLKGRoot(platform):
    return os.path.join (GetLastKnownGoodLocation(), platform.GetDirName())

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def GetToolsOutputRoot ():
    return os.path.join (globalvars.programOptions.outRootDirectory, DIRNAME_TOOLCACHE)

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def getInstallerRoot (platform, isStatic):         
    return os.path.join (getOutputRoot(platform, isStatic), DIRNAME_INSTALLER)

def getSavedProductManifestName():              return 'manifest.xml'

# Root of all source
s_srcRootDir = symlinks.normalizePathName ("${SrcRoot}") if 'SrcRoot' in os.environ else None

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def UpdateSrcRoot ():
    # This is called when SrcRoot is set by the command line. 
    global s_srcRootDir
    s_srcRootDir = symlinks.normalizePathName ("${SrcRoot}")

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def GetSrcRoot ():
    return s_srcRootDir

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def GetLastKnownGoodLocation ():
    return os.path.join (s_srcRootDir, "LastKnownGood")

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def GetTranskitLastKnownGoodLocation ():
    return os.path.join (s_srcRootDir, "LastKnownGoodTranskit")

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def GetSdkSourceLocation ():
    return os.path.join (s_srcRootDir, "SdkSources")

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def GetNuGetSourceLocation ():         
    return os.path.join (s_srcRootDir, "nuget")

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def GetUpackSourceLocation ():
    return os.path.join (s_srcRootDir, "upack")

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def GetADOBuildArtifactSourceLocation ():
    return os.path.join (s_srcRootDir, "adobuildartifact")

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def getDocumentationRoot():
    return os.path.join (s_srcRootDir, "Documentation")

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def getShortPartDescriptor(partFile, partName):
    return "{0}:{1}".format (os.path.basename (partFile)[ : -len(".PartFile.xml") ], partName)

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def GetXsdPath ():
    return os.path.join (s_srcRootDir, 'bentleybuild')

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def GetBBCacheDir ():
    return os.path.join (s_srcRootDir, DIRNAME_BBCACHE)

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def ToolCacheFromSourceRoot(srcRooDir):
    from . import translationkit
    if not translationkit.IsTranskitShell() and "ToolCache" in os.environ and os.environ["ToolCache"]:
        toolCacheSrc = os.environ["ToolCache"]
    else:
        toolCacheSrc = os.path.join(srcRooDir, DIRNAME_SRC_TOOLCACHE) + os.sep
    return toolCacheSrc

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def GetToolCacheSourceRoot():
    global s_toolCacheSrc
    if not s_toolCacheSrc:
        s_toolCacheSrc = ToolCacheFromSourceRoot(GetSrcRoot())
    return s_toolCacheSrc

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def GetBootsrapFileName ():
    return os.path.join (GetBBCacheDir(), FILENAME_BOOTSTRAPINFO)

