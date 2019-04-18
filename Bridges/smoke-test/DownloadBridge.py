#--------------------------------------------------------------------------------------
#
#  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
#
#--------------------------------------------------------------------------------------
import sys
import os
import argparse

tempDir = os.path.dirname(os.path.abspath(__file__))
scriptsDir = os.path.dirname(tempDir)
BASE_DIR = os.path.dirname(scriptsDir)
commonScriptsDir = os.path.join(BASE_DIR, 'CommonTasks')
sys.path.append(commonScriptsDir)#Path of SharePointData.py file
import BuildAutoDownloader

parser = argparse.ArgumentParser()
parser.add_argument('-s', '--Source', help = "Source Path of DgnDbImporter server", required=True)
parser.add_argument('-d', '--Dest', help = "Destination Path on your local machine", required=True)
parser.add_argument('-b', '--BuildVer', help = "Build Number (Only Initials) e.g. 060102", required=True)
parser.add_argument('-p', '--SubPath', help ="SubPath in the DgnDbImporter build path")
parser.add_argument('-l', '--Lang', help = "set Language Prefix for DgnDbImporter build")
parser.add_argument('-f', '--FileName', help = "File Name or File Extension (.exe ,.msi ,.zip etc)   If file name is not mentioned; Script will download all contents either from source or from sub path")

args = parser.parse_args()
sourcePath = args.Source
destPath = args.Dest
buildNumber = args.BuildVer

Build_DownloadPath, newBuildAvailable = BuildAutoDownloader.Download_From_Shared_Path(sourcePath,destPath,buildNumber,args.FileName,args.SubPath,args.Lang)

if newBuildAvailable == "true":
    print "New Build Downloaded"
else:
    print "No New Build Available For Testing"
    exit(1)
    #job should fail
