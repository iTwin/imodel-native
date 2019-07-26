#!/usr/bin/env python

import os, sys
sys.path.append(os.path.join(os.environ['SrcRoot'], 'BentleyBuild', 'bentleybuild'))
import symlinks as symlinks

'''
This creates a *physical* directory structure, with file *symlinks* in it.
Similar to bsicommon/CopyWithSymlinks.py, but is a whitelist for header files (vs. exclusion list).
'''

#--------------------------------------------------------------------------------------------------
def linkDir(srcDir, outDir):
    for childName in os.listdir(srcDir):
        childPath = os.path.join(srcDir, childName)
        outPath = os.path.join(outDir, childName)
        
        if os.path.isdir(childPath):
            linkDir(childPath, outPath)
            continue

        if not childName.lower().endswith(".h"):
            continue
        
        if not os.path.exists(outDir):
            os.makedirs(outDir)
        
        symlinks.createFileSymLink(outPath, childPath, False)

#--------------------------------------------------------------------------------------------------
def main():
    if (len (sys.argv) < 3):
        print("Syntax: {0} inputParentDirectory outputParentDirectory".format(sys.argv[0]))
        return 1

    inDir = os.path.realpath(sys.argv[1])
    outDir = os.path.realpath(sys.argv[2])

    linkDir(inDir,outDir)

    return 0

#--------------------------------------------------------------------------------------------------
if __name__ == '__main__':
    sys.exit(main())
