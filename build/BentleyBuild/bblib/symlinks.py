#---------------------------------------------------------------------------------------------
#  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
#  See LICENSE.md in the repository root for full copyright notice.
#---------------------------------------------------------------------------------------------
import os, threading, sys, time
py3 = (int(sys.version[0]) > 2)

class SymLinkError (Exception):
    def __init__(self, message, trace=None):
        self.errmessage = message
        if trace:
            self.stackTrace = trace
        else:
            import traceback
            self.stackTrace = traceback.extract_stack()
        Exception.__init__ (self)
    def __str__(self):
        return "\n" + self.errmessage

class SymLinkTargetError (SymLinkError):
    def __init__(self, message, trace=None):
        SymLinkError.__init__(self, message)

GET_SYM_LINK_STATUS_Success     = 0
GET_SYM_LINK_STATUS_NotSymLink  = 1
GET_SYM_LINK_STATUS_Error       = 2

# We need to do all symlink operations singly; otherwise 2 threads will try to make the same directory, etc.
s_symlinkLock = threading.Lock()

#-------------------------------------------------------------------------------------------
# WINDOWS SPECIFIC SYMLINKS
#-------------------------------------------------------------------------------------------
if os.name == 'nt':
    import pywintypes, struct, win32file, win32con, winioctlcon

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def isSymbolicLink (pathName):
        if py3:
            return os.path.islink(pathName)

        attr = win32file.GetFileAttributes (pathName)
        if attr == -1:
            return False

        return (0 != (attr & 0x400))

    #----------------------------------------------------------------------------------------
    # @bsimethod
    #----------------------------------------------------------------------------------------
    # Calls getSymlinkImmediateTarget until it does not reach a symbolic link. There may still be directory symbolic links embedded in the path.
    def getSymlinkFinalTarget(path):
        while True:
            if not isSymbolicLink(path):
                return path if os.path.exists (path) else None

            linkTargetPath = getLinkTargetRealPath(path)
            if None == linkTargetPath:
                break

            path = linkTargetPath

        return path if os.path.exists (path) else None

    #----------------------------------------------------------------------------------------
    # @bsimethod
    #----------------------------------------------------------------------------------------
    def getFinalPath(path):
        succeeded = False
        retries = 5
        while retries > 0:
            try:
                handle = win32file.CreateFile(path, win32file.GENERIC_READ, win32file.FILE_SHARE_READ, None, win32file.OPEN_EXISTING, win32file.FILE_FLAG_BACKUP_SEMANTICS, 0)
                succeeded = True
            except win32file.error as err:
                if err.winerror == 32:
                    pass
                else:
                    raise err
            if succeeded:
                break
            retries -= 1
            time.sleep (1)

        # Try again as to actually throw if the retries don't help.
        # The calling function already handles the error identification.
        if not succeeded:
            handle = win32file.CreateFile(path, win32file.GENERIC_READ, win32file.FILE_SHARE_READ, None, win32file.OPEN_EXISTING, win32file.FILE_FLAG_BACKUP_SEMANTICS, 0)

        finalName = None
        try:
            finalName = win32file.GetFinalPathNameByHandle(handle, win32con.FILE_NAME_OPENED)
        except win32file.error as err:
            if err.winerror == 2:
                # This is for docker where the source is mounted in the container.
                finalName = getSymlinkFinalTarget (path)
            if not finalName:
                raise err
        except win32file.error as err:
            raise err
        finally:
            handle.Close()

        if finalName:
            # GetFinalPathNameByHandle always returns a long-name path (e.g. \\?\C:\... or \\?\UNC\host\share\...).
            # Aside from being ugly, not all commands and shells support this syntax, so try to normalize.
            if len(finalName) > 6 and ":" == finalName[5]:
                return finalName[4:]

            if len(finalName) > 7 and "UNC" == finalName[4:7]:
                return "\\" + finalName[7:]

        return finalName

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def tryGetFinalSymlinkTarget(linkname):
        try:
            return [GET_SYM_LINK_STATUS_Success, getFinalPath(linkname), None]
        except Exception as err:
            return [GET_SYM_LINK_STATUS_Error, None, err]

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def resetFileTime (linkname, target):
        if not os.path.isfile(target): return

        fstat = os.stat (target)
        handle = win32file.CreateFile (linkname, win32file.GENERIC_WRITE, win32file.FILE_SHARE_WRITE, None,
                              win32file.OPEN_EXISTING, win32file.FILE_FLAG_OPEN_REPARSE_POINT|win32file.FILE_FLAG_BACKUP_SEMANTICS, 0)
        win32file.SetFileTime (handle, pywintypes.Time(fstat[9]), pywintypes.Time(fstat[7]), pywintypes.Time(fstat[8])) # pylint: disable=no-member
        handle.Close()

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def deleteSymLink (linkName):
        os.unlink (linkName)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def createOsSymLink (linkName, targetFile, isDir):
        try:
            dirFlag = 1 if isDir else 0
            win32file.CreateSymbolicLink (linkName, targetFile, dirFlag)
            return None
        except winioctlcon.pywintypes.error as err:  # pylint: disable=no-member
            # Perhaps another thread created this symlink already, so return success if it matches what we wanted
            if os.path.exists (linkName):
                if (checkSameTarget (linkName, targetFile)):
                    return None
            # Otherwise, something else went wrong so raise the error
            msg = "####CreateSymlink error: {0}, {1}".format (err.args[2], linkName)
            raise SymLinkError (msg)
    
    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def getLinkTargetRealPath(pathIn):
        pathOut = None
        handle = None
        try:
            handle = win32file.CreateFile(pathIn, win32file.GENERIC_READ, win32file.FILE_SHARE_READ, None, win32file.OPEN_EXISTING, win32file.FILE_FLAG_OPEN_REPARSE_POINT | win32file.FILE_FLAG_BACKUP_SEMANTICS, 0)
            bufIn =  win32file.AllocateReadBuffer(1024)
            bufOut = win32file.DeviceIoControl(handle, winioctlcon.FSCTL_GET_REPARSE_POINT, None, bufIn, None)
            headerFormat = 'lhhhhhhl'
            headerSize = struct.calcsize(headerFormat)
            # reparseTag, reparseDataLength, reserved, subNameOffset, subNameLength, printNameOffset, printNameLength, flags = struct.unpack(headerFormat, bufOut[:headerSize])
            _, _, _, _, _, printNameOffset, printNameLength, _ = struct.unpack(headerFormat, bufOut[:headerSize])
            pathStart = headerSize + printNameOffset
            pathEnd   = pathStart + printNameLength
            if py3:
                pathOut = bufOut[pathStart:pathEnd].tobytes().decode('utf-16')
            else:
                pathOut = bufOut[pathStart:pathEnd].decode('utf-16')
        except winioctlcon.pywintypes.error: # pylint: disable=no-member
            pass
            #typically win32error.ERROR_NOT_A_REPARSE_POINT (4390)
        finally:
            if handle:
                handle.Close()
        return pathOut

