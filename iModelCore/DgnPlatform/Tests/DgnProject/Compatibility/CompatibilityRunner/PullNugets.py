#---------------------------------------------------------------------------------------------
#  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
#  See COPYRIGHT.md in the repository root for full copyright notice.
#---------------------------------------------------------------------------------------------
import sys
import os
import zipfile
import imp
from shutil import rmtree, copyfile
from distutils.version import LooseVersion
sys.path.append(os.path.join(os.getenv("SrcRoot"),"bentleybuild"))
from bentleybuild import nugetpkg, symlinks

#-------------------------------------------------------------------------------------------
 # bsimethod                         Kyle.Abramowitz             05/2018
#-------------------------------------------------------------------------------------------
def DownloadPackage(pkgAddress, pkgName, version, localDir):
    pkgBaseName = '{0}.{1}'.format(pkgName, version)
    pkgDirName = localDir
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
    except BaseException as err:
        print >> sys.stderr, err
        sys.exit(1)


#------------------------------------------------------------------------
# bsimethod                         Kyle.Abramowitz             05/2018
#------------------------------------------------------------------------
def pullAllNugets(path, pathToNugetPuller, name, minimumVersion=None, ignoreVersionsSet=None):

    address = "http://nuget.bentley.com/nuget/default/"
    versions = nugetpkg.SearchVersionsFromServer(address, name)
    for v in versions:
        # ignore versions older than the passed minimum or if in the ignore set (until they have been deleted from the nuget server)
        if (minimumVersion and LooseVersion(v) < LooseVersion(minimumVersion)) or (ignoreVersionsSet and v in ignoreVersionsSet):
            continue
        # Dowload and save all versions
        localDir = path
        DownloadPackage(address, name, v, localDir)

#------------------------------------------------------------------------
# bsimethod                         Krischan.Eberle            07/2018
#------------------------------------------------------------------------
def unzipNugets(srcPath):
    # Remove extracted nugets to prevent unzipping overwrite issues
    for filename in os.listdir(srcPath):
        path = os.path.join(srcPath, filename)
        if (os.path.isdir(path)):
            rmtree(path, ignore_errors=True)
    # Extract all nugets
    for filename in os.listdir(srcPath):
        path = os.path.join(srcPath, filename)
        zip_ref = zipfile.ZipFile(path, "a")
        zip_ref.extractall(os.path.splitext(path)[0])
        zip_ref.close()

#------------------------------------------------------------------------
# bsimethod                         Krischan.Eberle            07/2018
#------------------------------------------------------------------------
def copyFiles(sourceFolder,targetFolder):
    for subElement in os.listdir(sourceFolder):
        sourcePath = os.path.join(sourceFolder, subElement)
        targetPath = os.path.join(targetFolder, subElement)
        if os.path.isdir(sourcePath):
            copyFiles(sourcePath,targetPath)
        else:
            if not os.path.exists(targetFolder):
                os.makedirs(targetFolder)
            copyfile(sourcePath,targetPath)

#------------------------------------------------------------------------
# bsimethod                         Krischan.Eberle            07/2018
#------------------------------------------------------------------------
def mergeTestFiles(testFilesNugetPath, targetDir):
    for subdir in os.listdir(testFilesNugetPath):
        path = os.path.join(testFilesNugetPath, subdir)
        if not os.path.isdir(path):
            continue
        testFilesNugetFolder = os.path.join(path, "TestFiles")
        if os.path.exists(testFilesNugetFolder):
            copyFiles(testFilesNugetFolder, targetDir)

 #------------------------------------------------------------------------
 # bsimethod                         Kyle.Abramowitz             05/2018
 #------------------------------------------------------------------------
