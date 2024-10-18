#---------------------------------------------------------------------------------------------
#  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
#  See LICENSE.md in the repository root for full copyright notice.
#---------------------------------------------------------------------------------------------

import os
from . import utils, globalvars

# Constants for different platforms.  These are used for comparison
winX86          = 0
winX64          = 1
WinRTx86        = 2
WinRTx64        = 3
WinRTARM        = 4
AndroidARM      = 5
AndroidARM7A    = 6
AndroidX86      = 7
LinuxX86        = 8
iOSX86          = 9
iOSARM          = 10
# iOSARMV7S       = 11 -- never used in practice
MacOSX64        = 12
LinuxX64        = 13
iOSX64          = 14
iOSARM64        = 15
iOSARMActive    = 16
Javascript      = 17
AndroidARM64    = 18
AndroidX64      = 19
MacOSARM64      = 20
iOSARM64Simulator = 21

# Remappings for macros used in partfiles based on the platform.
UNIX_LIB_EXTS = {
    "$(shlibext)"       : ".so",
    "$(libext)"         : ".so",    # implicit and explicit dll linking both use *.so
    "$(stlibext)"       : ".a",
    "$(libprefix)"      : "lib",
    "$(shlibprefix)"    : "lib",
    "$(stlibprefix)"    : "lib",
    "$(oext)"           : ".o",
    "$(shlibdebugext)"  : ".so",
    "$(stlibdebugext)"  : ".a",
    "$(exeext)"         : ""
    }

WIN_LIB_EXTS = {
    "$(shlibext)"       :".dll",
    "$(libext)"         :".lib",
    "$(stlibext)"       :".lib",
    "$(libprefix)"      :"",
    "$(shlibprefix)"    :"",
    "$(stlibprefix)"    :"",
    "$(oext)"           :".obj",
    "$(shlibdebugext)"  :".pdb",
    "$(stlibdebugext)"  :".pdb",
    "$(exeext)"         :".exe"
    }

if 'BBPW' in os.environ: # DMS speciality code until they update
    WIN_LIB_EXTS["$(debugext)"] = ".pdb"
              
APPLE_LIB_EXTS = {
    "$(shlibext)"       : ".dylib",
    "$(libext)"         : ".dylib",
    "$(stlibext)"       : ".a",
    "$(libprefix)"      : "lib",
    "$(shlibprefix)"    : "lib",
    "$(stlibprefix)"    : "lib",
    "$(oext)"           : ".o",
    "$(shlibdebugext)"  : ".so",
    "$(stlibdebugext)"  : ".a",
    "$(exeext)"         : ""
    }

JS_LIB_EXTS = {
    "$(shlibext)"       : ".bc",
    "$(libext)"         : ".bc",
    "$(stlibext)"       : ".bc",
    "$(libprefix)"      : "lib",
    "$(shlibprefix)"    : "lib",
    "$(stlibprefix)"    : "lib",
    "$(oext)"           : ".o",
    "$(shlibdebugext)"  : ".bc",
    "$(stlibdebugext)"  : ".bc",
    "$(exeext)"         : ".html"
}

WIN_STATIC_LIB_EXTS = {
    "$(shlibext)"       :".lib",
    "$(libext)"         :".lib",
    "$(stlibext)"       :".lib",
    "$(libprefix)"      :"",
    "$(shlibprefix)"    :"",
    "$(stlibprefix)"    :"",
    "$(oext)"           :".obj",
    "$(shlibdebugext)"  :".pdb",
    "$(stlibdebugext)"  :".pdb",
    "$(exeext)"         :".exe"
    }
              
APPLE_STATIC_LIB_EXTS = {
    "$(shlibext)"       :".a",
    "$(libext)"         :".a",
    "$(stlibext)"       :".a",
    "$(libprefix)"      :"lib",
    "$(shlibprefix)"    :"lib",
    "$(stlibprefix)"    :"lib",
    "$(oext)"           :".o",
    "$(shlibdebugext)"  :".a",
    "$(stlibdebugext)"  :".a",
    "$(exeext)"         :""
    }

UNIX_STATIC_LIB_EXTS = {
    "$(shlibext)"       : ".a",
    "$(libext)"         : ".a",
    "$(stlibext)"       : ".a",
    "$(libprefix)"      : "lib",
    "$(shlibprefix)"    : "lib",
    "$(stlibprefix)"    : "lib",
    "$(oext)"           : ".o",
    "$(shlibdebugext)"  : ".so",
    "$(stlibdebugext)"  : ".a",
    "$(exeext)"         : ""
    }