#-------------------------------------------------------------------------------------------
# LINUX SPECIFIC SYMLINKS
#-------------------------------------------------------------------------------------------
elif os.name == 'posix':
    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def isSymbolicLink (pathName):
        return os.path.islink (pathName)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def tryGetFinalSymlinkTarget(linkname):
        while isSymbolicLink(linkname):
            try:
                linkname = os.readlink(linkname)  # pylint: disable=no-member
            except Exception as err:
                return [GET_SYM_LINK_STATUS_Error, None, err]

        return [GET_SYM_LINK_STATUS_Success, linkname, None]

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def resetFileTime (_linkname, target):
        if not os.path.isfile(target):
            return

        os.stat (target)
        #os.utime (linkname, (fstat.st_atime, fstat.st_mtime))

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def deleteSymLink (linkName):
        os.unlink (linkName.rstrip(os.path.sep))
        # linkName = linkName.rstrip(os.path.sep)
        # if py3:
        #     os.unlink(linkName)
        # else:
        #     if os.path.isdir (linkName):
        #         os.rmdir (linkName)
        #     else:
        #         os.unlink (linkName)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def createOsSymLink (linkname, targetFile, _isDir):
        try:
            os.symlink (targetFile, linkname)  # pylint: disable=no-member
        except OSError as err:
            # Perhaps another thread created this symlink already, so return success if it matches what we wanted
            if os.path.exists (linkname):
                if (checkSameTarget (linkname, targetFile)):
                    return None
            # Otherwise, something else went wrong so raise the error
            msg = "####CreateSymlink error: {0} - {1} -> {2} (hint: check capitalization of target file path)".format (err, linkname, targetFile)
            raise SymLinkError (msg)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def getLinkTargetRealPath(pathIn):
        pathOut = None
        try:
            pathOut = os.path.realpath(pathIn)
        except:
            pass
        return pathOut

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def _getSymlinkTarget(linkname, throwError=True):
    # Internal version - no additional locking.
    status, linkTarget, err = tryGetFinalSymlinkTarget(linkname)
    
    # Allow non-symbolic links to be resolved in this same way. This ensures, for example on Windows, that path canonicalization occurs equally.
    if status not in [GET_SYM_LINK_STATUS_Success, GET_SYM_LINK_STATUS_NotSymLink]:
        errString = "Exception is: '{0}'".format (repr(err))
        target = getLinkTargetRealPath(linkname)
        if target and not os.path.exists(target):
            if not throwError:
                return target
            raise SymLinkError ("####Target \n'{0}' of link\n'{1}' does not exist.\n    (status {2}); an OS error occured. {3}".format(target, linkname, status, errString))
        else:
            raise SymLinkError ("####Cannot determine target of '{0}' (status {1}); an OS error occured. {2}".format(linkname, status, errString))

    return linkTarget

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def getSymlinkTarget (linkname):
    # External version - thread safe
    with s_symlinkLock:                  # Locks thread but releases it when function exits
        return _getSymlinkTarget (linkname)

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def symlinkTargetExists (linkname):
    with s_symlinkLock:
        status, _, _ = tryGetFinalSymlinkTarget(linkname)
        return True if GET_SYM_LINK_STATUS_Success == status else False

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def isTargetSame (linkname, testTarget):
    try:
        return _getSymlinkTarget(normalizePathName(testTarget)).lower() == _getSymlinkTarget(normalizePathName(linkname)).lower()
    except SymLinkError:
        return False

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def checkSameTarget (linkName, testTarget):
    try:
        currTarget = _getSymlinkTarget(linkName)
        testTarget = _getSymlinkTarget(testTarget)
    except SymLinkError:
        return False

    if testTarget.lower() != currTarget.lower():
        return False

    return True

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def __makeSureDirectoryExists (dirName):
    if os.path.exists(dirName):
        return
    try:
        os.makedirs (dirName)
    except OSError as err:
        if not os.path.exists(dirName):  # Directory was created in another thread, probably by bmake
            raise err

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def makeSureDirectoryExists (dirName):
    # This is the external interface
    with s_symlinkLock:                  # Locks thread but releases it when function exits
        __makeSureDirectoryExists (dirName)

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def makeSureBaseDirectoryExists (filename):
    dirname = os.path.dirname (filename)
    __makeSureDirectoryExists (dirname)

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def normalizePathName (pathName):
    # On windows abspath will convert \ to /, but the reverse isn't true on Posix.
    return os.path.abspath(os.path.expandvars(pathName)).replace ('\\', os.sep)

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def createFileSymLink (linkName, targetFile, checkSame=True, checkTargetExists=False, skipIntermediateLinks=False):
    with s_symlinkLock:                  # Locks thread but releases it when function exits
        linkName   = normalizePathName (linkName)
        targetFile = normalizePathName (targetFile)

        targetExists = os.path.exists (targetFile)

        if checkTargetExists and not targetExists:
            raise SymLinkError ("####Target File {0} does not exist".format (targetFile))

        if skipIntermediateLinks and targetExists and isSymbolicLink(targetFile):
            targetFile = _getSymlinkTarget (targetFile)

        linkExistanceCheck = os.path.islink(linkName) if py3 else os.path.exists(linkName)

        if linkExistanceCheck:
            if (checkSameTarget (linkName, targetFile)):
                if os.path.getmtime(targetFile) - os.path.getmtime(linkName) > 1:
                    # File got modified, update file links file-time so scripts tageting it know the original file was changed.
                    resetFileTime (linkName, targetFile)
                return 0
            if checkSame:
                raise SymLinkTargetError ("####Cannot link:\n '{0}' to: \n '{1}', because it is already linked to: \n '{2}'".format (linkName, targetFile, _getSymlinkTarget (linkName, throwError=False)))
            deleteSymLink (linkName)

        makeSureBaseDirectoryExists (linkName)
        createOsSymLink (linkName, targetFile, False)

        resetFileTime (linkName, targetFile)    # set link file to date/time of target
        return 1

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def createDirSymLink (linkName, targetDir, checkSame=True, checkTargetExists=False, skipIntermediateLinks=False):
    with s_symlinkLock:                  # Locks thread but releases it when function exits
        linkName  = normalizePathName(linkName)
        targetDir = normalizePathName(targetDir)

        targetExists = os.path.exists (targetDir)
        if checkTargetExists and not targetExists:
            raise SymLinkError ("####Target Directory {0} does not exist".format (targetDir))

        if skipIntermediateLinks and targetExists:
            targetDir = _getSymlinkTarget (targetDir)

        linkExistanceCheck = os.path.islink(linkName) if py3 else os.path.exists(linkName)

        if linkExistanceCheck:
            if (checkSameTarget (linkName, targetDir)):
                return 0
            if checkSame:
                raise SymLinkTargetError ("####Cannot link:\n '{0}' to: \n '{1}', because it is already linked to: \n '{2}'".format (linkName, targetDir, _getSymlinkTarget (linkName, throwError=False)))
            deleteSymLink (linkName)

        makeSureBaseDirectoryExists (linkName)
        createOsSymLink (linkName, targetDir, True)
        return 1

