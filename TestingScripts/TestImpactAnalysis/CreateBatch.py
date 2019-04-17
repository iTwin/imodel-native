#--------------------------------------------------------------------------------------
#
#     $Source: TestImpactAnalysis/CreateBatch.py $
#
#  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
#
#--------------------------------------------------------------------------------------
import sys
import os


#-------------------------------------------------------------------------------------------
# bsimethod                                     Majd.Uddin    02/2018
#-------------------------------------------------------------------------------------------
def main():
    command = sys.argv[1] + ' ' + sys.argv[2]
    filter = ""
    with open(sys.argv[3]) as f:
        lines = f.readlines()
        for line in lines:
            filter = filter + line
    command = command + ' ' + filter + ' | ' + sys.argv[4] + ' ' + sys.argv[5]

    logsDir = os.path.dirname(sys.argv[5])
    batFile = os.path.join(logsDir, 'run.bat')
    print 'writing to file: ' + batFile

    with open(batFile, 'w') as f:
        f.write(command)
        f.write('\n')
        f.write('exit')

main()