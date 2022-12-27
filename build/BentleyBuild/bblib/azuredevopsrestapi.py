#---------------------------------------------------------------------------------------------
#  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
#  See LICENSE.md in the repository root for full copyright notice.
#---------------------------------------------------------------------------------------------
import gzip, os, json, zipfile, gzip, sys, datetime, time, shutil
from . import compat, internal, utils, versionutils

py3 = (int(sys.version[0]) > 2)

# Python 2/3 compatability
if py3:
    from urllib.request import urlopen, Request
    from urllib.error import HTTPError, URLError
else:
    from future.moves.urllib.request import urlopen, Request
    from future.moves.urllib.error import HTTPError, URLError

SCOPE_PACKAGING_WRITE = "vso.packaging_write vso.drop_write"

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def GetSessionToken (authHeader, organization, scope):
    # Undocumented API and the workflow is stolen from Microsoft Credential Provider (https://github.com/microsoft/artifacts-credprovider)
    postSessionTokenUrl = "{}/{}/_apis/Token/SessionTokens?tokenType=SelfDescribing&api-version=6.1-preview.1".format(internal.AZURE_DEVOPS_SPS_SERVER, organization)
    currentTime = datetime.datetime.utcnow()
    validTo = currentTime + datetime.timedelta(hours=1) # Issue tokens for an hour since this is meant for a single session (request) so more shouldn't be required

    postContent = "{{\"displayName\":\"Issued from BentleyBuild\",\"scope\":\"{0}\",\"validTo\":\"{1}\"}}".format(scope, validTo.strftime('%Y-%m-%dT%H:%M:%SZ'))
    utils.showInfoMsg ("Requesting Session Token from Azure DevOps with the following content: {0}\n".format(postContent), utils.INFO_LEVEL_RarelyUseful)

    headers = {}
    headers ['Content-Type'] = "application/json"
    headers ['Authorization'] = authHeader
    sleepTime = 0.5
    attempt = 0
    while attempt < 5:
        attempt += 1

        try:
            request = Request(postSessionTokenUrl, postContent, headers)
            httpResponse = urlopen(request)
            httpCode = httpResponse.getcode()
            if 200 == httpCode:
                httpContent = httpResponse.read()
                break
            time.sleep (sleepTime)
            sleepTime = sleepTime + 1 if sleepTime < 7 else sleepTime # Try sleepging a little longer each time, but not too long.

        except HTTPError as e:
            if 401 == e.code:
                raise utils.BuildError ('Unauthorized to create a session token.\n')
            httpCode = e.code # May retry.
        except URLError:
            raise utils.BuildError ('Connection refused.\n')

    if httpCode != 200:
        raise utils.BuildError ('Failed to acquire a session token with scope: {0}. Return code: {1}\n'.format(scope, httpCode))

    sessionToken = json.loads(httpContent)
    utils.showInfoMsg ("Session Token acquired successfully.\n", utils.INFO_LEVEL_RarelyUseful)
    return sessionToken["token"]

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def GetBuildArtifactBdfFilePath (name):
    # We expect that the BDF files will be stored under the bdf build artifact
    return "bdf/{}.xml".format(name)

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def GetBuildArtifactLkgPath (platform, context, isStatic = False):
    # We expect that the LKG files will be stored under lkg build artifact seperated out by platform and context in sub-directories
    if isStatic:
        return "lkg/{}/static/{}".format(platform, context)
    return "lkg/{}/{}".format(platform, context)

s_buildDefinitions = {}
#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def GetBuildDefinitionId (authHeader, organization, project, build):
    #
    # https://docs.microsoft.com/en-us/rest/api/azure/devops/build/definitions/list?view=azure-devops-rest-5.1
    #
    url = "{}/{}/{}/_apis/build/definitions?name={}&api-version=5.1".format(internal.AZURE_DEVOPS_SERVER, organization, project, compat.quote(build))
    co_name = sys._getframe().f_code.co_name # pylint: disable=protected-access
    utils.showInfoMsg('{} url={} organization={} project={} build={}\n'.format(co_name, url, organization, project, build), utils.INFO_LEVEL_RarelyUseful)
    cache_key = '/'.join([organization.lower(), project.lower(), build])
    httpContent = s_buildDefinitions.get(cache_key, None)
    if httpContent:
        utils.showInfoMsg('{} using build definition from cache\n'.format(co_name), utils.INFO_LEVEL_RarelyUseful)
    else:
        httpCode, httpContent = utils.getUrlWithDataWithRetries(url, None, {}, authHeader)
        if httpCode != 200:
            raise utils.BuildError ('Failed to find build definition id for build {0}. Return code: {1}\n'.format(build, httpCode))
        s_buildDefinitions[cache_key] = httpContent

    buildDefinitions = json.loads (httpContent)
    if buildDefinitions["count"] != 1:
        raise utils.BuildError ('Expected 1 build definition for build {0}, but found {1}. The build name might be incorrect.\n'.format(build, buildDefinitions["count"]))

    definitionId = buildDefinitions["value"][0]["id"]
    utils.showInfoMsg ('Found build definition id {0} for build name {1}.\n'.format (definitionId, build), utils.INFO_LEVEL_SomewhatInteresting)
    return definitionId

