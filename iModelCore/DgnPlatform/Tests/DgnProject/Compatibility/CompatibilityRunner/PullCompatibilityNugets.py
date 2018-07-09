#----------------------------------------------------------------------
#
#     $Source: Tests/DgnProject/Compatibility/CompatibilityRunner/PullCompatibilityNugets.py $
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
    # Lower before we do the replace
    pkgGetUrl = '{0}/package/{1}/{2}'.format(pkgAddress, pkgName, version)
    try:
        nugetpkg.GetPackage(pkgGetUrl, pkgName, pkgPathName, version, localDir)
        return pkgPathName
    except utils.BuildError as err:
        print >> sys.stderr, err
        sys.exit(1)


 #------------------------------------------------------------------------
 # bsimethod                         Kyle.Abramowitz             05/2018
 #------------------------------------------------------------------------
def pullAllNugets(path, pathToNugetPuller, name):
    import nugetpkg
    from distutils.version import LooseVersion

    address = "http://nuget.bentley.com/nuget/default/"
    versions = nugetpkg.SearchVersionsFromServer(address, name)
    for v in versions:
        # ignore stale versions until they have been deleted fromthe nuget server
        if LooseVersion(v) < LooseVersion("2018.7.9.1"):
            continue
        # Dowload and save all versions
        localDir = path
        DownloadPackage(address, name, v, localDir)

 #------------------------------------------------------------------------
 # bsimethod                         Kyle.Abramowitz             05/2018
 #------------------------------------------------------------------------
def main():
    if len(sys.argv) < 3:
        print "Must give the <nuget path> <path to nuget python script> <path to datasets>"
        return

    nugetPath = sys.argv[1]
    sys.path.insert(0, sys.argv[2])
    pullAllNugets(nugetPath, sys.argv[2], "iModelSchemaEvolutionTestNuget_bim0200dev_x64")
    pullAllNugets(nugetPath, sys.argv[2], "iModelSchemaEvolutionTestNuget_bim0200dev_ec32_x64")
    dataPath = sys.argv[3]
    # Remove extracted nugets to prevent unzipping overwrite issues
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
    # Copy in the current dataset necessary to run forwards compatibility tests
    for subdir in os.listdir(nugetPath):
        path = os.path.join(nugetPath, subdir)
        if os.path.isdir(path):
            copytree(dataPath, os.path.join(path, "run", "NewData"))

if __name__ == "__main__":
    main()