#----------------------------------------------------------------------------------------
#
#  $Source: MergeFileLists.py $
#
#  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
#
#----------------------------------------------------------------------------------------
import os, sys
import bentley_symlinks

files = []

def merge (input):
    inputFile = open (input, 'r')
    for line in inputFile.readlines ():
        paths = line.split ()
        for pathAndName in paths:
            path = pathAndName
            if -1 != pathAndName.find (';'):
                path = path.split (';')[0]
            realPath = bentley_symlinks.getSymlinkTarget (path)
            exists = False
            for existingPath in files:
                if -1 != existingPath.find (';'):
                    existingPath = existingPath.split (';')[0]
                if bentley_symlinks.getSymlinkTarget (existingPath) == realPath:
                    exists = True
                    break
            if exists == False:
                files.append (pathAndName)
    return

def main ():
    numArgs = len (sys.argv)
    if numArgs > 2:
        i = 2
        while i < numArgs:
            merge (sys.argv[i])
            i = i + 1
            
        outputFile = open (sys.argv[1], 'w')
        for path in files:
            outputFile.write (path + '\n')
        
    else:
        print 'Specify input file and at least one output file'

if __name__ == '__main__':
    main()
