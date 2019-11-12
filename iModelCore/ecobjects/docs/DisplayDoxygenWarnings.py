#---------------------------------------------------------------------------------------------
#  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
#  See COPYRIGHT.md in the repository root for full copyright notice.
#---------------------------------------------------------------------------------------------
import os, sys

if __name__ == '__main__':
    if (len (sys.argv) < 2):
        print "Syntax: ", sys.argv[0], " <warningLogFile>"
        exit(1)

    warningLogFile = open (sys.argv[1], 'r')
    warningLines = warningLogFile.readlines()
    for warningLine in warningLines:
        print warningLine

    exit(0)