JS_STATIC_LIB_EXTS = {
    "$(shlibext)"       : ".bc",
    "$(libext)"         : ".bc",
    "$(stlibext)"       : ".bc",
    "$(libprefix)"      : "lib",
    "$(shlibprefix)"    : "lib",
    "$(stlibprefix)"    : "lib",
    "$(oext)"           : ".o",
    "$(shlibdebugext)"  : ".bc",
    "$(stlibdebugext)"  : ".bc",
    "$(exeext)"         : ".html"
}

LKG_Future = 'future'
LKG_Obsolete = 'obsolete'
              
#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
class Platform (object):
    def __init__(self, constant, dirname, xmlname, option, lkgGroup, transkit, exe, extmapDynamic, extmapStatic):
        self.m_constant = constant
        self.m_dirname = dirname           # Directory name in output tree, LKGs, etc.  Also TARGET_PROCESSOR_ARCHITECTURE
        self.m_xmlname = xmlname           # Platform specification in XML file
        self.m_option = option             # Command Line Option
        self.m_makeTranskit = transkit     # Whether to build transkit
        self.m_useExe = exe                # Whether to build transkit
        self.m_extmapDynamic = extmapDynamic
        self.m_extmapStatic = extmapStatic
        self.m_lkgGroup = lkgGroup

    #--------------------------------------------------------------------------------
    # @bsimethod
    #--------------------------------------------------------------------------------
    def __hash__ (self):
        ''' This might be unecessary for managing lists and sets of platform. In theory I would think it speeds up the operation...'''
        # Look for scenarios where list (set ([Platform1, Platform2, ..., PlatformN]))
        return self.m_constant

    #--------------------------------------------------------------------------------
    # @bsimethod
    #--------------------------------------------------------------------------------
    def __repr__(self):
        return "Platform: constant:{0} dirname:{1} xmlname:{2} option:{3}".format (self.m_constant, self.m_dirname, self.m_xmlname, self.m_option)

    #--------------------------------------------------------------------------------
    # bsimethod
    #--------------------------------------------------------------------------------
    def __str__(self):
        return self.m_xmlname

    #--------------------------------------------------------------------------------
    # bsimethod
    #--------------------------------------------------------------------------------
    # Currently only eq is used, but created full set just in case.
    def __eq__(self, other):
        if not other:
            return False
        return (self.m_xmlname == other.m_xmlname)

    def __ne__(self, other):
        if not other:
            return True
        return (self.m_xmlname != other.m_xmlname)

    def __lt__(self, other):
        if not other:
            return False
        return (self.m_xmlname < other.m_xmlname)

    def __le__(self, other):
        if not other:
            return False
        return (self.m_xmlname <= other.m_xmlname)

    def __gt__(self, other):
        if not other:
            return False
        return (self.m_xmlname > other.m_xmlname)

    def __ge__(self, other):
        if not other:
            return True
        return (self.m_xmlname >= other.m_xmlname)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetOption (self):
        return self.m_option

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetDirName (self):
        return self.m_dirname

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetExtensionMap (self, isStatic):
        # Returns a copy because the caller modifies it.
        if  isStatic:
            if not self.m_extmapStatic:
                raise utils.BuildError ('Platform {0} does not support STATIC building'.format (self.GetXmlName()))
            return self.m_extmapStatic.copy()
            
        else:
            if not self.m_extmapDynamic:
                raise utils.BuildError ('Platform {0} does not support DYNAMIC building'.format (self.GetXmlName()))
            return self.m_extmapDynamic.copy()

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetXmlName (self):
        return self.m_xmlname

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetArchitecture (self):
        return self.m_xmlname

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def WantTranskit (self):
        return self.m_makeTranskit

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def UseEXE (self):
        return self.m_useExe

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def IsType (self, platformType):
        # This is not good because it means that the logic is not encapsulated here
        #   Currently transkit RC file build.
        #   Essentially stuff that can only happen on Windows, and has to decided between x86 and x64.
        return self.m_constant == platformType

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def IsExcluded (self, excludedList):
        if None != excludedList:
            for plat in excludedList:
                if self.m_constant == plat.m_constant:
                    return True
        return False


