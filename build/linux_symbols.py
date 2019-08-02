#!/usr/bin/env python
#--------------------------------------------------------------------------------------
#  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
#--------------------------------------------------------------------------------------
import argparse, fnmatch, os, subprocess, sys, time

#----------------------------------------------------------------------------------------------------------------------------------------------------
def logError(msg): print('##vso[task.logissue type=error;]' + msg)
def logWarning(msg): print('##vso[task.logissue type=warning;]' + msg)

#----------------------------------------------------------------------------------------------------------------------------------------------------
def doubleToTimeString(start, end):
    d = (end - start)
    if d < 60.0: return "{:0.2f} second(s)".format(d)
    if d < 60.0*60.0: return "{:0.2f} minutes(s)".format(d / 60.0)
    return "{:0.2f} hour(s)".format(d / (60.0*60.0))

#----------------------------------------------------------------------------------------------------------------------------------------------------
def findBinaryFiles(dirToSearch, globPatterns):
    matchingFiles = []

    for path in os.listdir(dirToSearch):
        fullPath = os.path.join(dirToSearch, path)
        if os.path.isdir(fullPath):
            matchingFiles.extend(findBinaryFiles(fullPath, globPatterns))
            continue
        
        if any(g for g in globPatterns if fnmatch.fnmatch(path, g)):
            matchingFiles.append(fullPath)
    
    return matchingFiles

#----------------------------------------------------------------------------------------------------------------------------------------------------
def runCommand(cmd):
    startTime = time.time()
    print('> ' + cmd)
    status = subprocess.call(cmd, shell=True)
    endTime = time.time()
    
    if status != 0:
        logError('Command "' + cmd + '" failed with status code ' + str(status))
        return False
    
    print('Command completed successfully in ' + doubleToTimeString(startTime, endTime))
    return True

#----------------------------------------------------------------------------------------------------------------------------------------------------
def doCompress(files):
    for f in files:
        if not runCommand('objcopy --compress-debug-sections ' + f):
            return 1
    
    return 0

#----------------------------------------------------------------------------------------------------------------------------------------------------
def doUpload(files, authToken, orgSlug, projSlug):
    if not authToken:
        logError('No authToken provided to upload symbols.')
        return 1
    
    allFiles = ' '.join(files)
    
    # Print diagnostic information about the files first.
    if not runCommand('sentry-cli difutil check ' + allFiles):
        return 1
    
    if not runCommand('sentry-cli --auth-token ' + authToken + ' upload-dif -t elf -o ' + orgSlug + ' -p ' + projSlug + ' ' + allFiles):
        return 1
    
    return 0

#----------------------------------------------------------------------------------------------------------------------------------------------------
def doStrip(files):
    for f in files:
        if not runCommand('objcopy --strip-debug --strip-unneeded ' + f):
            return 1
    
    return 0

#----------------------------------------------------------------------------------------------------------------------------------------------------
def main():
    argParser = argparse.ArgumentParser(
        formatter_class=argparse.ArgumentDefaultsHelpFormatter,
        description='Walks a directory for binaries and performs various Linux symbol operations.'
    )
    argParser.add_argument('-d', dest='dirToSearch', default='.', help='Path to search')
    argParser.add_argument('-t', dest='authToken', help='Sentry API authentication token (required for upload)')
    argParser.add_argument('-o', dest='orgSlug', help='Sentry API organization slug (required for upload)')
    argParser.add_argument('-p', dest='projSlug', help='Sentry API project slug (required for upload)')
    argParser.add_argument('action', choices=['compress', 'upload', 'strip'], help='Action to perform')
    argParser.add_argument('globPatterns', nargs='*', default=['*.a', '*.so'], help='Filename glob pattern(s) to indicate binaries')
    args = argParser.parse_args()

    if 'upload' == args.action and (not args.authToken or not args.orgSlug or not args.projSlug):
        argParser.error('action "upload" requires -t, -o, and -p')

    args.rawDirToSearch = args.dirToSearch
    args.dirToSearch = os.path.realpath(args.rawDirToSearch)

    print('==================================================')
    print(os.path.basename(sys.argv[0]) + ' args:')
    print('    dirToSearch = ' + args.rawDirToSearch + ' (=> ' + args.dirToSearch + ')')
    print('    authToken = ' + ('<Yes>' if args.authToken else '<No>'))
    print('    orgSlug = ' + (args.orgSlug if args.orgSlug else '<None>'))
    print('    projSlug = ' + (args.projSlug if args.projSlug else '<None>'))
    print('    action = ' + args.action)
    print('    globPatterns = ' + ' '.join(args.globPatterns))
    print('==================================================')

    files = findBinaryFiles(args.dirToSearch, args.globPatterns)

    print('Matching files:')
    for f in files:
        print('    ' + f)
    print('==================================================')

    if len(files) == 0:
        logWarning('No files found to process.')
        return 0

    if args.action == 'compress': return doCompress(files)
    if args.action == 'upload': return doUpload(files, args.authToken, args.orgSlug, args.projSlug)
    if args.action == 'strip': return doStrip(files)

    logError('Unknown action.')
    return 1

#----------------------------------------------------------------------------------------------------------------------------------------------------
if __name__ == '__main__':
    sys.exit(main())
