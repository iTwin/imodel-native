#---------------------------------------------------------------------------------------------
#  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
#  See LICENSE.md in the repository root for full copyright notice.
#---------------------------------------------------------------------------------------------
import json, os
from . import buildpaths, cmdutil, globalvars, utils

NPM_PACKAGELOCK_FILENAME = 'package-lock.json'
NPM_NODE_MODULES = 'node_modules'

s_packageInfo = {}

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def DoSyncpull (syncPullTreeRoot, projectLocation):
    projectLocation = os.path.expandvars (projectLocation)
    # Subtract out source root
    sourceRoot = os.path.expandvars ('${SrcRoot}')
    projectTail = projectLocation[len(sourceRoot):]

    utils.showInfoMsg('Syncpulling NPM using relative directory: {0}'.format(projectTail), utils.INFO_LEVEL_SomewhatInteresting)

    remotePackageDir = os.path.join (syncPullTreeRoot, projectTail, NPM_NODE_MODULES)
    if not os.path.exists(remotePackageDir):
        raise utils.BuildError ("Unable to find remote directory NPM package directory: '{0}'.".format (remotePackageDir))

    wasMod = cmdutil.roboCopyDirCheckModified (remotePackageDir, os.path.join(projectLocation, NPM_NODE_MODULES))
    return [projectLocation] if wasMod else []

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def GetNpmExe ():
    newNodeName = 'Nodejs14'
    if globalvars.buildStrategy.HasLocalRepository (newNodeName):
        localRepo = globalvars.buildStrategy.FindLocalRepository (newNodeName)
    else:
        localRepo = globalvars.buildStrategy.FindLocalRepository ('Nodejs')  # Fallback for older branches.
    if os.name == 'nt':
        return os.path.join (os.path.expandvars (localRepo.GetLocalDir()), 'bentley', 'npm.cmd')
    elif os.name == 'posix':
        return os.path.join (os.path.expandvars (localRepo.GetLocalDir()), 'bentley', 'npm.sh')

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def GetFromServer (projectLocation):
    projectLocation = os.path.expandvars (projectLocation)
    # For Node 15
#    pkgLineRe = re.compile(r'npm timing reifyNode:(node_modules/)+(.*) Completed.*')
    projDir = projectLocation[len(buildpaths.GetSrcRoot())+1:]
    projDir = projDir.replace ('\\', '/')
    projDir = projDir+'/' if not projDir.endswith('/') else projDir

    # Verify that a shrinkwrap file was provided
    if not os.path.exists (os.path.join(projectLocation, NPM_PACKAGELOCK_FILENAME)):
        raise utils.BuildError ("There is no {1} in {0}.".format (projectLocation, NPM_PACKAGELOCK_FILENAME))

    if 'BentleyBuild_NoNpmPull' in os.environ:
        return []

    npmOutputLines = [] # Store these in case there's an error
    def processOutput (outputLine):
        outputLine = outputLine.rstrip()
        npmOutputLines.append (outputLine)
        utils.showInfoMsg(outputLine+'\n', utils.INFO_LEVEL_SomewhatInteresting)

    # CI has the advantage of using the currnt package file (not making updates to your source) but the disadvantage of doing a clean pull every time.
    # Install does an incremental pull, but checks the server for each package to see if there is a new version and updates the package-lock file so
    #   git shows a changed file.
    baseCommand = 'install' if 'BentleyBuild_NoNpmCI' in os.environ else 'ci'
    utils.showInfoMsg('Running NPM {1} command in directory {0}\n'.format (projectLocation, baseCommand), utils.INFO_LEVEL_Essential)
    
    # The --scripts-prepend-node-path option is needed for agents and other places where NPM isn't installed.
    npmFeedAddress = globalvars.buildStrategy.GetNpmFeedDefault().GetRemoteAddress()
    registry_option = r'--registry={}'.format(npmFeedAddress)
    utils.showInfoMsg('using NPM registry option {0}\n'.format (registry_option), utils.INFO_LEVEL_RarelyUseful)
    cmd = [GetNpmExe(), registry_option, baseCommand, '-d', '--scripts-prepend-node-path=true']

    numRetries = 5 if utils.isPrg() else 2  # One retry for a developer, but ok to do more when doing a ci
    retries = numRetries
    while retries > 0:
        retries -= 1
        utils.showInfoMsg('NPM {0} try {1} of {2}\n'.format (baseCommand, numRetries-retries, numRetries), utils.INFO_LEVEL_Interesting)
        status = cmdutil.runAndWait (cmd, outputProcessor=processOutput, workingDirectory=projectLocation)
        if not status:
            break
    if status:
        raise utils.BuildError ("Failed installing NPM package: '{0}'.\n{1}".format (projectLocation, '\n'.join(npmOutputLines)))

    # The rest of this is checking what if anything changed. For now I'm going to take a lazy approach and look to see if there
    # are more than one linkStuff lines; if there are then subpackages were pulled. We could actually validate the name
    # of the local package in the $(projectLocation)/package.json file, but that will be slower and may be unnecessary

    utils.showInfoMsg('Checking if any NPM items changed', utils.INFO_LEVEL_RarelyUseful)
    pkgs = set()
    for npmLine in npmOutputLines:
        # npm info linkStuff angular@1.6.4
        lsIndex = npmLine.find ('linkStuff')
        if lsIndex >= 0:
            pkgs.add (npmLine[lsIndex+9:].strip())
        # npm from node 14:  npm info lifecycle pify@2.3.0~preinstall: pify@2.3.0
        lsIndex = npmLine.find ('lifecycle')
        if lsIndex >= 0:
            pkgs.add (projDir + npmLine[lsIndex+19:].strip())
        # npm from node 15
        # npm timing reifyNode:node_modules/get-caller-file Completed in 8957ms