# Constant - used internally for comparisons and specifying a platform
# DirName - name of directory for output, saveLKGs, products
# XML - The name used in PartFile XML file to specify excludes and in MultiCompile and Installer parts to specify current platforms.
#       The same values are use in BuildStrategy XML to say which to build by default.
#       Also used as part of constants like BuildContextAndroid and ProductDirAndroid
#       And used to set TARGET_PROCESSOR_ARCHITECTURE for bmake.  
# Option - Command line option after -x.  Needed because original clever idea was -x86 and -x64, but now -xiOSX86 etc are added and we don't want to force -xx64.
# LKGGroup - When we pull LKGs, everything in the same group is pulled.  For example, Windows developers want both x86 and x64 so they can swith between them.
#            Use LKG_Future until there are plans to actually build something.  Use LKG_Obsolete for no longer used.  Better to remove them, but until you have time
#            to flush the partfiles you can mark it here.
# Transkit - Whether to generate a transkit
# Exe - whether Executables have a .EXE extension - used for BentleyBuildMake(.exe)
# ExtensionMap - Different platforms have different extensions for libraries.

#                               Constant        DirName         XML            Option          LKGGroup        Transkit    Exe     ExtensionMap      StaticExtensionMap
PlatformConstants = [ Platform (winX86,         'Winx86',       'x86',         'x86',          'win',          True,       True,   WIN_LIB_EXTS,     WIN_STATIC_LIB_EXTS   ),
                      Platform (winX64,         'Winx64',       'x64',         'x64',          'win',          True,       True,   WIN_LIB_EXTS,     WIN_STATIC_LIB_EXTS   ),
                      Platform (WinRTx86,       'WinRTx86',     'WinRTx86',    'winrtx86',     'winrt',        False,      True,   WIN_LIB_EXTS,     WIN_STATIC_LIB_EXTS   ),
                      Platform (WinRTx64,       'WinRTx64',     'WinRTx64',    'winrtx64',     'winrt',        False,      True,   WIN_LIB_EXTS,     WIN_STATIC_LIB_EXTS   ),
                      Platform (WinRTARM,       'WinRTARM',     'WinRTARM',    'winrtARM',     LKG_Future,     False,      False,  WIN_LIB_EXTS,     WIN_STATIC_LIB_EXTS   ),
                      Platform (AndroidARM,     'AndroidARM',   'AndroidARM',  'androidarm',   LKG_Obsolete,   False,      False,  UNIX_LIB_EXTS,    UNIX_STATIC_LIB_EXTS  ),
                      Platform (AndroidARM7A,   'AndroidARM7A', 'AndroidARM7A','androidarm7a', 'android',      False,      False,  UNIX_LIB_EXTS,    UNIX_STATIC_LIB_EXTS  ),
                      Platform (AndroidARM64,   'AndroidARM64', 'AndroidARM64','androidarm64', 'android',      False,      False,  UNIX_LIB_EXTS,    UNIX_STATIC_LIB_EXTS  ),
                      Platform (AndroidX64,     'AndroidX64',   'AndroidX64',  'androidx64',   'android',      False,      False,  UNIX_LIB_EXTS,    UNIX_STATIC_LIB_EXTS  ),
                      Platform (AndroidX86,     'AndroidX86',   'AndroidX86',  'androidx86',   LKG_Future,     False,      False,  UNIX_LIB_EXTS,    UNIX_STATIC_LIB_EXTS  ),
                      Platform (LinuxX86,       'LinuxX86',     'LinuxX86',    'linuxx86',     'linux',        False,      False,  UNIX_LIB_EXTS,    UNIX_STATIC_LIB_EXTS  ),
                      Platform (LinuxX64,       'LinuxX64',     'LinuxX64',    'linuxx64',     'linux',        False,      False,  UNIX_LIB_EXTS,    UNIX_STATIC_LIB_EXTS  ),
                      Platform (iOSX86,         'iOSX86',       'iOSX86',      'iosx86',       'ios',          False,      False,  APPLE_LIB_EXTS,   APPLE_STATIC_LIB_EXTS ),
                      Platform (iOSX64,         'iOSX64',       'iOSX64',      'iosx64',       'ios',          False,      False,  APPLE_LIB_EXTS,   APPLE_STATIC_LIB_EXTS ),
                      Platform (iOSARM,         'iOSARM',       'iOSARM',      'iosarm',       'ios',          False,      False,  APPLE_LIB_EXTS,   APPLE_STATIC_LIB_EXTS ),
                      Platform (iOSARM64,       'iOSARM64',     'iOSARM64',    'iosarm64',     'ios',          False,      False,  APPLE_LIB_EXTS,   APPLE_STATIC_LIB_EXTS ),
                      Platform (iOSARM64Simulator, 'iOSARM64Simulator', 'iOSARM64Simulator', 'iosarm64simulator', 'ios', False, False, APPLE_LIB_EXTS, APPLE_STATIC_LIB_EXTS ),
                      Platform (iOSARMActive,   'iOSARMActive', 'iOSARMActive','iosarmactive', 'ios',          False,      False,  APPLE_LIB_EXTS,   APPLE_STATIC_LIB_EXTS ),
                      Platform (MacOSX64,       'MacOSX64',     'MacOSX64',    'macosx64',     'macos',        False,      False,  APPLE_LIB_EXTS,   APPLE_STATIC_LIB_EXTS ),
                      Platform (MacOSARM64,     'MacOSARM64',   'MacOSARM64',  'macosarm64',   'macos',        False,      False,  APPLE_LIB_EXTS,   APPLE_STATIC_LIB_EXTS ),
                      Platform (Javascript,     'Javascript',   'Javascript',  'javascript',   'javascript',   False,      False,  JS_LIB_EXTS,      JS_STATIC_LIB_EXTS    )
                    ]
