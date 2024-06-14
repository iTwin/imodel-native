#---------------------------------------------------------------------------------------------
#  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
#  See LICENSE.md in the repository root for full copyright notice.
#---------------------------------------------------------------------------------------------
import json, os
from . import builddescriptionfile, buildpaths, utils

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def BootstrapInfoExists ():
    return os.path.exists (buildpaths.GetBootsrapFileName())

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def ReadBootstrapInfo (buildDescription):
    infoFileName = buildpaths.GetBootsrapFileName()
    if not os.path.exists (infoFileName):
        raise utils.BuildError ("Error: Bootstrap file does not exist: {0}".format (infoFileName))

    with open (infoFileName, 'r') as jsonFile:
        jsonData = json.load (jsonFile)

    for item in jsonData:
        buildDescription.AddBootstrapEntry (item['Name'], item['Type'], item['IsPrimary'], item.get('LocalDir', None))

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def GetPrimaryRepo ():
    bdf = builddescriptionfile.BuildDescription()
    ReadBootstrapInfo(bdf)
    for item in bdf.GetAllBootstrapInfo ():
        if item.IsPrimary:
            return item.Name

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def GetPrimaryRepoLocalDir ():
    bdf = builddescriptionfile.BuildDescription()
    ReadBootstrapInfo(bdf)
    for item in bdf.GetAllBootstrapInfo ():
        if item.IsPrimary:
            return item.LocalDir if item.LocalDir else item.Name

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def GetPrimaryBBConfigFilename ():
    if not os.path.exists (buildpaths.GetBootsrapFileName()):
        return None
    primaryRepoLocalDir = GetPrimaryRepoLocalDir()
    if not primaryRepoLocalDir:
        return None
    primaryRepoConfig = os.path.join (buildpaths.GetSrcRoot(), primaryRepoLocalDir, buildpaths.FILENAME_BBCONFIG)
    return primaryRepoConfig

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def GetPrimaryBBConfig ():
    primaryRepoConfig = GetPrimaryBBConfigFilename()
    if not primaryRepoConfig:
        return builddescriptionfile.BuildDescription() # So it will work in OS where there is no bbconfig.
    bdf = builddescriptionfile.BuildDescription(primaryRepoConfig)
    status, errMessage = bdf.ReadFromFile ()
    if status == 0:
        return bdf
    else:
        raise utils.BuildError ("Error: Reading config file failed: {0}".format (errMessage))