s_buildInfos = {}
#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def GetBuildInfo (authHeader, organization, project, buildDefinitionId, buildNumber, retainedByRelease):
    #
    # https://docs.microsoft.com/en-us/rest/api/azure/devops/build/builds/list?view=azure-devops-rest-5.1
    #
    if not buildNumber:
        buildNumber = "*"

    url = "{}/{}/{}/_apis/build/builds?buildNumber={}&resultFilter=succeeded,partiallySucceeded&definitions={}&api-version=5.1".format(internal.AZURE_DEVOPS_SERVER, organization, project, buildNumber, buildDefinitionId)
    co_name = sys._getframe().f_code.co_name # pylint: disable=protected-access
    utils.showInfoMsg('{} url={} organization={} project={} buildDefinitionId={} buildNumber={}\n'.format(co_name, url, organization, project, buildDefinitionId, buildNumber), utils.INFO_LEVEL_RarelyUseful)
    cache_key = '/'.join([organization.lower(), project.lower(), str(buildDefinitionId), buildNumber])
    httpContent = s_buildInfos.get(cache_key, None)
    if httpContent:
        utils.showInfoMsg('{} using build info from cache\n'.format(co_name), utils.INFO_LEVEL_RarelyUseful)
    else:
        httpCode, httpContent = utils.getUrlWithDataWithRetries(url, None, {}, authHeader)
        if httpCode != 200:
            raise utils.BuildError ('Failed to get build id for buildDefinitionId {0} and version {1}. Return code: {2}\n'.format(buildDefinitionId, buildNumber, httpCode))
        s_buildInfos[cache_key] = httpContent

    builds = json.loads (httpContent)
    buildDict = {}
    for build in builds['value']:
        buildDict[(versionutils.VersionWithSuffix(build["buildNumber"]), build["id"])] = build
    sortedKeys = sorted(buildDict, reverse=True)

    if builds["count"] == 0:
        raise utils.BuildError ('No builds could be found for build definition {0} with version {1}.\n'.format(buildDefinitionId, buildNumber))

    if not retainedByRelease == 'true':
        utils.showInfoMsg ('Found {0} builds that match definition id {1} and version "{2}"\n'.format (builds["count"], buildDefinitionId, buildNumber), utils.INFO_LEVEL_SomewhatInteresting)
        return buildDict[sortedKeys[0]]


    for key in sortedKeys:
        if buildDict[key]["retainedByRelease"] == True:
            utils.showInfoMsg ('Found {0} builds that match definition id {1} and version "{2}" that is retained by release\n'.format (builds["count"], buildDefinitionId, buildNumber), utils.INFO_LEVEL_VeryInteresting)
            return buildDict[key]

    raise utils.BuildError ('No builds could be found for build definition {0} with version {1} that are retained by release.\n'.format(buildDefinitionId, buildNumber))

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def GetBuildInfoFromBuildId (authHeader, organization, project, buildId):
    #
    # https://docs.microsoft.com/en-us/rest/api/azure/devops/build/builds/get?view=azure-devops-rest-5.1
    #
    if not buildId:
        raise utils.BuildError('Specified build Id is empty or None')

    url = "{}/{}/{}/_apis/build/builds/{}?api-version=5.1".format(internal.AZURE_DEVOPS_SERVER, organization, project, buildId)
    co_name = sys._getframe().f_code.co_name # pylint: disable=protected-access
    utils.showInfoMsg('{} url={} organization={} project={} buildId={}\n'.format(co_name, url, organization, project, buildId), utils.INFO_LEVEL_RarelyUseful)
    cache_key = '/'.join([organization.lower(), project.lower(), str(buildId)])
    httpContent = s_buildInfos.get(cache_key, None)
    if httpContent:
        utils.showInfoMsg('{} using build info from cache\n'.format(co_name), utils.INFO_LEVEL_RarelyUseful)
    else:
        httpCode, httpContent = utils.getUrlWithDataWithRetries(url, None, {}, authHeader)
        if httpCode != 200:
            raise utils.BuildError ('Failed to get build info for build id {0}. Return code: {1}\n'.format(buildId, httpCode))
        s_buildInfos[cache_key] = httpContent

    build = json.loads (httpContent)
    return build

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def GetBuildId (authHeader, organization, project, buildDefinitionId, buildNumber, retainedByRelease):
    #
    # https://docs.microsoft.com/en-us/rest/api/azure/devops/build/builds/list?view=azure-devops-rest-5.1
    #
    buildInfo = GetBuildInfo(authHeader, organization, project, buildDefinitionId, buildNumber, retainedByRelease)
    return buildInfo["id"]

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def GetBuildVersion (authHeader, organization, project, buildDefinitionId, buildNumber, retainedByRelease):
    #
    # https://docs.microsoft.com/en-us/rest/api/azure/devops/build/builds/list?view=azure-devops-rest-5.1
    #
    buildInfo = GetBuildInfo(authHeader, organization, project, buildDefinitionId, buildNumber, retainedByRelease)
    return buildInfo["buildNumber"]

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def GetBuildVersionFromBuildId (authHeader, organization, project, buildId):
    #
    # https://docs.microsoft.com/en-us/rest/api/azure/devops/build/builds/get?view=azure-devops-rest-5.1
    #
    buildInfo = GetBuildInfoFromBuildId(authHeader, organization, project, buildId)
    return buildInfo["buildNumber"]

