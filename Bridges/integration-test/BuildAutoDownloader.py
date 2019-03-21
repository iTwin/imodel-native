#Copied this from 

import ftplib
from urlparse import urlparse
import sys
import os
import socket
import shutil
import pickle
import subprocess
import argparse

return_list = []

def VerifySharedPath(path):    
    if not os.path.exists(path):        
        print path,"does not exists"
        sys.exit(1)

def Download_From_Shared_Path(srcPath,destPath,buildNo,FileName,subPath,language_prefix):
    
    newBuildAvailable = "false"
    
    VerifySharedPath(srcPath)    
    dirList = os.listdir(srcPath)    
    #find latest build directory using highest number in the directory list
    #LATEST = buildNo
    os.chdir(srcPath)
    files = sorted(os.listdir(os.getcwd()), key=os.path.getmtime)
    newest = files[-1]
    LATEST=newest
    if language_prefix == None:
        language_prefix = ''
    for dir in dirList:
       if buildNo in dir and language_prefix in dir:
            if LATEST < dir:
                LATEST = dir
    print '***LATEST build is '+ newest
    print 'FileName '+str(FileName)
    
    SRC_PATH = srcPath
        
    DEST_PATH = os.path.join(destPath,newest)
    
    if subPath != None: #subpath is given, join it with source path and verify 
        SRC_PATH = os.path.join(SRC_PATH,newest)        
        VerifySharedPath(SRC_PATH)

    print 'LATEST build will be downloaded from ',SRC_PATH
    #print os.listdir(SRC_PATH)
    if not os.path.exists(DEST_PATH):   #latest is build is not downloaded yet
        newBuildAvailable= "true"
        if FileName == '':     #check whether source path is a file or directory then copy it through respective functions.
            print 'inside if '
            try:
                shutil.copytree(SRC_PATH,DEST_PATH)               
            except shutil.Error as e:
                 print('Error: %s' % e)
                 return_list.insert(0,"Fail")
                 return_list.insert(1,LATEST)
                 return return_list
            except IOError as e:
                  print('Error: %s' % e.strerror)
                  return_list.insert(0,"Fail")
                  return_list.insert(1,LATEST)
                  return return_list
        else:
            print 'inside else '
            try:         
                if SRC_PATH.endswith(FileName):        
                    DEST_PATH=DEST_PATH.strip(FileName)
                    os.mkdir(DEST_PATH)
                    shutil.copy(SRC_PATH,DEST_PATH)
                    print "download in progress"
                else:
                    os.mkdir(DEST_PATH) 
                    files = os.listdir(SRC_PATH)  
                    files =[i for i in files if FileName in i]
                    print files
                    if len(files) >0 :
                        #FileName = FileName.strip('*')
                        for file in files:
                            FileName = file
                            FILE_PATH = os.path.join(DEST_PATH,FileName)
                            SRC_PATH_tem = os.path.join(SRC_PATH,FileName)     
                            print '*** Downloading "%s" Started: Please wait....'%FileName           
                            shutil.copy2(SRC_PATH_tem,FILE_PATH)        
                    else:
                        return_list.insert(0,"Fail")
                        return_list.insert(1,LATEST)
                        return return_list
            except shutil.Error as e:
                print('Error: %s' % e)
                return_list.insert(0,"Fail")
                return_list.insert(1,LATEST)
                return return_list
            except IOError as e:
                print('Error: %s' % e.strerror)
                return_list.insert(0,"Fail")
                return_list.insert(1,LATEST)
                return return_list

        print '*** Latest build is avaialable at %s ' % DEST_PATH
        print '*** Build is there. Happy Testing!'
        return_list.insert(0, "Pass")
        return_list.insert(1, DEST_PATH)
        return return_list, newBuildAvailable
    else:
        print "LATEST BUILD IS ALREADY DOWNLOADED"
        return_list.insert(0,"LATEST BUILD IS ALREADY DOWNLOADED")
        return_list.insert(1,LATEST)
        return return_list, newBuildAvailable

parser = argparse.ArgumentParser()
parser.add_argument('-s', '--Source', help = "Source Path of DgnDbImporter server", required=True)
parser.add_argument('-d', '--Dest', help = "Destination Path on your local machine", required=True)
parser.add_argument('-b', '--BuildVer', help = "Build Number (Only Initials) e.g. 060102", required=True)
parser.add_argument('-f', '--FileName', help = "File Name or File Extension (.exe ,.msi ,.zip etc)   If file name is not mentioned; Script will download all contents either from source or from sub path")

args = parser.parse_args()

Download_From_Shared_Path(args.Source,args.Dest,args.BuildVer,args.FileName,'','')

 
