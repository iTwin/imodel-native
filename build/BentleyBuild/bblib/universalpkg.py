#---------------------------------------------------------------------------------------------
#  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
#  See LICENSE.md in the repository root for full copyright notice.
#---------------------------------------------------------------------------------------------
import os
from . import symlinks, utils, versionutils, azurecli

UPackFeedTypes   = ['azurecli', 'fileshare']
UPACKFEED_azurecli      = UPackFeedTypes.index ('azurecli')     # Use Azure CLI
UPACKFEED_fileshare     = UPackFeedTypes.index ('fileshare')    # Use a symlink

#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
class UniversalPackage (object):
    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def __init__(self, pkgSource, upackFeed):
        self.m_pkgSource = pkgSource
        self.m_upackFeed = upackFeed

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetFeed (self):
        return self.m_upackFeed

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def ResolveVersionLocal(self, ignoreErrors=False):
        # This one is called during build.
        # If there's a wildcard in the version we want to resolve it so that we can link to the correct local copy.
        if '*' in self.m_pkgSource.m_version:
            _, version, _, _ = self.m_pkgSource.GetProvenance()
            if not version:
                if ignoreErrors:
                    return
                else:
                    raise utils.BuildError ('Unable to ascertain Upack Provenance for {0} {1}; was the package downloaded?'.format(self.m_pkgSource.m_name, self.m_pkgSource.m_version))
            self.m_pkgSource.m_version = version

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def BuildAction (self, thisPart):
        outputBuildContext = thisPart.GetOutputBuildContextDir(thisPart.m_info.m_buildContext, thisPart.IsStatic())
        utils.showInfoMsg ("Connecting Upack Package '{0}' to context '{1}'\n".format (self.m_pkgSource.m_name, outputBuildContext), utils.INFO_LEVEL_SomewhatInteresting)
        self.ResolveVersionLocal ()
        sourcePath = self.m_pkgSource.GetLocalPath()

        linkPath = os.path.join (outputBuildContext, 'upack', self.m_pkgSource.m_name)
        utils.showInfoMsg ("Linking Upack link '{0}' to source '{1}'\n".format (linkPath, sourcePath), utils.INFO_LEVEL_RarelyUseful)
        thisPart.PerformBindToDir (linkPath, sourcePath, checkSame=False, checkTargetExists=True, skipIntermediateLinks=True)

