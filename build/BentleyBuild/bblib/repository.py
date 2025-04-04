#---------------------------------------------------------------------------------------------
#  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
#  See LICENSE.md in the repository root for full copyright notice.
#---------------------------------------------------------------------------------------------
import os, re, shutil, sys, time, zipfile
from . import bbconfig, bbutils, builddescriptionfile, buildpaths, compat, cmdutil, lkgs, symlinks, utils, versionutils
from . import globalvars, azurecli, internal

RSYNC_FIXED_PATH = 'Fixed Path'

GIT_PULL_SAT    = 'gitPAT'
GIT_PULL_AZURE  = 'gitAzure'
GIT_PULL_CRED   = 'gitCredMgr'
GIT_PULL_GITHUB = 'gitGithubPAT'

s_gitUseSSH     = False  # Doesn't support Git LFS so it is no longer an option
s_hgUseSSH      = True if os.environ.get('BB_HG_SSH', None) else False

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def CreateLocalRepository (name, repoType, localDir):
    def IsType (rtype, choice): return rtype == choice or rtype == globalvars.repoTypes.index(choice)

    if IsType (repoType, globalvars.REPO_HG):
        return LocalHgRepository (name, localDir)
    elif IsType (repoType, globalvars.REPO_CVS):
        return LocalCvsRepository (name, localDir)
    elif IsType (repoType, globalvars.REPO_RSYNC):
        return LocalRsyncRepository (name, localDir)
    elif IsType (repoType, globalvars.REPO_GIT):
        return LocalGitRepository (name, localDir)
    elif IsType (repoType, globalvars.REPO_UPACK):
        return LocalUPackRepository (name, localDir)

#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
class LocalRepository (object):

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalRepository
    #-------------------------------------------------------------------------------------------
    def __init__(self, name, repoType, localDir):
        self.m_name = name # This must be immutable (see __hash__)
        self.m_getSource = 1
        self.m_origLocalDir = localDir  # This must be immutable (see __hash__)
        self.m_toolcache = None
        self.m_localDir = symlinks.normalizePathName(localDir)
        self.m_repoType = repoType
        self.m_provenance = None
        self.m_isPackage = False

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalRepository
    #-------------------------------------------------------------------------------------------
    # Currently only eq is used, but created full set just in case.
    def __eq__(self, other):
        if not other:
            return False
        return (self.GetDescriptiveString() == other.GetDescriptiveString())

    def __ne__(self, other):
        if not other:
            return True
        return (self.GetDescriptiveString() != other.GetDescriptiveString())

    def __lt__(self, other):
        if not other:
            return False
        return (self.GetDescriptiveString() < other.GetDescriptiveString())

    def __le__(self, other):
        if not other:
            return False
        return (self.GetDescriptiveString() <= other.GetDescriptiveString())

    def __gt__(self, other):
        if not other:
            return False
        return (self.GetDescriptiveString() > other.GetDescriptiveString())

    def __ge__(self, other):
        if not other:
            return True
        return (self.GetDescriptiveString() >= other.GetDescriptiveString())


    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalRepository
    #-------------------------------------------------------------------------------------------
    def __hash__(self):
        return hash( self.m_name + self.m_origLocalDir )

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalRepository
    #-------------------------------------------------------------------------------------------
    def __str__(self):
        return "Name='{0}', Type={1}, Directory='{2}'".format (self.m_name, self.GetType(), self.m_localDir)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalRepository
    #-------------------------------------------------------------------------------------------
    def __repr__(self):
        return "LocalRepository ([{0}] Type={1} {2})".format (self.m_name, self.GetType(), self.m_localDir)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalRepository
    #-------------------------------------------------------------------------------------------
    def GetLocalDir (self):
        if self.m_toolcache == None and globalvars.buildStrategy:
            self.m_toolcache = self.m_name.lower() in globalvars.buildStrategy.m_toolPackages['repo']
            if self.m_toolcache:
                if r'${SrcRoot}' in self.m_origLocalDir:
                    self.m_localDir = self.m_origLocalDir.replace (r'${SrcRoot}', buildpaths.GetToolCacheSourceRoot())
                elif r'${ToolCache}' in self.m_origLocalDir:
                    self.m_localDir = self.m_origLocalDir.replace (r'${ToolCache}', buildpaths.GetToolCacheSourceRoot())
                else:
                    raise utils.BuildError(r"Local Repository must start with ${ToolCache} or ${SrcRoot} to be used for ToolCache.")
                utils.showInfoMsg ('Converted ToolCache repository {0} to {1}\n'.format (self.m_origLocalDir, self.m_localDir), utils.INFO_LEVEL_RarelyUseful)

        return self.m_localDir

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalRepository
    #-------------------------------------------------------------------------------------------
    def GetPartFile (self, name):
        val = symlinks.normalizePathName(os.path.join (self.GetLocalDir(), os.path.expandvars(name)))

        if os.path.isfile(val):
            return val
        return val + ".PartFile.xml"

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalRepository
    #-------------------------------------------------------------------------------------------
    def GetType(self):
        return self.m_repoType

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalRepository
    #-------------------------------------------------------------------------------------------
    def GetDescriptiveString(self):
        return self.m_name + " (" + self.m_localDir + ")"

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalRepository
    #-------------------------------------------------------------------------------------------
    def RunRepoCmd (self, cmd, setcwd, outputProcessor=None):
        try:
            returnCode = cmdutil.runAndWait (cmd, setcwd, outputProcessor)
            return returnCode or 0
        except OSError as err:
            raise utils.BuildError (' '.join(cmd) + "\n" + err.__str__())

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalRepository
    #-------------------------------------------------------------------------------------------
    def RunNetworkRepoCmd (self, cmd, setcwd, outputProcessor=None):

        shouldRetry = [False]

        def processOutputForRetries (outputLine):
            shouldRetry[0] = self.IsRetryMessage (outputLine)
            if outputProcessor == None:
                utils.showInfoMsg (outputLine, utils.INFO_LEVEL_SomewhatInteresting)
            else:
                return outputProcessor (outputLine)

        retries, maxDelay = utils.GetRetryInfo ()
        sleepTime = 0
        attempt = 0
        while attempt < retries:
            attempt += 1
            try:
                returnCode = cmdutil.runAndWait (cmd, setcwd, processOutputForRetries)

                if not shouldRetry[0]:
                    return returnCode or 0

                utils.showInfoMsg ("Retry ({0}) after sleep ({1}): {2}\n".format (attempt, sleepTime, ' '.join(cmd)), utils.INFO_LEVEL_RarelyUseful)
                time.sleep (sleepTime)
                sleepTime = utils.GetNextSleepTime(sleepTime, maxDelay)

            except OSError as err:
                raise utils.BuildError (' '.join(cmd) + '\n' + err.__str__())

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalRepository
    #-------------------------------------------------------------------------------------------
    def IsRetryMessage (self, outputLine):  # pylint: disable=unused-argument
        return False

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalRepository
    #-------------------------------------------------------------------------------------------
    def Dump (self, indent):
        utils.showInfoMsg (" "*indent + self.__str__() + '\n', utils.INFO_LEVEL_Essential)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalRepository
    #-------------------------------------------------------------------------------------------
    def IsPushable (self):
        return False

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalRepository
    #-------------------------------------------------------------------------------------------
    def GetProvenance (self):
        assert (False) # Should always be subclassed

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalRepository
    #-------------------------------------------------------------------------------------------
    def GetLatestRevision (self, remoteRepo, pullToRev):  # pylint: disable=unused-argument
        return None

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalRepository
    #-------------------------------------------------------------------------------------------
    def GetProvenanceString (self):
        if "" == self.m_provenance:
            return None

        if None == self.m_provenance:
            if not os.path.exists (self.GetLocalDir()):
                self.m_provenance = ""
                return None

            try:
                self.m_provenance = self.GetProvenance () # pylint: disable=assignment-from-no-return
            except OSError as err:
                raise utils.StrategyError ("Can't get Provenance for LocalRepository {0}\n  {1}".format (self, err.__str__()))

        return self.m_provenance

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalRepository
    #-------------------------------------------------------------------------------------------
    def IsPackage (self):
        return self.m_isPackage

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalRepository
    #-------------------------------------------------------------------------------------------
    def RobocopySource (self, remoteRepo):

        copiedFileCount = [0]

        # Unfortunately, we need to parse every single verbose line of robocopy, since there's no way to make it shut up
        def parseRobocopyOutput (outputLine):
            utils.showInfoMsg (outputLine, utils.INFO_LEVEL_SomewhatInteresting)

            fileSummaryMatch = re.search (
                r"Files\s+:\s+(?P<total>\d+)\s+(?P<copied>\d+)\s+(?P<skipped>\d+)\s+(?P<mismatch>\d+)\s+(?P<failed>\d+)\s+(?P<extras>\d+)",
                outputLine
            )

            if fileSummaryMatch != None:
                copiedFileCount[0] = int (fileSummaryMatch.group ('copied'))
                failedFileCount = int (fileSummaryMatch.group ('failed'))
                if failedFileCount != 0:
                    utils.exitOnError (1, "Robocopy failed copying {0} files".format (failedFileCount))

        cmdutil.runAndWait (['robocopy', '/mir', remoteRepo.GetAddress(), self.GetLocalDir()], outputProcessor=parseRobocopyOutput)

        return copiedFileCount[0]

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalRepository
    #-------------------------------------------------------------------------------------------
    def MirrorSource (self, remoteRepo):
        if os.name == 'nt':
            return self.RobocopySource (remoteRepo)
        elif os.name == 'posix':
            return cmdutil.rsyncDir (remoteRepo.GetAddress(), self.GetLocalDir())
        raise utils.StrategyError ("Unknown OS ({0}) when trying to mirror repository {1} to {2}".format (os.name, remoteRepo.GetOrigAddress(), self.GetLocalDir()))

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalRepository
    #-------------------------------------------------------------------------------------------
    def GetToolCacheName (self):
        # This returns the name to use in the the output tree. Currently just the name.
        return self.m_name

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalRepository
    #-------------------------------------------------------------------------------------------
    def GetDiff (self):
        return []

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalRepository
    #-------------------------------------------------------------------------------------------
    def SetupUrl (self, remoteUrl):
        return remoteUrl

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalRepository
    #-------------------------------------------------------------------------------------------
    def GetStreamTipRevision (self, remoteUrl): # pylint: disable=unused-argument
        return None, None

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalRepository
    #-------------------------------------------------------------------------------------------
    def HasRepositoryLocally (self):
        return os.path.exists (self.GetLocalDir()) and len (os.listdir (self.GetLocalDir())) != 0

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalRepository
    #-------------------------------------------------------------------------------------------
    def CleanRepository (self):
        if os.path.exists (self.GetLocalDir()):
            utils.cleanDirectory (self.GetLocalDir(), deleteFiles=True)
            os.remove(self.GetLocalDir())

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalRepository
    #-------------------------------------------------------------------------------------------
    def CheckTypeChangedAndRemove(self, fromtype, totype):
        # For unattended builds, just rename the directory if converting from git to Hg or vice versa. 
        dotFrom = fromtype if fromtype.lower()=='cvs' else '.'+fromtype
        dotTo = totype if totype.lower()=='cvs' else '.'+totype
        if not utils.isUnattendedBuild():
            return False
        if os.path.exists(os.path.join(self.m_localDir, dotTo)) or not os.path.exists(os.path.join(self.m_localDir, dotFrom)):
            return False

        # For now I set it up to rename the directory to a new one. If these start to build up somewhere we may revisit.
        renamedDirBase = os.path.join (self.m_localDir, '..', os.path.basename(self.m_localDir)+'.'+fromtype)
        rev = 0
        renamedDir = renamedDirBase + '.{0}'.format(rev)
        while os.path.exists(renamedDir):
            rev += 1
            renamedDir = renamedDirBase + '.{0}'.format(rev)
        try:
            os.rename (self.m_localDir, renamedDir)
        except OSError as ex:
            raise utils.BuildError('Failure renaming {0} to {1}. The directory is in the way because it is {3} instead of {4}. Exception: {2}'.format(self.m_localDir, renamedDir, ex, fromtype, totype))
        return True

