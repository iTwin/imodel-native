#---------------------------------------------------------------------------------------------
#  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
#  See LICENSE.md in the repository root for full copyright notice.
#---------------------------------------------------------------------------------------------
import locale, json, subprocess, sys, os, re
from . import compat, globalvars, internal, symlinks, utils

# Constants for processOutput return values
TERMINATE_PROCESS = 1

# The encoding used by the shell
s_localeEncoding = locale.getpreferredencoding()

if os.name == 'nt':
    import win32process
    # Windows process priority classes
    processPriorities = [
                         win32process.IDLE_PRIORITY_CLASS,
                         win32process.BELOW_NORMAL_PRIORITY_CLASS,
                         win32process.NORMAL_PRIORITY_CLASS,
                         win32process.ABOVE_NORMAL_PRIORITY_CLASS,
                         win32process.HIGH_PRIORITY_CLASS,
                         win32process.REALTIME_PRIORITY_CLASS
                        ]

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def beginBackGroundIOPriority ():
        # Lowers the IO priority for the Python process if priority is set to zero. Hard to tell if it makes any difference.
        # I'm not sure if it translates to the spawned processes or not.
        if not win32process.IDLE_PRIORITY_CLASS == getProcessPriority ():
            return

        import win32api, win32con
        processHandle = win32api.OpenProcess(win32con.PROCESS_ALL_ACCESS, True, os.getpid())
        win32process.SetPriorityClass(processHandle , 1048576) #PROCESS_MODE_BACKGROUND_BEGIN 0x00100000
        win32api.CloseHandle (processHandle)

    #    win32process.SetPriorityClass(self.m_processHandle , 2097152) #PROCESS_MODE_BACKGROUND_END 0x00200000

else:
    processPriorities = [None, None, None, None, None, None]

    def beginBackGroundIOPriority ():
        pass

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def decodeCmdOutput (cmdOutputStr):
    # First try using the shell code page and then try unicode. The latter is done with replacing any unknown characters so worst case it will have a few blobs.
    try:
        retStr = cmdOutputStr.decode (s_localeEncoding)
    except UnicodeDecodeError:
        retStr = cmdOutputStr.decode ('utf-8', errors='replace')
    return retStr

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def runAndWait (command, workingDirectory=None, outputProcessor=None, commandInfoLevel=utils.INFO_LEVEL_SomewhatInteresting, procEnv=None, unixShellExecute=False):
    command, quotedCommand = processCommand (command)
    
    priority = getProcessPriority ()
    utils.showInfoMsg (' '.join(quotedCommand) + "\n", commandInfoLevel)

    try:
        if not __isRealFile (sys.stdout):         # happens when running under debugger
            runningProcess = subprocess.Popen (command, env=procEnv, stdout=subprocess.PIPE, stdin=subprocess.PIPE, stderr=subprocess.STDOUT, cwd=workingDirectory, creationflags=priority)
        else:
            if unixShellExecute and os.name == 'posix':
                runningProcess = subprocess.Popen (' '.join(command), shell=True, env=procEnv, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, cwd=workingDirectory, creationflags=priority)
            else:
                runningProcess = subprocess.Popen (command, env=procEnv, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, cwd=workingDirectory, creationflags=priority)
    except OSError as err:
        raise err # Just here as a place to put a breakpoint

    nextLine = runningProcess.stdout.readline()
    while nextLine:
        if compat.py3:
            nextLine = decodeCmdOutput(nextLine) 
        if outputProcessor == None:
            utils.showInfoMsg (nextLine, utils.INFO_LEVEL_SomewhatInteresting)
        else:
            outProcStatus = outputProcessor (nextLine)
            if TERMINATE_PROCESS == outProcStatus:
                runningProcess.terminate()
            
        nextLine = runningProcess.stdout.readline()
    
    status = runningProcess.wait()
    return status

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def runAndWaitSeparateErrors (command, workingDirectory=None, commandInfoLevel=utils.INFO_LEVEL_SomewhatInteresting, procEnv=None, unixShellExecute=False):
    # We need this method for Azure CLI which will dump a mountain of stuff to stderr even on a success.
    # Returns status, stdout, stderr. 
    command, quotedCommand = processCommand (command)
    
    priority = getProcessPriority ()
    utils.showInfoMsg (' '.join(quotedCommand) + '\n', commandInfoLevel)

    try:
        if not __isRealFile (sys.stdout):         # happens when running under debugger
            runningProcess = subprocess.Popen (command, env=procEnv, stdout=subprocess.PIPE, stdin=subprocess.PIPE, stderr=subprocess.PIPE, cwd=workingDirectory, creationflags=priority)
        else:
            if unixShellExecute and os.name == 'posix':
                runningProcess = subprocess.Popen (' '.join(command), shell=True, env=procEnv, stdout=subprocess.PIPE, stderr=subprocess.PIPE, cwd=workingDirectory, creationflags=priority)
            else:
                runningProcess = subprocess.Popen (command, env=procEnv, stdout=subprocess.PIPE, stderr=subprocess.PIPE, cwd=workingDirectory, creationflags=priority)
    except OSError as err:
        raise err # OS Error occured; can set breakpoint here

    stdoutBuf, stderrBuf = runningProcess.communicate()
    if compat.py3:
        stdoutBuf = decodeCmdOutput (stdoutBuf) 
        stderrBuf = decodeCmdOutput (stderrBuf) 

    return runningProcess.returncode, stdoutBuf.split('\n'), stderrBuf.split('\n')

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def runAndLog (command, logFile, cwdir):

    def processOutput (outputLine):
        # For some reason a lot of lines come back as \r\r\n so rstrip and append.
        utils.showAndLogMsg(outputLine.rstrip()+'\n', logFile, utils.INFO_LEVEL_Important)
        utils.flushInfoStream()

    return runAndWait (command, cwdir, processOutput)

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def runAndLogWithEnv (command, procEnv, logFile, cwdir):

    def processOutput (outputLine):
        # For some reason a lot of lines come back as \r\r\n so rstrip and append.
        utils.showAndLogMsg(outputLine.rstrip()+'\n', logFile, utils.INFO_LEVEL_Important)
        utils.flushInfoStream()

    return runAndWait (command, cwdir, processOutput, utils.INFO_LEVEL_SomewhatInteresting, procEnv)

