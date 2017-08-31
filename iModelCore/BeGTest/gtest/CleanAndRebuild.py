#--------------------------------------------------------------------------------------
#
#     $Source: gtest/CleanAndRebuild.py $
#
#  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
#
#--------------------------------------------------------------------------------------
import os, sys, re

#-------------------------------------------------------------------------------------------
# bsimethod                                     Ridha.Malik      08/2017
#-------------------------------------------------------------------------------------------
if __name__ == '__main__':

    if len(sys.argv) < 7:
        print 'Syntax: ' + sys.argv[0] + ' can not rerun Gtest.exe'
        exit(1)
    dir=sys.argv[1]
    breakonfailure=sys.argv[2]
    ignorefailure=sys.argv[3]
    exe=sys.argv[4]
    dirpath=sys.argv[5]
    targetplatform=sys.argv[6]
    anyFailures = True
    logfilePath=''
    path=''
    while (anyFailures):
        for root,dirs,files in os.walk (dir, topdown=True, onerror=None, followlinks=True):
            for file in files:
                if file.endswith('.log'):
                   logfilePath = os.path.join(root, file)
                   logfile=open(logfilePath,'r')
                   for line in logfile.readlines():
                       if line.find('FAILED TEST') != -1:
                          anyFailures = True
                          logfile.close()
                          open(logfilePath,'w').close()
                          command=exe +">"+logfilePath
                          os.system(command)
                          command=dirpath+"CheckLogfilesForFailures.py "+dir+" "+breakonfailure+" "+ignorefailure+" "+targetplatform
                          os.system(command)
                       else:
                           anyFailures = False
                           logfile.close()
    


