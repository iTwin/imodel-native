#----------------------------------------------------------------------
#
#  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
#
#----------------------------------------------------------------------
import sys
from filecmp import dircmp
import os.path

#------------------------------------------------------------------------
# bsimethod                         Kyle.Abramowitz             05/2018
#------------------------------------------------------------------------
def filesAreContainedIn(folderToTest, containerFolder):
    """
    Tests whether the content of folderToTest is contained in containerFolder
    Returns the first set of files which are not contained, or None if they are all contained.
    """
    comparison = dircmp(folderToTest, containerFolder)
    if (comparison.left_only):
        return comparison.left_only
    for subdir in comparison.common_dirs:
        notContainedFiles = filesAreContainedIn(os.path.join(folderToTest, subdir), os.path.join(containerFolder, subdir))
        if notContainedFiles <> None:
            return notContainedFiles
    return None

#------------------------------------------------------------------------
# bsimethod                         Kyle.Abramowitz             05/2018
#------------------------------------------------------------------------
def main():
    currentDatasetPath = sys.argv[1]
    oldDatasetPath = sys.argv[2]
    newTestFiles = filesAreContainedIn(currentDatasetPath, oldDatasetPath)
    if newTestFiles == None:
        sys.exit(0)
    else:
        print "New test files are available. Some of the new files:"
        print newTestFiles
        print "Run iModelEvolutionTests-PublishTestPackage job to create a new test file package."
        sys.exit(1)

if __name__ == "__main__":
    main()
