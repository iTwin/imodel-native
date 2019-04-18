#--------------------------------------------------------------------------------------
#
#  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
#
#--------------------------------------------------------------------------------------
import ftplib
from urlparse import urlparse
import ftputil
import sys
import os
import socket
import shutil
import pickle
import subprocess

return_list = []

def VerifySharedPath(path):    
    if not os.path.exists(path):        
        print path,"does not exists"
        sys.exit(1)

def Download_From_Shared_Path(srcPath,destPath,buildNo,FileName,subPath,language_prefix):
    
    newBuildAvailable = "false"
    
    VerifySharedPath(srcPath)    
    dirList = os.listdir(srcPath)    
    Lasttested_build_no=''
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
    
    SRC_PATH = os.path.join(srcPath,newest)
        
    VerifySharedPath(SRC_PATH)    

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
            if FileName == "navigator":
                print 'inside navigator if '
                os.mkdir(DEST_PATH)
                print 'dest path '+str(DEST_PATH)
                files = os.listdir(SRC_PATH)
                print 'files  '+str(files)
                #files =[i for i in files if FileName in i]
                print files
                if len(files) >0 :
                    #FileName = FileName.strip('*')
                    for file in files:
                        print 'file '+str(file)
                        DEST_PATH1 = os.path.join(DEST_PATH,file)
                        SRC_PATH1 = os.path.join(SRC_PATH,file)
                        print 'dest path '+str(DEST_PATH1)
                        print 'src pth '+str(SRC_PATH1)
                        os.mkdir(DEST_PATH1)           
                        subfiles=os.listdir(SRC_PATH1)
                        for sfile in subfiles:
                            print 'subfile '+str(sfile)
                            FileName = sfile
                            FILE_PATH = os.path.join(DEST_PATH1,FileName)
                            SRC_PATH_tem = os.path.join(SRC_PATH1,FileName)
                            print '*** Downloading "%s" Started: Please wait....'%FileName
                            shutil.copy2(SRC_PATH_tem,FILE_PATH)        
                else:
                    return_list.insert(0,"Fail")
                    return_list.insert(1,LATEST)
                    return return_list
            else:    
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


srcPath=sys.argv[1]
destPath=sys.argv[2]
buildNO=sys.argv[3]
fileName=sys.argv[4]
subPath=sys.argv[5]
langPrefix=sys.argv[6]

if (subPath == "null" and langPrefix == "null"):
    Download_From_Shared_Path(srcPath,destPath,buildNO,fileName,'','')
elif (langPrefix == "null"):
    Download_From_Shared_Path(srcPath,destPath,buildNO,fileName,subPath,'')
 
