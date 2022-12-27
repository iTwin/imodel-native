#---------------------------------------------------------------------------------------------
#  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
#  See LICENSE.md in the repository root for full copyright notice.
#---------------------------------------------------------------------------------------------
import json, os, base64, time
from . import cmdutil, compat, utils, versionutils

CLI_VERSION = '2.0.70'      # Don't know exact minimum
DEVOPS_VERSION = '0.15.0'   # Don't know exact minimum
s_azureCliPath = None
s_validCli = None

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def RunAzureCliCmd (cmd):
    startTime = compat.perfCounter()
    utils.showStatusMsg ("Running Azure Command Line: {0}\n".format (cmd),utils.INFO_LEVEL_SomewhatInteresting)
    status, stdoutLines, stderrLines = cmdutil.runAndWaitSeparateErrors (cmd)

    utils.showInfoMsg ('Azure Stdout lines:\n', utils.INFO_LEVEL_SomewhatInteresting)
    for line in stdoutLines:
        lineToLog = line
        if utils.isUnattendedBuild() and "\"accessToken\":" in lineToLog:
            startOfToken = lineToLog.find("\"", lineToLog.find("\"accessToken\":") + 14) + 1
            endOfToken = lineToLog.find("\"", startOfToken)
            token = lineToLog[startOfToken:endOfToken]
            lineToLog = lineToLog.replace(token, "**replaced in logging for security purposes**")

        utils.showInfoMsg (lineToLog + '\n', utils.INFO_LEVEL_SomewhatInteresting)

    utils.showInfoMsg ('Azure Stderr lines:\n', utils.INFO_LEVEL_SomewhatInteresting)
    for line in stderrLines:
        utils.showInfoMsg (line + '\n', utils.INFO_LEVEL_SomewhatInteresting)

    utils.showInfoMsg ("RunAzureCliCmd '{0}' took {1:0.2f} seconds\n".format(cmd, compat.perfCounter() - startTime), utils.INFO_LEVEL_RarelyUseful)

    return status, stdoutLines, stderrLines

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def RunAzureCliCmdJson (cmd):
    status, stdoutLines, stderrLines = RunAzureCliCmd (cmd)
    resultJson = None
    if status == 0:
        # Store the downloaded version which is returned as json. However, a lot of other stuff can flood in as well so have to pick out the right lines.
        combinedLine = '\n'.join(stdoutLines)

        startIndex1 = combinedLine.find ('{')
        startIndex2 = combinedLine.find ('[')
        startIndex = startIndex1 if startIndex1 >= 0 else startIndex2
        if startIndex2 >=0 and startIndex2 < startIndex:
            startIndex = startIndex2

        endIndex1 = combinedLine.rfind ('}')
        endIndex2 = combinedLine.rfind (']')
        endIndex = endIndex1 if endIndex1 >= 0 else endIndex2
        if endIndex2 >=0 and endIndex2 > endIndex:
            endIndex = endIndex2

        if startIndex >= 0 and endIndex >= 0:
            jsonResponse = combinedLine [startIndex:endIndex+1]
            try:
                resultJson = json.loads (jsonResponse)
            except ValueError:
                status = 1
                resultJson = None
                utils.showStatusMsg ("Unable to parse JSON {0}\nReturned from {1}\n".format(jsonResponse, ' '.join(cmd)),utils.INFO_LEVEL_SomewhatInteresting)

    return status, resultJson, CombineStdoutStdErr (stdoutLines, stderrLines)

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def CombineStdoutStdErr (stdoutLines, stderrLines):
    combinedLines = []
    combinedLines.append ('StdOut: \n')
    combinedLines.extend ([l.strip() +'\n' for l in stdoutLines])
    combinedLines.append ('StdErr: \n')
    combinedLines.extend ([l.strip() +'\n' for l in stderrLines])
    return combinedLines

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def GetAzureCliPath ():
    global s_azureCliPath
    
    if not s_azureCliPath:
        # Make sure it's installed. Currently it always ends up in the path. Couldn't find a spot in the registry though.
        exeName, delim = ('az.cmd', ';') if os.name == 'nt' else ('az', ':')
        for pathEntry in os.environ['PATH'].split(delim):
            azcli = os.path.join (pathEntry, exeName)
            if os.path.exists (azcli):
                IsAzureCliCorrectAndLoggedIn (azcli)
                if s_validCli:
                    s_azureCliPath = azcli
                break

        if not s_azureCliPath:
            raise utils.BuildError ("Can't find the Azure CLI in the path; please install Azure CLI")
    return s_azureCliPath

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def IsAzureCliCorrectAndLoggedIn (azCliExe):
    # This method is pretty slow; be sure to delay calls to in until when it's actually needed, and
    # even then we cache the result.
    global s_validCli
    if s_validCli != None:
        return s_validCli

    #########################
    # Next check if user is logged in
    if 'AZURE_DEVOPS_EXT_PAT' in os.environ:
        utils.showStatusMsg ("AZURE_DEVOPS_EXT_PAT is set to handle Azure CLI login\n",utils.INFO_LEVEL_SomewhatInteresting)
        loggedIn = True
    else:
        cmd = [azCliExe, "account", "show", "-o", "json"]
        _, stdoutLines, _ = RunAzureCliCmd (cmd)
        loggedIn = False
        for resLine in stdoutLines:
            if "Please run 'az login' to setup account" in resLine:
                loggedIn = False
                break
            elif '"user": {' in resLine:
                loggedIn = True

    if not loggedIn:
        s_validCli = False
        raise utils.BuildError ("Azure CLI is not logged in. On a developer box, please use 'az login' to log into Azure.\n" +
                                "If this is a Azure Devops pipeline build that needs access, set  AZURE_DEVOPS_EXT_PAT: $(System.AccessToken) " +
                                "in the env: for the task/script" )

    #########################
    # Then check the version of the CLI
    cmd = [azCliExe, "--version"]
    status, stdoutLines, stderrLines = RunAzureCliCmd (cmd)
    cliVersion = None
    for resLine in stdoutLines:
        # azure-cli                         2.0.75
        # azure-cli                         2.0.75 *     (when it needs to be updated)
        if resLine.startswith("azure-cli"):
            splitLine = resLine.split (' ')
            cliVersion = splitLine[-1].strip()
            if cliVersion == '*':
                cliVersion = splitLine[-2].strip()
            break

    if cliVersion == None or status != 0:
        s_validCli = False
        raise utils.BuildError ("'az --version' did not yeild the expected version (status = {0}, return = {1})".format(status, '\n'.join(CombineStdoutStdErr (stdoutLines, stderrLines))))

    if versionutils.Version (cliVersion) < versionutils.Version(CLI_VERSION):
        s_validCli = False
        raise utils.BuildError ('az version {0} is less than required version {1}.\nGoogle "azure cli install" and download and install the update.'.format (cliVersion, CLI_VERSION))
    
    #########################
    # Make sure the extension is installed
    cmd = [azCliExe, 'extension', 'list', "-o", "json"]
    status, resultJson, resultLines = RunAzureCliCmdJson (cmd)
    hasExtension = False
    if status != 0:
        raise utils.BuildError ('Error checking for az devops extension; got status={0} "{1}"'.format(status, resultLines))
    else:
        for ext in resultJson:
            # [{"extensionType": "whl",  "name": "azure-devops",  "version": "0.13.0"}] 
            if ext['name'] == 'azure-devops':
                hasExtension = True
                devopsVersion = ext['version']
                if versionutils.VersionWithSuffix(devopsVersion) < versionutils.VersionWithSuffix(DEVOPS_VERSION):
                    raise utils.BuildError ('azure-devops extension version is too old. Found {0} but need {1}.\nPlease use "az extension update --name azure-devops" to update'.format(devopsVersion, DEVOPS_VERSION))
                break

    if not hasExtension:
        s_validCli = False
        raise utils.BuildError ('azure-devops extension is not installed. Please use "az extension add --name azure-devops" to install it.')

    #########################
    # Check for bad Cli/devops match
    # For now there is just one and it is complicated to write a general case of aceptable version matchces so just doing the one that needs to be done.
    if versionutils.VersionWithSuffix(cliVersion) >= versionutils.VersionWithSuffix('2.30.0'):
        # CLI 2.30 requires 0.22 or higher otherwise pulling upacks fails
        requiredV = '0.22.0'
        if versionutils.VersionWithSuffix(devopsVersion) < versionutils.VersionWithSuffix(requiredV):
            raise utils.BuildError ('azure-devops extension version is too old for azure CLI. Found {0} but need {1} or higner.\nPlease use "az extension update --name azure-devops" to update'.format(devopsVersion, requiredV))

    #########################
    # Make sure the credentials are valid
    cmd = [azCliExe, "account", "get-access-token", "-o", "json"]
    status, stdoutLines, stderrLines = RunAzureCliCmd(cmd)

    if status != 0:
        # There's a problem with azure CLI where it corrupts the file by writing a shorter file but it leaves a few bytes at the end. See if we are in that situation and fix it
        if RepairAccessTokens():
            status, stdoutLines, stderrLines = RunAzureCliCmd(cmd)

    if status != 0:
        if any('The user might have changed or reset their password' in line for line in stderrLines):
            raise utils.BuildError("Have you changed or reset your password? Please use 'az login' again.\n")
        if 'AZURE_DEVOPS_EXT_PAT' not in os.environ:
            raise utils.BuildError("Azure CLI could not get access token\n")

    s_validCli = True
    return s_validCli

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def RepairAccessTokens ():
    # There's a problem with azure CLI where it corrupts the file by writing a shorter file but it leaves a few bytes at the end. See if we are in that situation and fix it
    if os.name != 'nt' or 'AZURE_DEVOPS_EXT_PAT' in os.environ: # Only seems to happen on Windows because everwhere else we use the PAT.
        return False

    utils.showStatusMsg ("Attempting Azure DevOps token repair\n",utils.INFO_LEVEL_SomewhatInteresting)
    tokenDir = os.path.expandvars (os.path.join ('${USERPROFILE}', '.azure'))
    tokenFileName = os.path.join (tokenDir, 'accessTokens.json')
    if not os.path.exists (tokenFileName):
        utils.showStatusMsg ("Token directory/file doesn't exist: {0}\n".format(tokenDir), utils.INFO_LEVEL_SomewhatInteresting)
        return False

    badJson = False
    with open(tokenFileName, 'rb') as tokenFile:
        try:
            json.load(tokenFile)
        except ValueError:
            utils.showStatusMsg ("Token file {0} appears bad\n".format(tokenFileName),utils.INFO_LEVEL_SomewhatInteresting)
            badJson = True
    
    if not badJson:
        utils.showStatusMsg ("Token file {0} parses ok.\n".format(tokenFileName),utils.INFO_LEVEL_SomewhatInteresting)
        return False

    # Check if someone else is trying to fix it
    LOCK_FILE_PATH = os.path.join(tokenDir, 'BB_repair_lock')
    if os.path.exists (LOCK_FILE_PATH):
        utils.showStatusMsg ("Token Repair Lock File exists; waiting\n",utils.INFO_LEVEL_SomewhatInteresting)
        time.sleep(5)
        if os.path.exists (LOCK_FILE_PATH): # If it's still there, try to remove it. Should not take this long.
            utils.showStatusMsg ("Token Repair Lock File still exists; will try to remove\n",utils.INFO_LEVEL_SomewhatInteresting)
            try:
                os.remove (LOCK_FILE_PATH)
            except:
                utils.showStatusMsg ("Failed removing Token Repair Lock File.\n",utils.INFO_LEVEL_SomewhatInteresting)
                return False
            if os.path.exists (LOCK_FILE_PATH):
                utils.showStatusMsg ("Unable to remove Token Repair Lock File.\n",utils.INFO_LEVEL_SomewhatInteresting)
                return False
        else:
            utils.showStatusMsg ("Token Repair Lock File is gone; we believe another process has done the repair.\n",utils.INFO_LEVEL_SomewhatInteresting)
            return True

    # Try a simple repair
    fixedJson = False
    try:
        with open (LOCK_FILE_PATH, 'wt'):
            utils.showStatusMsg ("Locking and attempting token file repair.\n",utils.INFO_LEVEL_SomewhatInteresting)
            with open(tokenFileName, 'rb') as tokenFile:
                tokenData = tokenFile.read()
            closeBracket = tokenData.find(b']')
            tokenData = tokenData[0:closeBracket+1]
            try:
                json.loads (tokenData)
            except ValueError:
                utils.showStatusMsg ("Token file repair did not yeild valid json.\n",utils.INFO_LEVEL_SomewhatInteresting)
                return False  # Failed to fix
            with open(tokenFileName, 'wb') as tokenFile:
                tokenFile.write (tokenData)
                fixedJson = True
                utils.showStatusMsg ("Token file repaired.\n",utils.INFO_LEVEL_SomewhatInteresting)
    finally:
        # Always clean up lock file
        if os.path.exists (LOCK_FILE_PATH):
            os.remove (LOCK_FILE_PATH)

    return fixedJson

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def GetAuthenticationHeader ():
    # If PAT is supplied use that first. The length check checks for someone using a personal PAT which will only work going through azure CLI
    if 'AZURE_DEVOPS_EXT_PAT' in os.environ and len(os.environ.get('AZURE_DEVOPS_EXT_PAT')) > 100:
        utils.showStatusMsg ('Using AZURE_DEVOPS_EXT_PAT because it is set.\n', utils.INFO_LEVEL_Interesting)
        return 'bearer ' + os.environ.get('AZURE_DEVOPS_EXT_PAT')

    # This method calls "az account get-access-token" and returns a formated authentication header that can be used with Azure HTTP services
    cmd = [GetAzureCliPath(), 'account', 'get-access-token', '-o', 'json']
    status, resultJson, resultLines = RunAzureCliCmdJson (cmd)

    if status != 0:
        # Try fixing the Azure CLI token file.
        if RepairAccessTokens():
            status, resultJson, resultLines = RunAzureCliCmdJson(cmd)

    if status == 0:
        utils.showStatusMsg ('Using Azure CLI access token.\n', utils.INFO_LEVEL_SomewhatInteresting)
        return "{0} {1}".format(resultJson['tokenType'], resultJson['accessToken'])

    utils.showInfoMsg ('Error: Failed to get access token from Azure CLI. Got status={0} "{1}".'.format(status, resultLines), utils.INFO_LEVEL_VeryInteresting)
    return None

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def EncodeBasicAuthHeader (pat):
    return b"Basic " + base64.b64encode(b":" + compat.getAscii (pat))