#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
class LocalCvsRepository (LocalRepository):

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalCvsRepository
    #-------------------------------------------------------------------------------------------
    def __init__(self, name, localDir):
        LocalRepository.__init__(self, name, globalvars.REPO_CVS, localDir)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalCvsRepository
    #-------------------------------------------------------------------------------------------
    def GetProvenance (self):
        prov = "Get CVS provenance!!!"
        # The pull tag is stored in the Checkout Success file
        tag = None
        try:
            checkFile = open(self.GetCvsSuccessFile(), 'rt')
            tagLine = checkFile.readline().strip()
            checkFile.close()

            if tagLine.startswith('-D'):
                tag = 'DATE:{0}'.format (tagLine[3:])
            elif tagLine.startswith ('-r'):
                tag = tagLine[3:]
        except:
            pass

        if tag:
            prov = tag
        else:
            # Should not get here.  Fallback to look it up in the Repository Tags file
            repoOptions = globalvars.buildStrategy.FindRepositoryOptions (self.m_name) 
            if repoOptions:
                prov = repoOptions.m_legacyTag
            else:
                raise utils.BuildError ('Cannot find tag information for CVS repository {0}\n   ({1}).\nNormally this is in the file "{2}"'.format(self.m_name, self.GetLocalDir(), self.GetCvsSuccessFile()))

        return prov

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalCvsRepository
    #-------------------------------------------------------------------------------------------
    def GetCvsSuccessFile (self):
        return os.path.join(self.GetLocalDir(), 'cvs', 'checkoutSuccess')

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalCvsRepository
    #-------------------------------------------------------------------------------------------
    def CheckCvsTag (self, pullToTag):
        tagType = '-r'

        if pullToTag:  # Usually comes from BDF
            if pullToTag.startswith ('DATE:'):
                tag = [pullToTag[5:], True]
                tagType = '-D'
            elif pullToTag == 'Get CVS provenance!!!': # Old BDFs will have this; not much we can do here.
                return False, ['-A', '-P']             # Using -P as an argument to CVS just because we need to pass 2 args back.
            else:
                tag = [pullToTag, True]
        else: # Look it up in RepositoryTags.BuildStrategy
            repoOptions = globalvars.buildStrategy.FindRepositoryOptions (self.m_name)
            if repoOptions:
                tag = repoOptions.GetLegacyTuple()

        if None == tag or None == tag[0]:
            # If we don't have a tag, update to a time stamp (the same stamp for all pulls)
            tagVal = '{0}'.format(utils.GetPullTime())
            return False, ['-D', tagVal]

        # Look at the tag file in the CVS repo; if we are already checked out to that tag then no need to pull again.
        tagval = [tagType, tag[0]]
        tagfilename = os.path.join (self.GetLocalDir(), "cvs", "tag")
        if not tag[1] or not os.path.isfile (tagfilename): # tag[1] is "PullOnce" value from strategy file
            return False, tagval

        skip = False
        tagfile = open(tagfilename, "rt")
        line = tagfile.readline()
        line = line.rstrip ("\n")
        if line[1:] == tag[0]:
            utils.showInfoMsg ("%%%Repository already updated to tag '{0}', skipping\n".format (tag[0]), utils.INFO_LEVEL_Interesting)
            skip = True
        tagfile.close()

        return skip, tagval

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalCvsRepository
    #-------------------------------------------------------------------------------------------
    def _GetTagFromCvsRepo (self, localDir):
        tagfilename = os.path.join (localDir, "cvs", "tag")
        if not os.path.isfile (tagfilename):
            return None
            
        tagfile = open(tagfilename, "rt")
        line = tagfile.readline()
        line = line.rstrip ("\n")
        tag = line[1:]
        
        return tag

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalCvsRepository
    #-------------------------------------------------------------------------------------------
    def CheckCvsTagForSyncPull (self, remoteRepo):
        myTag = self._GetTagFromCvsRepo (remoteRepo.GetAddress())
        localTag = self._GetTagFromCvsRepo (self.GetLocalDir())
        
        if None != myTag and None != localTag and myTag == localTag:
            return True  # skip
            
        return False

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalCvsRepository
    #-------------------------------------------------------------------------------------------
    def IsRetryMessage (self, outputLine):
        outputLine = outputLine.lower()
        if 'connection attempt failed because the connected party did not properly respond after a period of time' in outputLine or \
            'no such host is known' in outputLine:
            return True
        return False

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalCvsRepository
    #-------------------------------------------------------------------------------------------
    def BdfGetRevision (self, bdf):
        return bdf.GetCvsTag (self.m_name.lower())

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalCvsRepository
    #-------------------------------------------------------------------------------------------
    def TypeChangedAndUnattended(self):
        return self.CheckTypeChangedAndRemove('git', 'cvs')

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalCvsRepository
    #-------------------------------------------------------------------------------------------
    def UpdateSource (self, remoteRepo, forceUpdate, outputProcessor, pullToTag=None):
        skip, tagOption = self.CheckCvsTag(pullToTag)
        
        checkSuccessPath = self.GetCvsSuccessFile ()
        if skip and not os.path.exists(checkSuccessPath):
            skip = False
            
        if skip and not forceUpdate:
            return 0
          
        pathIndex = self.GetLocalDir().replace('/', '\\').rfind (remoteRepo.m_path.replace('/', '\\'))
        if -1 != pathIndex:
            repoPath = self.GetLocalDir()[:pathIndex]
            rootPath = self.GetLocalDir()[pathIndex:]
        else:
            utils.exitOnError (1, "Unable to find partial local path '{0}' in cvs checkout path '{1}'.  \nFor CVS these paths much match exactly except for slashes.".format (remoteRepo.m_path, self.GetLocalDir()))

        symlinks.makeSureDirectoryExists (self.GetLocalDir())
        
        if os.path.exists(checkSuccessPath):
            os.remove(checkSuccessPath)

        self.TypeChangedAndUnattended()

        # Specify CVS_USER environment variable if using machine not connected to Bentley domain.
        # CVS_USER=<USERNAME>[:<PASSWORD>] where [] marks optional section, and
        # <USERNAME> - (required) Bentley domain user name in a form of 'bentley\john.doe';
        # <PASSWORD> - (optional) Bentley domain user password. If left out, explicit call to cvs login is required prior to the check-out.
        address = remoteRepo.GetAddress()
        if 'CVS_USER' in os.environ:
            if '@' in os.environ['CVS_USER']:
                raise utils.BuildError ('The CVS_USER env var contains "@" which will cause the CVS commands to fail. Please fix the username or choose a different password.')
            if address.startswith(':sspi:'):
                address = ":sspi:{0}@{1}".format (os.environ['CVS_USER'], address[6:])

        # If we don't have the repo, get it
        if not skip:
            if remoteRepo.m_checkoutOptions:
                cmd = ["cvs", "-d{0}".format(address), "co", remoteRepo.m_checkoutOptions, tagOption[0], tagOption[1], rootPath]
            else:
                cmd = ["cvs", "-d{0}".format(address), "co",                               tagOption[0], tagOption[1], rootPath]
        else:
            repoPath = os.path.join (repoPath, rootPath)
            cmd = ["cvs",  "up",  "-d"]

        try:
            pullSuccess = self.RunNetworkRepoCmd (cmd, repoPath, outputProcessor)
        except utils.BuildError as err:
            # Check for cvs not in the path error.
            if 'The system cannot find the file specified' in err.errmessage:
                raise utils.BuildError (' '.join(cmd) + '\n' + err.errmessage + '\nPlease check that cvs.exe is in your path.')
            else:
                raise err

        if pullSuccess == 0:
            checkFile = open(checkSuccessPath,'wt')
            checkFile.write('{0} {1}'.format (tagOption[0], tagOption[1]))
            checkFile.close()

        return pullSuccess

#-------------------------------------------------------------------------------------------
# bsiclass
# Workaround class to preserve Unix permissions when extracting zip file.
# https://stackoverflow.com/questions/39296101/python-zipfile-removes-execute-permissions-from-binaries
#-------------------------------------------------------------------------------------------
class PermissionPreservingZipFile(zipfile.ZipFile):
    def _extract_member(self, member, targetpath, pwd):
        if not isinstance(member, zipfile.ZipInfo):
            member = self.getinfo(member)

        targetpath = super(PermissionPreservingZipFile, self)._extract_member(member, targetpath, pwd)

        attr = member.external_attr >> 16
        if attr != 0:
            os.chmod(targetpath, attr)
        return targetpath

#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
class LocalRsyncRepository (LocalRepository):

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalRsyncRepository
    #-------------------------------------------------------------------------------------------
    def __init__(self, name, localDir):
        LocalRepository.__init__(self, name, globalvars.REPO_RSYNC, localDir)
        
    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalRsyncRepository
    #-------------------------------------------------------------------------------------------
    def GetProvenance (self):
        prov = "Get Rsync provenance!!!"

        provFileName = os.path.join (self.GetLocalDir(), 'provenance.txt')
        if os.path.exists (provFileName):
            with open (provFileName, 'rt') as provFile:
                prov = provFile.read().strip()
        else:
            utils.showInfoMsg ("Repository '{0}' missing rsync provenance\n".format (self.m_name), utils.INFO_LEVEL_VeryInteresting)

        return prov

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalRsyncRepository
    #-------------------------------------------------------------------------------------------
    def BdfGetRevision (self, bdf):
        path = bdf.GetRsyncPath (self.m_name.lower())
        if path:
            # While we no longer store tags but rather only the full path, this helps with older builds.
            tag = bdf.GetRsyncTag (self.m_name.lower())
            if tag and tag != RSYNC_FIXED_PATH:
                path = path + '\\' + tag + '\\*'
        return path

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalRsyncRepository
    #-------------------------------------------------------------------------------------------
    def GetLatestMatchingRevision (self, fromDir, searchExpr):
        dirList = lkgs.GetRsyncDirListing (fromDir) # This is cached
        utils.showInfoMsg ("Checking for rsync version; checked directory {0} and found {1}\n".format (fromDir, repr(dirList)), utils.INFO_LEVEL_RarelyUseful)
        useGlob = isinstance (searchExpr, versionutils.GlobMatcher)

        matchingItems = {}
        for item in dirList:
            if searchExpr.MatchString (item):
                key = item if useGlob else versionutils.Version (item)
                matchingItems[key] = item

        keys = sorted (matchingItems.keys())

        if len (keys) < 1:
            return None

        return matchingItems[keys[-1]], dirList  # dirList for error messages

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalRsyncRepository
    #-------------------------------------------------------------------------------------------
    def GetRemoteDir (self, remoteRepo, tag):
        remoteDir =  os.path.join (remoteRepo.GetAddress(), remoteRepo.m_path)
        # If $(version) is used in the remote repo we need to parse out and choose the correct one.
        # Have to choos at that level first.
        versionTag = '$(version)'
        if remoteRepo.m_version and versionTag in remoteRepo.m_path:
            verIndex = remoteRepo.m_path.find (versionTag)
            verDir = os.path.join (remoteRepo.GetAddress(), remoteRepo.m_path[0:verIndex])

            newestVersion, dirList = self.GetLatestMatchingRevision (verDir, remoteRepo.m_version)
            if newestVersion == None:
                raise utils.BuildError ("Cannot find requested rsync repository version {0} at {1}. Found {2}".format (remoteRepo.m_version, verDir, repr(dirList)))

            trailingDir = remoteRepo.m_path[verIndex:].replace ('/','\\')
            trailSlashIndex = trailingDir.find ('\\')
            trailingDir = trailingDir [trailSlashIndex+1:]

            utils.showInfoMsg ("Found version {0} that matches version {1}\n".format (newestVersion, remoteRepo.m_version.StringForFilename()), utils.INFO_LEVEL_SomewhatInteresting)
            remoteDir = os.path.join (verDir, newestVersion, trailingDir) 

        if tag:
            try:
                searchVer = versionutils.Version (tag) # format for version numbers is: 1-2-3-4b3 08110702en1
            except:
                searchVer = versionutils.GlobMatcher (tag) # handle any other level of stuff like pwed081111*en*

            tagVer, dirList = self.GetLatestMatchingRevision (remoteDir, searchVer)
            if tagVer == None:
                raise utils.BuildError ("Cannot find requested rsync repository tag {0} at {1}. Found {2}".format (tag, remoteDir, repr(dirList)))
            remoteDir = remoteDir + '\\' + tagVer + '\\*'

        return remoteDir

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalRsyncRepository
    #-------------------------------------------------------------------------------------------
    def UpdateSource (self, remoteRepo, forceUpdate, outputProcessor, bdfPath=None):
        if self.m_toolcache and remoteRepo.m_path.lower().endswith(r'.zip'):
            return self.UpdateToolCacheZip (remoteRepo, forceUpdate, outputProcessor)

        tagData = globalvars.buildStrategy.FindRepositoryOptions (self.m_name)  # for rsync pulls, should return a wildcard filter

        if tagData:
            tag = tagData.m_legacyTag if tagData.m_legacyTag and tagData.m_legacyTag != "None" else None
            pullOnce = tagData.m_legacyPullOnce
        else:
            tag, pullOnce = None, True  # By default treat all Rsync repos as immutable and only pull once

        if bdfPath:
            remoteDir = os.path.join (remoteRepo.GetAddress(), bdfPath)
        else:
            remoteDir = self.GetRemoteDir (remoteRepo, tag)
        provFileName = os.path.join (self.GetLocalDir(), 'provenance.txt')

        # Skip if there's a prov line and it matches the source.
        if pullOnce and os.path.exists (provFileName):
            if self.GetProvenance () == remoteDir:
                return 0
        
        # Get the files
        cmdutil.rsyncDir (remoteDir, self.GetLocalDir(), outputProcessor=outputProcessor)

        # Store a provenance line for future use
        try:
            with open (provFileName, 'wt') as provFile:
                provFile.write (remoteDir)
        except:
            pass  # When rsyncing from PRG the directory is locked down; can't write to it.
        return 0  # rsyncDir excepts on error

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalRsyncRepository
    #-------------------------------------------------------------------------------------------
    def UpdateToolCacheZip (self, remoteRepo, _forceUpdate, outputProcessor):

        # Toolcache repos are considered immutable, so if the local dir exists then we won't pull it.
        if os.path.exists (self.GetLocalDir()):
            utils.showInfoMsg ('Skipping ToolCache directory {0} because it already exists.\n'.format (self.GetLocalDir()), utils.INFO_LEVEL_SomewhatInteresting)
            return 0
        
        remoteZip = os.path.join (remoteRepo.GetAddress(), remoteRepo.m_path)
        remoteZipFileName = os.path.basename (remoteRepo.m_path)

        localZipDir = os.path.join(buildpaths.GetToolCacheSourceRoot(), "zip")
        localZipFileName = os.path.join (localZipDir, remoteZipFileName)
        symlinks.makeSureDirectoryExists (localZipDir)
        
        cmdutil.rsyncFile (remoteZip, localZipDir, outputProcessor)
        with PermissionPreservingZipFile (localZipFileName) as zf:
            zf.extractall (self.GetLocalDir())
            
        # Once again it seems like the file is held open, so cycle with retries to remove it.
        retriesRemainig = 100
        while retriesRemainig:
            try:
                os.remove (localZipFileName)
                break
            except:
                retriesRemainig -= 1
                time.sleep (0.1)

        # Store a provenance line for future use
        provFileName = os.path.join (self.GetLocalDir(), 'provenance.txt')
        try:
            with open (provFileName, 'wt') as provFile:
                provFile.write (remoteZip)
        except:
            pass  # When rsyncing from PRG the directory is locked down; can't write to it.
        return 0