if 'BBPW' in os.environ: # DMS speciality code until they update
    PlatformConstants.append (Platform (101,     'Android',   'Android',  'android',   LKG_Obsolete,   False,      False,  UNIX_LIB_EXTS,    UNIX_STATIC_LIB_EXTS  ))
    PlatformConstants.append (Platform (102,     'Linux',     'Linux',    'linux',   LKG_Obsolete,   False,      False,  UNIX_LIB_EXTS,    UNIX_STATIC_LIB_EXTS  ))
    PlatformConstants.append (Platform (103,     'IOS',       'IOS',      'ios',   LKG_Obsolete,   False,      False,  UNIX_LIB_EXTS,    UNIX_STATIC_LIB_EXTS  ))

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def GetCommandLineOptions ():
    '''Get the full list of command line options for display in help'''
    clOpts = []
    for pconst in PlatformConstants:
        clOpts.append (pconst.m_option)
    return clOpts
    
#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def GetXmlOptions ():
    '''Get the full list of XML options for display in error messages'''
    xmlOpts = []
    for pconst in PlatformConstants:
        xmlOpts.append (pconst.m_xmlname)
    return xmlOpts

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def GetHostPlatformName ():
    # The platform to use based on the host; useful for tools
    return globalvars.defaultPlatform

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def GetHostPlatform ():
    # The platform to use based on the host; useful for tools
    return FindPlatformByXMLName ('$(hostplatform)')

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def FindPlatformByXMLName(xmlName):
    '''Get the appropriate platform object based on the value in an XML file'''
    xmlName = xmlName.lower()

    # Replace the hostPlatform macro
    if xmlName == "$(hostplatform)":
        xmlName = GetHostPlatformName().lower()

    for pconst in PlatformConstants:
        if pconst.m_xmlname.lower() == xmlName:
            return pconst
    
    return None

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def GetPlatformsFromByGroup (platformToMatch):
    # The idea is that if you are getting for one platform but want all the source for the 
    #   similar builds you should accept anything for that platform.  
    groupToMatch = platformToMatch.m_lkgGroup
    
    if groupToMatch == LKG_Obsolete:
        utils.showInfoMsg ('Pulling for obsolete platform ' + platformToMatch.GetOption() + '. Related platform LKGs will not be pulled.\n', utils.INFO_LEVEL_Interesting, utils.YELLOW)
        return [platformToMatch]
    elif groupToMatch == LKG_Future:
        utils.showInfoMsg ('Pulling for future platform ' + platformToMatch.GetOption() + '. Related platform LKGs will not be pulled.\n', utils.INFO_LEVEL_Interesting, utils.YELLOW)
        return [platformToMatch]
    
    matchedPlatforms = []
    for pconst in PlatformConstants:
        if pconst.m_lkgGroup == groupToMatch:
            matchedPlatforms.append (pconst)
    return matchedPlatforms

#--------------------------------------------------------------------------------
# @bsimethod
#--------------------------------------------------------------------------------
def GetPlatformsFromWildcard (xmlNameWithWildcard):
    '''Get list of platforms that have matching xmlNames against wildcard.'''
    matchedPlatforms = []
    for pconst in PlatformConstants:
        if utils.NameMatch (pconst.m_xmlname, xmlNameWithWildcard):
            matchedPlatforms.append (pconst)
    return matchedPlatforms

