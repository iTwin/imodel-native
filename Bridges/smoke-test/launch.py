import sys
import os
import argparse

tempDir = os.path.dirname(os.path.abspath(__file__))
scriptsDir = os.path.dirname(tempDir)
BASE_DIR = os.path.dirname(scriptsDir)
commonScriptsDir = os.path.join(BASE_DIR, 'CommonTasks')
sys.path.append(commonScriptsDir)#Path of SharePointData.py file
import BuildAutoDownloader

tempDir1 = os.path.dirname(os.path.abspath(__file__))
scriptsDir1 = os.path.dirname(tempDir1)
BASE_DIR1 = os.path.dirname(scriptsDir1)
bulkConversionDir = os.path.join(BASE_DIR1, 'DgnV8Conversion') #path of bulk conversion script

parser = argparse.ArgumentParser()
parser.add_argument('-s', '--Source', help = "Source Path of Bridge server", required=True)
parser.add_argument('-d', '--Dest', help = "Destination Path on your local machine", required=True)
parser.add_argument('-b', '--BuildVer', help = "Build Number (Only Initials) e.g. 060102", required=True)
parser.add_argument('-p', '--SubPath', help ="SubPath in the Bridge build path")
parser.add_argument('-l', '--Lang', help = "set Language Prefix for Bridge build")
parser.add_argument('-f', '--FileName', help = "File Name or File Extension (.exe ,.msi ,.zip etc)   If file name is not mentioned; Script will download all contents either from source or from sub path")


args = parser.parse_args()
sourcePath = args.Source
destPath = args.Dest
buildNumber = args.BuildVer

dirList = os.listdir(destPath)    
Lasttested_build_no=''
LATEST = buildNumber
for dir in dirList:
    if buildNumber in dir:
        if LATEST < dir:
            LATEST = dir

DEST = destPath+"\\"+LATEST+"\\"     
os.system(tempDir+"\SilentInstaller.py "+str(DEST)+" "+str(args.FileName))



 




