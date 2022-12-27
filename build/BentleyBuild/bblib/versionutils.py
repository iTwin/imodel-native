#---------------------------------------------------------------------------------------------
#  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
#  See LICENSE.md in the repository root for full copyright notice.
#---------------------------------------------------------------------------------------------
import re, os
from . import utils

#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
class Version ():
    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def __init__ (self, verString):
        if verString:
            self.m_verQuad = self.ParseString(verString)
        else:
            self.m_unbound = False
            self.m_verQuad = [None, None, None, None]

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def __str__ (self):
        verString = ''
        for ver in self.m_verQuad:
            if None == ver:
                if len(verString) > 0:
                    verString += '.'
                verString = verString + '*'
                break
            if len(verString) > 0:
                verString += '.'
            verString += str(ver)
        return verString

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def __repr__ (self):
        return str(self)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def __hash__ (self):
        return hash(repr(self))

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    # Methods for comparison and sorting.
    def __lt__(self, other): return self._Compare (other) < 0
    def __eq__(self, other): return self._Compare (other) == 0
    def __ne__(self, other): return self._Compare (other) != 0
    def __gt__(self, other): return self._Compare (other) > 0
    def __le__(self, other): return self._Compare (other) <= 0
    def __ge__(self, other): return self._Compare (other) >= 0

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def _Compare (self, other):
        if not other:
            return 1
        # -1 if self < other, 0 if self = other, +1 if self > other
        # This is for sorting full versions (via __cmp__) pulled from rsync. 
        # For partial versions 1.2.3 is considered < 1.2.3.1, but 3.4 > 1.2.3.4
        for iVer in range(len (self.m_verQuad)):
            if self.m_verQuad[iVer] == None and  other.m_verQuad[iVer] == None:
                return 0  # If we get to a point where the tuples end then they are equal
            elif self.m_verQuad[iVer] == None:
                return -1
            elif other.m_verQuad[iVer] == None:
                return 1
            elif self.m_verQuad[iVer] < other.m_verQuad[iVer]:
                return -1
            elif self.m_verQuad[iVer] > other.m_verQuad[iVer]:
                return 1
        return 0

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    @staticmethod
    def InitFromList (verTuple):
        ver = Version (None)
        ver.FromList (verTuple)
        return ver

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def ParseString (self, verString):
        if verString == '*':
            self.m_unbound = True
            return [None, None, None, None]
        self.m_unbound = False

        # Accept either . or - between tuples
        verString = verString.strip ().replace ('.', '-')
        digitRe = re.compile (r'^\d\d$')
        prgMatch = re.match (r'^\d{2,9}(\*|(en\d+))?$', verString)
        if prgMatch:
            verTuple = []
            matchString = verString
            eIndex = matchString.find ('e') # We clean out en123
            if eIndex > 0:
                matchString = matchString[0:eIndex]
            while len(matchString) > 0:
                if len (verTuple) == 3 and len (matchString) > 2: # Last tuple can be 2 or 3 digits.
                    if re.match (r'^\d\d\d$', matchString):
                        verTuple.append (int(matchString))
                        matchString = []
                else:
                    if matchString[0] == '*':     # 081102*
                        verTuple.append (None)
                        matchString = matchString[1:]
                    elif '*' in matchString[0:2]:     # 08115* - we strip off the * and assume they meant 081105*
                        verTuple.append (matchString[0:1])
                        matchString = matchString[2:]
                    elif digitRe.match (matchString[0:2]):
                        verTuple.append (int (matchString[0:2]))
                        matchString = matchString[2:]
                    else:
                        raise Exception ('Unparsed prg version string: "{0}"'.format (verString))
                        
        elif '-' in verString:
            verTuple = verString.split ('-')
      
            for i in range(len(verTuple)):
                if verTuple[i] == '*': # * is used in LKGVersion_ stuff (1-2-3-*).
                    verTuple[i] = None
                elif '*' in verTuple[i]: # 1-2-3* or 1-2-3-4*. At this point we treat it like they meant -* and strip off the *
                    verTuple[i] = int(verTuple[i].replace ('*', ''))
                elif verTuple[i].isdigit():
                    verTuple[i] = int (verTuple[i])
                else:
                    return verTuple
        else:
            raise Exception ('Unparsed version string: "{0}"'.format (verString))
        while len (verTuple) < 4:
            verTuple.append (None)
        return verTuple

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def FromList (self, verTuple):
        self.m_verQuad = verTuple[:]

        while len (self.m_verQuad) < 4:
            self.m_verQuad.append (None)
        for i in range(4):
            if self.m_verQuad[i] == '*':
                self.m_verQuad[i] = None
            elif isinstance(self.m_verQuad[i], str) and self.m_verQuad[i].isdigit():
                self.m_verQuad[i] = int(self.m_verQuad[i])

        return verTuple

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def StringForFilename (self):
        return str(self).replace ('.', '-')

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def StringForAzureBuildArtifact (self):
        verString = ''
        for ver in self.m_verQuad:
            if len(verString) > 0:
                verString += '.'
            if ver == None:
                verString += '*'
            elif isinstance(ver, int):
                verString += '{0:02d}'.format(ver)
            else:
                verString += str(ver)
        return verString

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def AsList (self):
        verList = str(self).split ('.')
        while len(verList) < 4:
            verList.append('*')
        return verList

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def MatchString (self, verStringToMatch):
        vToMatch = self.ParseString (verStringToMatch)
        return self.MatchTuple (vToMatch)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def MatchVersion (self, verToMatch):
        return self.MatchTuple (verToMatch.m_verQuad)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def MatchTuple (self, verTupleToMatch):
        # If this one is short it will match a longer one, but the opposite is not true;
        #  1.2 does not match 1.2.3 if that is what we are looking for.
        if len (verTupleToMatch) < len (self.m_verQuad):
            return False
        
        for iVer, ver in enumerate(self.m_verQuad):
            if ver == None: 
                break
            if ver != verTupleToMatch[iVer]:
                return False
                
        return True
        
    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def MatchVersionPadded (self, other):
        # We want 6.0.0 to match 6.0.0.0 but not 6.0.0.4
        v1 = self.InitFromList (self.m_verQuad)
        v2 = self.InitFromList (other.m_verQuad)
        
        for iVer, ver in enumerate(v1.m_verQuad):
            if ver == None and v2.m_verQuad[iVer] == 0:
                v1.m_verQuad[iVer] = 0
            elif ver == 0 and v2.m_verQuad[iVer] == None:
                v2.m_verQuad[iVer] = 0
        
        return v1.MatchTuple (v2.m_verQuad)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def NumValues (self):
        count = 0
        for iVal in range(len(self.m_verQuad)):
            if self.m_verQuad[iVal] != None:
                count += 1

        return count

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetFilledInWildcardVersion (self, fallbackVersion):
        wildcardList = self.AsList()
        fallbackList = fallbackVersion.AsList()
        filledList = [None, None, None, None]

        for i in range (4):
            filledList[i] = fallbackList[i] if wildcardList[i] == '*' else wildcardList[i]

        return Version.InitFromList (filledList)
        