#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
class LocalHgRepository (LocalRepository):

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalHgRepository
    #-------------------------------------------------------------------------------------------
    def __init__(self, name, localDir):
        LocalRepository.__init__(self, name, globalvars.REPO_HG, localDir)
    
    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalHgRepository
    #-------------------------------------------------------------------------------------------
    def BdfHasName(self, bdf):
        return bdf.HasName(self.m_name.lower())

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalHgRepository
    #-------------------------------------------------------------------------------------------
    def BdfGetRevision(self, bdf):
        return bdf.GetRevision(self.m_name.lower())

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalHgRepository
    #-------------------------------------------------------------------------------------------
    def BdfGetUrl(self, bdf):
        return bdf.GetUrl(self.m_name.lower())

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalHgRepository
    #-------------------------------------------------------------------------------------------
    def BdfAddEntry(self, bdf, guid, repoUrl, fromLKG=False):
        return bdf.AddEntry(self.m_name.lower(), guid, repoUrl, fromLKG)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalHgRepository
    #-------------------------------------------------------------------------------------------
    def BdfRemoveEntry(self, bdf):
        return bdf.RemoveEntry(self.m_name.lower())

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalHgRepository
    #-------------------------------------------------------------------------------------------
    def BdfUpdateEntry(self, bdf, guid, repoUrl, fromLKG=False):
        return bdf.UpdateEntry(self.m_name.lower(), guid, repoUrl, fromLKG)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalHgRepository
    #-------------------------------------------------------------------------------------------
    def IsPushable (self):
        return True

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalHgRepository
    #-------------------------------------------------------------------------------------------
    def IsRetryMessage (self, outputLine):
        outputLine = outputLine.lower()
        if 'existing connection was forcibly closed by the remote host' in outputLine or \
           'getaddrinfo failed' in outputLine or \
           'stream ended unexpectedly' in outputLine or \
           'connection attempt failed because the connected party did not properly respond after a period of time' in outputLine:
            return True
        return False

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalHgRepository
    #-------------------------------------------------------------------------------------------
    def SetupUrl (self, remoteUrl):
        # Allow switch to SSH
        if not s_hgUseSSH:
            return bbutils.CheckForHttps(remoteUrl)

        splUrl = compat.parseUrl (remoteUrl)

        serverPieces = splUrl.netloc.split('.')
        server = '.'.join(serverPieces[1:])
        repo = serverPieces[0]

        newUrl = 'ssh://{0}/{1}/{2}'.format (server, repo, splUrl.path)

        utils.showInfoMsg ('Changing to SSL {0} => {1}\n'.format(remoteUrl, newUrl), utils.INFO_LEVEL_SomewhatInteresting)

        return newUrl

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalHgRepository
    #-------------------------------------------------------------------------------------------
    def GetProvenance (self):
        # We use a combination of "hg parents" and "hg ident" here because the "hg ident" command
        # can't accept a --template argument to get its 40-character changeset hash. "hg ident --debug"
        # does give the 40-character hash, but it might also give extra text that we don't want to parse.
        identString = [None]

        def ParseHgIdentifyOutput (outputLine):
            utils.showInfoMsg (outputLine, utils.INFO_LEVEL_SomewhatInteresting)
            if None != identString[0]:  # only process the first line of output
                return
            identString[0] = outputLine.strip()

        parentRevs = self.GetParentRevisions()
        if len (parentRevs) > 0:
            provString = parentRevs[0]
        else:
            provString = ""

        self.ProcessHgCommand (["hg", "identify", "-i"], ParseHgIdentifyOutput)
        if identString[0].endswith ("+"):
            provString += "+"

        return provString

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalHgRepository
    #-------------------------------------------------------------------------------------------
    def ClobberHgChanges (self, outputProcessor):
        self.ProcessHgCommand (['hg', 'update', '-C'], outputProcessor)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalHgRepository
    #-------------------------------------------------------------------------------------------
    def ProcessHgCommand(self, command, outputProcessor, commandInfoLevel=utils.INFO_LEVEL_SomewhatInteresting, commandColor=utils.UNCHANGED):
        outputText = "Performing {1} on {0}\n".format(self.GetLocalDir(), command)
        utils.showInfoMsg (outputText, commandInfoLevel, commandColor)

        cmdutil.runAndWait (command, self.GetLocalDir(), outputProcessor)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalHgRepository
    #-------------------------------------------------------------------------------------------
    def GetHeadsCount (self, commandInfoLevel=utils.INFO_LEVEL_SomewhatInteresting):
        headsCount = [0]

        def ParseHgHeadsOutput (outputLine):
            utils.showInfoMsg (outputLine, utils.INFO_LEVEL_SomewhatInteresting)
            if outputLine.strip() == "Head found":
                headsCount[0] += 1

        self.ProcessHgCommand (["hg", "heads", '--template=Head found\\n'], ParseHgHeadsOutput, commandInfoLevel)
        return headsCount[0]

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalHgRepository
    #-------------------------------------------------------------------------------------------
    def GetTipRevision (self, commandInfoLevel=utils.INFO_LEVEL_SomewhatInteresting, useLocalNumber=False):
        tipRevision = [None]

        def ParseHgTipOutput (outputLine):
            utils.showInfoMsg (outputLine, utils.INFO_LEVEL_SomewhatInteresting)
            outputLine = outputLine.strip()
            if outputLine.startswith ("tip:"):
                tipRevision[0] = outputLine[4:]
        if False == useLocalNumber:
            self.ProcessHgCommand (['hg', 'tip', '--template=tip:{node}\\n'], ParseHgTipOutput, commandInfoLevel)
        else:
            self.ProcessHgCommand (['hg', 'tip', '--template=tip:{rev}\\n'], ParseHgTipOutput, commandInfoLevel)
        return tipRevision[0]

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalHgRepository
    #-------------------------------------------------------------------------------------------
    def DoesRevisionExist (self, revision, commandInfoLevel=utils.INFO_LEVEL_SomewhatInteresting):
        wasFound = [False]
        def ParseHgLogROutput (outputLine):
            utils.showInfoMsg (outputLine, utils.INFO_LEVEL_SomewhatInteresting)
            outputLine = outputLine.strip()
            if outputLine.startswith ("found"):
                wasFound[0] = True

        hgCmd = ['hg', 'log', '-r', revision, '--template=found']
        self.ProcessHgCommand (hgCmd, ParseHgLogROutput, commandInfoLevel)
        return wasFound[0]

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalHgRepository
    #-------------------------------------------------------------------------------------------
    def MergeRepo (self, RepoChangedFunc, updateStartTime):
        utils.showInfoMsg ("Performing non-interactive merge on " + self.GetDescriptiveString() + "\n", utils.INFO_LEVEL_Interesting)

        wasMergeSuccess = [None]
        localDeletedFileRe = re.compile (r'other .* changed .* which local .* deleted')
        remoteDeletedFileRe = re.compile (r'local .* changed .* which other .* deleted')

        def parseMergeOutput (outputLine):
            utils.showInfoMsg (outputLine, utils.INFO_LEVEL_SomewhatInteresting)

            if wasMergeSuccess[0] != None:
                return

            # Uncommitted changes - Mercurial won't allow a merge
            if outputLine.startswith ("abort: outstanding uncommitted changes"):
                failMessage = \
                    "\n\tMerge failed - local changes found.\n"     + \
                    "\t1. Commit or remove your local changes\n"    + \
                    "\t2. Run 'hg merge'\n"                         + \
                    "\t3. Run 'hg commit -m\"Merge.\"' to commit the merge"
                RepoChangedFunc (self, " [Merge FAILED]", failMessage, False, compat.perfCounter() - updateStartTime)
                wasMergeSuccess[0] = False

            # Even a non-interactive merge can issue some prompts. Abort and make the user finish this one later.
            elif (outputLine.startswith ("bb_prompt:  local changed") or 
                  outputLine.startswith ("bb_prompt: local changed")  or 
                  outputLine.startswith ("bb_prompt: remote changed") or 
                  localDeletedFileRe.search (outputLine) or 
                  remoteDeletedFileRe.search (outputLine)):
                failMessage = \
                    "\n\tMerge failed - user input was required during merging.\n"  + \
                    "\t1. Run 'hg merge' to merge interactively\n"                  + \
                    "\t2. Run 'hg commit -m\"Merge.\" to commit the merge"
                RepoChangedFunc (self, " [Merge FAILED]", failMessage, False, compat.perfCounter() - updateStartTime)
                wasMergeSuccess[0] = False
                return cmdutil.TERMINATE_PROCESS

            # General failure
            elif outputLine.startswith ("abort:"):
                RepoChangedFunc (self, " [Merge FAILED]", "Merge failed: " + outputLine.strip(), False, compat.perfCounter() - updateStartTime)

            # Completed merge, number of unresolved conflicts yet to be determined...
            elif "files unresolved" in outputLine:
                # must check with preceding <space> to avoid false-positives on "10 files unresolved" etc.
                if " 0 files unresolved" in outputLine:
                    RepoChangedFunc (self, " [Merge succeeded]", None, True, compat.perfCounter() - updateStartTime)
                    wasMergeSuccess[0] = True
                else:
                    failMessage = \
                    "\n\tMerge failed - conflicting changes found.\n" + \
                    "\t1. Run 'hg resolve --all' to merge manually\n" + \
                    "\t2. Run 'hg commit -m\"Merge.\"' to commit the merge"
                    RepoChangedFunc (self, " [Merge FAILED]", failMessage, False, compat.perfCounter() - updateStartTime)
                    wasMergeSuccess[0] = False

        # The ui.bentleybuild config option tells Mercurial to print extra output that BentleyBuild might need to parse. In this
        # case, we want Mercurial to print and flush the "remote changed <file> but local changed it" prompts instead of blocking.
        self.ProcessHgCommand (['hg', '--config', 'ui.bentleybuild=True', '--config', 'ui.merge=internal:merge', 'merge'], parseMergeOutput)

        def processCommitOutput( outputLine ):
            utils.showInfoMsg( outputLine, utils.INFO_LEVEL_SomewhatInteresting)

        if wasMergeSuccess[0]:
            self.ProcessHgCommand(['hg', 'commit', '-m', 'Merge.'], processCommitOutput)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalHgRepository
    #-------------------------------------------------------------------------------------------
    def GetParentRevisions (self, revision=None, commandInfoLevel=utils.INFO_LEVEL_SomewhatInteresting, useLocalNumber=False):
        parentRevisions = []

        def ParseHgParentsOutput (outputLine):
            utils.showInfoMsg (outputLine, utils.INFO_LEVEL_SomewhatInteresting)
            outputLine = outputLine.strip()
            if outputLine.startswith ("parent:"):
                parentRevisions.append (outputLine[7:])
        hgCmd = ["hg", "parents"]
        if False == useLocalNumber:
            hgCmd.append ("--template=parent:{node}\\n")
        else:
            hgCmd.append ("--template=parent:{rev}\\n")
        if None != revision:
            hgCmd.extend (['-r', revision])

        self.ProcessHgCommand (hgCmd, ParseHgParentsOutput, commandInfoLevel)
        return parentRevisions

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalHgRepository
    #-------------------------------------------------------------------------------------------
    def HasLocalChanges(self):
        if not os.path.exists (self.GetLocalDir()):
            return False
        hasLocalChanges = [False]

        def processIdentifyOutput (outputLine):
            if outputLine.startswith( "abort:" ):
                utils.exitOnError( 1, self.GetDescriptiveString() + ": " + outputLine )
            else:
                utils.showInfoMsg (outputLine, utils.INFO_LEVEL_SomewhatInteresting)

            # Only process the first line
            if hasLocalChanges[0]:
                return

            if not hasLocalChanges[0] and re.match (r'\w+\+', outputLine):
                hasLocalChanges[0] = True

        self.ProcessHgCommand (['hg', 'identify'], processIdentifyOutput)

        return hasLocalChanges[0]

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalHgRepository
    #-------------------------------------------------------------------------------------------
    def HasIncomingChanges (self, remoteRepo, pullToRev=None):
        hasChanges = [False]

        def processIncomingOutput (outputLine):
            if outputLine.startswith( "abort:" ):
                utils.exitOnError( 1, self.GetDescriptiveString() + ": " + outputLine )
            else:
                utils.showInfoMsg (outputLine, utils.INFO_LEVEL_SomewhatInteresting)

            if not hasChanges[0] and "Incoming changes found" == outputLine.strip():
                hasChanges[0] = True

        hgCmd = ['hg', 'incoming', '--template=Incoming changes found\\n', self.SetupUrl(remoteRepo.GetAddress())]
        if pullToRev:
            hgCmd.extend (['-r', pullToRev])

        self.ProcessHgCommand (hgCmd, processIncomingOutput)

        return hasChanges[0]

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalHgRepository
    #-------------------------------------------------------------------------------------------
    def HasOutgoingChanges (self, remoteRepo, pushToRev=None):
        hasChanges = [False]

        def processOutgoingOutput (outputLine):
            if outputLine.startswith( "abort:" ):
                utils.exitOnError( 1, self.GetDescriptiveString() + ": " + outputLine )
            else:
                utils.showInfoMsg (outputLine, utils.INFO_LEVEL_SomewhatInteresting)

            if not hasChanges[0] and "Outgoing changes found" == outputLine.strip():
                hasChanges[0] = True

        hgCmd = ['hg', 'outgoing', '--template=Outgoing changes found\\n', self.SetupUrl(remoteRepo.GetAddress())]
        if pushToRev:
            hgCmd.extend (['-r', pushToRev])

        self.ProcessHgCommand (hgCmd, processOutgoingOutput)

        return hasChanges[0]

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalHgRepository
    #-------------------------------------------------------------------------------------------
    def HasDraftOrHiddenChangeSets (self):
        hasDraftOrHiddenChangeSets = [False]

        def processDraftOrHiddenChangeSetsOutput (outputLine):
            utils.showInfoMsg (outputLine, utils.INFO_LEVEL_SomewhatInteresting)
            if not hasDraftOrHiddenChangeSets[0] and "" != outputLine.strip:
                hasDraftOrHiddenChangeSets[0] = True

        hgCmd = ['hg', 'log', '-r', 'draft() or hidden()']

        self.ProcessHgCommand (hgCmd, processDraftOrHiddenChangeSetsOutput)
        
        if hasDraftOrHiddenChangeSets[0]:
            return hasDraftOrHiddenChangeSets[0]

        hgCmd = ['hg', 'status', '-m']

        self.ProcessHgCommand (hgCmd, processDraftOrHiddenChangeSetsOutput)

        return hasDraftOrHiddenChangeSets[0]

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalHgRepository
    #-------------------------------------------------------------------------------------------
    def HasNonRevChangeSets (self, pullToRev):
        hasNonRevChangeSets = [False]

        def processNonRevChangeSetsOutput  (outputLine):
            if not hasNonRevChangeSets[0] and "" != outputLine.strip:
                hasNonRevChangeSets[0] = True
            utils.showInfoMsg (outputLine, utils.INFO_LEVEL_SomewhatInteresting)
        hgCmd = ['hg', 'log', '-r', 'ancestors(tip) and not ancestors('+ pullToRev + ')' ]

        self.ProcessHgCommand (hgCmd, processNonRevChangeSetsOutput)

        return hasNonRevChangeSets[0]

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalHgRepository
    #-------------------------------------------------------------------------------------------
    def RunWithRecovery (self, cmd, setcwd, outputProcessor):
        shouldRecover = [False]
        def processOutput (outputLine):
            if outputLine.startswith ("abort: abandoned transaction found!"):
                shouldRecover[0] = True
                utils.showInfoMsg ('Repository "{0}" has an abandoned transaction. Will try to recover...\n'.format(self.m_localDir), utils.INFO_LEVEL_Interesting)
            elif outputProcessor != None:
                outputProcessor(outputLine)

        hgReturn = self.RunNetworkRepoCmd (cmd, setcwd, processOutput)
        if shouldRecover[0]:
            cmd2 = ['hg', '-R', self.m_localDir, 'recover']
            if self.RunNetworkRepoCmd (cmd2, setcwd, outputProcessor) == 0:
                utils.showInfoMsg ('Repository "{0}" has recovered from an abandoned transaction. Trying again...\n'.format(self.m_localDir), utils.INFO_LEVEL_Interesting)
                hgReturn = self.RunNetworkRepoCmd (cmd, setcwd, processOutput)
            else:
                raise utils.BuildError ('Repository "{0}" could not recover from an abandoned transaction. Manually deleting the repository might be required.\n'.format(self.m_localDir))
        return hgReturn

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalHgRepository
    #-------------------------------------------------------------------------------------------
    def TypeChangedAndUnattended(self):
        return self.CheckTypeChangedAndRemove('git', 'hg')

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalHgRepository
    #-------------------------------------------------------------------------------------------
    def UpdateSource (self, remoteRepo, options=None, pullOutputProcessor=None, cloneOutputProcessor=None, isLocalClone=False, pullToRev=None, promptToStrip=False, rebase=False, leaveRepo=False):
        if cloneOutputProcessor == None:
            cloneOutputProcessor = pullOutputProcessor

        if not self.HasRepositoryLocally () or self.TypeChangedAndUnattended():
            self.CleanRepository() # in case when repository folder already exists
            cmd = ['hg', 'clone']
            if not isLocalClone:
                cmd.append ("--pull")
            cmd.extend ([self.SetupUrl(remoteRepo.GetAddress()), self.GetLocalDir()])

            # Make sure directory above where we want to check out source exists
            tmpDest = self.GetLocalDir()
            if tmpDest[-1:] == os.sep:
                tmpDest = tmpDest[:-1]
            destDir = tmpDest[:tmpDest.rfind (os.sep)]

            # Need to have parent directory exist to clone.
            if destDir and not os.path.exists (destDir):
                symlinks.makeSureDirectoryExists (destDir)

            outputProcessor = cloneOutputProcessor
        else:
            if not os.path.exists (os.path.join (self.GetLocalDir(), '.hg', 'store')):
                if os.path.exists (os.path.join (self.m_localDir, '.git')):
                    raise utils.BuildError ('Directory "{0}" exists but is a Git repo. Perhaps it is an older branch that needs hg.\nPlease move the directory and pull again.'.format(self.m_localDir))
                raise utils.BuildError ('Directory "{0}" exists but does not appear to be a repository (missing .hg/store).\nPlease move the directory and pull again.'.format(self.GetLocalDir()))

            if pullToRev:
                currentRevs = self.GetParentRevisions()
                if len(currentRevs) == 1 and currentRevs[0] == pullToRev:
                    utils.showInfoMsg ("Repository " + remoteRepo.m_name + " already identifies as " + pullToRev + "; NOT pulling.\n", utils.INFO_LEVEL_Interesting)
                    return

            cmd = ['hg', '--config', 'ui.merge=internal:merge', '-R', self.GetLocalDir(), 'pull', '-u']
            if rebase:
                cmd.append ('--rebase')
            if options:
                cmd.extend (options.split())
            
            cmd.append (self.SetupUrl(remoteRepo.GetAddress()))

            outputProcessor = pullOutputProcessor

        if pullToRev:
            cmd.append ("-r{0}".format (pullToRev))
        
        hgReturn = self.RunWithRecovery (cmd, None, outputProcessor)

        if pullToRev and os.path.exists (os.path.join (self.GetLocalDir(), '.hg', 'hgrc')):
            if self.HasNonRevChangeSets (pullToRev):
                if promptToStrip:
                    if self.HasDraftOrHiddenChangeSets ():
                        utils.showInfoMsg ("\nRepository " + remoteRepo.m_name + " has changesets that aren't in the pulled revision\nand non committed changesets", utils.INFO_LEVEL_Important, utils.RED)
                    else:
                        utils.showInfoMsg ("\nRepository " + remoteRepo.m_name + " has changesets that aren't in the pulled revision\nDo you want to strip them? 'y' to strip, 'n' to continue", utils.INFO_LEVEL_Important, utils.YELLOW)
                        sys.stdout.flush()
                        sys.stdin.flush()
                        ans = sys.stdin.read(1)
                        if ans.lower() == 'y':
                            cmd2 = ['hg', '-R', self.GetLocalDir(), 'strip', 'ancestors(tip) and not ancestors(' + pullToRev + ')' ]
                            self.RunRepoCmd (cmd2, None, outputProcessor)
                            cmd2 = ['hg', '-R', self.GetLocalDir(), 'update', pullToRev ]
                            self.RunRepoCmd (cmd2, None, outputProcessor)
                else:
                    utils.showInfoMsg ("\nRepository " + remoteRepo.m_name + " has changesets that aren't in the pulled revision\n", utils.INFO_LEVEL_Important, utils.YELLOW)


        # Change the default repository in the .hgrc to the push repo, not the pull repo
        # These are essentially always the same now in that they are the stream repo.  But we now we only want to change
        #  it if we are pushing to the stream that we were bootstrapped to.
        
        # Buildstrategy will not exist for transkit.
        if not leaveRepo and globalvars.buildStrategy != None:
            newHgrcFile = None
            # If it fails, no big deal.
            try:
                currentHgrc = compat.getConfigParser ()
                realHgrcFileName = os.path.join (self.GetLocalDir(), '.hg', 'hgrc')
                if os.path.exists (realHgrcFileName):
                    currentHgrc.read (realHgrcFileName)

                pushToRepo = globalvars.buildStrategy.FindRemoteRepository (self.m_name)
                if not currentHgrc.has_section ('paths'):
                    currentHgrc.add_section ('paths')
                currentHgrc.set ('paths', 'default', self.SetupUrl(pushToRepo.GetAddress()) + '\n')

                newHgrcFile = open (os.path.join (self.GetLocalDir(), '.hg', 'hgrc.new'), 'wt')
                currentHgrc.write (newHgrcFile)
                newHgrcFile.close()

                shutil.move (newHgrcFile.name, realHgrcFileName)
            except:
                if newHgrcFile != None:
                    newHgrcFile.close()

                try:
                    os.remove (newHgrcFile)
                except:
                    pass

        return hgReturn

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalHgRepository
    #-------------------------------------------------------------------------------------------
    def RollbackRebaseAndMerge (self, _remoteRepo, RepoChangedFunc, updateStartTime):
        # reverts any previous attempts at rebasing leaving the repository in a state with multiple heads
        # I'm not sure this is needed; usually it seems that when it fails the rebase just doesn't happen.
        self.RunRepoCmd(['hg', '-R', self.GetLocalDir(), 'rebase', '--abort'], None, None)
            
        # Attempt to merge the multiple heads. This should fail because rebasing did, but now a prompt can be shown
        # stating the repo needs fixed to commit a merge changeset. 
        # This approach will create a more descriptive history than just rebasing the fix merge conflicts
        self.MergeRepo (RepoChangedFunc, updateStartTime)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalHgRepository
    #-------------------------------------------------------------------------------------------
    def PushSource (self, remoteRepo):
        returnVal = [None]

        repoRe = re.compile(r"added \d+ changesets with \d+ changes to \d+ files")
        def processPushOutput (outputLine):
            if outputLine.startswith ("remote: "):
                outputLine = outputLine[8:]

            if outputLine.startswith( "abort:" ):
                utils.exitOnError( 1, self.GetDescriptiveString() + ": " + outputLine )
            elif outputLine.startswith ("waiting for lock on repository"):
                utils.showInfoMsg (outputLine, utils.INFO_LEVEL_Essential, utils.YELLOW)
            else:
                utils.showInfoMsg (outputLine, utils.INFO_LEVEL_SomewhatInteresting)

            if repoRe.match(outputLine):
                returnVal[0] = outputLine.strip()

        setcwd = self.GetLocalDir()
        cmd = ['hg', 'push', self.SetupUrl(remoteRepo.GetAddress())]
        self.RunNetworkRepoCmd (cmd, setcwd, processPushOutput)

        return returnVal[0]

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalHgRepository
    #-------------------------------------------------------------------------------------------
    def GetListOfModifiedFiles (self):
        statOutput = []

        def ParseHgStatOutput (outputLine):
            utils.showInfoMsg (outputLine, utils.INFO_LEVEL_SomewhatInteresting)
            if outputLine.strip() != "":
                statOutput.append (outputLine.strip())
        
        self.ProcessHgCommand (['hg', 'status'], ParseHgStatOutput)
        return statOutput

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalHgRepository
    #-------------------------------------------------------------------------------------------
    def GetDiff (self):
        diff = []

        def ParseHgDiffOutput (outputLine):
            utils.showInfoMsg (outputLine, utils.INFO_LEVEL_SomewhatInteresting)
            if outputLine.strip() != "":
                diff.append (outputLine.strip())
        
        self.ProcessHgCommand (['hg', 'diff'], ParseHgDiffOutput, utils.INFO_LEVEL_Essential, utils.YELLOW)
        return diff

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalHgRepository
    #-------------------------------------------------------------------------------------------
    def GetIncomingChanges (self, remoteRepo, pullToRev, hgOptions):
        incomingOutput = []

        def parseIncomingOutput (outputLine):
            if outputLine.startswith( "abort:" ):
                utils.exitOnError( 1, self.GetDescriptiveString() + ": " + outputLine )
            else:
                utils.showInfoMsg (outputLine, utils.INFO_LEVEL_SomewhatInteresting)

            incomingOutput.append (outputLine.strip())
        
        hgCmd = ['hg', 'incoming']
        hgCmd.extend (hgOptions)
        hgCmd.append (self.SetupUrl(remoteRepo.GetAddress()))
        if pullToRev:
            hgCmd.append ("-r{0}".format (pullToRev))
        self.ProcessHgCommand(hgCmd, parseIncomingOutput)

        if not incomingOutput[-1:] == ["no changes found"]:
            # Remove the first two lines, which contain something like:
            # comparing with c:\src\therepo
            # searching for changes
            return incomingOutput[2:]

        return None

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalHgRepository
    #-------------------------------------------------------------------------------------------
    def GetLatestRevision (self, remoteRepo, pullToRev):
        incomingChanges = self.GetIncomingChanges (remoteRepo, pullToRev, [r'--template={node}\n'])
        if not incomingChanges:
            return None
        
        return incomingChanges[-1].strip()

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalHgRepository
    #-------------------------------------------------------------------------------------------
    def GetOutgoingChanges (self, remoteRepo, brief, hgOptions):
        outgoingOutput = []

        def outgoingProcessor (outputLine):
            if outputLine.startswith( "abort:" ):
                utils.exitOnError( 1, self.GetDescriptiveString() + ": " + outputLine )
            else:
                utils.showInfoMsg (outputLine, utils.INFO_LEVEL_SomewhatInteresting)

            outgoingOutput.append (outputLine.strip())

        cmd = ['hg', 'outgoing']
        cmd.extend (hgOptions)
        cmd.append (self.SetupUrl(remoteRepo.GetAddress()))
        self.ProcessHgCommand (cmd, outgoingProcessor)

        if not outgoingOutput[-2:] == ["searching for changes", "no changes found"]:
            if brief:
                return []
            else:
                # Remove the first two lines, which contain something like:
                # comparing with c:\src\therepo
                # searching for changes
                return outgoingOutput[2:]

        return None

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalHgRepository
    #-------------------------------------------------------------------------------------------
    def AddFiles(self):
        if not self.IsPushable() or not os.path.exists(self.GetLocalDir()):
            return

        returnVal = [None]

        def ParseAddOutput(outputLine):
            if outputLine.startswith( "abort:" ):
                utils.exitOnError( 1, self.GetDescriptiveString() + ": " + outputLine )
            else:
                utils.showInfoMsg (outputLine, utils.INFO_LEVEL_SomewhatInteresting)

            commitMatch = re.match( 'adding (.*)', outputLine )
            if commitMatch:
                returnVal[0] = commitMatch.group(1)

        utils.showStatusMsg ("Performing hg add on {0}".format (self.m_name), utils.INFO_LEVEL_VeryInteresting)

        addCommand = ['hg', '-v', 'add']
        self.ProcessHgCommand (addCommand, ParseAddOutput)

        return returnVal[0]

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalHgRepository
    #-------------------------------------------------------------------------------------------
    def Commit(self, username, message, logFile):
        if not self.IsPushable() or not os.path.exists (self.GetLocalDir()):
            return

        returnVal = [None]

        def outputProcessor (outputLine):
            if outputLine.startswith( "abort:" ):
                utils.exitOnError( 1, self.GetDescriptiveString() + ": " + outputLine )
            else:
                utils.showInfoMsg (outputLine, utils.INFO_LEVEL_SomewhatInteresting)

            commitMatch = re.match( 'committed changeset (.*)', outputLine )
            if commitMatch:
                returnVal[0] = commitMatch.group(1)

        utils.showStatusMsg ("Performing hg commit on {0}".format (self.m_name), utils.INFO_LEVEL_VeryInteresting)

        commitCommand = ['hg', '-v', 'commit']
        if username != None:
            commitCommand.extend (['-u', username])
        if message != None:
            commitCommand.extend (['-m', message])
        if logFile != None:
            commitCommand.extend (['-l', logFile])

        self.ProcessHgCommand (commitCommand, outputProcessor)

        return returnVal[0]

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalHgRepository
    #-------------------------------------------------------------------------------------------
    def GetStreamTipRevision (self, remoteUrl):
        output = [None, None]
        def parseOutput (outputLine):
            utils.showInfoMsg (outputLine, utils.INFO_LEVEL_SomewhatInteresting)
            if not '=' in outputLine:
                return
            match = re.search ('(http.*) = ([A-Za-z0-9]+)', outputLine)
            if match:
                output[0] = match.group(2).strip()  # GUID
                output[1] = match.group(1).strip()  # URL

        cmd = ['hg', 'remotetip', remoteUrl]
        cmdutil.runAndWait (cmd, os.getcwd(), parseOutput)
        return output[0], output[1]

