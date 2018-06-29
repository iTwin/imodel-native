#----------------------------------------------------------------------
#
#     $Source: Tests/DgnProject/Compatibility/CompatibilityRunner/ShouldPublishPackage.py $
#
#  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
#
#----------------------------------------------------------------------
import sys
from filecmp import dircmp
import os.path

#------------------------------------------------------------------------
# Public Domain from: https://stackoverflow.com/questions/4187564/recursive-dircmp-compare-two-directories-to-ensure-they-have-the-same-files-and
# bsimethod                         Kyle.Abramowitz             05/2018
#------------------------------------------------------------------------
def is_same(dir1, dir2):
    """
    Compare two directory trees content.
    Return False if they differ, True is they are the same.
    """
    compared = dircmp(dir1, dir2)
    if (compared.left_only or compared.right_only or compared.diff_files 
        or compared.funny_files):
        return False
    for subdir in compared.common_dirs:
        if not is_same(os.path.join(dir1, subdir), os.path.join(dir2, subdir)):
            return False
    return True

#------------------------------------------------------------------------
# bsimethod                         Kyle.Abramowitz             05/2018
#------------------------------------------------------------------------
def main():
    currentDatasetPath = sys.argv[1]
    oldDatasetPath = sys.argv[2]
    if is_same(currentDatasetPath, oldDatasetPath):
        sys.exit(0)
    else:
        print "Current dataset is diffent than newest dataset. A new one needs to be published. Run iModelSchemaEvolutionTests-PublishDataset job"
        sys.exit(1)

if __name__ == "__main__":
    main()
