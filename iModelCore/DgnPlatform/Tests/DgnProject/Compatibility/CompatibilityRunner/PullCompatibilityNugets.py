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
from shutil import copyfile, rmtree

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
    #rmtree(pkgDirName, ignore_errors=True)
    # Lower before we do the replace
    pkgGetUrl = '{0}/package/{1}/{2}'.format(pkgAddress, pkgName, version)
    nugetpkg.GetPackage(pkgGetUrl, pkgName, pkgPathName, version, localDir)

    return pkgPathName

 #------------------------------------------------------------------------
 # bsimethod                         Kyle.Abramowitz             05/2018
 #------------------------------------------------------------------------
def pullAllNugets(path, pathToNugetPuller):
    import nugetpkg
    address = "http://nuget.bentley.com/nuget/default/"
    name = "iModelSchemaEvolutionTestNuget_bim0200dev_x64";
    versions = nugetpkg.SearchVersionsFromServer(address, name)

    for v in versions:
        # Dowload and save all versions
        pkgAddress = "http://nuget.bentley.com/nuget/default/"
        pkgName = "iModelSchemaEvolutionTestNuget_bim0200dev_x64"
        localDir = path
        DownloadPackage(pkgAddress, pkgName, v, localDir)

 #------------------------------------------------------------------------
 # bsimethod                         Kyle.Abramowitz             05/2018
 #------------------------------------------------------------------------
def main():
    if len(sys.argv) < 2:
        print "Must give the <nuget path> <path to nuget python script>"
        return

    nugetPath = sys.argv[1]
    sys.path.insert(0, sys.argv[2])
    pullAllNugets(nugetPath, sys.argv[2])
     # All these packages should be of the form DgnCompatiblityNugetxx.xx.xx.xx
    for filename in os.listdir(nugetPath):
        path = os.path.join(nugetPath, filename)
        if (os.path.isdir(path)):
            rmtree(path, ignore_errors=True)

    for filename in os.listdir(nugetPath):
        path = os.path.join(nugetPath, filename)
        zip_ref = zipfile.ZipFile(path, "a")
        zip_ref.extractall(os.path.splitext(path)[0])
        zip_ref.close()

if __name__ == "__main__":
    main()