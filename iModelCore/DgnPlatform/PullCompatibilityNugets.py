#----------------------------------------------------------------------
#
#     $Source: PullCompatibilityNugets.py $
#
#  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
#
#----------------------------------------------------------------------
import sys
import os
import zipfile
from shutil import copyfile, rmtree

#------------------------------------------------------------------------
# bsimethod                         Kyle.Abramowitz             05/2018
#------------------------------------------------------------------------
def pullAllNugets(path):
    rmtree(path)
    os.makedirs(path)
    #copyfile("C:\\Users\\kyle.abramowitz\\OneDrive - Bentley Systems, Inc\\Desktop\\ecobjects.nupkg", path + "ecobjects.nupkg")
    #copyfile("C:\\Users\\kyle.abramowitz\\OneDrive - Bentley Systems, Inc\\Desktop\\bim0200.nupkg", path + "bim0200.nupkg")

#------------------------------------------------------------------------
# bsimethod                         Kyle.Abramowitz             05/2018
#------------------------------------------------------------------------
def main():
    if len(sys.argv) < 2:
        print "Must give the nuget path"
        return
    
    nugetPath = sys.argv[1]
    pullAllNugets(nugetPath)
    # All these packages should be of the form DgnCompatiblityNugetxx.xx.xx.xx

    for filename in os.listdir(nugetPath):
        zip_ref = zipfile.ZipFile(nugetPath + filename, "r")
        split = os.path.join(nugetPath, os.path.splitext(filename)[0])
        if os.path.isdir(split):
            rmtree(split, ignore_errors=True)
        zip_ref.extractall(split)
        zip_ref.close()

if __name__ == "__main__":
    main()