#        match = pkgLineRe.match(npmLine)
#        if match:
#            pkgs.add (projDir + match.group(2))
            
    utils.showInfoMsg('All changed NPM packages: {0}'.format(repr(pkgs)), utils.INFO_LEVEL_RarelyUseful)
        
    if len(pkgs) > 1:  # Was anything updated?
        return list(pkgs)
    else:
        return []

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def _ReadDependencies (depList, dictToCheck):
    if not 'dependencies' in dictToCheck:
        return
        
    for name, dependent in dictToCheck['dependencies'].items():
        depList.append ((name, dependent.get('resolved', 'missing'), dependent.get('version', 'missing'), dependent.get('dev', False)))
        _ReadDependencies (depList, dependent)

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def GetListOfPackages (projectLocation):
    # Parse the shrinkwrap json and get all the packages that it contains.
    projectLocation = os.path.expandvars (projectLocation)
    srFileName = os.path.join(projectLocation, NPM_PACKAGELOCK_FILENAME)
    utils.showInfoMsg('Reading NPM packages from : {0}'.format(srFileName), utils.INFO_LEVEL_RarelyUseful)
    
    with open(srFileName, 'rt') as srFile:
        srData = json.load(srFile)

    foundDeps = []
    _ReadDependencies (foundDeps, srData)
    return foundDeps

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def _makeKey (name, version):
    return name + '|' + version

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def GetAllProjectPackageInfo (projectLocation):
    # We want to get the license for each of these packages from the package.json. However, it turns out
    #   that we have no idea where the package.json is for any given package; it may be at the top level
    #   or it may be nested deeply under one of the other packages. So the idea is to walk all the package.json's
    #   and accumulate all the license information up front
    projectLocation = os.path.expandvars (projectLocation)
    jsonFiles = []
    for root, _, filenames in os.walk(projectLocation):
        if 'package.json' in filenames:
            jsonFiles.append(os.path.join(root, 'package.json'))

    jsonPackages = {}
    for jfileName in jsonFiles:
        with open(jfileName, "rt") as jfile:
            try:
                fullJson = json.load (jfile, encoding='utf-8')
            except:
                continue    # Bad json files; nothing I can do.
        # Currently there is no copyright or license link provided.
        # If we need more, we can store the whole json file. Kept their names to make that easy.
        if 'license' in fullJson: # Internal packages tend not to have License.
            licString = fullJson.get ('license', None)
            if isinstance(licString, dict):  # Sometimes it's a dictionary (maybe old style?)
                licString = licString.get ('type', None)
            licInfo = {'name':fullJson['name'], 'version':fullJson['version'], 'license':licString, 'homepage':fullJson.get ('homepage', None)}
            jsonPackages[_makeKey(licInfo['name'], licInfo['version'])] = licInfo

    return jsonPackages
    
#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def GetPackageInfoForProject (name, version, projectLocation):
    if not projectLocation in s_packageInfo:
        s_packageInfo[projectLocation] = GetAllProjectPackageInfo (projectLocation)
    return s_packageInfo[projectLocation].get (_makeKey(name, version), None)

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def GetLicenseInfo (name, version, projectLocation):
    licInfo = GetPackageInfoForProject (name, version, projectLocation)
    if licInfo:
        return licInfo['homepage'], licInfo['license'], None, None
    return None, None, None, None