#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
class RemoteUniversalPackage (UniversalPackage):
    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def __init__(self, pkgSource, upackFeed):
        UniversalPackage.__init__ (self, pkgSource, upackFeed)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetBaseCmd (self, operation):
        upackFeed = self.GetFeed()

        return [azurecli.GetAzureCliPath(), 'artifacts', 'universal', 
                operation,
               '--organization', upackFeed.m_address,
               '--feed', upackFeed.m_feed,
               '--name', self.m_pkgSource.m_name.lower(),  # Universal package feed only accepts lowercase.
               '--version', self.m_pkgSource.m_version,
               '-o', 'json'
               ]

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetAllVersions (self, retries):
        def getUpackWithName (resultJson, name):
            for pkg in resultJson['value']:
                if pkg['normalizedName'].lower() == name.lower():
                    return pkg
            return None

        pkgVersions = []
        retry = 0
        upackFeed = self.GetFeed()
        while retry < retries:
            retry += 1
            utils.showInfoMsg ("Getting all versions of {0}. Retry {1} of {2}.\n".format (self.m_pkgSource.m_name, retry, retries),utils.INFO_LEVEL_SomewhatInteresting)

            # az devops invoke -o json --api-version 5.1-preview
            #     --organization https://dev.azure.com/myorg --area Packaging --resource Packages --route-parameters feedId=feedname 
            #     --query-parameters includeAllVersions=true includeUrls=false packageNameQuery=png protocolType=UPack
            cmd = [azurecli.GetAzureCliPath(), 'devops', 'invoke', '-o', 'json', '--api-version', '5.1-preview']
            cmd += ['--organization', upackFeed.m_address]
            cmd += ['--area', 'Packaging', '--resource', 'Packages']
            cmd += ['--route-parameters', 'feedId={0}'.format(upackFeed.m_feed)]
            cmd += ['--query-parameters', 'includeAllVersions=true', 'includeUrls=false', 'packageNameQuery={0}'.format(self.m_pkgSource.m_name), 'protocolType=UPack']
            status, resultJson, _ = azurecli.RunAzureCliCmdJson (cmd)
            if status == 0:
                # query returns not based on the exact name but packages that contain the string (might be that the ones only start with the string) so we have to get the exact one
                if resultJson['count'] > 0:
                    pkgDefinition = getUpackWithName(resultJson, self.m_pkgSource.m_name)
                    if not pkgDefinition:
                        break
                    for pkg in pkgDefinition['versions']:
                        pkgVersions.append(versionutils.VersionWithSuffix(pkg['version']))
                break

        return pkgVersions

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetLatestVersion(self):
        allVersions = self.GetAllVersions(5)
        allVersions = sorted(allVersions)
        return allVersions[-1]

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def ResolveVersion (self, pullToRev, retries):
        # This one is called during pull.
        # If there's a wildcard in the version we want to resolve it so that we don't have to repull a large package.
        pkgReqVer = pullToRev if pullToRev else self.m_pkgSource.m_origVersion
        if not '*' in pkgReqVer:
            return
        requestedVersion = versionutils.VersionWithSuffix(pkgReqVer)
        pkgVersions = sorted(self.GetAllVersions (retries), reverse=True)
        if not pkgVersions:
            utils.showInfoMsg ('Upack found no versions for {} with specified orig version {} (requested {})\n'.format (self.m_pkgSource.m_name, self.m_pkgSource.m_origVersion, pkgReqVer), utils.INFO_LEVEL_Important, utils.YELLOW)
            return
        for ver in pkgVersions:
            utils.showInfoMsg ('Upack {0} Found Version {1}\n'.format (self.m_pkgSource.m_name, ver),utils.INFO_LEVEL_SomewhatInteresting)
        for ver in pkgVersions:
            if requestedVersion.MatchVersion(ver):
                self.m_pkgSource.m_version = str(ver)
                return

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def Download (self, retries):
        retry = 0
        while retry < retries:
            utils.showInfoMsg ("Starting download of Upack {0}. Retry {1} of {2}.\n".format (self.m_pkgSource.m_name, retry, retries),utils.INFO_LEVEL_SomewhatInteresting)
            retry += 1
            status, returnedStrings = self._Download()
            if status == 0:
                break
            else:
                utils.showInfoMsg ("\n\nDownloading Upack {0} failed; {1} retries remain. Returned from azure CLI: \n".format (self.m_pkgSource.m_name, retries-retry),utils.INFO_LEVEL_Important, utils.YELLOW)
                utils.showInfoMsg (returnedStrings, utils.INFO_LEVEL_Important, utils.YELLOW)
        return status

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def _Download (self):
        returnedStrings = []
        hasVersionWildcard = '*' in self.m_pkgSource.m_version

        initialPath = self.m_pkgSource.GetLocalPath()
        cmd = self.GetBaseCmd ('download')
        cmd.extend (['--path', initialPath])
        
        status, resultJson, returnedStrings = azurecli.RunAzureCliCmdJson (cmd)
        if status == 0:
            # Store the downloaded version which is returned as json. However, a lot of other stuff can flood in as well so have to pick out the right lines.
            if 'Version' in resultJson:
                self.m_pkgSource.m_version = resultJson['Version']

            if hasVersionWildcard:
                if '*' in self.m_pkgSource.m_version:
                    raise utils.BuildError ('Upack unable to extract a version from {0} got {1}\n'.format(self.m_pkgSource.m_name, self.m_pkgSource.m_version))
                os.rename (initialPath, self.m_pkgSource.GetLocalPath())
            
        return status, ''.join(returnedStrings)

        # az artifacts universal download
        # --organization "https://dev.azure.com/myorg/"
        # --feed "my-feed-name"
        # --name internalsdk
        # --version 10.14.0-10
        # --path d:\code\source\nuget\universal
        
        # {
        #   "Description": "V8_InternalSDKNuget",
        #   "ManifestId": "ABCD1234ABCD1234ABCD1234ABCD1234ABCD1234ABCD1234ABCD1234ABCD1234AB",
        #   "SuperRootId": "EFGH5678EFGH5678EFGH5678EFGH5678EFGH5678EFGH5678EFGH5678EFGH5678EF",
        #   "Version": "10.14.0-10"
        # }

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def Upload (self, pathToUpload, description, retries=5):
        retry = 0
        while retry < retries:
            utils.showInfoMsg ("Starting upload of Upack {0}. Retry {1} of {2}.\n".format (self.m_pkgSource.m_name, retry, retries),utils.INFO_LEVEL_SomewhatInteresting)
            retry += 1
            status, returnedStrings = self._Upload(pathToUpload, description)
            if status == 0:
                break
            else:
                utils.showInfoMsg ("Uploading Upack {0} failed; {1} retries remain. Returned from azure CLI:\n".format (self.m_pkgSource.m_name, retries-retry),utils.INFO_LEVEL_Important, utils.YELLOW)
                utils.showInfoMsg (returnedStrings, utils.INFO_LEVEL_Important, utils.YELLOW)
        return status, returnedStrings

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def _Upload (self, pathToUpload, description):
        utils.showInfoMsg ('Publishing UPackProduct: {0} {1} "{2}"\n'.format(self.m_pkgSource.m_name.lower(), self.m_pkgSource.m_version, description), utils.INFO_LEVEL_Interesting)
        cmd = self.GetBaseCmd ('publish')
        cmd.extend (['--path', pathToUpload, '--description', description])
        status, _, fullOutput = azurecli.RunAzureCliCmdJson (cmd)
        return status, ''.join(fullOutput)

        # az artifacts universal publish
        # --organization "https://dev.azure.com/myorg/"
        # --feed "my-feed-name"
        # --name "my-package-name"  (All lowercase only)
        # --version "10.15.0"
        # --description "My description"
        # --path "d:\code\out\win64\upacks\internalsdk"
        
        # {
        #   "ManifestId":  "ABCD1234ABCD1234ABCD1234ABCD1234ABCD1234ABCD1234ABCD1234ABCD1234AB",
        #   "SuperRootId": "EFGH5678EFGH5678EFGH5678EFGH5678EFGH5678EFGH5678EFGH5678EFGH5678EF"
        # }


