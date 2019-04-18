#--------------------------------------------------------------------------------------
#
#  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
#
#--------------------------------------------------------------------------------------
import argparse, os, subprocess, sys

#----------------------------------------------------------------------------------------------------------------------------------------------------
def set_vsts_env(key, value):
    print('##vso[task.setvariable variable={0}]{1}'.format(key, value))

#----------------------------------------------------------------------------------------------------------------------------------------------------
def main():
    argParser = argparse.ArgumentParser(description="Increments, commits, and pushes the imodeljsnodeaddon package version.")
    argParser.add_argument("-f", "--file", required=True, help='Path to the file with a version to update')
    argParser.add_argument("-b", "--bump", required=True, help='Increment pattern to apply (e.g. 0.1.0 to go from x.123.y to x.124.y); prepend with @ for a hard-coded version')
    argParser.add_argument("-n", "--dryRun", default=False, action='store_true', help='Prints diagnostic output without actually making modifications')

    args = argParser.parse_args()

    verFileName = os.path.expandvars(args.file)
    if not verFileName or not os.path.exists(verFileName) or os.path.isdir(verFileName):
        sys.stderr.write('Bad version file: "' + verFileName + '".\n')
        return 1

    bump = args.bump
    if not bump or not '.' in bump:
        sys.stderr.write('Bad version bump specifier: "' + bump + '".\n')
        return 1

    if '@' == bump[0]:
        newVer = bump[1:]

        print('Setting absolute version "' + newVer + '" in "' + verFileName + '".')

        if not args.dryRun:
            with open(verFileName, 'w') as verFile:
                verFile.write(newVer)
                verFile.write('\n')

    else:
        with open(verFileName, 'r') as verFile:
            oldVer = verFile.read().strip()

        verSplit = [int(i) for i in oldVer.split('.')]
        bumpSplit = [int(i) for i in bump.split('.')]

        if len(bumpSplit) > len(verSplit):
            sys.stderr.write('Bump version (' + bump + ') too large for existing version (' + oldVer + ').\n')
            return 1

        didBump = False
        for i in range(len(bumpSplit)):
            # Reset more minor quads if a prior major one bumped if not otherwise specified
            if didBump and (bumpSplit[i] == 0):
                verSplit[i] = 0
            else:
                verSplit[i] += bumpSplit[i]
            
            if not didBump:
                didBump = (0 != bumpSplit[i])

        verSplit = [str(i) for i in verSplit]
        newVer = '.'.join(verSplit)

        print('Incrementing version "' + oldVer + '" to "' + newVer + '" in "' + verFileName + '".')

        if not args.dryRun:
            with open(verFileName, 'w') as verFile:
                verFile.write(newVer)
                verFile.write('\n')

    cmd = 'git add ' + verFileName
    print(cmd)
    if not args.dryRun:
        os.system(cmd)
    
    cmd = 'git commit -m "' + newVer + '" --author="platform-bld <platform-bld@bentley.com>"'
    print(cmd)
    if not args.dryRun:
        os.system(cmd)

    cmd = 'git rev-parse HEAD'
    print(cmd)
    if not args.dryRun:
        rev = subprocess.check_output(cmd.split(' '))
        set_vsts_env("NewHeadRevision", rev)

    return 0

#----------------------------------------------------------------------------------------------------------------------------------------------------
if __name__ == '__main__':
    sys.exit(main())