#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
class LocalGitRepository (LocalRepository):

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalGitRepository
    #-------------------------------------------------------------------------------------------
    def __init__(self, name, localDir):
        LocalRepository.__init__(self, name, globalvars.REPO_GIT, localDir)
        
    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalGitRepository
    #-------------------------------------------------------------------------------------------
    def BdfHasName(self, bdf):
        return bdf.HasGitName(self.m_name.lower())

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalGitRepository
    #-------------------------------------------------------------------------------------------
    def BdfGetRevision(self, bdf):
        return bdf.GetGitRevision(self.m_name.lower())

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalGitRepository
    #-------------------------------------------------------------------------------------------
    def BdfGetUrl(self, bdf):
        return bdf.GetGitUrl(self.m_name.lower())

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalGitRepository
    #-------------------------------------------------------------------------------------------
    def BdfAddEntry(self, bdf, guid, repoUrl, fromLKG=False):
        return bdf.AddGitEntry(self.m_name.lower(), guid, repoUrl, fromLKG)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalGitRepository
    #-------------------------------------------------------------------------------------------
    def BdfRemoveEntry(self, bdf):
        return bdf.RemoveGitEntry(self.m_name.lower())

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalGitRepository
    #-------------------------------------------------------------------------------------------
    def BdfUpdateEntry(self, bdf, guid, repoUrl, fromLKG=False):
        return bdf.UpdateGitEntry(self.m_name.lower(), guid, repoUrl, fromLKG)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalGitRepository
    #-------------------------------------------------------------------------------------------
    def IsPushable (self):
        return True

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalGitRepository
    #-------------------------------------------------------------------------------------------
    def IsRetryMessage (self, outputLine):
        outputLine = outputLine.lower()
        if 'existing connection was forcibly closed by the remote host' in outputLine or \
           'could not resolve host' in outputLine or \
           'This request was blocked because there are too many clones running at the same time' in outputLine or \
           'Error in the pull function' in outputLine or \
           'connection attempt failed because the connected party did not properly respond after a period of time' in outputLine or \
           'The requested URL returned error: 503' in outputLine or \
           'failed to receive handshake, SSL/TLS connection failed' in outputLine:
            return True
        return False

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalGitRepository
    #-------------------------------------------------------------------------------------------
    def SetupUrl (self, remoteUrl):
        # Allow switch to SSH
        # We are disabling this for now because Git LFS does not work over SSH.
        ServersThatSupportSSH = ['visualstudio.com', 'azure.com', 'github.com']

        if not s_gitUseSSH:
            return remoteUrl

        supportsSSH = False
        serverUrl = None
        splUrl = compat.parseUrl (remoteUrl)
        for serv in ServersThatSupportSSH:
            if serv in splUrl.netloc.lower():
                supportsSSH = True
                serverUrl = serv
                break

        if not supportsSSH:
            return remoteUrl

        if serverUrl == 'azure.com':
            # https://orgname@dev.azure.com/orgname/projname/_git/reponame
            # git@ssh.dev.azure.com:v3/orgname/projname/reponame

            pathPieces = splUrl.path.strip('/').split('/')
            org = pathPieces[0]
            team = pathPieces[1]
            repo = pathPieces[3]

            newUrl = 'git@ssh.dev.azure.com:v3/{0}/{1}/{2}'.format(org, team, repo)
        elif serverUrl == 'visualstudio.com':
            # https://orgname.visualstudio.com/projname/_git/reponame
            # orgname@vs-ssh.visualstudio.com:v3/orgname/projname/reponame

            serverPieces = splUrl.netloc.split('.')
            server = '.'.join(serverPieces[1:])
            repo = serverPieces[0]

            pathPieces = splUrl.path.strip('/').split('/')
            pathPre = '/'.join(pathPieces[0:-2])
            pathPost = pathPieces[-1]

            newUrl = '{0}@vs-ssh.{1}:v3/{0}/{2}/{3}'.format (repo, server, pathPre, pathPost)

        utils.showInfoMsg ('Changing to SSH {0} => {1}\n'.format(remoteUrl, newUrl), utils.INFO_LEVEL_SomewhatInteresting)

        return newUrl

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalGitRepository
    #-------------------------------------------------------------------------------------------
    def ParseGenericOutput (self, outputLine):
        utils.showInfoMsg (outputLine, utils.INFO_LEVEL_SomewhatInteresting)
        if outputLine.startswith( "fatal:" ):
            utils.exitOnError( 1, self.GetDescriptiveString() + ": " + outputLine )

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalGitRepository
    #-------------------------------------------------------------------------------------------
    def ProcessCommand(self, command, outputProcessor, commandInfoLevel=utils.INFO_LEVEL_SomewhatInteresting, commandColor=utils.UNCHANGED):
        outputText = "Performing {1} on {0}\n".format(self.GetLocalDir(), command)
        utils.showInfoMsg (outputText, commandInfoLevel, commandColor)

        return self.RunRepoCmd (command, None, outputProcessor)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalGitRepository
    #-------------------------------------------------------------------------------------------
    def GetAuthBaseCommand (self, args, remoteRepo):
        return self.GetBaseCommand (args, gitCmd=remoteRepo.GetAuthorizedGit ())

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalGitRepository
    #-------------------------------------------------------------------------------------------
    def GetBaseCommand (self, args, gitCmd=None):
        cmd = gitCmd or ['git']
        cmd.extend (['-C', self.GetLocalDir()])
        if args:
            cmd.extend (args)
        return cmd

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalGitRepository
    #-------------------------------------------------------------------------------------------
    def ProcessGitCommand(self, command, outputProcessor, commandInfoLevel=utils.INFO_LEVEL_SomewhatInteresting, commandColor=utils.UNCHANGED):
        outputText = "Performing {1} on {0}\n".format(self.GetLocalDir(), command)
        utils.showInfoMsg (outputText, commandInfoLevel, commandColor)

        cmdutil.runAndWait (command, self.GetLocalDir(), outputProcessor)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalGitRepository
    #-------------------------------------------------------------------------------------------
    def GetHeadsCount (self, commandInfoLevel=utils.INFO_LEVEL_SomewhatInteresting):
        headsCount = [0]

        def ParseGitBranchOutput (outputLine):
            self.ParseGenericOutput(outputLine)
            headsCount[0] += 1

        self.ProcessCommand (self.GetBaseCommand(['branch']), ParseGitBranchOutput, commandInfoLevel=commandInfoLevel)
        return headsCount[0]

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalGitRepository
    #-------------------------------------------------------------------------------------------
    def GetListOfModifiedFiles (self):
        statOutput = []

        def ParseGitStatOutput (outputLine):
            self.ParseGenericOutput(outputLine)
            if outputLine.strip() != "":
                statOutput.append (outputLine.strip())
        
        self.ProcessCommand (self.GetBaseCommand(['status', '-s']), ParseGitStatOutput)
        return statOutput

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalGitRepository
    #-------------------------------------------------------------------------------------------
    def GetDiff (self):
        diff = []

        def ParseGitDiffOutput (outputLine):
            utils.showInfoMsg (outputLine, utils.INFO_LEVEL_SomewhatInteresting)
            if outputLine.strip() != "":
                diff.append (outputLine.strip())
        
        self.ProcessCommand (self.GetBaseCommand(['diff']), ParseGitDiffOutput, utils.INFO_LEVEL_Essential, utils.YELLOW)
        return diff

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalGitRepository
    #-------------------------------------------------------------------------------------------
    def GetCurrentBranch (self):
        currentBranch = [None]

        def ParseGitBranchOutput (outputLine):
            self.ParseGenericOutput(outputLine)
            outputLine = outputLine.strip()
            if outputLine:
                if outputLine.startswith ('*'): # Uncommitted files are ok
                    currentBranch[0] = outputLine[2:]

        self.RunRepoCmd (self.GetBaseCommand(['branch']), None, ParseGitBranchOutput)
        return currentBranch[0]

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalGitRepository
    #-------------------------------------------------------------------------------------------
    def GetBranches (self):
        branches = [None]

        def ParseGitBranchOutput (outputLine):
            self.ParseGenericOutput(outputLine)
            outputLine = outputLine.strip()
            if outputLine:
                if outputLine.startswith ('*'): # Uncommitted files are ok
                    branches.append(outputLine[2:])
                else:
                    branches.append(outputLine)

        self.RunRepoCmd (self.GetBaseCommand(['branch']), None, ParseGitBranchOutput)
        return branches

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalGitRepository
    #-------------------------------------------------------------------------------------------
    def GetRemoteBranches (self):
        refs = []

        GIT_REFS_REMOTE_ORIGIN = "refs/remotes/origin/"

        def ParseGitRemoteBranchesOutput (outputLine):
            self.ParseGenericOutput(outputLine)
            outputLine = outputLine.strip()
            if outputLine and outputLine.startswith(GIT_REFS_REMOTE_ORIGIN):
                # example output:
                # refs/remotes/origin/HEAD
                # refs/remotes/origin/master
                # refs/remotes/origin/version/10-17-0-199
                ref = outputLine[len(GIT_REFS_REMOTE_ORIGIN):]
                if ref != "HEAD":
                    refs.append(ref)

        self.RunRepoCmd (self.GetBaseCommand(['for-each-ref', '--format=%(refname)']), None, ParseGitRemoteBranchesOutput)
        return refs

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalGitRepository
    #-------------------------------------------------------------------------------------------
    def GetTags (self):
        tags = [None]

        def ParseGitTagOutput (outputLine):
            self.ParseGenericOutput(outputLine)
            outputLine = outputLine.strip()
            if outputLine:
                tags.append(outputLine)

        self.RunRepoCmd (self.GetBaseCommand(['tag']), None, ParseGitTagOutput)
        return tags

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalGitRepository
    #-------------------------------------------------------------------------------------------
    def GetDefaulBranch (self):
        # Main has replaced master so have to check. Can also be set by a team.
        defaultBranchName = [None]

        def ParseGitOutput (outputLine):
            utils.showInfoMsg (outputLine, utils.INFO_LEVEL_SomewhatInteresting)
            if outputLine.startswith( "fatal:" ):
                utils.exitOnError( 1, self.GetDescriptiveString() + ": " + outputLine )
            # Should be only one line:
            #   origin/master 
            #   origin/main
            outputLine = outputLine.strip()
            if outputLine:
                defaultBranchName[0] = outputLine.split('/')[-1]

        self.RunRepoCmd (self.GetBaseCommand(['rev-parse', '--abbrev-ref', 'origin/HEAD']), None, ParseGitOutput)
        return defaultBranchName[0]

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalGitRepository
    #-------------------------------------------------------------------------------------------
    def Checkout (self, gitRef, remoteRepo, outputProcessor):
        # Pull to the default branch if nothing is specified. The default can technically be changed, and maybe we
        #   should use something 'git remote show origin'. Github also has its own way of changing the default branch.
        refToCheckout = gitRef if gitRef else self.GetDefaulBranch()
        utils.showInfoMsg ('Updating Git repo {0} to branch {1}\n'.format(self.m_name, refToCheckout), utils.INFO_LEVEL_Interesting)
        cmd = self.GetAuthBaseCommand(['checkout', refToCheckout], remoteRepo)
        status = self.RunRepoCmd(cmd, None, outputProcessor)
        return status

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalGitRepository
    #-------------------------------------------------------------------------------------------
    def CreateAndCheckoutBranch (self, branch):
        self.RunRepoCmd (self.GetBaseCommand(['checkout', '-b', branch]), None, self.ParseGenericOutput)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalGitRepository
    #-------------------------------------------------------------------------------------------
    def HasLocalChanges (self):
        hasChanges = [None]

        def ParseGitStatusOutput (outputLine):
            utils.showInfoMsg (outputLine, utils.INFO_LEVEL_SomewhatInteresting)
            if outputLine.startswith( "fatal:" ):
                utils.exitOnError( 1, self.GetDescriptiveString() + ": " + outputLine )
            if None != hasChanges[0]:  # only process the first line of output
                return
            # I think blank means clean while anything else means dirty, or errors.
            if outputLine.strip():
                if not outputLine.startswith ('?'): # Uncommitted files are ok
                    hasChanges[0] = True

        self.RunRepoCmd (self.GetBaseCommand(['status', '-s']), None, ParseGitStatusOutput)
        return hasChanges[0]

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalGitRepository
    #-------------------------------------------------------------------------------------------
    def HasIncomingChanges (self, remoteRepo, pullToRev=None):
        incomingChanges = [False]

        def parseOutput(outputLine):
            utils.showInfoMsg (outputLine, utils.INFO_LEVEL_SomewhatInteresting)
            if outputLine.startswith('fatal: no upstream configured for branch'):
                return # If there is nothing upstream then there can be no incoming.
            elif outputLine.startswith( "fatal:" ):
                utils.exitOnError(1, self.GetDescriptiveString() + ": " + outputLine )
            else:
                utils.showInfoMsg(outputLine, utils.INFO_LEVEL_SomewhatInteresting)

            if 'incoming changes' in outputLine:
                incomingChanges[0] = True

        self.RunNetworkRepoCmd(self.GetAuthBaseCommand(['fetch'], remoteRepo), None, None)
        logCmd = '..@{u}'
        if pullToRev and pullToRev != '0':
            logCmd = pullToRev + logCmd
        self.RunRepoCmd(self.GetBaseCommand(['log', logCmd, '--pretty=format:"incoming changes"']), None, parseOutput)

        return incomingChanges[0]

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalGitRepository
    #-------------------------------------------------------------------------------------------
    def HasOutgoingChanges (self, remoteRepo, _pushToRev=None):
        outgoingChanges = [False]

        def parseOutput(outputLine):
            utils.showInfoMsg (outputLine, utils.INFO_LEVEL_SomewhatInteresting)
            if outputLine.startswith( "fatal:" ):
                utils.exitOnError(1, self.GetDescriptiveString() + ": " + outputLine )
            else:
                utils.showInfoMsg(outputLine, utils.INFO_LEVEL_SomewhatInteresting)

            if 'outgoing changes' in outputLine:
                outgoingChanges[0] = True

        self.RunNetworkRepoCmd(self.GetAuthBaseCommand(['fetch'], remoteRepo), None, None)
        self.RunRepoCmd(self.GetBaseCommand(['log', '--branches', '--not', '--remotes=origin', '--pretty=format:"outgoing changes"']), None, parseOutput)

        return outgoingChanges[0]

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalGitRepository
    #-------------------------------------------------------------------------------------------
    def GetProvenance (self):
        identString = [None]

        def ParseGitRevOutput (outputLine):
            utils.showInfoMsg (outputLine, utils.INFO_LEVEL_SomewhatInteresting)
            if outputLine.startswith( "fatal:" ):
                utils.exitOnError( 1, self.GetDescriptiveString() + ": " + outputLine )
            if None != identString[0]:  # only process the first line of output
                return
            identString[0] = outputLine.strip()

        self.RunRepoCmd (self.GetBaseCommand(['rev-parse', 'HEAD']), None, ParseGitRevOutput)
        if self.HasLocalChanges():
            identString[0] += "+"

        return identString[0]

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalGitRepository
    #-------------------------------------------------------------------------------------------
    def GetTipRevision (self, commandInfoLevel=utils.INFO_LEVEL_SomewhatInteresting, useLocalNumber=False):
        tipRevision = [None]

        def ParseTipOutput (outputLine):
            utils.showInfoMsg (outputLine, utils.INFO_LEVEL_SomewhatInteresting)
            outputLine = outputLine.strip()
            if outputLine:
                tipRevision[0] = outputLine
        
        if False == useLocalNumber:
            self.ProcessCommand(self.GetBaseCommand(['log', '-1', '--pretty=%H']), ParseTipOutput, commandInfoLevel)

        return tipRevision[0]

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalGitRepository
    #-------------------------------------------------------------------------------------------
    def DoesRevisionExist (self, revision, commandInfoLevel=utils.INFO_LEVEL_SomewhatInteresting):
        wasFound = [False]
        def ParseOutput (outputLine):
            utils.showInfoMsg (outputLine, utils.INFO_LEVEL_SomewhatInteresting)
            outputLine = outputLine.strip()
            if outputLine.startswith ("found"):
                wasFound[0] = True

        self.ProcessCommand (self.GetBaseCommand(['show', revision, '-q', '--pretty=format:found']), ParseOutput, commandInfoLevel)
        return wasFound[0]

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalGitRepository
    # Returns the HEAD revision GUID (e.g. via `rev-parse HEAD`).
    #-------------------------------------------------------------------------------------------
    def GetRevision(self, gitPointer, outputProcessor=None):
        headRev = [""]

        def scrapeRevision(line):
            headRev[0] = line.strip()
            if outputProcessor:
                outputProcessor(line)

        self.RunRepoCmd(self.GetBaseCommand(["rev-parse", gitPointer]), None, scrapeRevision)

        return headRev[0]

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalGitRepository
    #-------------------------------------------------------------------------------------------
    def ChangeUrl(self, newUrl):
        if not newUrl:
            return

        def PrintOutput (outputLine):
            utils.showInfoMsg (outputLine, utils.INFO_LEVEL_SomewhatInteresting)

        self.ProcessCommand (self.GetBaseCommand(['remote', 'set-url', 'origin', newUrl]), PrintOutput)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalGitRepository
    #-------------------------------------------------------------------------------------------
    def GetUrl(self):
        resultUrl = [None]
        def CheckOutput (outputLine):
            utils.showInfoMsg (outputLine, utils.INFO_LEVEL_SomewhatInteresting)
            splitLine = outputLine.split()
            if splitLine[0] == 'origin' and splitLine[2] == '(fetch)':
                resultUrl[0] = splitLine[1]

        self.ProcessCommand (self.GetBaseCommand(['remote', '-v']), CheckOutput)
        return resultUrl[0]

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalGitRepository
    #-------------------------------------------------------------------------------------------
    def LfsPull(self, remoteRepo):
        def ShowOutput (outputLine):
            utils.showInfoMsg (outputLine, utils.INFO_LEVEL_Interesting) # Can be turned down once this actually works
            if ('not installed' in outputLine):
                utils.showInfoMsg ('WARNING: Git LFS is not installed. To fix this you need to both install git lfs and run "git lfs install" to update the config files. {0}\n'.format(outputLine), utils.INFO_LEVEL_Essential, utils.YELLOW)

        cmd = remoteRepo.GetAuthorizedGit()
        cmd = self.GetAuthBaseCommand(['lfs', 'pull'], remoteRepo)
        utils.showInfoMsg ('running git lfs pull {}\n'.format(repr(cmd)), utils.INFO_LEVEL_Interesting) # Can be turned down once this actually works
        status = self.RunNetworkRepoCmd(cmd, None, ShowOutput)
        return status

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalGitRepository
    #-------------------------------------------------------------------------------------------
    def CheckIfGitRepo(self):
        if not os.path.exists(os.path.join(self.m_localDir, '.git')):
            if os.path.exists(os.path.join(self.m_localDir, '.hg')):
                raise utils.BuildError('Directory "{0}" exists but it is a Mercurial repo. Perhaps the type has changed to git.\nPlease move the directory and pull again.'.format(self.m_localDir))
            if os.path.exists(os.path.join(self.m_localDir, 'cvs')):
                raise utils.BuildError('Directory "{0}" exists but it is a CVS repo. Perhaps the type has changed to git.\nPlease move the directory and pull again.'.format(self.m_localDir))
            raise utils.BuildError('Directory "{0}" exists but does not appear to be a repository (missing .git).\nPlease move the directory and pull again.'.format(self.m_localDir))

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalGitRepository
    #-------------------------------------------------------------------------------------------
    def TypeChangedAndUnattended(self):
        return self.CheckTypeChangedAndRemove('hg', 'git') or self.CheckTypeChangedAndRemove('cvs', 'git')

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalGitRepository
    #-------------------------------------------------------------------------------------------
    def ServerChanged(self, remoteRepo):
        oldUrl = self.GetUrl()
        if not oldUrl:
            utils.showInfoMsg("Unable to get URL for exising repo {0} \n".format(self.m_localDir), utils.INFO_LEVEL_Important)
            return False
        oldLoc = RemoteRepository.GetServerFromUrl(oldUrl)

        newLoc = remoteRepo.GetServerUrl()
        utils.showInfoMsg("Server check (Change={2}): {0} -> {1} \n".format(oldLoc, newLoc, oldLoc!=newLoc), utils.INFO_LEVEL_SomewhatInteresting)
        return newLoc != oldLoc

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalGitRepository
    #-------------------------------------------------------------------------------------------
    def CheckIfServerChanged(self, remoteRepo):
        if self.ServerChanged (remoteRepo):
            raise utils.BuildError('Directory "{0}" exists but it is pointing to a different Git server. Because some repos changed drastically moving to GitHub we require manual intervention.\nPlease move the directory and pull again.'.format(self.m_localDir))

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalGitRepository
    #-------------------------------------------------------------------------------------------
    def GitServerChangedAndUnattended(self, remoteRepo):
        if not utils.isUnattendedBuild():
            return False
        return self.ServerChanged (remoteRepo)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalGitRepository
    # Clones a new copy of the requested repo, and leaves it at the tip of master.
    #-------------------------------------------------------------------------------------------
    def _CloneTip(self, remoteRepo, options, outputProcessor, shallow=False):
        # Ensure parent directory exists.
        parentDir = self.GetLocalDir().rstrip(os.path.sep).split()[0]
        if not os.path.exists(parentDir):
            symlinks.makeSureDirectoryExists(parentDir)

        cmd = remoteRepo.GetAuthorizedGit()
        cmd.extend(['clone', '-v'])

        repoOptions = globalvars.buildStrategy.FindRepositoryOptions(self.m_name)
        if repoOptions:
            if repoOptions.m_tag:
                cmd.extend(['-b', repoOptions.m_tag])
            elif repoOptions.m_branch:
                cmd.extend(['-b', repoOptions.m_branch])
            elif repoOptions.m_legacyTag:
                cmd.extend(['-b', repoOptions.m_legacyTag])

        if shallow:
            cmd.extend(['--depth', '1'])

        if options:
            cmd.extend(options.split())

        cmd.extend([self.SetupUrl(remoteRepo.GetAddress()), self.GetLocalDir()])

        def localOutputProcessor (line):
            if 'Host key verification failed' in line:
                _, quotedCommand = cmdutil.processCommand (cmd)
                utils.exitOnError(1, 'Git error:  {0}\nThis usually requires you to clone the repository manually once to get the server into the system. \n   {1}\n'.format(line.strip(), ' '.join(quotedCommand)) )
            if outputProcessor is not None:
                outputProcessor (line)
            else:
                utils.showInfoMsg (line, utils.INFO_LEVEL_SomewhatInteresting)

        status = self.RunNetworkRepoCmd(cmd, None, localOutputProcessor)

        return status
    
    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalGitRepository
    # Clones a new copy of the requested repo, and if pullToRev is provided, will roll it back as specified.
    #-------------------------------------------------------------------------------------------
    def _CloneSource(self, remoteRepo, pullToRev, options, pullOutputProcessor, cloneOutputProcessor, shallow, branchName):
        outputProcessor = cloneOutputProcessor if cloneOutputProcessor else pullOutputProcessor

        status = self._CloneTip(remoteRepo, options, outputProcessor, shallow)
        if status:
            return status

        # I'm not sure how you get here, but old code guraded against it, and I have seen 0's in the TRL before...
        if pullToRev == '0':
            utils.exitOnError(1, self.GetDescriptiveString() + ": Cannot pull to revision '" + pullToRev + "'" )
            return 1

        if pullToRev:
            status = self.Checkout(pullToRev, remoteRepo, outputProcessor)
        elif branchName:
            status = self.Checkout(branchName, remoteRepo, outputProcessor)

        return status

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalGitRepository
    # Updates the repository to the tip revision, merging/rebasing as necessary.
    #-------------------------------------------------------------------------------------------
    def _UpdateNonTRLSource(self, remoteRepo, options, pullOutputProcessor, forceBranch, branchName):
        self.CheckIfGitRepo()

        if utils.isUnattendedBuild():
            if self.HasLocalChanges():
                utils.showInfoMsg("Repository " + remoteRepo.m_name + " has uncommitted changes; resetting the working directory.\n", utils.INFO_LEVEL_Important)
                status = self.RunNetworkRepoCmd(self.GetAuthBaseCommand(['reset', '--hard'], remoteRepo), None, pullOutputProcessor)
                if status:
                    return status

        self.CheckIfServerChanged(remoteRepo)
        # Maybe we should limit this to unattended builds.
        self.ChangeUrl (remoteRepo.GetAddress())

        repoOptions = globalvars.buildStrategy.FindRepositoryOptions(self.m_name)
        forceCheckout = forceBranch or repoOptions.m_forceCheckout if repoOptions else forceBranch

        if repoOptions and repoOptions.m_tag:
            wantedTag = repoOptions.m_tag
            currentRevision = self.GetRevision("HEAD")
            wantedTagRevision = self.GetRevision(wantedTag)

            #check head guid and tag guid
            tagCheckedOut = (currentRevision == wantedTagRevision)

            if forceCheckout and not tagCheckedOut:
                tags = self.GetTags()
                if wantedTag in tags:
                    status = self.Checkout(wantedTag, remoteRepo, pullOutputProcessor)
                    if status:
                        return status
                    tagCheckedOut = True

            if self.HasLocalChanges():
                raise utils.StrategyError ("You have local changes in repository {0}, please commit them and try again".format(self.m_name))

            if self.HasUnpushedCommits():
                raise utils.StrategyError ("You have unpushed commits in repository {0}, please push then and try again".format(self.m_name))

            cmd = self.GetAuthBaseCommand(['fetch', '--all', '--tags'], remoteRepo)
            status = self.RunNetworkRepoCmd(cmd, None, pullOutputProcessor)
            if status == 0 and not tagCheckedOut:
                status = self.Checkout(wantedTag, remoteRepo, pullOutputProcessor)
                if utils.isUnattendedBuild():
                    self.LfsPull (remoteRepo) # Recover from failed previous lfs
                utils.showInfoMsg("You are in a 'detached HEAD' state in repository {0}".format(self.m_name), utils.INFO_LEVEL_Important)

            return status

        else:
            currentBranch = self.GetCurrentBranch()
            wantedBranch = None
            if repoOptions:
                if repoOptions.m_branch:
                    wantedBranch = repoOptions.m_branch
                elif repoOptions.m_legacyTag:
                    wantedBranch = repoOptions.m_legacyTag
            if not wantedBranch and branchName:
                wantedBranch = branchName
            if not wantedBranch:
                if forceBranch:
                    wantedBranch = self.GetDefaulBranch() # No branch specified with forceBranch means to pull main/master.
                    if wantedBranch != currentBranch:
                        utils.showInfoMsg("Repo {0} forcing from branch {1} to default branch {2} because of forceBranch".format(self.m_name, currentBranch, wantedBranch), utils.INFO_LEVEL_Important)
                else:            
                    wantedBranch = currentBranch

            branchCheckedOut = (currentBranch == wantedBranch)
            if forceCheckout and not branchCheckedOut:
                if 'detached' in wantedBranch:
                    raise utils.StrategyError ("Repo {0} is trying to force a pull to detached HEAD {1} from current branch {2}".format(self.m_name, wantedBranch, currentBranch))
                localBranches = self.GetBranches()
                if wantedBranch in localBranches:
                    status = self.Checkout(wantedBranch, remoteRepo, pullOutputProcessor)
                    if status:
                        return status
                    branchCheckedOut = True
            elif not branchCheckedOut:
                utils.ShowAndDeferMessage("Git Repo {0} requested branch {1} but is on branch {2}\n".format(self.GetDescriptiveString(), wantedBranch, currentBranch), utils.INFO_LEVEL_Important, utils.LIGHT_BLUE)

            cmd = self.GetAuthBaseCommand(['pull', '-v', '--prune'], remoteRepo)

            # Only want to rebase changesets that do not exist in a remote repository, otherwise do a merge
            if self.HasUnpushedCommits():
                cmd.append('--rebase=true')

            if options:
                cmd.extend(options.split())

            # NEEDSWORK - how to integrate rempoteReop, and not just existing origin?
            cmd.append('origin')

            status = self.RunNetworkRepoCmd(cmd, None, pullOutputProcessor)

            if status == 0 and forceCheckout and not branchCheckedOut:
                status = self.Checkout(wantedBranch, remoteRepo, pullOutputProcessor)

            if utils.isUnattendedBuild():
                self.LfsPull (remoteRepo) # Recover from failed previous lfs
        return status

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalGitRepository
    # Tries to intelligently fetch remote changes and get to the requested pullToRev.
    #-------------------------------------------------------------------------------------------
    def _UpdateTRLSource(self, remoteRepo, pullToRev, options, pullOutputProcessor):
        self.CheckIfGitRepo()

        # There are otherwise several simultaneous moving parts to account for:
        #   Uncommitted changes
        #   Outgoing changes
        #   Did the local HEAD simply move to another existing rev
        #   Did the TRL actually change
        #
        # As noted above, we have to do a full fetch, and then figure out how to get into the desired state.
        # Since "getting into the desired state" can be destructive, be overly cautious up-front and err on the side of making the user decide.

        self.CheckIfServerChanged(remoteRepo)

        # No-op?
        headRev = self.GetRevision("HEAD", pullOutputProcessor)
        if pullToRev == headRev:
            utils.showInfoMsg("\nRepository {0} already to revision {1}\n".format(remoteRepo.m_name, pullToRev), utils.INFO_LEVEL_Interesting)
            if utils.isUnattendedBuild():
                self.LfsPull (remoteRepo) # Recover from failed previous lfs
            return 0

        #..........................................................................................
        # Otherwise, arbitrarily deciding that you want to at least fetch.

        cmd = self.GetAuthBaseCommand(['fetch', '-v', '--prune'], remoteRepo)
        if options:
            cmd.extend(options.split())

        cmd.append('origin')

        status = self.RunNetworkRepoCmd(cmd, None, pullOutputProcessor)
        if status:
            return status

        hasUncommittedChanges = self.HasLocalChanges()
        hasUnpushedCommits = self.HasUnpushedCommits()

        #..........................................................................................
        # Try to get back to, or integrate, the requested revision.

        # If no changes, can just safely reset. Reset may have to hit the server for LFS so must be auth.
        if not hasUncommittedChanges and not hasUnpushedCommits:
            status = self.Checkout(pullToRev, remoteRepo, pullOutputProcessor)
            return status

        # Uncommitted changes will block a rebase.
        if hasUncommittedChanges:
            utils.showInfoMsg("\nRepository " + remoteRepo.m_name + " has uncommitted changes; it will NOT be rebased or reset to requested revision " + pullToRev + ".\n", utils.INFO_LEVEL_Important, utils.RED)
            return 1

        if utils.isUnattendedBuild():
            utils.showInfoMsg("\nRepository " + remoteRepo.m_name + " has outgoing changes but this is an unattended build so ignoring and checking out requested revision " + pullToRev + ".\n", utils.INFO_LEVEL_Important, utils.YELLOW)
            status = self.Checkout(pullToRev, remoteRepo, pullOutputProcessor)
            self.LfsPull (remoteRepo) # Recover from failed previous lfs
        else:
            # Otherwise warn and try to rebase so the user can later push. Reset may have to hit the server for LFS so must be auth.
            utils.showInfoMsg("\nRepository " + remoteRepo.m_name + " has outgoing changes; attempting to rebase to requested revision " + pullToRev + ".\n", utils.INFO_LEVEL_Important, utils.YELLOW)
            status = self.RunNetworkRepoCmd(self.GetAuthBaseCommand(['rebase', pullToRev], remoteRepo), None, pullOutputProcessor)

        return status

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalGitRepository
    #-------------------------------------------------------------------------------------------
    def UpdateSource (self, remoteRepo, options=None, pullOutputProcessor=None, cloneOutputProcessor=None, _isLocalClone=False, pullToRev=None, _promptToStrip=False, shallow=False, forceBranch=False):
        # Repos in the TRL (e.g. pullToRev != None) are more complicated vs. HG because Git cannot pull to a revision.
        # Further, if the TRL revision is behind the remote HEAD, your only two options are to use detached heads, or a destructive command (reset --hard).
        # At present, we have decided against detached heads, so we're left using a destructive command. Because it's destructive, we have to be very careful.

        #..........................................................................................
        # Get the Data
        
        if pullToRev and pullToRev.startswith (builddescriptionfile.GIT_BRANCH_TAG_IDENTIFIER):
            branchName = pullToRev[len(builddescriptionfile.GIT_BRANCH_TAG_IDENTIFIER):]
            pullToRev = None
        else:
            branchName = None

        # Clone is similar enough for GUID, Branch and Tip cases.
        # TRL is short for TeamRevisionList which contained GUIDs.
        if not self.HasRepositoryLocally() or self.TypeChangedAndUnattended() or self.GitServerChangedAndUnattended(remoteRepo):
            self.CleanRepository() # in case when repository folder already exists
            status = self._CloneSource(remoteRepo, pullToRev, options, pullOutputProcessor, cloneOutputProcessor, shallow, branchName)
        # Because git doesn't use the same commands for branches and revisions, we have to put in an identifier.
        elif branchName:
            status = self._UpdateNonTRLSource(remoteRepo, options, pullOutputProcessor, forceBranch, branchName)
        elif not pullToRev:
            status = self._UpdateNonTRLSource(remoteRepo, options, pullOutputProcessor, forceBranch, None)
        else:
            status = self._UpdateTRLSource(remoteRepo, pullToRev, options, pullOutputProcessor)
        
        if status:
            return status

        #..........................................................................................
        # Reset the origin URL to help those that use repo commands directly.

        if globalvars.buildStrategy:
            # If it fails, no big deal.
            try:
                pushToRepo = globalvars.buildStrategy.FindRemoteRepository(self.m_name, self.m_repoType)
                self.RunRepoCmd(self.GetAuthBaseCommand(["remote", "-v", "set-url", "origin", self.SetupUrl(pushToRepo.GetAddress())], remoteRepo), None)
            except:
                pass

        return 0

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalGitRepository
    #-------------------------------------------------------------------------------------------
    def RollbackRebaseAndMerge (self, remoteRepo, outputParser):
        # reverts any previous attempts at rebasing leaving the repository in a state with multiple heads
        self.RunRepoCmd(self.GetAuthBaseCommand(['rebase', '--abort'], remoteRepo), None, None)
            
        # Attempt to merge the multiple heads. This should fail because rebasing did, but now a prompt can be shown
        # stating the repo needs fixed to commit a merge changeset. 
        # This approach will create a more descriptive history than just rebasing the fix merge conflicts
        self.RunRepoCmd(self.GetBaseCommand(['merge']), None, outputParser)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalGitRepository
    #-------------------------------------------------------------------------------------------
    def PushSource (self, remoteRepo):
        returnVal = [None]
    
        def processPushOutput(outputLine):
            utils.showInfoMsg (outputLine, utils.INFO_LEVEL_SomewhatInteresting)
            if outputLine.startswith("fatal:") or outputLine.startswith("error:"):
                utils.exitOnError(1, self.GetDescriptiveString() + ": " + outputLine)
            else:
                utils.showInfoMsg(outputLine, utils.INFO_LEVEL_SomewhatInteresting)

            #returnVal = outputLine.strip()
            returnVal[0] = 'Pushed'

        self.RunNetworkRepoCmd(self.GetAuthBaseCommand(['push', '--set-upstream', 'origin', 'HEAD'], remoteRepo), None, processPushOutput)
