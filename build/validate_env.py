#--------------------------------------------------------------------------------------
#
#  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
#
#--------------------------------------------------------------------------------------
import os, sys

#----------------------------------------------------------------------------------------------------------------------------------------------------
def set_vsts_env(key, value):
    print(key + ' = ' + value)
    print('##vso[task.setvariable variable={0}]{1}'.format(key, value))

#----------------------------------------------------------------------------------------------------------------------------------------------------
def logError(msg):
    print('##vso[task.logissue type=error;]' + msg)

#----------------------------------------------------------------------------------------------------------------------------------------------------
def logWarning(msg):
    print('##vso[task.logissue type=warning;]' + msg)

#----------------------------------------------------------------------------------------------------------------------------------------------------
def main():
    if ('STRATS_TO_RELEASE' in os.environ) and os.environ['STRATS_TO_RELEASE']:
        if (os.name == 'nt') and ('SUBNET_PRG' not in os.environ):
            logError('Release builds must run in a PRG agent queue.')
            return 1

        print("STRATS_TO_RELEASE is '" + os.environ['STRATS_TO_RELEASE'] + "', so setting IS_FOR_RELEASE.")
        set_vsts_env('IS_FOR_RELEASE', 'true')

        return 0

    if 'SUBNET_PRG' in os.environ:
        logWarning('CI builds should NOT be run in the PRG agent queue.')

    print("STRATS_TO_RELEASE is NOT set, so NOT setting IS_FOR_RELEASE.")
    set_vsts_env('IS_FOR_RELEASE', 'false')

    return 0

#----------------------------------------------------------------------------------------------------------------------------------------------------
if __name__ == '__main__':
    sys.exit(main())