def main():
    if len(sys.argv) < 4:
        print "Arg 1: <nuget path in src>"
        print "Arg 2: <Artefacts root folder in out>"
        print "Arg 3: <nuget folder in Artefacts root folder>"
        print "Arg 4: <path to nuget python script>"
        return

    nugetPathInSrc = sys.argv[1]
    artefactsPath = sys.argv[2]
    nugetPath = sys.argv[3]
    nugetScript = sys.argv[4]
    sys.path.insert(0, nugetScript)

    # Relevant nugets (all others must be ignored until they are deleted from the server):
    # 2018.9.14.1: nuget before EC3.2 merge
    bim02devMinimumNugetVersion = "2018.9.14.2"
    bim02devIgnoreNugetVersions = {"2018.9.19.1","2018.9.20.1", "2018.9.25.1", "2018.10.18.3", "2018.10.19.1", "2018.11.8.4", "2018.11.9.1", "2019.1.14.1"}

    # Relevant nugets (all other must be ignored until they are deleted from the server):
    # 2019.04.20.1: nuget after ECDbMap and ECDbFileInfo downgrade to EC3.1
    bim02MinimumNugetVersion = "2019.2.2.1"
    bim02IgnoreNugetVersions = {"2019.4.20.1"}

    # Relevant nugets (all others must be ignored until they are deleted from the server):
    # 2019.4.20.1: nuget after ECDbMap and ECDbFileInfo downgrade to EC3.1
    # Nugets from 2018.9.21.1 to 2019.1.14.2 must be ignored as they contain EC3.2 version ECDb schemas that were downgraded to EC3.1 later on
    imodel02MinimumNugetVersion = "2019.2.2.1"
    imodel02IgnoreNugetVersions = {"2019.4.20.1"}

    # The current EC3.3 tests won't work because they need to have the latest version of the CoreCA schema.
    # ec33MinimumNugetVersion = "2019.4.1.1"
    # ec33IgnoreNugetVersions = {}

    # Test runners are downloaded into src as pulling them in with every TMR might take too long
    # They are symlinked into the out dir afterwards
    pullAllNugets(nugetPathInSrc, nugetScript, "iModelEvolutionTestRunnerNuget_bim0200dev_x64", bim02devMinimumNugetVersion, bim02devIgnoreNugetVersions)
    pullAllNugets(nugetPathInSrc, nugetScript, "iModelEvolutionTestRunnerNuget_bim0200_x64", bim02MinimumNugetVersion, bim02IgnoreNugetVersions)
    pullAllNugets(nugetPathInSrc, nugetScript, "iModelEvolutionTestRunnerNuget_imodel02_x64", imodel02MinimumNugetVersion, imodel02IgnoreNugetVersions)
    # pullAllNugets(nugetPathInSrc, nugetScript, "iModelEvolutionTestRunnerNuget_ec33_x64", ec33MinimumNugetVersion, ec33IgnoreNugetVersions)
    unzipNugets(nugetPathInSrc)
    # Test files can be downloaded in out folder directly
    testFileNugetPath = os.path.join(nugetPath, "testfiles")
    pullAllNugets(testFileNugetPath, nugetScript, "iModelEvolutionTestFilesNuget_bim0200dev_x64", bim02devMinimumNugetVersion, bim02devIgnoreNugetVersions)
    pullAllNugets(testFileNugetPath, nugetScript, "iModelEvolutionTestFilesNuget_bim0200_x64", bim02MinimumNugetVersion, bim02IgnoreNugetVersions)
    pullAllNugets(testFileNugetPath, nugetScript, "iModelEvolutionTestFilesNuget_imodel02_x64", imodel02MinimumNugetVersion, imodel02IgnoreNugetVersions)
    # pullAllNugets(testFileNugetPath, nugetScript, "iModelEvolutionTestFilesNuget_ec33_x64", ec33MinimumNugetVersion, ec33IgnoreNugetVersions)
    unzipNugets(testFileNugetPath)
    # Copy test files from all nugets into a single central folder
    targetDir = os.path.join(artefactsPath, "TestFiles")
    mergeTestFiles(testFileNugetPath, targetDir)

if __name__ == "__main__":
    main()