#        self.RunNetworkRepoCmd(self.GetAuthBaseCommand(['push', 'origin', 'HEAD'], remoteRepo), None, processPushOutput)

        return returnVal[0]

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalGitRepository
    #-------------------------------------------------------------------------------------------
    def GetIncomingChanges (self, remoteRepo, pullToRev, _hgOptions):
        incomingOutput = []

        def parseIncomingOutput (outputLine):
            utils.showInfoMsg (outputLine, utils.INFO_LEVEL_SomewhatInteresting)
            if outputLine.startswith( "fatal:" ):
                utils.exitOnError(1, self.GetDescriptiveString() + ": " + outputLine )

            incomingOutput.append (outputLine.strip())

        # need two commands to check incoming for git.. First pulls the updates but does not merge
        # ?? not sure if we want to get rid of the changes fetched, or leave them in an unmerged state???
        self.RunNetworkRepoCmd(self.GetAuthBaseCommand(['fetch'], remoteRepo), None, None)

        # It's hard to know what to do here. If we're supposed to be on a branch, list just for that branch.
        branchName = None
        if pullToRev and pullToRev.startswith (builddescriptionfile.GIT_BRANCH_TAG_IDENTIFIER):
            branchName = pullToRev[len(builddescriptionfile.GIT_BRANCH_TAG_IDENTIFIER):]
            pullToRev = None

        logCmd = '..@{u}'
        if branchName:
            logCmd = '..origin/{0}'.format(branchName)
        elif pullToRev:
            if pullToRev == '0':
                utils.exitOnError( 1, self.GetDescriptiveString() + ": Cannot check incoming to revision '" + pullToRev + "'" )
            else:
                logCmd = '..' + pullToRev

        self.RunRepoCmd(self.GetBaseCommand(['log', logCmd, '--pretty=format:----------------------------%n%an    %ad    [%h]%n%B%n', '--date=short']), None, parseIncomingOutput)

        if incomingOutput:
            if branchName:
                incomingOutput = ['On branch {0}\n'.format(branchName)] + incomingOutput
            return incomingOutput

        return None

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalGitRepository
    #-------------------------------------------------------------------------------------------
    def GetLatestRevision (self, remoteRepo, pullToRev): # pylint: disable=unused-argument
        lastRev = []

        # If we have a pull-to rev, then that's where we are going, unless it's where we are at.
        if pullToRev and pullToRev != '0':
            curRev = self.GetTipRevision()
            if curRev != pullToRev:
                return pullToRev

        # Otherwise we just get the tip of the current branch.
        def parseLastRevOutput (outputLine):
            utils.showInfoMsg (outputLine, utils.INFO_LEVEL_SomewhatInteresting)
            if outputLine.startswith( "fatal:" ):
                utils.exitOnError(1, self.GetDescriptiveString() + ": " + outputLine )

            # commit 4598c7366f7846e34e00676496c6be7f9459aace (origin/master, origin/HEAD)
            outputLine = outputLine.strip()
            if outputLine.startswith('commit'):
                splitLine = outputLine.split(' ')
                lastRev.append (splitLine[1])

        # need two commands to check incoming for git.. First pulls the updates but does not merge; second gets GUID at tip of current branch.
        self.RunNetworkRepoCmd(self.GetAuthBaseCommand(['fetch'], remoteRepo), None, None)
        self.RunRepoCmd(self.GetBaseCommand(['log', '..@{u}', '-1']), None, parseLastRevOutput)

        if lastRev:
            return lastRev[0]

        return None

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalGitRepository
    #-------------------------------------------------------------------------------------------
    def GetOutgoingChanges (self, remoteRepo, brief, _hgOptions):
        outgoingOutput = []

        def outgoingProcessor (outputLine):
            utils.showInfoMsg (outputLine, utils.INFO_LEVEL_SomewhatInteresting)
            if outputLine.startswith( "fatal:" ):
                utils.exitOnError( 1, self.GetDescriptiveString() + ": " + outputLine )
            outgoingOutput.append (outputLine.strip())

        self.RunNetworkRepoCmd(self.GetAuthBaseCommand(['fetch'], remoteRepo), None, None)
        # Still not sure how to get a list of all changes on all branches that haven't been pushed. 
        # @{u} compares the current branch to upstream, but if we are going to require feature branches then it complains that the branch doesn't exist on remote.
