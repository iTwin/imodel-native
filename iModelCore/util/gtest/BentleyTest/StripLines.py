#----------------------------------------------------------------------------------------
#
#  $Source: BentleyTest/StripLines.py $
#
#  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
#
#----------------------------------------------------------------------------------------
import os, sys, string, datetime, stat
import xml.dom.minidom as minidom
import sys

# We don't always know what the current working directory is, so we can't use a plain relative
# path. Look up the directory of this file instead.
sys.path.append(os.path.dirname(__file__) + '\\..\\..\\..\\bsicommon\\build\\bentleybuild')

from utils import *

#-----------------------------------------------------------------------------#
#                                               Kevin.Nyman         11/09
#-------+---------+---------+---------+---------+---------+---------+---------#
def forceFileFlushAndClose (f):
    f.flush ()
    os.fsync(f.fileno())
    f.close() 

#-------------------------------------------------------------------------------------------
# bsimethod                                     Kevin.Nyman        03/2010 
#-------------------------------------------------------------------------------------------
def LogToFileAndStripToScreen(filename, totalCount, forcedIndex = 0):
    f = open (filename, "w") 
    lines = [] 
    isBuffering = False
    currentTestIndex = 0
    showRewritableLine (" ", INFO_LEVEL_Important, GREEN)

    try:
        while True:
            line = raw_input ('')
            f.write (line + "\n" )
            if True:
                failPos = line.find ("[  FAILED  ]")
                runPos = line.find ("[ RUN      ]")
                okPos = line.find ("[       OK ]")
                stackPos = line.find ("[  STACK INFO  ]")

                if -1 != failPos: # Test failed, dump the lines starting at the [ RUN      ] marker.
                    for l in lines:
                        showErrorMsg (l)
                    showErrorMsg (line + "\n")
                    lines = []
                    isBuffering = False
                else:
                    color = GREEN
                    if -1 != stackPos:
                        color = YELLOW
                    if 0 != forcedIndex:
                        showRewritableLine (str(forcedIndex) + "/" + str(totalCount) + ": " + line, INFO_LEVEL_Important, color)
                    else:
                        showRewritableLine (str(currentTestIndex) + "/" + str(totalCount) + ": " + line, INFO_LEVEL_Important, color)
                if -1 != runPos: 
                    isBuffering = True
                    currentTestIndex += 1
                if -1 != okPos:    
                    isBuffering = False
                    lines = []
                if isBuffering:
                    lines.append (line + "\n")
            f.flush() 
    except EOFError:
        if len(lines) > 0: # something happened most likely an exception that terminated the exe normally
            for l in lines:
                showErrorMsg(l) #, INFO_LEVEL_Important, RED)

        pass 

    forceFileFlushAndClose (f)

#-------------------------------------------------------------------------------------------
# bsimethod                                     Kevin.Nyman        03/2010 
#-------------------------------------------------------------------------------------------
if __name__ == '__main__':
    if len (sys.argv) > 3:
        LogToFileAndStripToScreen (sys.argv[1], sys.argv[2], sys.argv[3])
    else:
        LogToFileAndStripToScreen (sys.argv[1], sys.argv[2])
