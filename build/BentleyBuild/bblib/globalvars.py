#---------------------------------------------------------------------------------------------
#  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
#  See LICENSE.md in the repository root for full copyright notice.
#---------------------------------------------------------------------------------------------

import platform, os, sys

# Global constants


# These are variables that are referenced from multiple modules

# The build strategy that will be used
buildStrategy = None

# Build errors that have occurred
buildErrors = []

# Warnings that we want to dump at the end, to make sure they are noticed.
# These are different than buildErrors in that they won't make the build fail
# It is for things we want the user to see at the end of the build, but not stop the build for.
deferredMessages = []

currentAction = None

LIB_TYPE_Parent = 'Parent'
LIB_TYPE_Dynamic = 'Dynamic'
LIB_TYPE_Static = 'Static'
libtypeOptions = [LIB_TYPE_Parent, LIB_TYPE_Dynamic, LIB_TYPE_Static]

CREDENTIAL_PROVIDER_BASIC = 0
CREDENTIAL_PROVIDER_TOKEN = 1
CREDENTIAL_PROVIDER_MCP   = 2
CREDENTIAL_PROVIDER_AZ    = 3
CREDENTIAL_PROVIDER_AUTO  = 4
CredentialProviderTypes   = ["basic", "token", "microsoftcredentialprovider", "azurecli", "auto"]

# Force Azure CLI to be used when authentication is required
useAzureCliAuth = True if os.environ.get('BB_AUTH_USING_AZURE_CLI', None) else False

# repositories that should not be synced
skipPull = os.environ.get('BB_SKIP_PULL', "").lower().split(';')

# Provenance log file suffix
provfileName = "_provenance.log"
installerWixDefintionFile = ".wix.xml"

# The options passed in from the command line
programOptions = None

# The strategy file currently being processed
currentStrategyFile = None

# The current stream
streamName = None

# The version of bb at startup; used to clear caches.
bbVersion = None

# Use bogus version number if we can't extract it.  999 is intentional; with 8.64 already in the pipeline, 99 may not be high enough.
defaultVersion = ('9', '999', '99', '99')

BUILD_OPTION_Always = 0
BUILD_OPTION_Never  = 1
BUILD_OPTION_Once   = 2
buildOptions = ["Always", "Never", "Once"]

# Part scope
PART_SCOPE_Public = 0
PART_SCOPE_Private = 1
partScopeOptions = ["public", "private"]

def IsMacArm ():
    return 'darwin' == sys.platform and 'arm' in platform.machine()

def IsMacX64 ():
    return 'darwin' == sys.platform and 'x86_64' in platform.machine()

# API number is used in partfiles. 
API_NUMBER                  = "ApiNumber"
API_NUMBER_MACRO            = '$(' + API_NUMBER + ')'

# Types of sources
REPO_HG          = 'hg'
REPO_CVS         = 'cvs'
REPO_RSYNC       = 'rsync'
REPO_GIT         = 'git'
REPO_NPM         = 'npm'
REPO_NUGET       = 'nuget'
REPO_SDK         = 'sdk'
REPO_UPACK       = 'upack'
REPO_ADOBUILDARTIFACT = 'adobuildartifact'

repoTypes        = [REPO_HG, REPO_CVS, REPO_RSYNC, REPO_GIT, REPO_UPACK]
sourceTypes      = repoTypes + [REPO_NPM, REPO_NUGET, REPO_SDK, REPO_ADOBUILDARTIFACT]

# We always check these, even if they aren't referenced explicitly by the build strategy
COMMON_DEFAULT_REPOS = [ "bentleybuild", "bsicommon",  "buildstrategies"]
WIN_DEFAULT_REPOS   = COMMON_DEFAULT_REPOS
APPLE_DEFAULT_REPOS = COMMON_DEFAULT_REPOS
APPLEARM_DEFAULT_REPOS = COMMON_DEFAULT_REPOS
LINUX_DEFAULT_REPOS = COMMON_DEFAULT_REPOS

TOOLS_BMAKE = 'bmake'
TOOLS_BSISCRIPTS = 'bsiscripts'
TOOLS_BSITOOLS = 'bsitools'
TOOLS_CWRSYNC = 'cwrsync'
TOOLS_SIGN_TOOL_CLIENT = 'SignToolClient'
TOOLS_NUGET = 'Nuget.Commandline'

# (name, type) 
MULTI_PLATFORM_TOOLS = [(TOOLS_BMAKE, REPO_UPACK)]
if 'BSI' in os.environ:
    MULTI_PLATFORM_TOOLS = MULTI_PLATFORM_TOOLS + [(TOOLS_BSISCRIPTS, REPO_GIT)]

WIN_TOOLS = MULTI_PLATFORM_TOOLS + [(TOOLS_BSITOOLS, REPO_UPACK)] 
if 'BSI' in os.environ:
    WIN_TOOLS = WIN_TOOLS + [(TOOLS_CWRSYNC, REPO_UPACK), (TOOLS_SIGN_TOOL_CLIENT, REPO_NUGET), (TOOLS_NUGET, REPO_NUGET)] # Would prefer NuGet only when NuGetProduct specified, but would need to define a bogus part so for now everyone gets it.
APPLE_TOOLS = MULTI_PLATFORM_TOOLS
APPLEARM_TOOLS = MULTI_PLATFORM_TOOLS
LINUX_TOOLS = MULTI_PLATFORM_TOOLS

# Platform names that can act as hosts
WIN_HOST = 'x64'
APPLE_HOST = 'MacOSX64'
APPLEARM_HOST = 'MacOSX64'
LINUX_HOST = 'LinuxX64'
HOST_PLATFORMS = [WIN_HOST, LINUX_HOST, APPLE_HOST, APPLEARM_HOST]  

if IsMacArm():
    defaultRepos = APPLEARM_DEFAULT_REPOS
    defaultTools = APPLEARM_TOOLS
    defaultPlatform = APPLEARM_HOST
elif IsMacX64():
    defaultRepos = APPLE_DEFAULT_REPOS
    defaultTools = APPLE_TOOLS
    defaultPlatform = APPLE_HOST
elif 'linux2' == sys.platform or 'linux' == sys.platform: # Former is py2, latter is py3
    defaultRepos = LINUX_DEFAULT_REPOS
    defaultTools = LINUX_TOOLS
    defaultPlatform = LINUX_HOST
else:
    defaultRepos = WIN_DEFAULT_REPOS
    defaultTools = WIN_TOOLS
    defaultPlatform = WIN_HOST