#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
class VersionWithSuffix(Version):
    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def __init__(self, verString):
        self.m_str = verString
        self.m_noSuffix, self.m_suffix = SplitPrereleaseSuffix(verString)
        self.m_prerelaseSuffix, self.m_buildSuffix = self.SplitSuffix(self.m_suffix)
        self.m_parent = Version(self.m_noSuffix)
        utils.showInfoMsg ('VersionWithSuffix "{0}" suffix "{1}"\n'.format (repr(self.m_parent), self.m_suffix), utils.INFO_LEVEL_TemporaryDebugging)
        Version.__init__(self, self.m_noSuffix)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def __str__(self):
        return self.m_str

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def __repr__(self):
        reprString = 'VersionWithSuffix: parent: ' + repr(self.m_parent) + ' suffix: '
        if self.m_suffix:
            reprString += self.m_suffix
        else:
            reprString += 'None'
        return reprString

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def MatchVersionPadded(self, other):
        parentMatch = Version.MatchVersionPadded(self.m_parent, other.m_parent)
        if parentMatch:
            if self.m_suffix == other.m_suffix:
                return True
        return False

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    # wrapper methods for getting members
    def Str(self): return self.m_str
    def NoSuffix(self): return self.m_noSuffix
    def Suffix(self): return self.m_suffix
    def Parent(self): return self.m_parent

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    # Methods for comparison and sorting.
    def __lt__(self, other): return self._Compare (other) < 0
    def __eq__(self, other): return self._Compare (other) == 0
    def __ne__(self, other): return self._Compare (other) != 0
    def __gt__(self, other): return self._Compare (other) > 0
    def __le__(self, other): return self._Compare (other) <= 0
    def __ge__(self, other): return self._Compare (other) >= 0

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def __hash__(self):
        return hash(self.m_str)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def ComparePreleaseSuffix (self, other):
        # From Semver: Precedence for two pre-release versions with the same major, minor, and patch version MUST be 
        # determined by comparing each dot separated identifier from left to right until a difference is found as follows: 
        # identifiers consisting of only digits are compared numerically and identifiers with letters or hyphens are 
        # compared lexically in ASCII sort order. Numeric identifiers always have lower precedence than non-numeric identifiers. 
        # A larger set of pre-release fields has a higher precedence than a smaller set, if all of the preceding identifiers are equal.
        
        isNumRe = re.compile(r'^[0-9]+$')
        def splitSuffix (suffix):
            return [int(item) if isNumRe.match(item) else item for item in suffix.split('.')]
            
        def cmpVal (a, b): # cmp is gone in py3; this is the suggested route.
            return (a > b) - (a < b)
            
        def compareSubs (subA, subB):
            if isinstance(subA, int) and isinstance(subB, int):
                return cmpVal(subA, subB)
            elif isinstance(subA, int):
                return -1
            elif isinstance(subB, int):
                return 1
            else:
                return cmpVal(subA, subB)
            
        selfList = splitSuffix (self.m_prerelaseSuffix if self.m_prerelaseSuffix else '')
        otherList = splitSuffix (other.m_prerelaseSuffix if other.m_prerelaseSuffix else '')
        
        for selfSub, otherSub in zip(selfList, otherList):
            comp = compareSubs(selfSub, otherSub)
            if comp != 0:
                return comp

        return compareSubs(len(selfList), len(otherList))

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def CompareSuffix (self, other):
        if not self.m_prerelaseSuffix and other.m_prerelaseSuffix:  # no suffix sorts higher
            return 1
        elif  self.m_prerelaseSuffix and not other.m_prerelaseSuffix:
            return -1
        elif self.m_prerelaseSuffix and other.m_prerelaseSuffix:
            cmpVal = self.ComparePreleaseSuffix (other)
            if cmpVal != 0:
                return cmpVal
                
        # Technically Build suffixes should be the same, but realistically we have to sor them somehow.
        if self.m_buildSuffix == other.m_buildSuffix:
            return 0
        elif not self.m_buildSuffix and other.m_buildSuffix: # no suffix sorts higher
            return 1
        elif self.m_buildSuffix and not other.m_buildSuffix: # any suffix sorts lower (i.e., alpha, beta)
            return -1
        elif self.m_buildSuffix > other.m_buildSuffix:
            return 1
        elif self.m_buildSuffix < other.m_buildSuffix:
            return -1

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def _Compare(self, other):
        parentCmp = Version._Compare(self.m_parent, other.m_parent)
        if parentCmp > 0:
            return 1
        elif parentCmp < 0:
            return -1
        elif parentCmp == 0:
            return self.CompareSuffix (other)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def SplitSuffix (self, suffix):
        if not suffix:
            return None, None
        # Part of regex from semver.py
        suffixRe = re.compile(r'^(\-(?P<prerelease>(?:0|[1-9A-Za-z-][0-9A-Za-z-]*)(\.(?:0|[1-9A-Za-z-][0-9A-Za-z-]*))*))?(\+(?P<build>[0-9A-Za-z-]+(\.[0-9A-Za-z-]+)*))?$', re.VERBOSE)
        match = suffixRe.match(suffix)
        if match:
            return match.group('prerelease'), match.group('build')
        return None, None

