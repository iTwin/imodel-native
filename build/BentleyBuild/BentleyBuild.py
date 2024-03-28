#!/usr/bin/python3
#---------------------------------------------------------------------------------------------
#  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
#  See LICENSE.md in the repository root for full copyright notice.
#---------------------------------------------------------------------------------------------
import os, sys

VERSION = "3.1"

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def printPythonError (printMsg):
    # Fo use only before we can import util.
    import bblib.colorconsole as colorconsole
    with colorconsole.setTextColor (0x000C, colorconsole.UNCHANGED):   # RED
        sys.stdout.write(printMsg)

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def pyVerStr (pyver):
    return '.'.join([str(x) for x in pyver])

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
# Validate the Python versions before imports.
MINIMUM_PYTHON_3_VERSION = (3, 5)
progName = os.path.basename(sys.argv[0])   # This is what ArgumentParser subs in for %(prog)s also

pyVerString = 'Python v{0} or greater'.format (pyVerStr(MINIMUM_PYTHON_3_VERSION))
#   pyVerString += ' or python 3 v{0} or greater'.format(pyVerStr(MINIMUM_PYTHON_3_VERSION))

if sys.version_info[0] == 2:
    printPythonError ("Error: {0} requires {1} but running Python: {2}\n".format (progName, pyVerString, pyVerStr(sys.version_info)))
    sys.exit (1)
elif sys.version_info[0] == 3:
    if sys.version_info < MINIMUM_PYTHON_3_VERSION:
        printPythonError ("Error: {0} requires {1}, but running Python: {2}\n".format (progName, pyVerString, pyVerStr(sys.version_info)))
        sys.exit (1)
else:
    printPythonError ("Error: Unexpected version of python running: {0}\n".format (sys.version_info))
    sys.exit (1)

# Make sure it's not 32 bit.
if sys.maxsize <= 2**32:
    msg = r"""
=============================================================================================
BentleyBuild requires 64-bit python. Please upgrade your python to the x64 version.
When you run bb for the first time it will ask to install lxml and xxhash
=============================================================================================

"""
    printPythonError (msg)
    sys.exit (1)

try:
    import bblib.utils as utils
    import bblib.cmdutil as cmdutil
    import bblib.globalvars as globalvars
    import bblib.buildpaths as buildpaths
except ImportError as ex:
    # May be able to catch something specific here so leaving mechanism; used to catch some missing imports
    raise ex

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def checkForPythonPrereqs():
    prereqFile = None
    if buildpaths.GetSrcRoot(): # If SrcRoot is on the command line it will not be set up at this point. Devs use env vars so should be good.
        teamConfigDir = buildpaths.GetBBCacheDir()
        prereqFile = os.path.join (teamConfigDir, 'prereq.check')
        if os.path.exists (prereqFile):
            with open (prereqFile, 'rt') as pfile:
                prereqPyPath = pfile.readline().strip()
                pfile.close()
            if prereqPyPath == sys.executable:
                return

    checker = cmdutil.PythonPrereqChecker ()
    checker.CheckPackage ('lxml', '3.0')
    checker.CheckPackage ('xxhash', '1.0')

    if prereqFile:
        with open (prereqFile, 'wt') as pfile:
            pfile.write(sys.executable)

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def getBBRevision ():
    results = {'id':'0', 'date':'0-0-0', 'branch':'unknown'}
    def ParseGitLogOutput (outputLine):
        outputLine = outputLine.strip()
        if outputLine.startswith ("ID:"):
            results['id'] = outputLine[4:]
        elif outputLine.startswith ("Date:"):
            results['date'] = outputLine[6:]

    localDir = os.path.dirname(os.path.realpath(__file__))

    gitLogCmd = ['git', '-C', localDir, 'log', '-1', '--pretty=format:ID: %h%nDate: %as']
    cmdutil.runAndWait (gitLogCmd, localDir, ParseGitLogOutput)

    def ParseGitBranchOutput (outputLine):
        results['branch'] = outputLine.strip()

    gitBranchCmd = ['git', '-C', localDir, 'rev-parse', '--abbrev-ref', 'HEAD']
    cmdutil.runAndWait (gitBranchCmd, localDir, ParseGitBranchOutput)

    return ' Rev: {0}  {1}  [{2}]'.format (results['id'], results['date'], results['branch'])

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def main():

    # Prebuild checks
    try:
        checkForPythonPrereqs()
    except utils.BBException as err:
        utils.showErrorMsg (err.errmessage)
        utils.showErrorCallStack (err)
        return 1

    messages = []
    globalvars.bbVersion = VERSION + getBBRevision()
    messages.append (("BentleyBuild, version {0}\n".format (globalvars.bbVersion), utils.INFO_LEVEL_VeryInteresting))
    messages.append (("Python version {0}\n".format(sys.version.split(" (")[0]), utils.INFO_LEVEL_VeryInteresting))

    import bblib.runbb as runbb
    return runbb.RunBentleyBuild (messages)

#--------------------------------------------------------------------------------
# @bsimethod
#--------------------------------------------------------------------------------
if __name__ == '__main__':    sys.exit (main())

