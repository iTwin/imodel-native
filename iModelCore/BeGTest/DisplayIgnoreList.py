#--------------------------------------------------------------------------------------
#
#     $Source: DisplayIgnoreList.py $
#
#  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
#
#--------------------------------------------------------------------------------------
import os, sys

if __name__ == '__main__':
    if (len (sys.argv) < 2):
        print "Syntax: ", sys.argv[0], " <ignoreListRootDir>"
        exit(1)

    rootDir = sys.argv[1]
    box = len(sys.argv) > 2

    prefix = ''
    if box == True:
        prefix = '*  '

    if rootDir.endswith ('/'):
        rootDir = rootDir[0:len(rootDir)-1]

    for (walkRoot, walkDirectoryNames, walkFileNames) in os.walk (rootDir, topdown=True, onerror=None, followlinks=True):

        relDir = walkRoot[len(rootDir)+1:]

        for filename in walkFileNames:

            if not os.path.basename (filename) == "ignore_list.txt":
                continue

            ignoreListFilePath = os.path.join (walkRoot, filename)
            ignoreListFile = open (ignoreListFilePath, 'r')
            ignoredlines = ignoreListFile.readlines()
            ignored = ''
            for ignoredlineRaw in ignoredlines:
                ignoredline = ignoredlineRaw
                # strip comments
                startComment = ignoredline.find ('#')
                if startComment != -1:
                    ignoredline = ignoredline[0:startComment]
                # strip leading and trailing blanks
                ignoredline = ignoredline.strip()
                # see if the line identifies anything other than performance tests (which we always suppress)
                if len(ignoredline) == 0:
                    continue
                if ignoredline.lower().find ('performance') != -1:
                    continue
                # Yes, this looks like a directive to ignore a real test. Report it.    
                ignored = ignored + prefix + ignoredlineRaw.strip() + '\n'

            if len(ignored) > 0:
                ignored = ignored[0:len(ignored)-1]
                print prefix + 'Some tests are being ignored'
                print prefix
                print prefix + os.path.join (relDir, filename)
                print prefix + '--------------------------------'
                print ignored
                print prefix


    exit(0)