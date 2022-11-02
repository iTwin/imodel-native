#---------------------------------------------------------------------------------------------
#  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
#  See LICENSE.md in the repository root for full copyright notice.
#---------------------------------------------------------------------------------------------
import sys,shutil

if __name__ == '__main__':
    dest = len(sys.argv)-1

    for i in range(0,dest):
        shutil.copy (sys.argv[i], sys.argv[dest])