#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
def getProcessPriority ():
    # The only use for this value is to pass into subprocess.Popen creationFlags=, which is nt-only.
    # Note that for whatever reason, passing 0 when posix works, whereas None does not.
    if 'nt' != os.name:
        return 0

    if None == globalvars.programOptions or None == globalvars.programOptions.processPriority:
        return 0
    return processPriorities[globalvars.programOptions.processPriority]

#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
def processCommand (command):
    if isinstance(command, str):
        command = command.strip().split()  # Need to split into list for Unix.
    quotedCommand = ['"{0}"'.format(arg) if ' ' in arg else arg for arg in command]
    return command, quotedCommand

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def containsUnexpandedVars (strToMatch):
    # Find tokens of the format ${varname} in a string
    return (None != re.search (r"\$\{.*\}", strToMatch))


#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
# The return code from Robocopy is a bit map, defined as follows:
#     Hex   Decimal  Meaning if set
#     0x10  16       Serious error. Robocopy did not copy any files.
#                    Either a usage error or an error due to insufficient access privileges
#                    on the source or destination directories.
# 
#     0x08   8       Some files or directories could not be copied
#                    (copy errors occurred and the retry limit was exceeded).
#                    Check these errors further.
# 
#     0x04   4       Some Mismatched files or directories were detected.
#                    Examine the output log. Some housekeeping may be needed.
# 
#     0x02   2       Some Extra files or directories were detected.
#                    Examine the output log for details. 
# 
#     0x01   1       One or more files were copied successfully (that is, new files have arrived).
# 
#     0x00   0       No errors occurred, and no copying was done.
#                    The source and destination directory trees are completely synchronized.
def robocopyStatus (status):
    chkStatus = status & ~0x3
    # Return in our usual format - 0 for success.
    return 0 if chkStatus == 0 else status

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def roboCopyDir (fromDir, toDir, outputProcessorFunc=None): #pylint: disable=function-redefined
                                                            # Method gets overwritten with "cmdutil.roboCopyDir=...". Could use a better tactic.
    cmd = getRoboCopyCommand (fromDir, toDir)

    if containsUnexpandedVars (fromDir):
        utils.showInfoMsg ("Warning: robocopy source path contains unexpanded variables: {0}\n".format (fromDir), utils.INFO_LEVEL_VeryInteresting, utils.YELLOW)

    if not os.path.exists (fromDir):
        raise utils.PartPullError ("Cannot find source directory for robocopy: {0}".format (fromDir), None)

    status = runAndWait (cmd, outputProcessor=outputProcessorFunc)
    return robocopyStatus (status)
        
