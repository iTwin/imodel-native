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
    command = sys.argv[1] + ' ' + sys.argv[2] + ' | ' + sys.argv[3] + ' ' + sys.argv[4]

    logsDir = os.path.dirname(sys.argv[4])
    batFile = os.path.join(logsDir, 'run.bat')
    print 'writing to file: ' + batFile

    with open(batFile, 'w') as f:
        f.write(command)
        f.write('\n')
        f.write('exit')

main()