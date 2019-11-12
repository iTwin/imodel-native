#---------------------------------------------------------------------------------------------
#  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
#  See COPYRIGHT.md in the repository root for full copyright notice.
#---------------------------------------------------------------------------------------------
import os, sys, tempfile

#----------------------------------------------------------------------------------------------------------------------------------------------------
def set_vsts_env(key, value): print('##vso[task.setvariable variable={0}]{1}'.format(key, value))

#----------------------------------------------------------------------------------------------------------------------------------------------------
def main():
    if len(sys.argv) < 2:
        sys.stderr.write('Must provide path to BAT file.\n')
        sys.exit(1)

    commandString = ' '.join(x if not ' ' in x else '"' + x + '"' for x in sys.argv[1:])
    
    envBeforePath = tempfile.mktemp()
    envAfterPath = tempfile.mktemp()
    
    # Run the command and capture the environment change
    #..............................................................................................
    captureCommand = 'cmd.exe /c set > "{0}" && {1} && set > "{2}"'.format(envBeforePath, commandString, envAfterPath)
    
    os.system(captureCommand)

    # Read the before and after environments
    #..............................................................................................
    if not os.path.exists(envBeforePath):
        sys.stderr.write('ERROR: Capture command did not create the "before" environment file.\n')
        sys.exit(1)

    beforeEnv = []
    with open(envBeforePath) as envBeforeFile:
        beforeEnv = [line.strip() for line in envBeforeFile.readlines()]
    
    os.unlink(envBeforePath)

    if not os.path.exists(envAfterPath):
        sys.stderr.write('ERROR: Capture command did not create the "after" environment file.\n')
        sys.exit(1)

    afterEnv = []
    with open(envAfterPath) as envAfterFile:
        afterEnv = [line.strip() for line in envAfterFile.readlines()]

    os.unlink(envAfterPath)

    # Compute the difference between the environments.
    # Just capture entire changed variables... I don't think we need to care about exactly what changed
    #..............................................................................................
    envDiffSet = set(afterEnv).difference(set(beforeEnv))

    # Makes printouts easier to search through
    envDiff = list(envDiffSet)
    envDiff.sort()

    # Print commands
    #..............................................................................................
    for envVar in envDiff:
        key,value = envVar.split('=', 1)
        value = value.strip().replace('/', '\\')
        set_vsts_env(key, value)
    
    # Done
    #..............................................................................................
    return 0

#----------------------------------------------------------------------------------------------------------------------------------------------------
if __name__ == '__main__':
    sys.exit(main())