#        gitCmd = self.GetBaseCommand(['log', '@{u}..', '--pretty=format:----------------------------%n%an    %ad    [%h]%n%B%n', '--date=short'])
        gitCmd = self.GetBaseCommand(['log', '--branches', '--not', '--remotes=origin', '--pretty=format:----------------------------%n%an    %ad    [%h]%n%B%n', '--date=short'])
        self.RunRepoCmd(gitCmd, None, outgoingProcessor)
        
        if outgoingOutput:
            if brief:
                return []
            else:
                return outgoingOutput

        return None

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalGitRepository
    #-------------------------------------------------------------------------------------------
    def HasUnpushedCommits (self):
        outgoingChanges = [False]

        def parseOutput(outputLine):
            utils.showInfoMsg (outputLine, utils.INFO_LEVEL_SomewhatInteresting)
            if outputLine.startswith( "fatal:" ):
                utils.exitOnError(1, self.GetDescriptiveString() + ": " + outputLine )
            else:
                utils.showInfoMsg(outputLine, utils.INFO_LEVEL_SomewhatInteresting)

            if 'outgoing changes' in outputLine:
                outgoingChanges[0] = True

        self.RunRepoCmd(self.GetBaseCommand(['log', 'HEAD', '--not', '--remotes=origin', '--pretty=format:"outgoing changes"']), None, parseOutput)

        return outgoingChanges[0]

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalGitRepository
    #-------------------------------------------------------------------------------------------
    def CheckForRemovedBranches (self):
        # Always shows deferred messages about branches that have been merged on the server.
        goneRepos = []

        def outgoingProcessor (outputLine):
            utils.showInfoMsg (outputLine, utils.INFO_LEVEL_SomewhatInteresting)
            if outputLine.startswith( "fatal:" ):
                utils.exitOnError( 1, self.GetDescriptiveString() + ": " + outputLine )
            else:
                utils.showInfoMsg (outputLine, utils.INFO_LEVEL_SomewhatInteresting)

            if '[gone]' in outputLine:
                splitRepo = outputLine.strip().split(' ')
                repoName = splitRepo[1] if splitRepo[0] == '*' else splitRepo[0]
                goneRepos.append (repoName)

        gitCmd = self.GetBaseCommand(['branch', '-v'])
        self.RunRepoCmd(gitCmd, None, outgoingProcessor)

        if not goneRepos:
            return None

        utils.ShowAndDeferMessage('The following branches appear to have been merged and can be cleaned up locally if \n' + 
                                  'you are no longer using them. If the changesets were squashed on the server you will\n' +
                                  'have to use -D instead of -d but -D will also kill any changesets not pushed to the server.\n', utils.INFO_LEVEL_Essential)
        for repo in goneRepos:
            utils.ShowAndDeferMessage('   git branch -d {0}\n'.format (repo), utils.INFO_LEVEL_Essential)
        utils.ShowAndDeferMessage('\n', utils.INFO_LEVEL_Essential)

        return goneRepos

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalGitRepository
    #-------------------------------------------------------------------------------------------
    def AddFiles(self):
        if not self.IsPushable() or not os.path.exists(self.GetLocalDir()):
            return None

        returnVal = [None]

        def outputProcessor(outputLine):
            utils.showInfoMsg (outputLine, utils.INFO_LEVEL_SomewhatInteresting)
            if outputLine.startswith('fatal:'):
                utils.exitOnError( 1, self.GetDescriptiveString() + ": " + outputLine )
            else:
                utils.showInfoMsg (outputLine, utils.INFO_LEVEL_SomewhatInteresting)

            match = re.match('add \'(.*)\'', outputLine)
            if match:
                returnVal[0] = match.group(1)

        utils.showStatusMsg ("Performing git add on {0}".format (self.m_name), utils.INFO_LEVEL_VeryInteresting)

        gitCmd = self.GetBaseCommand(['add', '.'])
        self.RunRepoCmd(gitCmd, None, outputProcessor)
        
        return returnVal[0]

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalGitRepository
    #-------------------------------------------------------------------------------------------
    def Commit(self, username, message, logFile):
        if not self.IsPushable() or not os.path.exists (self.GetLocalDir()):
            return

        returnVal = [None]

        def outputProcessor(outputLine):
            utils.showInfoMsg (outputLine, utils.INFO_LEVEL_SomewhatInteresting)
            if outputLine.startswith('fatal:'):
                utils.exitOnError( 1, self.GetDescriptiveString() + ": " + outputLine )
            else:
                utils.showInfoMsg (outputLine, utils.INFO_LEVEL_SomewhatInteresting)

            commitMatch = re.match(r'.*\s(.*)]', outputLine)
            if commitMatch:
                returnVal[0] = commitMatch.group(1)

        utils.showStatusMsg ("Performing git commit on {0}".format (self.m_name), utils.INFO_LEVEL_VeryInteresting)

        gitCmd = self.GetBaseCommand(['commit', '-a'])
        if username != None:
            gitCmd.extend(['--author', username])
        if message != None:
            gitCmd.extend(['-m', message])
        if logFile != None:
            gitCmd.extend(['-F', logFile])
        
        self.RunRepoCmd(gitCmd, None, outputProcessor)

        return returnVal[0]

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalGitRepository
    #-------------------------------------------------------------------------------------------
    def GetStreamTipRevision (self, remoteUrl):
        if not os.path.exists (self.GetLocalDir()):
            utils.showInfoMsg ('Git local repository {0} does not exist; will not look for Stream Revision.\n'.format(self.GetLocalDir()), utils.INFO_LEVEL_Interesting)
            return None, None
        guid = self.GetRevision ("HEAD")
        return guid, remoteUrl