#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def roboCopyDirCheckModified (fromDir, toDir):
    # Split out this function because checking the output will be slower if you don't care about it
    modified = [False]
    def processOutput (outputLine):
        utils.showInfoMsg (outputLine, utils.INFO_LEVEL_SomewhatInteresting)
        if modified[0] == False:
            #  New File  \t\t    1264\t\\\\builds\\prgbuilds-r...
            #  Older     \t\t    7355\t\\\\builds\\prgbuilds-r
            #  Newer     \t\t    7164\t\\\\builds\\prgbuilds-r
            strippedLine = outputLine.strip()
            if strippedLine.startswith ('New File') or strippedLine.startswith ('Newer') or strippedLine.startswith ('Older'):
                modified[0] = True

    startTime = compat.perfCounter()
    status = roboCopyDir (fromDir, toDir, processOutput)
    utils.showInfoMsg ("roboCopyDirCheckModified from '{0}' to '{1}' took {2:0.2f} seconds\n".format(
        fromDir, toDir, compat.perfCounter() - startTime), utils.INFO_LEVEL_RarelyUseful)
    return robocopyStatus (status), modified[0]

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def getRoboCopyCommand(fromDir, toDir):
    if os.name == 'nt':
        return ['robocopy', fromDir, toDir, '/e', '/mir', '/R:5', '/W:6', '/MT:8', '/NP', '/njs', '/njh', '/ndl', '/xd', 'CVS']
    else:
        runAndWait(['mkdir', '-p', toDir])

        # If the incoming is a directory, need to make sure there's a trailing slash for rsync so it copes the contents (not the root folder)
        effectiveFromDir = fromDir
        if os.path.isdir(effectiveFromDir) and not effectiveFromDir.endswith(os.path.sep):
            effectiveFromDir += os.path.sep

        return ['rsync', '-aL', '--delete', '--exclude=CVS', effectiveFromDir, toDir]

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def roboCopyFiles (fromName, toDir):    #pylint: disable=function-redefined
                                        # Method gets overwritten with "cmdutil.roboCopyFiles=...". Could use a better tactic.
    if os.name == 'nt':
        fromDir, fromFile = os.path.split (fromName)
        status = runAndWait (['robocopy', '/njs', '/njh', '/ndl', fromDir, toDir, fromFile])
        return robocopyStatus (status)
    else:
        runAndWait(['mkdir', '-p', toDir])
        status = runAndWait (['rsync', '-aL', fromName, toDir], unixShellExecute=True)
        return status

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def rsyncListDir (fromDir):
    dirList = []
    # dr-xr-xr-x           0 2012/11/28 17:33:16 8-25-2-23\n
    # -rw-r--r--     14,221,312 2016/08/30 08:33:36 BGRSmall.bim
    prog = re.compile (r"[dl\-]\S+\s+\S+ \S+ \S+ (\S+)")

    def parseRsyncCaptureOutput (outputline):
        nameMatch = prog.search (outputline)
        if nameMatch:
            dirList.append (nameMatch.group(1).strip())

    remoteDir = fromDir + '/*'
    __rsyncRun (remoteDir, '', '--list-only', allowRetries=True, outProc=parseRsyncCaptureOutput)

    if 0 == len(dirList):
        raise utils.PartPullError ("No files in directory from rsync: {0}".format (remoteDir), None)
        
    return dirList

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def rsyncStandardArg ():
    return '-vrzt --delete --perms' 

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def rsyncDir (fromDir, toDir, outputProcessor=None, skipPermissionReset=None):      #pylint: disable=function-redefined
                                                                                    # Method gets overwritten with "cmdutil.rsyncDir=...". Could use a better tactic.

    # Since robocopy wants the destination dir, but rsync will create it, check and see
    #   if the destination and source match, and if so truncate the destination.
    lastIndex = fromDir.rfind (os.sep)
    if lastIndex == -1 : lastIndex = 0
    fromDestDir = fromDir[lastIndex:]

    lastIndex = toDir.rfind (os.sep)
    if lastIndex == -1 : lastIndex = 0
    toDestDir = toDir[lastIndex:]

    if fromDestDir == toDestDir:
        toDir = toDir[0:lastIndex]
    filesRead =  __rsyncRun (fromDir, toDir, rsyncStandardArg(), allowRetries=True, outProc=outputProcessor)

    # rsync with --perms creates a mess of security descriptors. Rsync without --perms still doesn't respect the inherited permissions
    # and makes a different mess of the security descriptors. So we go back after the fact and just reset it.
    if not skipPermissionReset:
        resetPermissions (toDir)

    return filesRead

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def rsyncFile (fromFile, toFile, outProc=None):
    return __rsyncRun (fromFile, toFile, rsyncStandardArg(), allowRetries=True, outProc=outProc)

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def __getRsyncExe ():
    if os.name == 'nt':
        if 'RsyncPath' in os.environ:
            return os.path.join (os.environ['RsyncPath'], 'rsync.exe')

        try:
            return utils.GetToolFile (globalvars.TOOLS_CWRSYNC, 'rsync.exe')
        except utils.BuildError:
            pass

        # Used by hybrid CVS builds to find rsync.exe
        if 'SrcUtil' in os.environ:
            cwExe = os.path.join (os.environ['SrcUtil'], 'platform', 'bin', 'cwrsync', 'rsync.exe')
            if os.path.exists (cwExe):
                return cwExe
            raise utils.PartPullError ("Rsync not found in SrcUtil; cannot find rsync", None)

        # Have to use if from source because this is used during pull rather than build.
        upackSource = globalvars.buildStrategy.GetUpackSource ('cwrsync', None)
        cwExe = os.path.join (upackSource.GetLocalDownloadDir(), upackSource.GetPackageDirectory(), 'rsync.exe')
        if os.path.exists (cwExe):
            return cwExe
        else:
            utils.showInfoMsg ("Cannot find rsync in ToolCache", utils.INFO_LEVEL_VeryInteresting, utils.RED)

        raise utils.PartPullError ("Cannot find rsync in SrcRoot/bsitools, RsyncPath, or ToolCache", None)

    elif os.name == 'posix':
        return 'rsync'