#--------------------------------------------------------------------------------
# @bsimethod
#--------------------------------------------------------------------------------
def GetPlatformsFromCSV (platformCSV, handleNoPlatformMatch=None):
    '''Get unique list of platforms that have matching xmlNames against csv of platform xmlname wildcards.'''
    platformCSV = platformCSV.strip()
    platformWildcards = [] 
    if "" != platformCSV:
        platformWildcards = platformWildcards + [platform.strip() for platform in platformCSV.split (",")]

    platforms = [] 
    for platformWildcard in platformWildcards:
        matchingPlatforms = GetPlatformsFromWildcard (platformWildcard)
        if 0 == len (matchingPlatforms) and None != handleNoPlatformMatch:
            handleNoPlatformMatch (platformCSV, platformWildcard)
        platforms.extend (matchingPlatforms)

    return list(set(platforms))

#--------------------------------------------------------------------------------
# @bsimethod
#--------------------------------------------------------------------------------
def UpdatePlatformListByName (listofPlatforms, platformCSV, handleNoPlatformMatch=None):
    validPlatforms = GetPlatformsFromCSV (platformCSV, handleNoPlatformMatch)
    listofPlatforms.extend (validPlatforms)

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def FindPlatformByName(platformName):
    '''Get the appropriate platform object based on the value passed in  on the command line'''
    platformName = platformName.lower()
    for pconst in PlatformConstants:
        if pconst.m_option.lower() == platformName:
            return pconst
    
    return None

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def ResolvePlatformList (platformString, isCmd, errorMessage):
    platformList = []
    for thisPlatform in platformString.replace(',', '+').split ('+'):
        if isCmd:
            platformObj = FindPlatformByName (thisPlatform)
            choices = GetCommandLineOptions()
        else:
            platformObj = FindPlatformByXMLName(thisPlatform)
            choices = GetXmlOptions()

        if platformObj == None:
            raise utils.StrategyError (errorMessage.format (thisPlatform, choices))

        platformList.append (platformObj)

    return platformList

#-------------------------------------------------------------------------------------------
# @bsimethod
#-------------------------------------------------------------------------------------------
def ResolvePlatform (platformNameAsArgument):
    if platformNameAsArgument != None:
        return ResolvePlatformList (platformNameAsArgument, True, "ERROR: Unknown architecture argument value '{0}'. \n '-a' or '--architecture' must be one of {1}\n")
    
    if 'BuildArchitecture' in os.environ:
        return ResolvePlatformList (os.environ['BuildArchitecture'].lower(), False, "ERROR: Unknown architecture value '{0}' provided in the BuildArchitecture environment variable. It must be one of {1}\n")

    # Added warning 20Oct2016. Should remove this block in the future and just ignore Env var.
    if 'DEFAULT_TARGET_PROCESSOR_ARCHITECTURE' in os.environ:
        utils.ShowAndDeferMessage ('DEFAULT_TARGET_PROCESSOR_ARCHITECTURE has been superceded in BentleyBuild and will be removed. Please update your environment to use BuildArchitecture instead.\n', utils.INFO_LEVEL_VeryInteresting, utils.YELLOW)
        return ResolvePlatformList (os.environ['DEFAULT_TARGET_PROCESSOR_ARCHITECTURE'].lower(), False, "ERROR: Unknown architecture value '{0}' provided in the DEFAULT_TARGET_PROCESSOR_ARCHITECTURE environment variable. It must be one of {1}\n")
    
    return globalvars.buildStrategy.m_defaultPlatform

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def GetPlatformByValue (val):
    '''Get a specific platform; this is usually used for a default.'''
    for pconst in PlatformConstants:
        if pconst.m_constant == val:
            return pconst
    return None

#-------------------------------------------------------------------------------------------
# We seem to need to list all the platform directories in the environment for some reason??
#  This will spit out stuff like BuildContextX64, BuildContextAndroid or ProductDirX86
# bsimethod
#-------------------------------------------------------------------------------------------
def CreateAllEnvironmetVariables (prefix, dirMethod, localEnv):
    for pconst in PlatformConstants:
        utils.appendEnvVar (localEnv, prefix + pconst.GetXmlName(), dirMethod(pconst)+os.sep)
