#----------------------------------------------------------------------
#
#     $Source: Tests/DgnProject/Compatibility/CompatibilityRunner/CopyFile.py $
#
#  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
#
#----------------------------------------------------------------------

from shutil import copyfile;
import os
import sys

#------------------------------------------------------------------------
# bsimethod                         Kyle.Abramowitz             05/2018
#------------------------------------------------------------------------
def main():
    if len(sys.argv) < 2:
        print "Must give the <src> and <dst>"
        return
    
    src = sys.argv[1]
    dst = sys.argv[2]

    os.remove(dst)
    copyfile(src, dst);

if __name__ == "__main__":
    main()