#--------------------------------------------------------------------------------------
#
#     $Source: Android/CreateAssetsManifest.py $
#
#  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
#
#--------------------------------------------------------------------------------------
import os, sys
import hashlib

if __name__ == '__main__':
    if (len (sys.argv) < 3):
        print "Syntax: ", sys.argv[0], " manifestFile assetsDirectory"
        exit (1)
        
    manifestFile    = sys.argv[1]
    assetsDirectory = sys.argv[2]
    if assetsDirectory.endswith ('/') or assetsDirectory.endswith ('\\'):
        assetsDirectory = assetsDirectory[0 : len (assetsDirectory) - 1]

    assets = ''
    hash   = hashlib.sha1 ()
    for (walkRoot, walkDirectoryNames, walkFileNames) in os.walk (assetsDirectory, topdown=True, onerror=None, followlinks=True):
        for filename in walkFileNames:
            if os.path.islink (filename):
                filename = os.path.realpath (filename)
            sourcePath = os.path.join (walkRoot, filename)
            
            assets += os.path.relpath (sourcePath, assetsDirectory) + '\n'
            f = open (sourcePath, 'rb')
            hash.update (f.read ())
            f.close ()
    
    manifest = open (manifestFile, 'w')    
    manifest.write (hash.hexdigest () + '\n')
    manifest.write (assets)
    manifest.close ()
    exit (0)