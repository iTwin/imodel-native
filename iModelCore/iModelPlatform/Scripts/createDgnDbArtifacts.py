#---------------------------------------------------------------------------------------------
#  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
#  See COPYRIGHT.md in the repository root for full copyright notice.
#---------------------------------------------------------------------------------------------

from pathlib import Path
from shutil import copyfile, rmtree
from os import rename, environ

# This file is to be run after DgnDbTest Fixture from IModelPlatform GTests has run successfully

def getCurrentECDbProfileVersion(fileToWriteTo: Path):
    # Create a txt file anbd write the current ECDb profile version to it
    ecdbFile = open(Path(environ['SrcRoot'], "imodel-native\iModelCore\ECDb\PublicAPI\ECDb\ECDb.h"), "r")
    for line in ecdbFile.readlines():
        if line.find("CurrentECDbProfileVersion") != -1:
            index = line.find(" ProfileVersion(")
            if index != -1:
                ecdbProfileVersion = str(line[index + 16 : line.find(");")]).replace(", ", '.')
                ecdbFile.close()
                fileToWriteTo.write_text(ecdbProfileVersion)
                return ecdbProfileVersion
    return ""

def copySeedFileToFolder(productDir: Path, artifactDir: Path, ecdbProfileVersion: str):
    # Copy bim file to be used as seed for profile version tests
    fileToCopy = productDir.joinpath("iModelPlatform-Gtest\\run\Output\DgnDbTest\ProjectProfileVersions.bim")
    seedFile = artifactDir.joinpath("static_" + ecdbProfileVersion + "_by_3.1.0.2.bim")

    if fileToCopy.exists():
        copyfile(fileToCopy, artifactDir.joinpath("ProjectProfileVersions.bim"))
        rename(artifactDir.joinpath("ProjectProfileVersions.bim"), seedFile)
    else:
        print("Seed file to copy not found")

if __name__ == "__main__":
    outDirFile = Path(environ['SrcRoot'], "imodel-native\iModelCore\iModelPlatform\Scripts\OutputDir.txt")
    if outDirFile.exists():
        # Get the output Product folder path
        productDir = Path(outDirFile.read_text().rstrip(), "Product")
        artifactDir = productDir.joinpath("ProfileVersionArtifacts")
        if artifactDir.exists():
            rmtree(artifactDir)
        artifactDir.mkdir(parents=True, exist_ok=True)

        ecdbProfileVersion = getCurrentECDbProfileVersion(artifactDir.joinpath("ECDbProfileVersion.txt"))
        if ecdbProfileVersion:
            copySeedFileToFolder(productDir, artifactDir, ecdbProfileVersion)
        else:
            print("Error retrieving ECDb profile version")
    else:
        print("OutDir.txt file not found")