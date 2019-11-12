#---------------------------------------------------------------------------------------------
#  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
#  See COPYRIGHT.md in the repository root for full copyright notice.
#---------------------------------------------------------------------------------------------
import os
import re
import sys
#-------------------------------------------------------------------------------------------
# bsimethod                                     Ridha.Malik      06/2018
#-------------------------------------------------------------------------------------------
def PreventAutoIgnoreTests(FailingTestsList):
    filedir=os.path.dirname(os.path.abspath(__file__))
    filepath=os.path.join(filedir,"IgnoreFilters.txt")
    filteredFile = open (filepath, 'r')
    lines = filteredFile.readlines()
    filteredlist=[]
    for filteredline in lines:
        # strip comments
        startComment = filteredline.find ('#')
        if startComment != -1 :
            filteredline = filteredline[0:startComment]
            # strip leading and trailing blanks
            filteredline = filteredline.strip()
            # see if the line identifies anything other than performance tests (which we always suppress)
            if len(filteredline) == 0:
                continue
            if filteredline.lower().find ('performance') != -1:
                continue
            filteredlist.append(filteredline)
        else:
            if not filteredline.strip():continue
            filteredline=filteredline.strip()
            filteredlist.append(filteredline)
    for filters in filteredlist:
        for failingtest in FailingTestsList:
            if filters.startswith("*") and filters.endswith("*"):
               filters=filters.replace("*",'')
            pattern=re.compile(filters)
            testobj=re.search(pattern,failingtest)
            if testobj != None:
                print ("*** compatibility test failed " +failingtest)
                return 1
    return 0
