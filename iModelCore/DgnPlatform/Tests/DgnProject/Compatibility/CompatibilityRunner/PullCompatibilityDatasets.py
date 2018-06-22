#----------------------------------------------------------------------
#
#     $Source: Tests/DgnProject/Compatibility/CompatibilityRunner/PullCompatibilityDatasets.py $
#
#  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
#
#----------------------------------------------------------------------
import sys
import os
import zipfile
import imp
from shutil import copyfile, copytree, rmtree

#-------------------------------------------------------------------------------------------
# bsimethod                         Kyle.Abramowitz             05/2018
# Download a nuget package from a server over http
#-------------------------------------------------------------------------------------------
def DownloadPackage(pkgAddress, pkgName, version, localDir):
    import nugetpkg
    pkgBaseName = '{0}.{1}'.format(pkgName, version)
    pkgDirName = localDir
    import symlinks
    if not os.path.exists(pkgDirName):
        symlinks.makeSureDirectoryExists(pkgDirName)

    pkgPathName = os.path.join(pkgDirName, '{0}.nupkg'.format(pkgBaseName))
    # Have we already downloaded and unzipped this package -- "package" folder is the last to unzip
    if os.path.exists(pkgPathName):
        return
    #rmtree(pkgDirName, ignore_errors=True)
    # Lower before we do the replace
    pkgGetUrl = '{0}/package/{1}/{2}'.format(pkgAddress, pkgName, version)
    nugetpkg.GetPackage(pkgGetUrl, pkgName, pkgPathName, version, localDir)

    return pkgPathName

 #------------------------------------------------------------------------
 # bsimethod                         Kyle.Abramowitz             05/2018
 # Pull all test data nugets to path
 #------------------------------------------------------------------------
def pullAllNugets(path, pathToNugetPuller, name):
    import nugetpkg
    address = "http://nuget.bentley.com/nuget/default/"
    versions = nugetpkg.SearchVersionsFromServer(address, name)
    excludeVersions = ["2018.6.5.1", "2018.5.31.15"]
    for v in versions:
        if v in excludeVersions:
            continue
        localDir = path
        DownloadPackage(address, name, v, localDir)

 #------------------------------------------------------------------------
 # bsimethod                         Kyle.Abramowitz             05/2018
 #------------------------------------------------------------------------
def main():
    if len(sys.argv) < 2:
        print "Must give the <nuget path> <path to nuget python script>"
        return

    nugetPath = sys.argv[1]
    sys.path.insert(0, sys.argv[2])
    pullAllNugets(nugetPath, sys.argv[2], "iModelSchemaEvolutionTestFilesNuget_bim0200dev_x64")
    #TODO remove once we merge back
    pullAllNugets(nugetPath, sys.argv[2], "iModelSchemaEvolutionTestFilesNuget_bim0200dev_ec32_x64")
    # Remove old extracted nugets to prevent issues with extraction overwrites
    for filename in os.listdir(nugetPath):
        path = os.path.join(nugetPath, filename)
        if (os.path.isdir(path)):
            rmtree(path, ignore_errors=True)
    # Extract all nugets
    for filename in os.listdir(nugetPath):
        path = os.path.join(nugetPath, filename)
        zip_ref = zipfile.ZipFile(path, "a")
        zip_ref.extractall(os.path.splitext(path)[0])
        zip_ref.close()
    # Extract datasets out of nugets and put into seeddata folder. Only 1 file per version for now. #TODO
    extractedDataDir = os.path.join(nugetPath, "SeedData")
    if not os.path.exists(extractedDataDir):
        os.makedirs(extractedDataDir)
    for subdir in os.listdir(nugetPath):
        path = os.path.join(nugetPath, subdir)
        if os.path.isdir(path) and subdir != "Datasets":
            datasetPath = os.path.join(path, "Datasets")
            for db in os.listdir(datasetPath):
                dbPath = os.path.join(datasetPath, db)
                for version in os.listdir(dbPath):
                    if not os.path.exists(os.path.join(extractedDataDir, db, version)):
                        copytree(os.path.join(dbPath, version), os.path.join(extractedDataDir, db, version))

if __name__ == "__main__":
    main()