s_containerIds = {}
#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def GetContainerID (authHeader, organization, project, buildId):
    getArtifactsUrl = "{}/{}/{}/_apis/build/builds/{}/artifacts?api-version=5.1".format(internal.AZURE_DEVOPS_SERVER, organization, project, buildId)
    co_name = sys._getframe().f_code.co_name # pylint: disable=protected-access
    utils.showInfoMsg('{} url={} organization={} project={} buildId={}\n'.format(co_name, getArtifactsUrl, organization, project, buildId), utils.INFO_LEVEL_RarelyUseful)
    cache_key = '/'.join([organization.lower(), project.lower(), str(buildId)])
    httpContent = s_containerIds.get(cache_key, None)
    if httpContent:
        utils.showInfoMsg('{} using build info from cache\n'.format(co_name), utils.INFO_LEVEL_RarelyUseful)
    else:
        httpCode, httpContent = utils.getUrlWithDataWithRetries(getArtifactsUrl, None, {}, authHeader)
        if httpCode != 200:
            raise utils.BuildError ('Failed to get artifacts for buildId {0}. Return code: {1}\n'.format(buildId, httpCode))
        s_containerIds[cache_key] = httpContent

    artifacts = json.loads(httpContent)
    if artifacts["count"] == 0:
        raise utils.BuildError ('No artifacts could be found for buildId {0}.\n'.format(buildId))

    data = artifacts["value"][0]["resource"]["data"].split("/")
    return data[1]