#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def __rsyncRun (fromName, toDir, options, allowRetries, outProc = None):
    startTime = compat.perfCounter()
    cwExe = __getRsyncExe ()
    local = __fixAndCreateRsyncDestDir (toDir)
    remote = fromName.replace ('\\', '/')
    # Count the number of files copied
    endOfSectionRe = re.compile ('sent [0-9]+ bytes.*')
    copiedFileCount = [0, 0]
    retryErrorFound = [False]

    # Rsync can fail remotely; give it a couple retries so pull doesn't have to start from scratch
    totalRetries = 3 if allowRetries else 1
    retriesRemaining = [totalRetries]

    # Build the command
    cmd = [cwExe]
    cmd.extend (options.split())
    cmd.extend ([remote, local])

    # Store output to dump in case of error (regardless of verbosity level)
    bufferedRsyncOutput = []
    for _ in range(0,totalRetries):
        bufferedRsyncOutput.append([])
    def bufferAndCheckRsyncOutput (outputLine):
        utils.showInfoMsg (outputLine, utils.INFO_LEVEL_SomewhatInteresting)
        bufferedRsyncOutput[totalRetries-retriesRemaining[0]-1].append(outputLine)

        outputLine = outputLine.strip()
        if 'Connection timed out' in outputLine:
            retryErrorFound[0] = True
        elif 'Connection reset by peer' in outputLine:
            retryErrorFound[0] = True
        if 0 == copiedFileCount[1]:  # Looking for start
            if outputLine.startswith ('sending incremental file list') or outputLine.startswith ('receiving incremental file list'):
                copiedFileCount[1] = 1
        elif 1 == copiedFileCount[1]:  # Counting files
            if endOfSectionRe.match (outputLine):
                copiedFileCount[1] = 2
            elif len (outputLine) > 0:
                copiedFileCount[0] += 1
        else: # hit end of file section
            pass

    status = 0
    while retriesRemaining[0] > 0:
        retriesRemaining[0] -= 1
        retryErrorFound[0] = False
        status = runAndWait (cmd, outputProcessor=bufferAndCheckRsyncOutput)
        if retriesRemaining[0] and True == retryErrorFound[0]:
            failMsg = 'rsync failure; retrying. {0} retries remain\n'.format(retriesRemaining[0])
            utils.showInfoMsg (failMsg, utils.INFO_LEVEL_SomewhatInteresting)
            bufferedRsyncOutput[totalRetries-retriesRemaining[0]-1].append(failMsg + "\n")
            continue
        if not status: # Success
            break
        if status and False == retryErrorFound[0]:
            break

    if outProc:
        for outputLine in bufferedRsyncOutput[totalRetries-retriesRemaining[0]-1]:
            outProc(outputLine)

    utils.showInfoMsg ("__rsyncRun from '{0}' to '{1}' took {2:0.2f} seconds\n".format(fromName, toDir, compat.perfCounter() - startTime), utils.INFO_LEVEL_RarelyUseful)

    if status:
        raise utils.PartPullError ('Error rsyncing.\nFailing Command: "{0}"\nCommand output: \n{1}'.format (' '.join(cmd), "".join([outputLine for attemptBuffer in bufferedRsyncOutput for outputLine in attemptBuffer])), None)
    return copiedFileCount[0]

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def __fixAndCreateRsyncDestDir (toDir):
    if toDir == '':  # For listing a directory, we use a blank destination
        return toDir
    if os.name == 'nt' and toDir[1] != ':':
        raise utils.PartPullError ("Must have a full output path for rsync ({0})".format(toDir), None)
    symlinks.makeSureDirectoryExists (toDir)

    local = toDir
    if toDir[1] == ':':
        local = '/cygdrive/' + toDir[0] + toDir[2:]
    return local.replace ('\\', '/')

