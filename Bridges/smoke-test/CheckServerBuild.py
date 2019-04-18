#--------------------------------------------------------------------------------------
#
#  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
#
#--------------------------------------------------------------------------------------
import sys
import os
import csv

def Download_From_Shared_Path(srcPath,buildNo):
    newBuildAvailable = "false"    
    dirList = os.listdir(srcPath)    
    Lasttested_build_no=''
    #find latest build directory using highest number in the directory list
    LATEST = buildNo
    os.chdir(srcPath)
    files = sorted(os.listdir(os.getcwd()), key=os.path.getmtime)
    newest = files[-1]
    LATEST=newest
    for dir in dirList:
       if buildNo in dir:
            if LATEST < dir:
                LATEST = dir
    print '***LATEST build is '+ newest

    with open(filePath+'\Version.log', mode='w') as employee_file:
        employee_writer = csv.writer(employee_file, delimiter=',', quotechar='"', quoting=csv.QUOTE_MINIMAL)
        LATEST=LATEST.replace('-','.')
        employee_writer.writerow([newest])                   

srcPath=sys.argv[1]
buildNO=sys.argv[2]
filePath=sys.argv[3]

Download_From_Shared_Path(srcPath,buildNO)

 