#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
class LocalUniversalPackage (UniversalPackage):
    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def __init__(self, pkgSource, upackFeed):
        UniversalPackage.__init__ (self, pkgSource, upackFeed)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def Upload (self, pathToUpload, _description):
        utils.ShowAndDeferMessage("Not uploading local UPack: {0}\n".format(pathToUpload), utils.INFO_LEVEL_Essential, utils.YELLOW)
        return 0, ''

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def ResolveVersion(self, _pullToRev, _):
        # Can't write with '*' in version, so replace it with 99.99 for local
        if '*' in self.m_pkgSource.m_version:
            versionSplit = self.m_pkgSource.m_version.split('.')
            versionTuple = ['99','99','99','99']
            for iVer in range(4):
                if len(versionSplit) > iVer and not '*' in versionSplit[iVer]:
                    versionTuple[iVer] = versionSplit[iVer]
                else:
                    break
            version = '.'.join(versionTuple)
            self.m_pkgSource.m_version = version

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def Download (self, _retries):
        upackFeed = self.GetFeed()
        sourceDir = os.path.join (upackFeed.m_address, self.m_pkgSource.m_name)
        if not os.path.isdir(sourceDir):
            raise utils.BuildError ('Local UPack directory "{0}" does not exist'.format(sourceDir))
        linkDir = self.m_pkgSource.GetLocalPath()
        symlinks.createDirSymLink (linkDir, sourceDir, checkSame=True, checkTargetExists=True, skipIntermediateLinks=True)
        return 0

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def GetUniversalPackage (pkgSource, upackFeed):
    if upackFeed.m_type == UPACKFEED_fileshare:
        return LocalUniversalPackage (pkgSource, upackFeed)
    else:
        return RemoteUniversalPackage (pkgSource, upackFeed)