#-------------------------------------------------------------------------------------------
# This is only to determine if we're running under a debugger
# bsimethod
#-------------------------------------------------------------------------------------------
def __isRealFile(rfile):
    if not hasattr(rfile, 'fileno'):
        return False
    try:
        tmp = os.dup(rfile.fileno())
    except:
        return False
    else:
        os.close(tmp)
        return True

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def resetPermissions (permissionDir):
    if os.name != 'nt':
        return

    if os.environ.get ('NO_RSYNC_PERM_ADJUST', None): # Back door while we have this coming on line. Should be removed later.
        return

    cmd = ['icacls.exe', permissionDir, '/T', '/Q', '/reset']
    runAndWait (cmd)

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def signFileCommand (binaryToSign, signToolExe, logFile=None, _signFileCmdParams="", cwDir=None):
    if 'BSI_SIGNING_TOKEN' in os.environ:
        remotesigningclient = utils.GetToolFile (globalvars.TOOLS_SIGN_TOOL_CLIENT, internal.SIGNING_CLIENT)
        cmd = [remotesigningclient, "sign", "--service-url", internal.SIGNING_SERVICE_URL, binaryToSign]
    else:
        signFileCmd = utils.GetSharedMkiFile ("signfile.py")
        cmd = [sys.executable, signFileCmd, signToolExe, binaryToSign]
        return runAndLogWithEnv (cmd, None, logFile, cwDir)

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
class PythonPrereqChecker(object):
    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def __init__ (self):
        self.PIP_COMMAND = [sys.executable, '-m', 'pip', 'install']
        self.m_pipChecked = False
        self.m_minVerCache = {}

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def PythonPackageAvailable (self, pkgName):
        import pkgutil
        return True if pkgutil.find_loader(pkgName) else False

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def InstallItem (self, itemName, command):
        if utils.isUnattendedBuild():
            utils.showInfoMsg ('Error: required package must be installed with the following command: {0}\n'.format(' '.join(command)), utils.INFO_LEVEL_Essential, utils.RED)

            if sys.platform in ['darwin', 'linux2', 'linux']:
                utils.showInfoMsg ('    You seem to be on a Unix-based system.\n', utils.INFO_LEVEL_Essential)
                utils.showInfoMsg ('    If the above command fails with a permission error, please run it again with sudo:\n', utils.INFO_LEVEL_Essential)
                utils.showInfoMsg ('    sudo ' + ' '.join(command) + '\n', utils.INFO_LEVEL_Essential)

            sys.exit (1)

        utils.showInfoMsg ('You are missing a required Python package. Would you like  to install {0} now? [y/n]'.format(itemName), utils.INFO_LEVEL_Essential)
        utils.flushInfoStream ()
        ans = sys.stdin.readline()
        if ans.lower()[0] == 'y':
            # If the user is using a system copy of Python on a Unix system, pip requires elevated
            # privledges to install. We don't otherwise need (or want) to run BB under sudo, so just
            # instruct the user to call it manually.
            def scanForPermIssue(line):
                if sys.platform not in ['darwin', 'linux2', 'linux']:
                    return 0

                # Example errors I have seen on MacOS:
                #   OSError: [Errno 13] Permission denied: '/Library/Python/2.7/site-packages/xxhash-1.0.1.dist-info'
                #   error: could not create '/Library/Python/2.7/site-packages/xxhash': Permission denied
                if not re.search(r'error.*permission denied', line, flags=re.IGNORECASE):
                    return 0

                utils.showInfoMsg('\n**************************************************\n', utils.INFO_LEVEL_Essential)
                utils.showInfoMsg('You must manually install ' + itemName + ' with elevated privleges.\n', utils.INFO_LEVEL_Essential)
                utils.showInfoMsg('Run: sudo ' + ' '.join(command) + '\n', utils.INFO_LEVEL_Essential)
                utils.showInfoMsg('**************************************************\n\n', utils.INFO_LEVEL_Essential)
                
                utils.exitOnError(1, 'Failed to install python package ' + itemName)

            utils.showInfoMsg ('Running command: {0}\n'.format(' '.join(command)), utils.INFO_LEVEL_Essential)
            runAndWait (command, outputProcessor=scanForPermIssue)
        else:
            utils.showInfoMsg ('Error: required package not installed\n', utils.INFO_LEVEL_Essential, utils.RED)
            #sys.exit (1)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def EnsureMinVersion(self, itemName, minVersion):
        # Normalize to M.m.b format
        # Python version specs defined here: https://www.python.org/dev/peps/pep-0386/
        minVer = minVersion.split ('.')
        while len(minVer) < 3:
            minVer.append('0')

        # Check cache for performance
        if itemName in self.m_minVerCache and minVer in self.m_minVerCache[itemName]:
            return None

        try:
            import pkg_resources
            pkgVersion = pkg_resources.get_distribution(itemName).version
            verList = pkgVersion.split ('.')
            if verList[0] == '0':
                utils.showInfoMsg ('Pip version of {0} = {1}\n'.format(itemName, pkgVersion), utils.INFO_LEVEL_RarelyUseful)
                utils.showErrorMsg ('Error: Unable to get version of pip package {0}\n'.format(itemName))
                sys.exit (1)

            while len(verList) < 3:
                verList.append('0')

            if not (verList[0] >= minVer[0]) and (verList[1] >= minVer[1]) and (verList[2] >= minVer[2]):
                cmd = self.PIP_COMMAND + [itemName] + ['-U'] # -U for upgrade
                if utils.isPrg():
                    utils.showInfoMsg ('Error: required package must be updated with the following command: {0}\n'.format(' '.join(cmd)), utils.INFO_LEVEL_Essential, utils.RED)
                    sys.exit (1)
                else:
                    utils.showInfoMsg ('Your version of a required Python package is too low ({0} < {1}). Would you like to update {2} now? [y/n] '.format('.'.join(verList), '.'.join(minVer), itemName), utils.INFO_LEVEL_Essential)
                    ans = sys.stdin.readline()
                    if ans.lower()[0] == 'y':
                        utils.showInfoMsg ('Running command: {0}\n'.format(' '.join(cmd)), utils.INFO_LEVEL_Essential)
                        runAndWait (cmd)
                    else:
                        utils.showInfoMsg ('Error: required package not updated\n', utils.INFO_LEVEL_Essential, utils.RED)
                        #sys.exit (1)

            # Cache that this package meets this minVer requirement
            if itemName in self.m_minVerCache:
                self.m_minVerCache[itemName] = self.m_minVerCache[itemName] + [minVer]
            else:
                self.m_minVerCache[itemName] = [minVer]

        except Exception as err:
            raise err

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def CheckPip(self):
        # Only want to do this if we need to install something
        if self.m_pipChecked:
            return
            
        # Check if pip is installed at all
        if not self.PythonPackageAvailable ('pip'):
            utils.showErrorMsg ('Error: Missing python package: {0}\n'.format('pip'))
            if os.name == 'nt':
                if compat.py3:
                    self.InstallItem ('pip', [sys.executable, r'\\winxx\DevProg\Python\PackageManagers\get-pip-py3.py'])
                else:
                    self.InstallItem ('pip', [sys.executable, r'\\winxx\DevProg\Python\PackageManagers\get-pip.py'])
            else:
                utils.showErrorMsg ('Please run "sudo easy_install pip"\n')
                sys.exit (1)
            
        # Check the pip version. Trying to import it and check __version__ failed in some of the ActiveState installs so gave up on that.
        pipVersion = []
        pipErrLines = []
        pipVersion.append ('0.0.0')
        def ParsePipOutput (line):
            # pip 1.4.1 from c:\python27\lib\site-packages (python 2.7)
            # pip 8.1.2 from C:\DevTools\Python27\lib\site-packages (python 2.7)
            pipErrLines.append('CheckPip.ParsePipOutput: "' + line.strip() + '"')
            if not line.startswith ('pip'):
                return
            spaceIndex = 5
            while line[spaceIndex] != ' ' and spaceIndex<len(line):
                spaceIndex+=1
            pipVersion[0] = line[4:spaceIndex]
        
        cmd = [sys.executable, '-m', 'pip', '--version']
        pipErrLines.append('CheckPip: Running "' + ' '.join(cmd) + '"')
        runAndWait (cmd, outputProcessor=ParsePipOutput)
        pipErrLines.append('CheckPip: Captured "' + pipVersion[0] + '"')
        verList = pipVersion[0].split ('.')
        if verList[0] == '0':
            utils.showErrorMsg ('Error: Unable to get version of pip using: {0}\n'.format(cmd))
            for l in pipErrLines:
                print(l)
            sys.exit (1)

        if int(verList[0]) < 6:  # Older versions do not support trusted-host; not sure but think 6 is where it appeared.
            utils.showErrorMsg ('Error: The version of the pip python package is too old\n')
            self.InstallItem ('[pip version 8 update]', self.PIP_COMMAND[:-1] + [sys.executable, '-m', 'pip', '--upgrade'])
        elif int(verList[0]) < 8:
            utils.showErrorMsg ('Error: The version of the pip python package is too old\n')
            self.InstallItem ('[pip version 8 update]', self.PIP_COMMAND + [sys.executable, '-m', 'pip', '--upgrade'])

        self.m_pipChecked = True

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def CheckPackage (self, itemName, minVersion=None):
        lowerName = itemName.lower()
        if not self.PythonPackageAvailable (lowerName):
            self.CheckPip ()
            cmd = self.PIP_COMMAND + [lowerName]
            utils.showErrorMsg ('Error: python package {0} not available.\n'.format(itemName))
            self.InstallItem (itemName, cmd)
        if minVersion != None:
            self.CheckPip ()
            self.EnsureMinVersion(lowerName, minVersion)