#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
class LocalUPackRepository (LocalRepository):

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalUPackRepository
    #-------------------------------------------------------------------------------------------
    def __init__(self, name, localDir):
        LocalRepository.__init__(self, name, globalvars.REPO_UPACK, localDir)
        self.m_isPackage = True
        self.m_upackSource = None

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalUPackRepository
    #-------------------------------------------------------------------------------------------
    def GetProvenance (self):
        # Since non-part upacks still have to be handled via the list, wanted to let them all go that way.
        # However parts are expected to have a provenance, so getting it here.
        _, version, _, _ = self.m_upackSource.GetProvenance ()
        return version

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalUPackRepository
    #-------------------------------------------------------------------------------------------
    def SetUpackSource (self, upackSource):
        self.m_upackSource = upackSource

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalUPackRepository
    #-------------------------------------------------------------------------------------------
    def BdfGetRevision (self, bdf):
        return bdf.GetUpackVersion (self.m_name)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalUPackRepository
    #-------------------------------------------------------------------------------------------
    def StrategyGetRevision (self):
        return self.m_upackSource.m_version

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalUPackRepository
    #-------------------------------------------------------------------------------------------
    def GetLatestRevision (self, remoteRepo, pullToRev):
        if pullToRev and pullToRev != self.m_upackSource.m_version: 
            return pullToRev

        # If it sepcified in the strategy as an exact version then that locks it
        latestVersion = None
        packageVersion = self.m_upackSource.m_version
        if not '*' in packageVersion:
            latestVersion = packageVersion
            utils.showInfoMsg ("Version {0} of UPack {1} repo speficied in strategy; can not change\n".format(packageVersion, self.m_upackSource.m_name), utils.INFO_LEVEL_SomewhatInteresting)
        if not latestVersion:
            latestVersion = str(self.m_upackSource.GetUpkg().GetLatestVersion())
        return latestVersion if latestVersion != self.GetTipRevision() else None

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalUPackRepository
    #-------------------------------------------------------------------------------------------
    def GetTipRevision (self, commandInfoLevel=utils.INFO_LEVEL_SomewhatInteresting, useLocalNumber=False): # pylint: disable=unused-argument
        return str(self.m_upackSource.GetVersionFromProvenance())

#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
class RemoteRepository (object):
    def __init__(self, name, address, checkoutOptions, path, version=None):
        self.m_name = name
        self.m_origAddress = address
        self.m_checkoutOptions = checkoutOptions.strip() if checkoutOptions else None
        self.m_path = path
        self.m_version = versionutils.Version(version) if version else None
        self.m_gitAuthMethod = None
        self.m_gitToken = None
        self.m_resolvedAddress = None

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:RemoteRepository
    #-------------------------------------------------------------------------------------------
    def __str__(self):
        addr = self.GetOrigAddress()
        if self.m_path:
            addr = os.path.join (addr, self.m_path)
        if self.m_version:
            addr += ' (ver: {0})'.format(self.m_version)
        return addr

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:RemoteRepository
    #-------------------------------------------------------------------------------------------
    def __repr__(self):
        return "RemoteRepository ([{0}] {1})".format (self.m_name, str(self))

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:RemoteRepository
    #-------------------------------------------------------------------------------------------
    def GetOrigAddress(self):
        return self.m_origAddress

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:RemoteRepository
    #-------------------------------------------------------------------------------------------
    def GetAddress(self):
        # Wait to resolve this until needed. Most commands don't care.
        if not self.m_resolvedAddress:
            self.m_resolvedAddress = os.path.expandvars(self.m_origAddress) if '$' in self.m_origAddress else self.m_origAddress
            if '$' in self.m_resolvedAddress:
                # For unattended build it must be defined. For Developers we remove it and rely on Git Credential Manager
                if utils.isUnattendedBuild():
                    raise utils.BuildError ('Error: Could not resolve environent variable for RemoteRepository "{0}": {1}'.format(self.m_name, self.m_origAddress))
                else:
                    varRe = r'\$\{\w+\}@'
                    match = re.search (varRe, self.m_resolvedAddress)
                    while match:
                        self.m_resolvedAddress = self.m_resolvedAddress.replace (match.group(0), '')
                        match = re.search (varRe, self.m_resolvedAddress)

        return self.m_resolvedAddress

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:RemoteRepository
    #-------------------------------------------------------------------------------------------
    def CloneAndChangeAddress (self, newAddress):
        return RemoteRepository (self.m_name, newAddress, self.m_checkoutOptions, self.m_path, version=self.m_version)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:RemoteRepository
    #-------------------------------------------------------------------------------------------
    @staticmethod
    def GetServerFromUrl (fullUrl):
        urlSpl = compat.parseUrl (fullUrl)
        loc = urlSpl.netloc.lower()
        if '@' in loc:
            loc = loc.split('@')[1]
        return loc

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:RemoteRepository
    #-------------------------------------------------------------------------------------------
    def GetServerUrl (self):
        return self.GetServerFromUrl (self.GetAddress())

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:RemoteRepository
    #-------------------------------------------------------------------------------------------
    def ChooseGitAuthMethodForAzure (self):
        gitToken = None
        gitAuthMethod = None

        if not 'azure' in self.GetServerUrl().lower():
            return gitAuthMethod, gitToken

        if 'BB_GIT_SAT' in os.environ:
            gitToken = os.environ['BB_GIT_SAT']
            gitAuthMethod = GIT_PULL_SAT
            utils.showInfoMsg ('Found BB_GIT_SAT in environment; using token for git authentication\n', utils.INFO_LEVEL_RarelyUseful)
        elif 'AZURE_DEVOPS_EXT_PAT' in os.environ:
            gitToken = os.environ['AZURE_DEVOPS_EXT_PAT']
            gitAuthMethod = GIT_PULL_SAT
            utils.showInfoMsg ('Found AZURE_DEVOPS_EXT_PAT in environment; using token for git authentication\n', utils.INFO_LEVEL_RarelyUseful)
        elif 'BB_AUTH_USING_AZURE_CLI' in os.environ:
            gitToken = azurecli.GetAuthenticationHeader()
            gitAuthMethod = GIT_PULL_AZURE
            if not gitToken:
                raise utils.BuildError ('Error: Failed to get access token from Azure CLI.')
            utils.showInfoMsg ('Found BB_AUTH_USING_AZURE_CLI in environment; using Azure CLI for git authentication\n', utils.INFO_LEVEL_RarelyUseful)
        return gitAuthMethod, gitToken

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:RemoteRepository
    #-------------------------------------------------------------------------------------------
    def ChooseGitAuthMethodForGithub (self):
        gitToken = None
        gitAuthMethod = None

        if not 'github' in self.GetServerUrl().lower():
            return gitAuthMethod, gitToken

        if 'BB_GITHUB_PAT' in os.environ:
            gitToken = os.environ['BB_GITHUB_PAT']
            gitAuthMethod = GIT_PULL_GITHUB
            utils.showInfoMsg ('Found BB_GITHUB_PAT in environment; using token for git authentication\n', utils.INFO_LEVEL_RarelyUseful)
        return gitAuthMethod, gitToken

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:RemoteRepository
    #-------------------------------------------------------------------------------------------
    def ChooseGitAuthMethod (self):
        if self.m_gitAuthMethod != None:
            return self.m_gitAuthMethod, self.m_gitToken

        self.m_gitAuthMethod, self.m_gitToken = self.ChooseGitAuthMethodForAzure()
        if self.m_gitAuthMethod != None:
            return self.m_gitAuthMethod, self.m_gitToken

        self.m_gitAuthMethod, self.m_gitToken = self.ChooseGitAuthMethodForGithub()
        if self.m_gitAuthMethod != None:
            return self.m_gitAuthMethod, self.m_gitToken

        if os.name != 'nt':
            # For non-Windows, the default is to use the Git Credential Manager Core
            self.m_gitAuthMethod = GIT_PULL_CRED
            self.m_gitToken = None
            utils.showInfoMsg ('os is Posix; using Git Credential Manager for git authentication\n', utils.INFO_LEVEL_RarelyUseful)
        else:
            # For Windows developer the default is to use the Git Credential Manager Core which relies on AD
            self.m_gitAuthMethod = GIT_PULL_CRED
            self.m_gitToken = None
            utils.showInfoMsg ('os is Windows; using Git Credential Manager for git authentication\n', utils.INFO_LEVEL_RarelyUseful)

        return self.m_gitAuthMethod, self.m_gitToken

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:LocalGitRepository
    #-------------------------------------------------------------------------------------------
    def GetAuthorizedGit (self):
        # The System.AccessToken (SAT) is different than a private Personal Access Token (PAT). A SAT is ~1000 chars and
        # appears to be a JWT while a PAT is ~50 chars and appears to be a GUID.
        #
        # Azure CLI uses AZURE_DEVOPS_EXT_PAT as either SAT or PAT and produce something that looks like the SAT. 
        # For the token coming from Azure CLI, it is used with the tokenType and accessToken items from the json. This is 
        # Bearer just like the SAT from what I've seen.
        #
        # For a PAT you would use https://user:PAT@dev.azure.com... (where user can be any characters; it is not used). 
        # A PAT should also work with "Basic base64encode(:+PAT)". The colon is important. See azurecli.EncodeBasicAuthHeader().
        # It is not implemented because we have not had a use case yet.
        #
        # The System.AccessToken is used with the "bearer" and then the SAT. There is not particular value to running it
        # through the Azure CLI but it only costs time.
        #
        # Implementation from: https://github.com/microsoft/azure-pipelines-agent/issues/1601#issuecomment-394490087
        #
        # Github requires the PAT as part of the URL string:
        #    git clone https://ghp_abcd1234abcd1234abcd1234@github.com/projectName/foo

        baseCmd = ['git']
        method, pat = self.ChooseGitAuthMethod ()
        if method == GIT_PULL_AZURE:
            return baseCmd + ['-c', 'http.extraheader=AUTHORIZATION: {0}'.format(pat)]
        elif method == GIT_PULL_SAT:
            return baseCmd + ['-c', 'http.extraheader=AUTHORIZATION: bearer {0}'.format(pat)]
        elif method == GIT_PULL_GITHUB:
            return baseCmd
        elif method == GIT_PULL_CRED:
            return baseCmd
        else:
            raise utils.BuildError ('Error: Unknown GIT authentication method.')

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # Class:RemoteRepository
    #-------------------------------------------------------------------------------------------
    def Dump (self, indent):
        utils.showInfoMsg (" "*indent + "Name='{0}', Address='{1}'".format (self.m_name, self.GetOrigAddress()), utils.INFO_LEVEL_Essential)
        if globalvars.buildStrategy and None != globalvars.buildStrategy.FindRepositoryOptions(self.m_name):
            utils.showInfoMsg (", Tag='{0}'".format(globalvars.buildStrategy.FindRepositoryOptions(self.m_name).m_legacyTag), utils.INFO_LEVEL_Essential)
        utils.showInfoMsg ("\n", utils.INFO_LEVEL_Essential)