#-------------------------------------------------------------------------------------------
# This class is so that we can pass either a Version or just a simple Glob filter.
#
# bsimethod
#-------------------------------------------------------------------------------------------
class GlobMatcher (object):
    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def __init__ (self, pattern):
        self.m_pattern = pattern

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def MatchString (self, stringToMatch):
        return utils.NameMatch (stringToMatch, self.m_pattern)

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def SplitPrereleaseSuffix(versionStr):
    if not versionStr:
        return None, None
    suffix = None
    # exclude non-numeric suffix after final dash
    versionWithSuffixPattern = r"^([\[\(]*)([0-9,\.\-\*]*)(\-[a-zA-Z]*[^\]^\)]*)([\]\)]*)"
    versionMatch = re.match(versionWithSuffixPattern, versionStr)
    if versionMatch:
        if versionMatch.group(3): # has alphabetic suffix (usually -alpha or -beta) which we remove for comparison
            utils.showInfoMsg ("SplitPrereleaseSuffix splitting {0} from {1}\n".format (versionMatch.group(3), versionStr), utils.INFO_LEVEL_TemporaryDebugging)
            versionStr = versionMatch.group(1) + versionMatch.group(2) + versionMatch.group(4)
            suffix = versionMatch.group(3)
    # exclude non-numeric build metadata after final plus-sign
    versionStr, suffix = SplitBuildMetadata(versionStr, suffix)
    utils.showInfoMsg ("SplitPrereleaseSuffix returning version {} suffix {}\n".format (versionStr, suffix), utils.INFO_LEVEL_TemporaryDebugging)
    return versionStr, suffix

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def SplitBuildMetadataFrom(versionStr):
    metadata = None
    if not versionStr:
        return versionStr, metadata
    # exclude non-numeric build metadata after final plus-sign in versionStr
    if '+' in versionStr:
        versionList = versionStr.split('+')
        versionStr = versionList[0]
        if len(versionList) > 1:
            metadata = '+' + '+'.join(versionList[1:])
            utils.showInfoMsg ("SplitBuildMetadataFrom ignoring semver 2.0 build metadata {} in {}\n".format (metadata, versionStr), utils.INFO_LEVEL_TemporaryDebugging)
    return versionStr, metadata

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def SplitBuildMetadata(versionStr, suffix):
    # exclude and IGNORE non-numeric build metadata after final plus-sign in versionStr OR suffix
    versionStr, _ = SplitBuildMetadataFrom(versionStr)
    suffix, _     = SplitBuildMetadataFrom(suffix)
    utils.showInfoMsg ("SplitBuildMetadata returning version {} suffix {}\n".format (versionStr, suffix), utils.INFO_LEVEL_TemporaryDebugging)
    return versionStr, suffix
    