#--------------------------------------------------------------------------------
# @bsimethod
#--------------------------------------------------------------------------------
def getVSVarsFromRegistryDir (versionsToCheck):
    """ Try to get vsvars32.bat by looking up Visual Studio Installations from the registry.
    versionsToCheck is an array of versions. 10.0 for VS2010, 9.0 for VS2008, 8.0 for VS2005.
    returns None if not found.
    """

    pathToUse = None
    for version in versionsToCheck:
        if version == "16.0" or version == "17.0":
            installDirs = getVSInstallationPaths (version)
            if len(installDirs) != 0 and None != installDirs[0]:
                pathToUse = os.path.join (installDirs[0], "Common7\\Tools\\")
                return pathToUse
        elif version == "15.0":
            vsRegRoot = "SOFTWARE\\WOW6432Node\\Microsoft\\VisualStudio\\SxS\\VS7"
            installDir, _ = utils.queryForRegistryEntry (vsRegRoot, version, force32bitRegistry=True)
            if None != installDir:
                pathToUse = os.path.join (installDir, "Common7\\Tools\\")
                return pathToUse
        else:
            vsRegRoot = "SOFTWARE\\Microsoft\\VisualStudio\\{0}".format (version)
            installDir, _ = utils.queryForRegistryEntry (vsRegRoot, "InstallDir", force32bitRegistry=True)

            if None != installDir:
                pathToUse = installDir.replace ("\\IDE\\", "\\Tools\\")
                if os.path.isfile (os.path.join (pathToUse, "vsvars32.bat")):
                    return pathToUse

        utils.showInfoMsg ("Couldn't find VSVars for version {0}.\n".format(version), utils.INFO_LEVEL_SomewhatInteresting)

    return None

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def getVSInstallationPaths (versionToGet):
    vsPaths = []

    def append(outputLine):
        vsPaths.append(outputLine.strip())

    vswherePath = os.path.join (os.environ["PROGRAMFILES(X86)"], "Microsoft Visual Studio\\Installer\\vswhere.exe")

    searchString = versionToGet
    if searchString == "16.0":
        searchString = "(16.0,17.0]"
    elif searchString == "17.0":
        searchString = "(17.0,18.0]"

    if os.path.exists(vswherePath):
        cmd = [vswherePath, "-products", "Microsoft.VisualStudio.Product.Enterprise", "Microsoft.VisualStudio.Product.Professional", "Microsoft.VisualStudio.Product.BuildTools", "-prerelease", "-version", searchString, "-property", "installationPath"]
        runAndWait(cmd, outputProcessor=append)

    return vsPaths

