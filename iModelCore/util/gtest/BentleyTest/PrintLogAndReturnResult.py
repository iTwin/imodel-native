#--------------------------------------------------------------------------------------
#
#     $Source: BentleyTest/PrintLogAndReturnResult.py $
#
#  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
#
#--------------------------------------------------------------------------------------
import os, sys, string, datetime, stat
import subprocess

#-------------------------------------------------------------------------------------------
# bsimethod                                     Kevin.Nyman        03/2010 
#-------------------------------------------------------------------------------------------
def getResults (logfile):
    f = open(logfile, "r")
    result = 0
    
    for line in f.readlines():
        line = line.strip('\n ')
        if line.startswith ("RunTests.py exited with: "):
            result = int(line.split(" ")[3])
        else:
            print line
    f.close()

    return result

#-------------------------------------------------------------------------------------------
# bsimethod                                     Kevin.Nyman        03/2010 
#-------------------------------------------------------------------------------------------
if __name__ == '__main__':
    lastFail = 0
    results = {}
    for i in range (1, len (sys.argv)):
        result = getResults (sys.argv[i]) 
        results[sys.argv[i]] = result
        if 0 != result:
            lastFail = result 
            print "Test output reported tests failed in {0} with a result of {1}.".format (sys.argv[i], str (result)) 

    for key in results.keys():
        print "{0} exited with {1}.".format (key, str(results[key]))

    if 0 != lastFail:
        exit (lastFail)