#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def RemoveRangePunctuation(versionStr):
    if not versionStr:
        return None, None
    versionPunctPattern = r"^([\[\(]*)([0-9,\.\-\*]*)([\]\)]*)"
    versionPunctMatch = re.match(versionPunctPattern, versionStr)
    if versionPunctMatch:
        if versionPunctMatch.group(2): # has heart of versionStr without range prefix or suffix
            versionRangePattern = r"^([^,]*)(,)(.*)"
            versionRangeMatch = re.match(versionRangePattern, versionPunctMatch.group(2))
            if versionRangeMatch:
                utils.showInfoMsg ("RemoveRangePunctuation returning {0},{2} from {3}\n".format (versionRangeMatch.group(1), versionRangeMatch.group(3), versionStr), utils.INFO_LEVEL_TemporaryDebugging)
                return versionRangeMatch.group(1), versionRangeMatch.group(3)
            else:
                return versionPunctMatch.group(1), versionPunctMatch.group(1)
        else:
            return versionStr, versionStr
    else:
        return None, None

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def ConvertVersionToSemver (verString):
    # Convert our 4-part version to a semver (3-part). We decided to use 1.2.3-4 for the cleanest appearance.
	# The Universal Package server doesn't surppot +.
    SemVerRegex = re.compile( # regex from semver python package
        r"""
        ^
        (?P<major>(?:0|[1-9][0-9]*))
        \.
        (?P<minor>(?:0|[1-9][0-9]*))
        \.
        (?P<patch>(?:0|[1-9][0-9]*))
        (\-(?P<prerelease>
            (?:0|[1-9A-Za-z-][0-9A-Za-z-]*)
            (\.(?:0|[1-9A-Za-z-][0-9A-Za-z-]*))*
        ))?
        (\+(?P<build>
            [0-9A-Za-z-]+
            (\.[0-9A-Za-z-]+)*
        ))?
        $
        """, re.VERBOSE)

    if SemVerRegex.match(verString):
        return verString

    verList = Version (verString).AsList()
    semVer = '{0}.{1}.{2}'.format (verList[0], verList[1], verList[2])
    if len(verList) > 3:
        semVer = '{0}-{1}'.format(semVer, verList[3])
    return semVer

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def testVersionClass():
    import sys
    def OutputMessage (msg):
        sys.stdout.write (msg + '\n') # Just avoid using print for Lint reasons.

    v1 = Version ('1.2.3.4')
    
    if str(v1) != '1.2.3.4':
        OutputMessage ('Failed string full tuple: {0}'.format(v1))
    
    if v1.StringForFilename() != '1-2-3-4':
        OutputMessage ('Failed filename string full tuple: {0}'.format(v1.StringForFilename()))
    
    match1 = v1.MatchString ('1-2-3-4')
    if not match1: OutputMessage ('should have matched by string but did NOT for full tuple with dashes')

    match1 = v1.MatchString ('1.2.3.4')
    if not match1: OutputMessage ('should have matched by string but did NOT for full tuple with dots')

    match1 = v1.MatchString ('3.2.1.17')
    if match1: OutputMessage ('should NOT have matched by string but did for full tuple')
    
    match1 = v1.MatchString ('1.2.3')
    if match1: OutputMessage ('should NOT have matched by string but did for partial test tuple')
    
    v1 = Version ('1-2-3')
    if str(v1) != '1.2.3':
        OutputMessage ('Failed string partial tuple: {0}'.format(v1))
    if v1.StringForFilename() != '1-2-3':
        OutputMessage ('Failed filename string partial tuple: {0}'.format(v1.StringForFilename()))

    match1 = v1.MatchString ('1-2-3-4')
    if not match1: OutputMessage ('should have matched by string but did NOT for patrial tuple with dashes')

    match1 = v1.MatchString ('1.2.3.4')
    if not match1: OutputMessage ('should have matched by string but did NOT for patrial tuple')

    match1 = v1.MatchString ('3.2.1.17')
    if match1: OutputMessage ('should NOT have matched by string but did for patrial tuple')
    
    v1 = Version.InitFromList ([1,2,3])

    match1 = v1.MatchTuple ([1,2,3,4])
    if not match1: OutputMessage ('should have matched by tuple but did NOT for patrial tuple with dashes')

    match1 = v1.MatchTuple ([1,2,3,4])
    if not match1: OutputMessage ('should have matched by tuple but did NOT for patrial tuple')

    match1 = v1.MatchTuple ([3,2,1,17])
    if match1: OutputMessage ('should NOT have matched by tuple but did for patrial tuple')
    
    match1 = v1.MatchTuple ([1,2])
    if  match1: OutputMessage ('should have matched by tuple but did NOT for patrial test tuple')

    match1 = v1.MatchVersionPadded (Version('1.2.3.0'))
    if  not match1: OutputMessage ('should have matched by tuple but did NOT for padded test tuple')

    match1 = v1.MatchVersionPadded (Version('1.2.3.1'))
    if   match1: OutputMessage ('should NOT have matched by tuple but did  for padded test tuple')

    verList = []
    verList.append (Version('9.8.7.6'))
    verList.append (Version('1.2.3.4'))
    verList.append (Version('8.7.6.5'))
    verList.append (Version('1.3.3.4'))
    verList.append (Version('7.6.5.4'))
    verList.append (Version('1.3.3.4'))
    verList.append (Version('1.3.4.4'))
    verList.append (Version('1.3.4.5'))
    verList.append (Version('1.3.4.5'))
    verList.append (Version('1.13.14.15'))
    
    verList.sort()
    for item in verList:
        OutputMessage ('verList: ' + str(item))

    vp = Version ('01020304') # 2-digit build number
    if str(vp) != '1.2.3.4':
        OutputMessage ('Failed prg string full tuple: {0}'.format(vp))

    vpe = Version ('010203040') # 3-digit build number
    if str(vpe) != '1.2.3.40':
        OutputMessage ('Failed prg string full tuple 3-digit build: {0}'.format(vpe))

    vp = Version ('01020304en1') # 2-digit build number
    if str(vp) != '1.2.3.4':
        OutputMessage ('Failed prg string full tuple en1: {0}'.format(vp))

    vp = Version ('010203*') # 2-digit build number
    if str(vp) != '1.2.3':
        OutputMessage ('Failed prg string full tuple en1: {0}'.format(vp))

    vp = Version ('0102035*') # 2-digit build number
    if str(vp) != '1.2.3.5':
        OutputMessage ('Failed prg string full tuple en1: {0}'.format(vp))

    vp = Version ('01023*') # 2-digit build number
    if str(vp) != '1.2.3':
        OutputMessage ('Failed prg string full tuple en1: {0}'.format(vp))

    vp = Version ('010203en1') # 2-digit build number
    if str(vp) != '1.2.3':
        OutputMessage ('Failed prg string full tuple en1: {0}'.format(vp))

    v0 = Version ('*') 
    if str(v0) != '':
        OutputMessage ('Failed for Any Version: {0}'.format(v0))

    try:
        vp = Version ('0102030508')
        OutputMessage ('Failed by accepting an invalid version string')
    except:
        pass

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def pushPartStrategyVersionToEnv (env, part):
    if not 'BB_TEST_STRATEGY_VERSION' in os.environ:
        return

    if not "REL_V" in env:
        utils.appendEnvVar (env, "REL_V", "99")
    if not "MAJ_V" in env:
        utils.appendEnvVar (env, "MAJ_V", "99")
    if not "MIN_V" in env:
        utils.appendEnvVar (env, "MIN_V", "99")
    if not "SUBMIN_V" in env:
        utils.appendEnvVar (env, "SUBMIN_V", "99")

    if not part.m_version:
        return

    versionList = part.m_version.AsList()

    if versionList[0] != "*":
        utils.appendEnvVar (env, "REL_V", versionList[0])
    if versionList[1] != "*":
        utils.appendEnvVar (env, "MAJ_V", versionList[1])
    if versionList[2] != "*":
        utils.appendEnvVar (env, "MIN_V", versionList[2])
    if versionList[3] != "*":
        utils.appendEnvVar (env, "SUBMIN_V", versionList[3])

if __name__ == '__main__':
    testVersionClass()