g_microsoftprovidertokens = {}

#---------------------------------------------------------------------------------------
# bsimethod
#---------------------------------------------------------------------------------------
def GetMicrosoftProviderToken (address):
    if os.name != 'nt':
        return None  # On Linux/Mac use Azure CLI.
        
    if address in g_microsoftprovidertokens:
        return g_microsoftprovidertokens[address]

    tokens = []
    outputLines = []
    def outputFunction (outputLine):
        if outputLine.startswith("{"):
            credentials = json.loads(outputLine)
            tokens.append("Bearer {}".format(credentials["Password"]))
        else:
            outputLines.append(outputLine)
            utils.showInfoMsg (outputLine, utils.INFO_LEVEL_RarelyUseful)

    status = runAndWait ([utils.GetToolFile ('microsoft_credential_provider', 'CredentialProvider.Microsoft.exe'), "-U", address, "-F", "Json", "-I", "true"], outputProcessor=outputFunction)
    if status:
        utils.showInfoMsg ("Failed to get Microsoft Provider Token:\n{0}".format ('\n'.join(outputLines)),  utils.INFO_LEVEL_VeryInteresting)
        return None
    if len (tokens) != 1:
        utils.showInfoMsg ("Expected number of tokens from Microsoft Provider Token is 1. Got {0}:\n{1}".format (len (tokens), '\n'.join(tokens)), utils.INFO_LEVEL_VeryInteresting)
        return None

    g_microsoftprovidertokens[address] = tokens[0]
    return tokens[0]

