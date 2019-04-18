#--------------------------------------------------------------------------------------
#
#  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
#
#--------------------------------------------------------------------------------------
import sys,re

if __name__ == '__main__':
    fileName = sys.argv[1]

    oFile = open(fileName, 'r')
    lines = oFile.readlines()
    oFile.close()

    oFile = open(fileName, 'w')
    for line in lines:
        testline = line.strip()
        testline = testline.strip('\n')
        if 0 != len(testline):
            oFile.write (line)
    oFile.close()