s_artifactDownloadUrls = {}
#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def GetArtifactDownloadUrl (authHeader, organization, project, buildId, artifactName):
    #
    # https://docs.microsoft.com/en-us/rest/api/azure/devops/build/builds/list?view=azure-devops-rest-5.1
    #
    url = "{}/{}/{}/_apis/build/builds/{}/artifacts?artifactName={}&api-version=5.1".format(internal.AZURE_DEVOPS_SERVER, organization, project, buildId, artifactName)
    co_name = sys._getframe().f_code.co_name # pylint: disable=protected-access
    utils.showInfoMsg('{} url={} organization={} project={} buildId={} artifactName={}\n'.format(co_name, url, organization, project, buildId, artifactName), utils.INFO_LEVEL_RarelyUseful)
    cache_key = '/'.join([organization.lower(), project.lower(), str(buildId), artifactName.lower()])
    httpContent = s_artifactDownloadUrls.get(cache_key, None)
    if httpContent:
        utils.showInfoMsg('{} using build info from cache\n'.format(co_name), utils.INFO_LEVEL_RarelyUseful)
    else:
        httpCode, httpContent = utils.getUrlWithDataWithRetries(url, None, {}, authHeader)
        if httpCode != 200:
            raise utils.BuildError ('Failed to get {0} artifact download url for build {1}. Return code: {2}\n'.format(artifactName, buildId, httpCode))
        s_artifactDownloadUrls[cache_key] = httpContent

    artifact = json.loads (httpContent)
    return artifact["resource"]["downloadUrl"]

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def GetArtifactFileDownloadUrl(organization, containerId, itemPath):
    # Should probably use -> https://docs.microsoft.com/en-us/rest/api/azure/devops/build/artifacts/get%20file?view=azure-devops-rest-5.1
    # But official documentation does not yet support proper GetFile. https://github.com/MicrosoftDocs/vsts-rest-api-specs/issues/404

    itemPath = itemPath.replace("/", "%2F").replace(" ", "%20")
    artifact = itemPath.split("%2F")[0]
    return "{}/{}/_apis/resources/Containers/{}/{}?itemPath={}".format(internal.AZURE_DEVOPS_SERVER, organization, containerId, artifact, itemPath)

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def ListArtifacts (authHeader, organization, project, buildId):
    url = "https://dev.azure.com/{}/{}/_apis/build/builds/{}/artifacts?api-version=5.1".format(
        organization, project, buildId
    )
    httpCode, httpContent = utils.getUrlWithDataWithRetries(url, None, {}, authHeader)

    if httpCode != 200:
        raise utils.BuildError ('Failed to list artifacts: organization - {0}, project - {1}, buildId - {2}. Return code: {3}\n'.format(
            organization, project, buildId, httpCode))
    
    return json.loads(httpContent)

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def GetArtifactDirectoryDownloadUrl(organization, containerId, itemPath):
    # Should probably use -> https://docs.microsoft.com/en-us/rest/api/azure/devops/build/artifacts/get%20file?view=azure-devops-rest-5.1
    # But official documentation does not yet support proper GetFile. https://github.com/MicrosoftDocs/vsts-rest-api-specs/issues/404

    itemPath = itemPath.replace("/", "%2F").replace(" ", "%20")
    artifact = itemPath.split("%2F")[0]
    return "{}/{}/_apis/resources/Containers/{}/{}?itemPath={}&%24format=zip".format(internal.AZURE_DEVOPS_SERVER, organization, containerId, artifact, itemPath)

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def IsBuildArtifactUpToDate (downloadUrl, destination):
    provFileName = os.path.join (destination, 'provenance.txt')
    if not os.path.exists (provFileName):
        return False

    provFile = open(provFileName, 'rt')
    if not provFile:
        raise utils.BuildError ('Cannot open provenance file {0}\n'.format(provFileName))

    prov = provFile.read().strip().split('|')
    if prov[1] == downloadUrl:
        return True
    else:
        return False

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def DownloadBuildArtifactFile (authHeader, downloadUrl, destination):
    archiveLocation = destination + ".gz"
    httpCode = compat.getUrlFile(downloadUrl, archiveLocation, authHeader)
    if httpCode == 404:
        raise utils.BuildError("Build artifact file doesn't exist at {0}".format(downloadUrl))
    if not os.path.exists(archiveLocation):
        raise utils.BuildError ('Failed to download BDF file from {0} to {1}\n'.format(downloadUrl, destination))

    try:
        with gzip.open(archiveLocation, 'rb') as f_in, open(destination, 'wb') as f_out:
            f_out.write(f_in.read())
    except Exception as e:
        rethrow = True
        if sys.version_info >= (3, 8):
            if isinstance(e, gzip.BadGzipFile): # pylint: disable=no-member
                rethrow = False
        else:
            if isinstance(e, IOError):
                rethrow = False

        if rethrow:
            raise e
        else:
            os.remove(destination)

    if not os.path.exists(destination):
        os.rename(archiveLocation, destination)
    else:
        os.remove(archiveLocation)

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def DownloadBuildArtifactDirectory (authHeader, downloadUrl, destination, archiveSubDirectory, version):
    archiveLocation = os.path.join(destination, "{0}_{1}.zip".format(os.path.basename(destination), version))

    if not os.path.exists(archiveLocation):
        if os.path.exists(destination):
            utils.cleanDirectory (destination, deleteFiles=True)
            os.rmdir(destination)

        httpCode = compat.getUrlFile(downloadUrl, archiveLocation, authHeader)
        if httpCode == 404:
            raise utils.BuildError("Build artifact doesn't have directory {0}".format(archiveSubDirectory))
        if not os.path.exists(archiveLocation):
            raise utils.BuildError ('Failed to download Azure Build Artifact directory from {0} to {1}\n'.format(downloadUrl, destination))

    if len(os.listdir(destination)) > 1:
        return False # Already downloaded and extracted, return False to indicate no update

    correctedArchiveSubDirectory = archiveSubDirectory.replace("\\", "/") + "/"
    correctedArchiveSubDirectory = correctedArchiveSubDirectory.lower()
    archive = zipfile.ZipFile(archiveLocation, 'r')
    for zip_info in archive.infolist():
        if zip_info.filename[-1] == '/':
            continue
        if zip_info.filename.lower().startswith(correctedArchiveSubDirectory):
            zip_info.filename = zip_info.filename[len(correctedArchiveSubDirectory):]
        archive.extract(zip_info, destination)
    archive.close()
    return True

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def DownloadAndExtractBuildArtifact (authHeader, downloadUrl, destination, artifactName):
    if IsBuildArtifactUpToDate(downloadUrl, destination):
        utils.showInfoMsg ('Skipping artifact download as it is up to date\n', utils.INFO_LEVEL_VeryInteresting)
        return

    pkgPathName = os.path.join(destination, artifactName + ".zip")
    DownloadBuildArtifact(authHeader, downloadUrl, destination, artifactName, pkgPathName)
    ExtractFiles(artifactName, destination, pkgPathName)
    os.remove(pkgPathName)

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def DownloadAndExtractBuildArtifactWithGivenName (authHeader, downloadUrl, destination, artifactAlias):
    pkgPathName = os.path.join(destination, artifactAlias + ".zip")
    DownloadBuildArtifact(authHeader, downloadUrl, destination, artifactAlias, pkgPathName)
    ExtractFilesToGivenLocation(artifactAlias, os.path.join (destination, artifactAlias), pkgPathName)
    os.remove(pkgPathName)

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def ExtractFilesToGivenLocation(artifactName, destination, pkgPathName):
    utils.showInfoMsg ('Extracting artifact {0} to {1}\n'.format (artifactName, destination), utils.INFO_LEVEL_VeryInteresting)

    if not os.path.exists(destination):
        os.mkdir(destination)

    with zipfile.ZipFile(pkgPathName, "r") as z:
        z.extractall(destination)

        artifactDirPath = os.path.join(destination, os.listdir(destination)[0])
        
        for item in os.listdir(artifactDirPath):
            shutil.move(os.path.join(artifactDirPath, item), destination)

        if not os.listdir(artifactDirPath):
            os.rmdir(artifactDirPath)

    utils.showInfoMsg ('Artifact {0} successfully extracted to {1}\n'.format (artifactName, destination), utils.INFO_LEVEL_VeryInteresting)

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def ExtractFiles(artifactName, destination, pkgPathName):
    utils.showInfoMsg ('Extracting artifact {0} to {1}\n'.format (artifactName, destination), utils.INFO_LEVEL_VeryInteresting)

    archive = zipfile.ZipFile(pkgPathName, 'r')
    archive.extractall(destination)
    archive.close()

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def DownloadBuildArtifact(authHeader, downloadUrl, destination, artifactName, pkgPathName):
    utils.showInfoMsg ('Downloading artifact "{0}" \n from: {1} \n to: {2}\n'.format (artifactName, downloadUrl, pkgPathName), utils.INFO_LEVEL_VeryInteresting)

    httpCode, pkgGetContent = utils.getUrlWithDataWithRetries(downloadUrl, None, {}, authHeader)
    if httpCode != 200:
        raise utils.BuildError ('Failed to download artifact from url {0}\n'.format(downloadUrl))

    artifactpath = os.path.join (destination, artifactName)
    if os.path.exists(artifactpath):
        utils.cleanDirectory (artifactpath, deleteFiles=True)
        os.rmdir(artifactpath)

    if not os.path.exists(destination):
        os.makedirs (destination)

    pkgFile = open(pkgPathName, 'wb')
    if not pkgFile:
        raise utils.BuildError ('Cannot open package file {0} for write\n'.format(pkgPathName))
    pkgFile.write(pkgGetContent)
    pkgFile.close()