#-------------------------------------------------------------------------------------------
# Method to symlink hg.exe from the path to a target dir (used for transkit delivery)
# bsimethod
#-------------------------------------------------------------------------------------------
def createHgSymlink (linkName):
    exeName, delim = ('hg.exe', ';') if os.name == 'nt' else ('hg', ':')
    for pathEntry in os.environ['PATH'].split(delim):
        hgExe = os.path.join (pathEntry, exeName)
        if os.path.exists (hgExe):
            return createDirSymLink (linkName, pathEntry)

    raise SymLinkError ("###CreateSymLink error: Cannot find {0} in the PATH".format (exeName))

def main():
    testingDir = "D:\\testingpython"
    if os.path.exists(testingDir):
        import shutil
        # py2 fails to delete symlinks that don't have a target
        shutil.rmtree (testingDir, ignore_errors=True)
    os.makedirs(testingDir)

    testDir1 = os.path.join(testingDir, "test1")
    testDir2 = os.path.join(testingDir, "test2")
    linkDir1 = os.path.join(testingDir, "link1")
    os.makedirs(testDir1)
    os.makedirs(testDir2)

    # Test directory symlink creation
    if createDirSymLink(linkDir1, testDir1) != 1:
        print ("Failed symlink creation to directory")

    # Test directory symlink detection
    if not isSymbolicLink(linkDir1):
        print ("Failed to identify a directory symbolic link")

    # Test directory symlink deletion
    deleteSymLink(linkDir1)
    if py3:
        if os.path.exists(linkDir1) or os.path.islink(linkDir1):
            print ("Failed to delete directory symlink")
    else:
        if os.path.exists(linkDir1):
            print ("Failed to delete directory symlink")

    createDirSymLink(linkDir1, testDir1)
    os.rmdir(testDir1)

    # Test directory symlink detection with target deleted
    if not isSymbolicLink(linkDir1):
        print ("Failed to identify a symbolic directory link when target is deleted")

    # Test directory symlink creation failure if different target is specified
    exceptSuccessful = False
    try:
        createDirSymLink(linkDir1, testDir2)
    except SymLinkTargetError:
        exceptSuccessful = True
    if not exceptSuccessful:
        print ("Failed to raise error for trying to create a link to a different target with one already existing")

    os.makedirs(testDir1)
    # Test directory symlink creation when link and target exist
    if createDirSymLink(linkDir1, testDir1) != 0:
        print ("Failed symlink creation to directory for same target")

    os.removedirs(testDir2)
    # Test directory symlink creation when changing the target is allowed
    if createDirSymLink(linkDir1, testDir2, checkSame=False) != 1:
        print ("Failed symlink creation to directory for existing link to different target")

    testFile1 = os.path.join(testingDir, "test1.txt")
    testFile2 = os.path.join(testingDir, "test2.txt")
    linkFile1 = os.path.join(testingDir, "link1.txt")
    with open(testFile1, 'w') as _fp: # pylint: disable=unused-variable
        pass
    with open(testFile2, 'w') as _fp: # pylint: disable=unused-variable
        pass

    # Test if you can get a target from not a symlink
    targetOfExistingFile = getSymlinkTarget(testFile1)
    if not targetOfExistingFile:
        print ("Failed to get target for existing file")

    # Test file symlink creation
    if createFileSymLink(linkFile1, testFile1) != 1:
        print ("Failed symlink creation to file")

    # Test file symlink detection
    if not isSymbolicLink(linkFile1):
        print ("Failed to identify a file symbolic link")

    # Test file symlink deletion
    deleteSymLink(linkFile1)
    if py3:
        if os.path.exists(linkFile1) or os.path.islink(linkFile1):
            print ("Failed to delete file symlink")
    else:
        if os.path.exists(linkFile1):
            print ("Failed to delete file symlink")

    createFileSymLink(linkFile1, testFile1)
    os.remove(testFile1)

    # Test file symlink detection with target deleted
    if not isSymbolicLink(linkFile1):
        print ("Failed to identify a symbolic file link when target is deleted")

    # Test file symlink creation failure if different target is specified
    exceptSuccessful = False
    try:
        createFileSymLink(linkFile1, testFile2)
    except SymLinkTargetError:
        exceptSuccessful = True
    if not exceptSuccessful:
        print ("Failed to raise error for trying to create a link to a different target with one already existing")

    with open(testFile1, 'w') as _fp:
        pass
    # Test file symlink creation when link and target exist
    if createFileSymLink(linkFile1, testFile1) != 0:
        print ("Failed symlink creation to file for same target")

    os.remove(testFile1)
    # Test file symlink creation when changing the target is allowed
    if createFileSymLink(linkFile1, testFile2, checkSame=False) != 1:
        print ("Failed symlink creation to file for existing link to different target")

    print ("Finished tests. If there are no failures printed above, they are successful.")

if __name__ == '__main__':
    main()