#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
class RemoteRepositoriesByType (object):

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def __init__(self):
        # Store repositories by type so that we can transition from Hg to Git and still use LKGs.
        self.m_entries = {  globalvars.REPO_HG : {}, 
                            globalvars.REPO_GIT : {}, 
                            # These were never created, but may be in the future. Easy to allow this way.
                            globalvars.REPO_UPACK : {}, 
                            globalvars.REPO_RSYNC : {}, 
                            globalvars.REPO_CVS : {}
                         }

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def AddRepo (self, remoteRepo, repoType):
        if repoType in self.m_entries:
            self.m_entries[repoType][remoteRepo.m_name.lower()] = remoteRepo
        else:
            raise utils.StrategyError ("Unknown repo type for remote repository list: {0}".format(repoType))

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetRepo (self, remoteName, repoType):
        if repoType:
            return self.m_entries[repoType].get (remoteName.lower(), None)
        else: # None means look in all lists
            remoteName = remoteName.lower()
            for entryList in self.m_entries:
                if remoteName in self.m_entries[entryList]:
                    return self.m_entries[entryList][remoteName]
        return None

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def Dump (self, indent):
        totalEntries = 0
        for entry in self.m_entries:
            totalEntries += len(entry)
        if totalEntries == 0:
            utils.showInfoMsg( " - None - \n", utils.INFO_LEVEL_Essential )
        else:
            for entry in self.m_entries:
                for repo in sorted (entry.values()):
                    repo.Dump(indent+4)

#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
class RemoteRepositoryList (object):

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def __init__(self):
        self.m_strategyEntries = {}         # The fallback list from RepositoryLists.BuildStrategy and other strategy files
        self.m_streamLocations = {}         # Repositories on this stream
        self.m_streamRevisions = RemoteRepositoriesByType()         # Repositories from other streams, locked to a GUID
        self.m_streamRevsionList = None
        self.m_cmdLineEntries = RemoteRepositoriesByType()          # If we are pulling to a BDF from command line
        self.m_cmdLineBdf = None
        self.m_strategyBdfEntries = RemoteRepositoriesByType()         # If we are pulling to a BDF from the build strategy
        self.m_strategyBdf = None
        self.m_strategyOverrides = {}       # If there is a strategy BDF we override in some situations.

    def GetStreamRevList (self): return self.m_streamRevsionList
    def GetStreamLocations (self): return self.m_streamLocations
        
    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def _parseStrategyDom (self, repos, entries):
        for repo in repos:
            name = repo.getAttribute("Name")
            if "" == name:
                raise utils.StrategyError ("No Name given for RemoteRepository")

            address = repo.getAttribute("Address")
            if "" == address:
                raise utils.StrategyError ("No Address given for RemoteRepository " + name)

            # https://myorg@dev.azure.com/myorg/projectName/_git/foo
            if 'dev.azure.com' in address.lower() and not internal.BENTLEY_LFS_USER in address.lower():
                utils.ShowAndDeferMessage("Missing '{}' in RemoteRepository {} ({}). Please make it '{}'\n".format(internal.BENTLEY_LFS_USER, name, address, internal.BENTLEY_AZURE_LFS), utils.INFO_LEVEL_Essential, utils.YELLOW)

            entries[name.lower()] = RemoteRepository (name, address, repo.getAttribute("CheckoutOptions"), repo.getAttribute("Path"), version=repo.getAttribute("Version"))

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def AddStrategyEntries (self, dom):
        if [] != utils.getDomElementsByName(dom, "RemoteRepositoryList"):
            raise utils.StrategyError ("RemoteRepositoryList has been deprecated. You should remove the RemoteRepositoryList elements and leave the RemoteRepository elements at the root.")
        # This should always overwrite so last in wins.
        repos = utils.getDomElementsByName(dom, "RemoteRepository")
        self._parseStrategyDom (repos, self.m_strategyEntries)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def AddPackageEntry (self, remoteRepo):
        # For now I'm just adding these to the strategyEntries (RepositoryList.BuildStrategy) rather than making it separate. I think it should sort correctly.
        if not remoteRepo.m_name.lower() in self.m_strategyEntries:
            self.m_strategyEntries[remoteRepo.m_name.lower()] = remoteRepo

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def AddEntriesFromConfig (self):
        # Get the config file
        self.m_streamRevsionList = bbconfig.GetPrimaryBBConfig()
        for repo in self.m_streamRevsionList.GetAllRepoNames ():
            self.m_streamRevisions.AddRepo (RemoteRepository (repo, self.m_streamRevsionList.GetUrl(repo.lower()), None, None), globalvars.REPO_HG)
        for repo in self.m_streamRevsionList.GetAllGitRepoNames():
            self.m_streamRevisions.AddRepo (RemoteRepository (repo, self.m_streamRevsionList.GetGitUrl(repo.lower()), None, None), globalvars.REPO_GIT)
        for repo in self.m_streamRevsionList.GetAllCvsNames():
            self.m_streamRevisions.AddRepo (RemoteRepository (repo, self.m_streamRevsionList.GetCvsUrl(repo.lower()), '-r'+self.m_streamRevsionList.GetCvsTag(repo.lower()), self.m_streamRevsionList.GetCvsPath(repo.lower())), globalvars.REPO_CVS)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    # def AddTeamEntries (self, streamName, buildStrategy):  # TODO - change to bbconfig
    #     return self.AddTeamEntriesFromStream (streamName, buildStrategy, getLocalStreamLocFile(streamName), getLocalStreamRevFile(streamName))

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def UpdateTeamRepositoryList (self, _streamName):
        self.m_streamLocations = {}
        self.m_streamRevisions = RemoteRepositoriesByType()
        self.m_streamRevsionList = None

        self.AddEntriesFromConfig ()

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def ClearTeamRepositoryList (self):
        self.m_streamRevsionList = None

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def BaseOnStrategy (self, other):
        self.m_strategyEntries = other.m_strategyEntries

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def AddCmdLineBdf (self, bdf):
        if not bdf:
            self.m_cmdLineBdf = None
            self.m_cmdLineEntries = RemoteRepositoriesByType()
        else:
            self.m_cmdLineBdf = bdf
            for repo in self.m_cmdLineBdf.GetAllRepoNames ():
                self.m_cmdLineEntries.AddRepo (RemoteRepository (repo, self.m_cmdLineBdf.GetUrl(repo.lower()), None, None), globalvars.REPO_HG)
            for repo in self.m_cmdLineBdf.GetAllGitRepoNames():
                self.m_cmdLineEntries.AddRepo (RemoteRepository (repo, self.m_cmdLineBdf.GetGitUrl(repo.lower()), None, None), globalvars.REPO_GIT)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def AddStrategyBdf (self, bdf, ignoreList):
        self.m_strategyBdfEntries = RemoteRepositoriesByType()
        self.m_strategyOverrides = {} 
        if not bdf:
            self.m_strategyBdf = None
        else:
            self.m_strategyBdf = bdf

            if ignoreList:
                for repo in ignoreList:
                    if self.m_strategyBdf.HasName(repo):
                        self.m_strategyBdf.RemoveEntry(repo)

            for repoName in self.m_strategyBdf.GetAllRepoNames ():
                self.m_strategyBdfEntries.AddRepo (RemoteRepository (repoName, self.m_strategyBdf.GetUrl(repoName.lower()), None, None), globalvars.REPO_HG)
                
                if not repoName in self.m_streamLocations:
                    continue
                    
                if self.m_streamLocations[repoName].GetOrigAddress().lower() != self.m_strategyBdf.GetUrl(repoName).lower():
                    # Pull the other repo to the tip
                    utils.showInfoMsg ("Shifting BDF repo {0} to the tip because it is not coming from the same URL as the bbconfig\n  {1}\n  {2}\n".format (repoName, self.m_streamLocations[repoName].GetOrigAddress(), self.m_strategyBdf.GetUrl(repoName)), utils.INFO_LEVEL_SomewhatInteresting)
                    self.m_strategyOverrides[repoName.lower()] = self.m_streamLocations[repoName]

            for repoName in self.m_strategyBdf.GetAllGitRepoNames():
                self.m_strategyBdfEntries.AddRepo (RemoteRepository (repoName, self.m_strategyBdf.GetGitUrl(repoName.lower()), None, None), globalvars.REPO_GIT)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def FindRepository (self, name, repoType):
        name = name.lower()

        # To support PullToBuildDescriptionFile in the Strategy we need a convoluted scheme.
        #   The special magic is in step 2.  If you are building using a branch, the teamLocation usually 
        #   will override the strategyBDF, so you'll end up getting the tip of the everything.  Thus we check 
        #   if you are pulling from the same location and assume you want the strategy version.
        #
        # 1. Command-line BDF (Generally will have everything in StrategyBDF)
        # 2. If strat BDF and item in TL and in strat BDF:
        #      If Items in TL and Strat have the same URL:
        #        StrategyBDF
        # 3. TeamLocations
        # 4. Strategy BDF
        # 5. TeamRevisionList
        # 6. BuildStrategies

        # First check the command line BDF
        remoteRepo = self.m_cmdLineEntries.GetRepo (name, repoType)
        if remoteRepo:
            return remoteRepo

        # In Strategy BDF but also on stream 
        if name in self.m_strategyOverrides:
            return self.m_strategyOverrides[name]

        # Stream Locations 
        if name in self.m_streamLocations:
            return self.m_streamLocations[name]

        # Strategy BDF
        remoteRepo = self.m_strategyBdfEntries.GetRepo (name, repoType)
        if remoteRepo:
            return remoteRepo
        
        # Team Revision List
        remoteRepo = self.m_streamRevisions.GetRepo (name, repoType)
        if remoteRepo:
            return remoteRepo

        # Now fall back to the RevisionLists in BuildStrategies
        if name in self.m_strategyEntries:
            return self.m_strategyEntries[name]
            
        raise utils.StrategyError ("Can't find RemoteRepository '{0}'".format(name))

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetRevision (self, localRepo, onlyCmdLine):
        # First check the specific BDF
        if self.m_cmdLineBdf and localRepo.BdfGetRevision (self.m_cmdLineBdf):
            return localRepo.BdfGetRevision (self.m_cmdLineBdf)
        if onlyCmdLine:
            return None
        if self.m_strategyBdf and localRepo.BdfGetRevision (self.m_strategyBdf) and not localRepo.m_name.lower() in self.m_strategyOverrides:
            return localRepo.BdfGetRevision (self.m_strategyBdf)
        # Fall back to TeamRevisionList
        return localRepo.BdfGetRevision (self.m_streamRevsionList) if self.m_streamRevsionList else None

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetSdkSourceRevision (self, name):
        # First check the specific BDF
        if self.m_cmdLineBdf and self.m_cmdLineBdf.HasSdkSourceName (name.lower()):
            return self.m_cmdLineBdf.GetSdkSourceVersion (name.lower())
        if self.m_strategyBdf and self.m_strategyBdf.HasSdkSourceName (name.lower()) and not name.lower() in self.m_strategyOverrides:
            return self.m_strategyBdf.GetSdkSourceVersion (name.lower())
        # TeamRevisionList currently does not support SdkSource tags.
        return None
        
    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetNugetRevision (self, name):
        # First check the specific BDF
        if self.m_cmdLineBdf:
            return self.m_cmdLineBdf.GetNugetVersionString (name.lower())
        if self.m_strategyBdf and not name.lower() in self.m_strategyOverrides:
            return self.m_strategyBdf.GetNugetVersionString (name.lower())
        # TeamRevisionList currently does not support NuGet tags.
        return None

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetUpackRevision (self, name):
        # First check the specific BDF
        if self.m_cmdLineBdf:
            return self.m_cmdLineBdf.GetUpackVersion (name)
        if self.m_strategyBdf and not name.lower() in self.m_strategyOverrides:
            return self.m_strategyBdf.GetUpackVersion (name)
        # TeamRevisionList currently does not support upack tags.
        return None

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetBundlePath (self, name):
        if self.m_cmdLineBdf:
            return self.m_cmdLineBdf.GetBundlePath (name)
        if self.m_strategyBdf:
            return self.m_strategyBdf.GetBundlePath (name)
        return None

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def Dump (self, indent):
        utils.showInfoMsg( "BDF revisions: \n", utils.INFO_LEVEL_Essential )
        self.m_cmdLineEntries.Dump(indent)

        utils.showInfoMsg( "\nStream Revision List revisions: \n", utils.INFO_LEVEL_Essential )
        self.m_streamRevisions.Dump(indent)

        utils.showInfoMsg( "\nStream Locations revisions: \n", utils.INFO_LEVEL_Essential )
        for repo in sorted (self.m_streamLocations.values()):
            repo.Dump(indent+4)

        utils.showInfoMsg( "\nStrategy revisions: \n", utils.INFO_LEVEL_Essential )
        for repo in sorted (self.m_strategyEntries.values()):
            repo.Dump(indent+4)

