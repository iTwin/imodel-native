#---------------------------------------------------------------------------------------------
#  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
#  See LICENSE.md in the repository root for full copyright notice.
#---------------------------------------------------------------------------------------------
import argparse, datetime, difflib, fnmatch, glob, os, re, shutil, stat, sys, threading, time, uuid, xml, zipfile
import xml.dom.minidom as minidom
from . import builddescriptionfile, buildpaths, compat, globalvars, internal, symlinks
from contextlib import contextmanager

INFO_LEVEL_None                = 0
INFO_LEVEL_Essential           = 1
INFO_LEVEL_Important           = 2
INFO_LEVEL_VeryInteresting     = 3
INFO_LEVEL_Interesting         = 4
INFO_LEVEL_SomewhatInteresting = 5
INFO_LEVEL_RarelyUseful        = 6
INFO_LEVEL_TemporaryDebugging  = 7

if os.name == 'nt':
    from . import colorconsole
    BLACK           = colorconsole.BLACK
    GREY            = 0x0008
    DARK_BLUE       = 0x0001
    BLUE            = 0x0009
    DARK_GREEN      = 0x0002
    GREEN           = 0x000A
    TURQUOISE       = 0x0003
    LIGHT_BLUE      = 0x000B
    DARK_RED        = 0x0004
    RED             = 0x000C
    MAGENTA         = 0x0005
    PINK            = 0x000D
    MUSTARD         = 0x0006
    YELLOW          = 0x000E
    LIGHT_GREY      = 0x0007
    WHITE           = colorconsole.WHITE
    UNCHANGED       = colorconsole.UNCHANGED
else:
    BLACK           = '\033[90m'
    GREY            = '\033[97m'
    DARK_BLUE       = '\033[94m'
    BLUE            = '\033[94m'
    DARK_GREEN      = '\033[92m'
    GREEN           = '\033[92m'
    TURQUOISE       = '\033[96m'
    LIGHT_BLUE      = '\033[94m'
    DARK_RED        = '\033[91m'
    RED             = '\033[91m'
    MAGENTA         = '\033[95m'
    PINK            = '\033[95m'
    MUSTARD         = '\033[93m'
    YELLOW          = '\033[93m'
    LIGHT_GREY      = '\033[97m'
    WHITE           = '\033[90m'
    UNCHANGED       = 0x0

#    Black      0:30       Dark Gray    1;30
#    Red        0;31       Bold Red     1;31
#    Green      0;32       Bold Green   1;32
#    Yellow     0;33       Bold Yellow  1;33
#    Blue       0;34       Bold Blue    1;34
#    Purple     0;35       Bold Purple  1;35
#    Cyan       0;36       Bold Cyan    1;36
#    Light Gray 0;37       White        1;37


g_verbosity = INFO_LEVEL_VeryInteresting

g_lineWriter = None # Instantiated after the LineWriter class

g_threadLocalStorage = threading.local()

g_pullDate = -1  # Use GetPullTime

g_retryInfo = []

s_hasXmlParseErrors = False     # If we have parsing errors, wait until all files are parsed and then exit.

s_variableExpanderStrategy = None  # This is used while the strategy is loading.

py3 = (int(sys.version[0]) > 2)

#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
class BentleyBuildXmlElement (minidom.Element):
    def getAttribute (self, attname):
        # Currently internal stuff is expanded as it is read from the XML file. For the env,
        # stuff is expanded later. I tried expanding everything here and the toolcache stuff
        # is currently an impediment.
        attrValue = minidom.orig_Element.getAttribute (self, attname)  
        if globalvars.buildStrategy:
            return globalvars.buildStrategy.ExpandInternalVariables (attrValue)
        elif s_variableExpanderStrategy: # Used during strategy load
            return s_variableExpanderStrategy.ExpandInternalVariables (attrValue)
        return attrValue

# Override a few minidom classes with our subclasses
minidom.orig_Element = minidom.Element
minidom.Element = BentleyBuildXmlElement

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def parseXml (xmlfile):
    # Always validate
    validateXml (xmlfile)
    return minidom.parse (xmlfile)

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def setExpanderStrategy (strat):
    global s_variableExpanderStrategy
    s_variableExpanderStrategy = strat

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def validateXml(xmlFile):
    # Skip if host OS is Linux (known bad lxml integration)
#    if 'linux2' == sys.platform or 'linux' == sys.platform:
#        showInfoMsg("Skipping XML validation due to host platform: {0}\n".format(xmlFile), INFO_LEVEL_SomewhatInteresting)
#        return None

    # Skip if coming from LKG's or packages.  Nothing we can do anyway.
    # It would be nice to refine it a little to validate packages coming from source (symlinks)
    if '\\LastKnownGood\\' in xmlFile or '\\upack\\' in xmlFile or '\\nuget\\' in xmlFile:
        return None
    
    # First dig out the XSD; do this using just file reading.
    with open (xmlFile, 'rb') as curFile:
        allLines = curFile.readlines()

    schemaRe = re.compile ('xsi:noNamespaceSchemaLocation="(.*?)"')
    schemaFile = None

    for line in allLines:
        line = str(line)
        match = schemaRe.search (line)
        if match:
            schemaFileBase = match.group(1)
            schemaFile = os.path.join (os.path.dirname(xmlFile), schemaFileBase)
            schemaFile = os.path.normpath (schemaFile)
            break

    if not schemaFile:
        if not ('languageSpecifications.xml' in xmlFile or 'treeConfiguration.xml' in xmlFile or xmlFile.endswith('_provenance.log')):
            showInfoMsg ("Skipping XML validation because there is no schema: {0}\n".format(xmlFile), INFO_LEVEL_SomewhatInteresting)
        return None

    xsdPath = buildpaths.GetXsdPath()
    bbSchemaFile = os.path.join(xsdPath, os.path.basename(schemaFile))

    if isBsi() and not schemaFile.lower().startswith(xsdPath.lower()) and not (globalvars.programOptions.outRootDirectory and xmlFile.lower().startswith(globalvars.programOptions.outRootDirectory.lower())):
        relSchemaFile = os.path.relpath (bbSchemaFile, os.path.dirname(xmlFile)).replace ('\\', '/')
        fullMsg = 'The noNamespaceSchemaLocation in {0} points to the incorrect file which breaks editor help and validation.\nPlease change it to: {1}\n'.format(xmlFile, relSchemaFile)
        ShowAndDeferMessage(fullMsg, INFO_LEVEL_Essential, YELLOW)

    if os.path.exists(bbSchemaFile):
        schemaFile = bbSchemaFile
    elif not os.path.exists(schemaFile):
        showInfoMsg ("Skipping XML validation because there the schema file could not be found: {0}\n".format(schemaFile), INFO_LEVEL_SomewhatInteresting)
        return None

    from lxml import etree
    global s_hasXmlParseErrors
    with open(schemaFile, 'rb') as schema:
        try:
            xmlSchema = etree.XMLSchema(etree.parse(schema))
        except Exception as schemaErrors:
            fullMsg = 'Error parsing XSD file {0}\n{1}\n'.format(schemaFile, schemaErrors)
            ShowAndDeferMessage(fullMsg, INFO_LEVEL_Essential, YELLOW)
            return None

    etree.XMLParser(schema=xmlSchema)
    with open(xmlFile, 'rb') as curfile:
        try:
            doc = etree.parse(curfile)
            xmlSchema.assertValid(doc)
        except etree.DocumentInvalid as xmlErrors:
            fullMsg = 'Error checking XML file {0}\n   against schema {1}\n{2}\n'.format(xmlFile, schemaFile, xmlErrors.error_log) # pylint: disable=no-member
            ShowAndDeferMessage(fullMsg, INFO_LEVEL_Essential, YELLOW)
            s_hasXmlParseErrors = True
        except Exception as error:
            fullMsg = 'Exception checking XML file {0}\n   against schema {1}\n{2}\n'.format(xmlFile, schemaFile, error)
            ShowAndDeferMessage(fullMsg, INFO_LEVEL_Essential, YELLOW)
            s_hasXmlParseErrors = True

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def HasXmlParseErrors ():
    if globalvars.programOptions.ignoreXmlErrors:
        return False
    return s_hasXmlParseErrors

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def isPrg ():
    if "PRG" in [var.upper() for var in os.environ]:
        return True
    return False

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def isSubnetPrg ():
    return "SUBNET_PRG" in os.environ

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def isBuildAgent ():
    return 'TF_BUILD' in os.environ

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def isUnattendedBuild ():
    return isSubnetPrg() or isBuildAgent()

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def isBsi ():
    return 'BSI' in os.environ

#-------------------------------------------------------------------------------------------
#-------------------------------------------------------------------------------------------
g_overrideSubnetPrg = False
g_wasOverrideSubnetPrgChecked = False

#-------------------------------------------------------------------------------------------
# After all these changes and checking subnet_PRG and PRG etc. I think I should
# just use this method to see if the PRG keys exist and use them if they are available.
# That is basically what this is doing, but only if it detects PRG* or SUBNET_PRG.
#
#-------------------------------------------------------------------------------------------
def hasPrgSigningKeys ():
    global g_wasOverrideSubnetPrgChecked
    global g_overrideSubnetPrg
    if not g_wasOverrideSubnetPrgChecked:
        g_wasOverrideSubnetPrgChecked = True  # Only warn about this once per build.

        rightsCompliantPath = getPrivateKeyFilePath (StrongNameSignatureType.PRG_RIGHTS_COMPLIANT)
        nonrightsCompliantPath = getPrivateKeyFilePath (StrongNameSignatureType.PRG_NON_RIGHTS_COMPLIANT)
        prgKeysExist = os.path.exists (nonrightsCompliantPath) and os.path.exists (rightsCompliantPath)

        msg = None
        if isSubnetPrg():
            msg = "\n'SUBNET_PRG' was found in environment."
        elif isPrg ():
            msg = "\n'PRG' was found in environment but 'SUBNET_PRG' was not."

        if msg != None:
            if prgKeysExist:
                msg += "\n{0} and {1} were found.".format (rightsCompliantPath, nonrightsCompliantPath)
                msg += "\nTreating this build as a PRG build for transkits."
            else:
                msg += "\n{0} and {1} were not found.".format (rightsCompliantPath, nonrightsCompliantPath)
                msg += "\nTreating this build as a non-PRG build for transkits."
            g_overrideSubnetPrg = prgKeysExist
            msg += "\nThis will affect which strong name keys are used during the build.\n "
            showInfoMsg (msg, INFO_LEVEL_VeryInteresting, YELLOW)

    return g_overrideSubnetPrg

#-------------------------------------------------------------------------------------------
# bsienum
#-------------------------------------------------------------------------------------------
class StrongNameSignatureType(object):
    # value is the public key token of the specific signature type
    PRG_RIGHTS_COMPLIANT = "9bfed12b64a9b7df"
    PRG_NON_RIGHTS_COMPLIANT = "4bf6c96a266e58d4"

    NORMAL_RIGHTS_COMPLIANT = "94fba2293a532598"
    NORMAL_NON_RIGHTS_COMPLIANT = "b4d201f8319cef86"

    TEST = "ad7265ffc9957eb4"

    NONE = None

def getStrongNameSignatureType(publicKeyToken):
    typeArray = [StrongNameSignatureType.PRG_RIGHTS_COMPLIANT, StrongNameSignatureType.PRG_NON_RIGHTS_COMPLIANT, StrongNameSignatureType.NORMAL_RIGHTS_COMPLIANT, StrongNameSignatureType.NORMAL_NON_RIGHTS_COMPLIANT, StrongNameSignatureType.TEST]
    if publicKeyToken in typeArray:
        return publicKeyToken
    return StrongNameSignatureType.NONE

#--------------------------------------------------------------------------------
# bsimethod
#--------------------------------------------------------------------------------
def getPrivateKeyFilePath(signatureType):
    # For example, Mac systems will return None for getPrgKeyFileDirectory, which will crash os.path.join.
    if getPrgKeyFileDirectory() is None:
        return ""

    if signatureType == StrongNameSignatureType.PRG_RIGHTS_COMPLIANT:
        return os.path.join(getPrgKeyFileDirectory(), internal.SNK_PRGRC)

    if signatureType == StrongNameSignatureType.PRG_NON_RIGHTS_COMPLIANT:
        return os.path.join(getPrgKeyFileDirectory(), internal.SNK_PRGNRC)

    if signatureType == StrongNameSignatureType.NORMAL_RIGHTS_COMPLIANT:
        return os.path.join(GetBsiScriptsFile(internal.SNK_RC))

    if signatureType == StrongNameSignatureType.NORMAL_NON_RIGHTS_COMPLIANT:
        return os.path.join(GetBsiScriptsFile(internal.SNK_NRC))

    if signatureType == StrongNameSignatureType.TEST:
        return os.path.join(GetBsiScriptsFile(internal.SNK_TEST))

    return None

#--------------------------------------------------------------------------------
# bsimethod
#--------------------------------------------------------------------------------
def getPublicKeyFilePath(signatureType):

    if signatureType == StrongNameSignatureType.PRG_RIGHTS_COMPLIANT:
        return os.path.join(GetBsiScriptsFile(internal.SNK_PRGPUBRC))

    if signatureType == StrongNameSignatureType.PRG_NON_RIGHTS_COMPLIANT:
        return os.path.join(GetBsiScriptsFile(internal.SNK_PRGPUBRC))

    # Using private keyfiles for the rest since they exist in all cases not like PRG ones
    if signatureType == StrongNameSignatureType.NORMAL_RIGHTS_COMPLIANT:
        return os.path.join(GetBsiScriptsFile(internal.SNK_RC))

    if signatureType == StrongNameSignatureType.NORMAL_NON_RIGHTS_COMPLIANT:
        return os.path.join(GetBsiScriptsFile(internal.SNK_NRC))

    if signatureType == StrongNameSignatureType.TEST:
        return os.path.join(GetBsiScriptsFile(internal.SNK_TEST))

    return None

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def getPrgKeyFileDirectory ():
    if os.name == 'nt':
        import pythoncom
        from win32com.shell import shell, shellcon  # pylint: disable=import-error,no-name-in-module
        try:
            publicDocumentsDir = shell.SHGetSpecialFolderPath(0, shellcon.CSIDL_COMMON_DOCUMENTS)
        except pythoncom.com_error: # pylint: disable=no-member
            showInfoMsg("Failed to get CSIDL_COMMON_DOCUMENTS", INFO_LEVEL_Important)
            return None
        return publicDocumentsDir
    else:
        return None

#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
def getNow(): return datetime.datetime.now().strftime("%a %b %d %Y %H:%M:%S")

#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
def GetBMakeEXE(): return os.path.join (buildpaths.GetToolsOutputRoot(), 'bmake', 'bmake')

#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
def GetBentleyBuildMakeEXE(): return os.path.join (buildpaths.GetToolsOutputRoot(), 'bmake', 'bentleybuildmake')

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def IsSameFile (filePath1, filePath2):
    filePath1 = os.path.normpath (filePath1).lower ()
    filePath2 = os.path.normpath (filePath2).lower ()
    return filePath1 == filePath2

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def IsSourceModified (targetList, sourceList):
    for file2 in sourceList:
        if not os.path.exists (file2):
            raise BuildError ("Cannot find file {0}".format (file2))
        if symlinks.isSymbolicLink (file2):
            file2 = symlinks.getSymlinkTarget (file2)
        for file1 in targetList:
            if not os.path.exists (file1):
                return True
            if symlinks.isSymbolicLink (file1):
                file1 = symlinks.getSymlinkTarget (file1)
            if os.path.getmtime(file2) > os.path.getmtime(file1):
                return True
    return False

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def compareFileTimes (file1, file2):
    if symlinks.isSymbolicLink (file1):
        file1 = symlinks.getSymlinkTarget (file1)
    if symlinks.isSymbolicLink (file2):
        file2 = symlinks.getSymlinkTarget (file2)

    try:
        return  (os.path.getmtime(file2)-os.path.getmtime(file1)) < 1
    except os.error:
        return False

#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
class BBException (Exception):
    # Use this class to differentiate between bb and python exceptions coming back in threads.

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def __init__(self, message, trace=None):
        Exception.__init__(self)
        self.errmessage = message
        if trace:
            self.stackTrace = trace
        else:
            import traceback
            self.stackTrace = traceback.extract_stack()
            
    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def IsBBException (self): return True
    
    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def __str__ (self):
        return self.errmessage
        
    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def __repr__ (self):
        return 'BentleyBuild:' + self.__class__.__name__ +'("' + self.errmessage + '")'

#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
class FatalError (BBException):
    def __init__(self, errVal=1, message="", trace=None):
        assert (type(errVal) is int)
        self.errVal = errVal
        BBException.__init__ (self, message, trace)

#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
class BuildError (BBException):
    def __init__(self, message, trace=None):
        BBException.__init__ (self, message, trace)

#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
class StrategyError (BuildError):
    def __init__(self, message, trace=None):
        BuildError.__init__ (self, message, trace=trace)
        if globalvars.currentStrategyFile:
            self.errmessage = "{0}(1) error : {1}".format (globalvars.currentStrategyFile,message)
        else:
            self.errmessage = message

#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
class PartBuildError (BuildError):
    def __init__(self, message, part, domNode=None, trace=None):
        if None == part:
            BuildError.__init__(self, message, trace=trace)
        else:
            BuildError.__init__(self, message + part.fullPath() + "\n", trace=trace)
        if None != domNode and hasattr (domNode, "parse_position"):
            self.m_lineNumber = domNode.parse_position[0]

#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
class PartPullError (BuildError):
    def __init__(self, message, partinfo, domNode=None, trace=None):
        if None == partinfo:
            BuildError.__init__(self, message, trace=trace)
        else:
            BuildError.__init__(self, message + '\n' + str(partinfo), trace=trace)
        if None != domNode and hasattr (domNode, "parse_position"):
            self.m_lineNumber = domNode.parse_position[0]

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def exitOnError (errVal, message="", postErrorMsgCallback=None, trace=None):
    """Exit with status, except if 'ignoreErrors' flag is on. postErrorMsgCallback is a callback that is invoked after the error message is printed so users can print suggestions etc."""
    if not message.endswith ('\n'):
        message += '\n'

    ignoringErrors = globalvars.currentAction and globalvars.currentAction.ShouldIgnoreErrors()

    if ignoringErrors:
        showInfoMsg (message, INFO_LEVEL_Essential, RED)
        appendBuildError (message)

    if not ignoringErrors:
        ShowDeferredMessages (clear=True)

    if None != postErrorMsgCallback:
        postErrorMsgCallback (errVal, message)

    if not ignoringErrors:
        raise FatalError (errVal, message, trace=trace)

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def prependTimeStamp (baseString):
    if not 'BB_TIMESTAMP' in os.environ:
        return baseString

    # [YYYY-MM-DD HH:MM:SS, ~ThreadID]
    return '[{0}, ~{1}] {2}'.format (datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S"),  compat.getThreadId(), baseString)

#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
class LineWriter (object):

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def __init__(self, rewriteThreshold):

        self.m_rewriteThreshold = rewriteThreshold

        def writeFirstLineRewritable( message, outputLevel, textColor ):
            self.__writeText( message, outputLevel, textColor, True )
        self.m_writeRewritableLine = writeFirstLineRewritable

        def writeFirstLineNormal( message, outputLevel, textColor ):
            self.__writeText( message, outputLevel, textColor, False )
        self.m_writeNormalLine = writeFirstLineNormal

        self.m_outputStream = sys.stdout

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # "message" cannot have new lines in it for this to work.
    #-------------------------------------------------------------------------------------------
    def printRewritableLine(self, message, screenOutputLevel, _logLevel, textColor):
        message = prependTimeStamp(message)
        if self.__canRewriteLines():
            self.m_writeRewritableLine( message, screenOutputLevel, textColor )
        else:
            self.m_writeNormalLine( message + "\n", screenOutputLevel, textColor )

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def writeMsg(self, message, outputLevel, textColor=UNCHANGED):
        self.writePlainMsg( prependTimeStamp(message), outputLevel, textColor )

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def writePlainMsg(self, message, outputLevel, textColor=UNCHANGED):
        # No timestamp - called from log buffer
        self.m_writeNormalLine (message, outputLevel, textColor)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def writeError(self, message):
        self.m_outputStream = sys.stderr
        try:
            self.writeMsg( message, INFO_LEVEL_Essential, RED )
        finally:
            self.m_outputStream = sys.stdout

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def __canRewriteLines(self):
        return g_verbosity <= self.m_rewriteThreshold

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # From: http://code.activestate.com/recipes/440694/
    #-------------------------------------------------------------------------------------------
    def __getWindowsShellWindowWidth(self):
        from ctypes import windll, create_string_buffer

        # stdin handle is -10
        # stdout handle is -11
        # stderr handle is -12

        h = windll.kernel32.GetStdHandle(-12)
        csbi = create_string_buffer(22)
        res = windll.kernel32.GetConsoleScreenBufferInfo(h, csbi)

        if res:
            import struct
#            (bufx, bufy, curx, cury, wattr, left, top, right, bottom, maxx, maxy) = struct.unpack("hhhhHhhhhhh", csbi.raw)
            (_, _, _, _, _, left, _, right, _, _, _) = struct.unpack("hhhhHhhhhhh", csbi.raw)
            sizex = right - left + 1
            #sizey = bottom - top + 1
        else:
            sizex, _ = 80, 25 # can't determine actual size - return default values

        return sizex

    #-------------------------------------------------------------------------------------------
    # bsimethod
    # From: http://stackoverflow.com/questions/566746/how-to-get-console-window-width-in-python
    #-------------------------------------------------------------------------------------------
    def __getUnixShellWindowWidth(self):
        def getTerminalSize():
            def ioctl_GWINSZ(fd):
                try:
                    import fcntl, termios, struct
                    cr = struct.unpack('hh', fcntl.ioctl(fd, termios.TIOCGWINSZ, '1234'))
                except:
                    return None
                return cr
            cr = ioctl_GWINSZ(0) or ioctl_GWINSZ(1) or ioctl_GWINSZ(2)
            if not cr:
                try:
                    fd = os.open(os.ctermid(), os.O_RDONLY)  # pylint: disable=no-member
                    cr = ioctl_GWINSZ(fd)
                    os.close(fd)
                except:
                    pass
            if not cr:
                try:
                    cr = (os.environ['LINES'], os.environ['COLUMNS'])
                except:
                    cr = (25, 80)
            return int(cr[1]), int(cr[0])

        (width, _) = getTerminalSize()
        return width

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def __writeText(self, message, outputLevel, textColor=UNCHANGED, isLineRewritable=False):
        if self.__writeRawText( message, outputLevel, textColor ):

            def writeNormalLine( thisMessage, thisOutputLevel, thisTextColor ):
                if isLineRewritable:
                    thisMessage = "\n" + thisMessage

                self.__writeText( thisMessage, thisOutputLevel, thisTextColor, False )

            self.m_writeNormalLine = writeNormalLine

            def writeRewritableLine( thisMessage, thisOutputLevel, thisTextColor ):
                thisMessage = thisMessage.lstrip('\r\n').rstrip()

                thisMessage = '\r' + self.__padMessage (thisMessage)

                # If the previously written line was not rewritable and did not end with a newline
                if not isLineRewritable and not message.endswith( '\n' ):
                    thisMessage = '\n' + thisMessage

                self.__writeText( thisMessage, thisOutputLevel, thisTextColor, True )

            self.m_writeRewritableLine = writeRewritableLine

            return True
        return False

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def __writeRawText (self, rawText, outputLevel, textColor=UNCHANGED):
        if os.name == 'nt':
            return self.__writeWindowsRawText (rawText, outputLevel, textColor)
        elif os.name == 'posix':
            return self.__writePosixRawText (rawText, outputLevel, textColor)
        else:
            sys.stdout.write ('Unknown platform! ' + os.name + '\n')
            sys.exit(1)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def __writeWindowsRawText (self, rawText, outputLevel, textColor):
        if outputLevel != None and (outputLevel > INFO_LEVEL_None and outputLevel <= g_verbosity):
            rawText = compat.getEncodableString(rawText)
            if textColor != UNCHANGED:
                with colorconsole.setTextColor (textColor, UNCHANGED):
                    self.m_outputStream.write(rawText)
            else:
                self.m_outputStream.write(rawText)

            return True
        return False

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def __writePosixRawText (self, rawText, outputLevel, _textColor):
        if outputLevel != None and (outputLevel > INFO_LEVEL_None and outputLevel <= g_verbosity):
            # NEEDSWORK colorizing

            self.m_outputStream.write(rawText)

            return True
        return False

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def __padMessage(self, message):
        # If a line causes the shell to scroll down a line, then a carriage return won't work to rewrite
        # both lines.  Limit the line length so this doesn't happen.  Subtract 1 because we need the cursor
        # to stay on the current line.
        if os.name == 'nt':
            allowedWindowWidth = self.__getWindowsShellWindowWidth() - 1
        elif os.name == 'posix':
            allowedWindowWidth = self.__getUnixShellWindowWidth() - 1
        else:
            sys.stdout.write ('Unknown platform! ' + os.name + '\n')
            sys.exit(1)

        if message.__len__() > allowedWindowWidth:
            return message[:allowedWindowWidth - 3] + "..."
        return message + ' ' * (allowedWindowWidth - message.__len__())

g_lineWriter = LineWriter( INFO_LEVEL_VeryInteresting )

#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
class LogBuffer (object):

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def __init__ (self):
        self.m_lineStorage = []
        self.m_currentLines = []
        self.m_currentColor = UNCHANGED
        self.m_lineStorage.append ((self.m_currentColor, self.m_currentLines))

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def printRewritableLine(self, message, _screenOutputLevel, logOutputLevel, textColor):
        if willShow(logOutputLevel):
            self.Append (message+'\n', textColor)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def writeMsg(self, message, outputLevel, textColor=UNCHANGED):
        if willShow(outputLevel):
            self.Append (message, textColor)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def writeError(self, message):
        self.Append (message, RED)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def Empty (self):
        return len (self.m_lineStorage) == 1 and len (self.m_currentLines) == 0

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def Append (self, line, color=UNCHANGED):
        line = prependTimeStamp(line)
        if color == self.m_currentColor:
            self.m_currentLines.append (line)
        else:
            self.m_currentLines = [line]
            self.m_currentColor = color
            self.m_lineStorage.append ((self.m_currentColor, self.m_currentLines))

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def AppendDeferred (self, line, level, color=UNCHANGED):
        self.Append (line, color)

        if not (line, level) in globalvars.deferredMessages:
            globalvars.deferredMessages.append((line, level))

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def WriteToFile (self, logFile):
        for (_, lineBuffer) in self.m_lineStorage:
            for line in lineBuffer:
                logFile.write(line)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def WriteToScreen (self):
        for (color, lineBuffer) in self.m_lineStorage:
            for line in lineBuffer:
                g_lineWriter.writePlainMsg (line, INFO_LEVEL_Important, color)
        flushInfoStream()

#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
class FileLogBuffer (LogBuffer):
    # A log buffer that we can treat as a file.
    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def __init__ (self, logFileName, append=False):
        LogBuffer.__init__ (self)
        self.m_logFileName = logFileName
        self.m_append = append
        self.m_logFile = None

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def OpenLog (self):
        if not self.m_logFile:
            # Defer opening the file until the first usage to avoid empty files
            if not self.m_append:
                symlinks.makeSureBaseDirectoryExists (self.m_logFileName)
                options = "wt"
            else:
                options = "at"

            self.m_logFile = compat.openLog (self.m_logFileName, options)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def Append (self, line, color=UNCHANGED):
        LogBuffer.Append (self, line, color)
        if not self.m_logFile:
            self.OpenLog()
        self.m_logFile.write (line)
        self.m_logFile.flush() # Mike wants the logfiles to be live so need to flush frequently.

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def write (self, line):
        self.Append (line)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def flush (self):
        if self.m_logFile:
            self.m_logFile.flush()

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def __enter__(self): # To support With syntax
        return self

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def __exit__(self, exc_type, exc_value, traceback): # To support With syntax
        if self.m_logFile:
            self.m_logFile.close()
            
#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def addBufferToCurrentThread ():
    assert (not hasattr(g_threadLocalStorage, 'logger')) # Don't add if there is one there
    g_threadLocalStorage.logger = LogBuffer()

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def getThreadLineWriter ():
    # If no logger is set on the thread then use the screen printer. This makes it always
    # work for all the scripts outside of bb that use some of these modules.
    if hasattr (g_threadLocalStorage, "logger"):
        return g_threadLocalStorage.logger
    else:
        return g_lineWriter

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def revisionSourceIsBDFServer (source):
    lowerSource = source.lower()
    return True if lowerSource.startswith ("bdfserver") else False

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def getBdfFromDb (product, versionList, retries=5):
    try:
        bdfMsg = []
        serverBDF, status = builddescriptionfile.BuildDescription.QueryDatabaseForBuildDescription (product, versionList, retries=retries, messages=bdfMsg)
        for msg in bdfMsg:
            showInfoMsg (msg, INFO_LEVEL_VeryInteresting)
        if status:
            showInfoMsg("Could not get BDF from server. Returned: {0}\n".format(status), INFO_LEVEL_VeryInteresting, RED)
            raise FatalError (message="Error: could not get BDF from server (nonzero status). \n")

    except builddescriptionfile.BuildDescriptionError as e:
        showInfoMsg("{0}\n".format(e), INFO_LEVEL_VeryInteresting, RED)
        raise FatalError (message="Error: could not get BDF from server. \n")
    return serverBDF

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def parseRevisionSource (source, actualFile = None):
    source = os.path.expandvars (source)
    lowerSource = source.lower()
    if "tip" == lowerSource:
        return None

    elif revisionSourceIsBDFServer(lowerSource):
        bdfList = lowerSource.split(':')
        if len(bdfList) != 3:
            showInfoMsg("Could not parse input. Use format bdfserver:[PRODUCT]:[VERSION].\n", INFO_LEVEL_VeryInteresting, RED)
            raise FatalError (message="Error: input command formatted incorrectly.\n")
        product = bdfList[1]
        version = bdfList[2]

        versionList = re.split('\\.|\\,|\\-', version)
        serverBDF = getBdfFromDb (product, versionList)

        if None != actualFile:
            actualFile[0] = source  # Return so it can be logged / displayed.
        return serverBDF

    else:
        fileList = glob.glob (source)
        if len(fileList) > 1:
            source = fileList[-1]
        if None != actualFile:
            actualFile[0] = source  # Return so it can be logged / displayed.

    return builddescriptionfile.BuildDescription (source)

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def promptForStreamName (treeConfigPath, defaultStream):
    if None == defaultStream:
        defaultStream = ""

    showInfoMsg ("\nUnable to find a source tree configuration file for '{0}'.\n".format (os.path.expandvars ("${SrcRoot}")) + \
                "A configuration file will be created at: {0}\n\n".format (treeConfigPath), INFO_LEVEL_Essential)

    if isPrg():
        raise BuildError ("Tree configuration file {0} does not exist; it should have been created by bootstrap. Make sure paths (and case on non-windows) are correct.".format(treeConfigPath))

    showInfoMsg ("What stream name should this source tree use? [default: {0}] ".format (defaultStream), INFO_LEVEL_Essential)

    streamName = sys.stdin.readline().strip()
    if "" == streamName:
        streamName = defaultStream

    return streamName.lower()

#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
class TreeConfiguration (object):

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def __init__ (self, path):
        self.m_streamName = None
        self.readFromPath (path)

    def readFromDom (self, xmlDom):
        treeDom = getDomElementsByName (xmlDom, "TreeConfiguration")
        if len (treeDom) != 1:
            raise BuildError ("Tree configuration file must have one root element named 'TreeConfiguration'")
        treeDom = treeDom[0]

        streamDom = getDomElementsByName (treeDom, "Stream")
        if len (streamDom) != 1:
            raise BuildError ("Tree configuration file must have exactly one element named 'Stream'")
        streamDom = streamDom[0]

        streamName = streamDom.getAttribute ("Name")
        if "" == streamName:
            raise BuildError ("'Stream' element must have a 'Name' attribute")
        self.m_streamName = streamName.lower()

    def readFromPath (self, xmlPath):
        if not os.path.exists (xmlPath):
            raise BuildError ("Error reading local configuration '{0}': file does not exist.".format (xmlPath))

        xmlDom = parseXml (xmlPath)

        try:
            self.readFromDom (xmlDom)
        except BuildError as error:
            raise BuildError ("{0}: {1}".format (xmlPath, error.errmessage))

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def hideOutput (_line):
    pass

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def hideOutputErrorCheck (line):
    if line.startswith("waiting"):
        exitOnError( 1, line )

#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
class FlowConfiguration (object):

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def __init__ (self, path):
        self.m_allowOutgoing = []
        self.m_allowIncoming = []
        self.m_lkgStream = None
        self.readFromPath (path)

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def readFromDom (self, xmlDom):
        flowDom = getDomElementsByName (xmlDom, "FlowConfiguration")
        if len (flowDom) != 1:
            raise BuildError ("Flow configuration file must have one root element named 'FlowConfiguration'")
        flowDom = flowDom[0]

        allowedIncomingElements = getDomElementsByName (flowDom, "AllowedIncomingStreams")
        if len (allowedIncomingElements) != 1:
            raise BuildError ("Flow configuration file must have exactly one element named 'AllowedIncomingStreams'")
        allowedIncomingDom = allowedIncomingElements[0]

        allowedStreams = getDomElementsByName (allowedIncomingDom, "Stream")
        for stream in allowedStreams:
            streamName = stream.getAttribute ("Name")
            if "" == streamName:
                raise BuildError ("Each 'Stream' element in the AllowedIncomingStreams must have a 'Name' attribute")
            self.m_allowIncoming.append (streamName)

        allowedOutgoingElements = getDomElementsByName (flowDom, "AllowedOutgoingStreams")
        if len (allowedOutgoingElements) != 1:
            raise BuildError ("Flow configuration file must have exactly one element named 'AllowedOutgoingStreams'")
        allowedOutgoingDom = allowedOutgoingElements[0]

        allowedStreams = getDomElementsByName (allowedOutgoingDom, "Stream")
        for stream in allowedStreams:
            streamName = stream.getAttribute ("Name")
            if "" == streamName:
                raise BuildError ("Each 'Stream' element in the AllowedOutgoingStreams must have a 'Name' attribute")
            self.m_allowOutgoing.append (streamName)

        LKGStream = getDomElementsByName (flowDom, "LKGStream")
        if len(LKGStream) > 0:
            self.m_lkgStream = LKGStream[-1].getAttribute("StreamName")
            if not self.m_lkgStream:
                raise BuildError ("Flow configuration file LKGStream must have StreaName element")

        # Sort now, although only need sorted when writing
        self.m_allowIncoming.sort()
        self.m_allowOutgoing.sort()

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def readFromPath (self, xmlPath):
        if not os.path.exists (xmlPath):
            raise BuildError ("Error reading flow configuration '{0}': file does not exist.".format (xmlPath))

        xmlDom = parseXml (xmlPath)

        try:
            self.readFromDom (xmlDom)
        except BuildError as error:
            raise BuildError ("{0}: {1}".format (xmlPath, error.errmessage))

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def allowIncoming (self, streamName):
        streamNameToCheck = streamName.lower()
        for stream in self.m_allowIncoming:
            if stream.lower() == streamNameToCheck:
                return True
        return False

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def allowOutgoing (self, streamName):
        streamNameToCheck = streamName.lower()
        for stream in self.m_allowOutgoing:
            if stream.lower() == streamNameToCheck:
                return True
        return False

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def GetLkgStream (self):
        return self.m_lkgStream

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def SetLkgStream (self, streamName):
        self.m_lkgStream = streamName

    #-------------------------------------------------------------------------------------------
    # bsimethod
    #-------------------------------------------------------------------------------------------
    def WriteToFile (self, fileName):
        fileName = os.path.abspath (fileName)

        listfileDir = os.path.split (fileName)[0]
        symlinks.makeSureDirectoryExists (listfileDir)
        lfile = open (fileName, 'wt')
        if not lfile:
            return 1,  "Error: could not open Flow Configuration file '{0}'.".format (fileName)

        flowDom = minidom.Document()

        flowConfNode = flowDom.createElement ("FlowConfiguration")
        flowDom.appendChild (flowConfNode)

        #  Incoming
        incomingNode = flowDom.createElement ("AllowedIncomingStreams")
        flowConfNode.appendChild (incomingNode)

        for streamName in self.m_allowIncoming:
            incomingStreamNode = flowDom.createElement ("Stream")
            incomingStreamNode.setAttribute ("Name", streamName)
            incomingNode.appendChild (incomingStreamNode)

        #  Outgoing
        outgoingNode = flowDom.createElement ("AllowedOutgoingStreams")
        flowConfNode.appendChild (outgoingNode)

        for streamName in self.m_allowOutgoing:
            outgoingStreamNode = flowDom.createElement ("Stream")
            outgoingStreamNode.setAttribute ("Name", streamName)
            outgoingNode.appendChild (outgoingStreamNode)

        # LKG Source
        if self.m_lkgStream:
            LkgStreamNode = flowDom.createElement ("LKGStream")
            LkgStreamNode.setAttribute ("StreamName", self.m_lkgStream)
            flowConfNode.appendChild (LkgStreamNode)

        flowDom.writexml(lfile, "", "  ", "\n", "UTF-8")
        lfile.close ()
        flowDom.unlink()  # Recommended by current docs to free doms this way

        try:
            pass
        except Exception as err:
            return 1, "Error generating FlowConfiguration file: {0}".format (err.args[0])

        return 0, "Succeeded"

#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
class ArgumentParser (argparse.ArgumentParser):

    def __init__ (self, **keywordArgs):
        argparse.ArgumentParser.__init__ (self, **keywordArgs)

    # Overrides argparse.ArgumentParser.error
    def error (self, message):
        showInfoMsg ("Error: " + message + "\n\n", INFO_LEVEL_Essential, RED)
        self.print_help()
        sys.exit (1)

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def splitSources(inSources):
    sources= []
    lines = inSources.split('\n')
    for line in lines:
        entries = line.split(',')
        for entry in entries:
            entry = entry.lstrip(" ").rstrip(" ")
            if "" != entry:
                sources.append (entry)
    return sources

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def defineEnvVar (localEnv, var):
    """Define an environment variable, both in the local process and for subsequent sub-processes."""
    val = "1"
    equal = var.find("=")
    if -1 != equal:
        val = var[equal+1:]
        var = var[:equal]
    appendEnvVar (localEnv, var, val)

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def appendEnvVar (localEnv, key, value):
    # Need to check if the env var is already set to retain its case.
    if not key in localEnv: # do a quick lookup to see if already exists by exact-case.
        lowerKey = key.lower()
        
        for localEnvKey in localEnv.keys():
            if localEnvKey.lower() == lowerKey:
                key = localEnvKey
                break

    key = compat.getStringForEnv (key)
    value = compat.getStringForEnv (value)

    localEnv[key] = value

#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
def getDomElementsByName (dom, *elementNameList):
    elems = []
    for child in dom.childNodes:
        if child.nodeType != child.ELEMENT_NODE:
            continue
        for elementName in elementNameList:
            if child.nodeName == elementName:
                elems.append(child)
                break

    return elems

#--------------------------------------------------------------------------------
# @bsimethod
#--------------------------------------------------------------------------------
def xml_removeWhiteSpaceNodes (parent):
    for child in list(parent.childNodes):
        if child.nodeType == xml.dom.Node.TEXT_NODE and child.data.isspace():
            parent.removeChild (child)
        else:
            xml_removeWhiteSpaceNodes (child)

#--------------------------------------------------------------------------------
# @bsimethod
#--------------------------------------------------------------------------------
def xml_removeWhiteSpaceNodesFile (filename):
    with open (filename, "rt") as xmlfile:
        parsedDom = minidom.parse (xmlfile)
    xml_removeWhiteSpaceNodes (parsedDom)
    with open (filename, "wt") as xmlfile:
        parsedDom.writexml (xmlfile, "", "    ", "\n")
    parsedDom.unlink()

#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
def willShow (verbosityLevel):
    if verbosityLevel != None and (verbosityLevel > INFO_LEVEL_None and verbosityLevel <= g_verbosity):
        return True
    return False

#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
def getDomElementValue (dom, elementName):
    elements = getDomElementsByName(dom, elementName)
    if 0 == len(elements):
        raise BuildError ("Cannot find '{0}' element".format (elementName))

    if 1 < len(elements):
        raise BuildError ("Too many '{0}' elements".format (elementName))

    fc = elements[0].firstChild
    if fc.nodeType != fc.TEXT_NODE:
        raise BuildError ("Invalid value for '{0}'".format(elementName))

    return fc.data

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def getEnvOrDomElementValue (dom, elementName):
    elements = getDomElementsByName(dom, elementName)
    if 0 == len(elements):
        raise BuildError ("Cannot find '{0}' element".format (elementName))

    if 1 < len(elements):
        raise BuildError ("Too many '{0}' elements".format (elementName))

    envMacro = elements[0].getAttribute ("EnvMacro")
    if len (envMacro) > 0 and envMacro:
        if envMacro in os.environ:
            return os.environ [envMacro]

    fc = elements[0].firstChild
    if fc.nodeType != fc.TEXT_NODE:
        raise BuildError ("Invalid value for '{0}'".format(elementName))

    return fc.data

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def resolveOption (val, options, source):
    val = val.lower()
    if not val in options:
        raise StrategyError (source + " must be one of " + options.__str__() + " but is value " + val)

    return options.index(val)

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def resolveBoolean (val, source):
    if not resolveOption (val, ['false', 'true'], source):
        return False
    return True

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def getOptionalAttr (attrName, defaultVal, choices, explanation, dom):
    attrStr = dom.getAttribute (attrName)
    if "" == attrStr:
        return defaultVal
    return  resolveOption(attrStr, choices, explanation)

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def getOptionalBoolAttr (attrName, defaultVal, explanation, dom):
    boolStr = dom.getAttribute (attrName)
    if "" == boolStr:
        return defaultVal
    return  resolveBoolean (boolStr, explanation)

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def isBlank(val):
    return None == val or "" == val

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def showRewritableLine (message, outputLevel, textColor=UNCHANGED):
    getThreadLineWriter().printRewritableLine (message, outputLevel, outputLevel, textColor)

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def showStatusMsg (message, outputLevel, textColor=UNCHANGED, logLevel=None):
    # In the multi-threading world, a status line is a rewritable line when printing
    #  to the screen so the user knows something is happening, but is a lower-status
    #  item when logging.
    if logLevel == None:
        logLevel = min (outputLevel+2, INFO_LEVEL_RarelyUseful)
    getThreadLineWriter().printRewritableLine (message, outputLevel, logLevel, textColor)

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def showInfoMsg (msg, level, foregroundColor=UNCHANGED):
    # The abort: prefix is for hg errors
    # The BMAKE: prefix is for bmake errors and warnings
    if msg.startswith( 'BMAKE: Warning' ):
        foregroundColor = YELLOW
    elif msg.startswith( 'abort:' ) or msg.startswith( 'BMAKE:' ):
        foregroundColor = RED
    getThreadLineWriter().writeMsg( msg, level, foregroundColor )

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def showSummaryHeader (summary):
    addSeparatorLine("\n")
    showInfoMsg (summary + '\n', INFO_LEVEL_Essential)
    addSeparatorLine()

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def showRepositoryHeader (localRepo):
    showInfoMsg(  localRepo.GetDescriptiveString() + ": \n", INFO_LEVEL_Essential, LIGHT_BLUE )

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def flushInfoStream ():
    sys.stdout.flush()

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def addSeparatorLine ( prefix='', suffix='\n\n', foregroundColor=UNCHANGED, infoLevel=INFO_LEVEL_Important):
    showInfoMsg (prefix + getSeparatorLine() + suffix, infoLevel, foregroundColor)

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def getSeparatorLine ():
    return "_"*110

 #-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def appendBuildError (msg):
    globalvars.buildErrors.append(msg)

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def showErrorMsg (msg):
    appendBuildError (msg)
    getThreadLineWriter().writeError (msg)

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def showErrorCallStack (err):
    stackToPrint = getErrorCallStack (err)
    if stackToPrint:
        showCallStack (stackToPrint)

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def getErrorCallStack (err):
    if not hasattr (err, 'stackTrace'):
        return None

    import traceback
    stackToPrint = traceback.format_list(err.stackTrace)
    if len (stackToPrint) == 0:
        return None
    stackToPrint = '\n'.join(stackToPrint)
    stackToPrint = stackToPrint.replace ('\n\n', '\n')
    return stackToPrint

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def showCallStack (stackToPrint):
    if not stackToPrint or len(stackToPrint)==0:
        return

    msg = '=== Call stack where exception occurred, to help debug.  See actual error above or below. ===\n'
    showInfoMsg ('\n'+msg, INFO_LEVEL_RarelyUseful)
    showInfoMsg (stackToPrint, INFO_LEVEL_RarelyUseful)
    showInfoMsg (msg, INFO_LEVEL_RarelyUseful)

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def showCurrentCallStack (message=''):
    # Use this to dump the current call stack if you want to see where something happens.
    import traceback
    stackToPrint = traceback.format_list(traceback.extract_stack())
    sys.stdout.write ('\n\n  === ' + message + ' ===\n\n')
    for item in stackToPrint:
        sys.stdout.write (item + '\n')
    sys.stdout.write ('\n\n\n')

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def showAndLogMsg (msg, logFile, level):
    if None != logFile:
        logFile.write(msg)

    showInfoMsg (msg, level)

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def ShowAndDeferMessage (msg, level, foregroundColor=UNCHANGED):
    if not (msg, level) in globalvars.deferredMessages:
        globalvars.deferredMessages.append((msg, level))
    showInfoMsg (msg, level, foregroundColor)

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def ShowDeferredMessages (clear=False, message=None):
    if not globalvars.deferredMessages or len (globalvars.deferredMessages) == 0:
        return

    if not message:
        message = ' Deferred Messages: '

    showInfoMsg ('\n' + '_'*10 + message + '_'*10 + '\n\n', INFO_LEVEL_VeryInteresting, LIGHT_BLUE)
    for item in globalvars.deferredMessages:
        showInfoMsg (item[0] + '\n', item[1])

    if clear:
        globalvars.deferredMessages = []

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def processResponseFile (args):
    if len(args)<2 or not args[1].startswith ("@"):
        return args

    filename = args[1][1:]
    try:
        argfile  = open (filename, "rt")
    except IOError as err:
        sys.stderr.write ("Unable to open response file '{0}': {1}\n".format (filename, err.strerror))
        exit(1)

    import shlex
    newArgs =[]
    newArgs.append (args[0])
    for line in argfile.readlines():
        for thisarg in shlex.split(line):
            newArgs.append (thisarg)
    argfile.close()

    return newArgs

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def cleanDirectory (root, deleteFiles=False, level=INFO_LEVEL_VeryInteresting):
    root = compat.getUnicode(root)
    root = symlinks.normalizePathName (root)
    if not os.path.isdir (root):
        return

    showStatusMsg ("Removing Directory {0}".format (root.encode('ascii', 'ignore')), level)
    fileList = os.listdir(root)
    for curfile in fileList:
        testname = os.path.join(root, curfile)
        try:
            if symlinks.isSymbolicLink(testname):
                symlinks.deleteSymLink (testname)
            else:
                if os.path.isdir(testname):
                    try:
                        cleanDirectory (testname, deleteFiles)
                        os.rmdir(testname)
                    except:
                        # Try a second time; something seems to hold files open
                        showStatusMsg ("retrying deletion of directory {0}".format (testname.encode('ascii', 'ignore')), level)
                        time.sleep (1)
                        cleanDirectory (testname, deleteFiles)
                        os.rmdir(testname)
                else:
                    if deleteFiles:
                        os.chmod(testname, stat.S_IWRITE)
                        os.remove (testname)
                    else:
                        showErrorMsg ("BuildContexts must only contain links: {0}\n".format (testname.encode('ascii', 'ignore')))
        except OSError as err:
            showInfoMsg ("Clean Error: " + err.__str__() + "\n", INFO_LEVEL_Essential)

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def deleteFileWithRetries (fileName, level=INFO_LEVEL_VeryInteresting):
    if not os.path.exists (fileName):
        return
    retries = 5
    while retries > 0:
        try:
            os.remove (fileName)
        except OSError as err:
            lastErr = err
        if not os.path.exists (fileName):
            return
        showStatusMsg ("retrying deletion of file {0}".format (fileName.encode('ascii', 'ignore')), level)
        retries -= 1
        time.sleep (1)
        
    showErrorMsg ("Error deleting file : {0}\n{1}".format (fileName.encode('ascii', 'ignore'), lastErr))

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def copyFileWithRetries (srcPath, destPath, level=INFO_LEVEL_VeryInteresting):
    retries = 5
    while retries > 0:
        try:
            shutil.copy(srcPath, destPath)
        except OSError as err:
            lastErr = err
        if os.path.exists (destPath):
            return
        showStatusMsg ("retrying copying file {} to {}".format (srcPath.encode('ascii', 'ignore'), destPath.encode('ascii', 'ignore')), level)
        retries -= 1
        time.sleep (1)

    showErrorMsg ("Error copying file {} to {}\n{}".format (srcPath.encode('ascii', 'ignore'), destPath.encode('ascii', 'ignore'), lastErr))

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def dirwalk(dirToWalk):
    if not os.path.isdir(dirToWalk):
        yield dirToWalk
    else :
        for f in os.listdir(dirToWalk):
            fullpath = os.path.join(dirToWalk,f)
            if os.path.isdir(fullpath) and not os.path.islink(fullpath):
                for x in dirwalk(fullpath): 
                    yield x
            else:
                yield fullpath

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def listAllFiles (pathToCheck):
    if not os.path.exists (pathToCheck):
        return []
    allFiles = []
    for curfile in dirwalk (pathToCheck):
        allFiles.append (curfile)
    return allFiles

#--------------------------------------------------------------------------------
# @bsimethod
#--------------------------------------------------------------------------------
def changePermissionOfDirContents (root, changeIfFileIsNot=os.W_OK, permissions=stat.S_IWRITE, level=INFO_LEVEL_VeryInteresting):
    for root, _, files in os.walk (root):
        for name in files:
            fname = os.path.join (root, name)
            # Only change if it isn't writable.
            if not os.access (fname, changeIfFileIsNot):
                showStatusMsg ("Changing permissions of {0} to {1}".format (fname, str(permissions)), level)
                os.chmod (fname, permissions)

#--------------------------------------------------------------------------------
# @bsimethod
#--------------------------------------------------------------------------------
def setContentsOfDirectoryReadOnly (destination, level=INFO_LEVEL_VeryInteresting, extensionsToSkip=None):
    """
    Make directory contents readonly.
    extensionsToSkip is an array of tuples [(a1,b1), (a2,b2) ... ]
        a is regular expression to match on the filename.
        b is the reason why it should be excluded.
    """
    showStatusMsg ("Making contents of {0} readonly.".format (destination), level)
    compiledRegexes = []
    if extensionsToSkip == None:
        extensionsToSkip = []

    for regex in extensionsToSkip:
        prog = re.compile (regex[0], re.I)
        compiledRegexes.append ((regex, prog))

    for root, _, files in os.walk(destination):
        for name in files:
            skipWrite = False
            patternMatched = None
            for item in compiledRegexes:
                compiledRegex = item[1]
                if None != compiledRegex.match (name):
                    skipWrite = True
                    patternMatched = item[0]
                    break

            fullFilepath = os.path.join(root, name)
            if skipWrite:
                showStatusMsg ("   Skipping '{0}'. {1}\n".format (fullFilepath, patternMatched[1]), level)
            else:
                # Windows kind of supports chmod; on *nix, just strip write status, leaving potential execute bits in place
                if os.name == 'nt':
                    os.chmod(fullFilepath, stat.S_IREAD)
                else:
                    currMode = os.stat(fullFilepath).st_mode
                    newMode = currMode ^ (stat.S_IWUSR | stat.S_IWGRP | stat.S_IWOTH)
                    os.chmod(fullFilepath, newMode)

#--------------------------------------------------------------------------------
# @bsimethod
#--------------------------------------------------------------------------------
def iterateFilesInDirectory (directory, callbackMethod):
    for root, _, files in os.walk(directory):
        for name in files:
            fullFilepath = os.path.join(root, name)
            callbackMethod (fullFilepath)

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def GetVersionForProvenance ():
    # Try to get it from environment (PRG)
    relV = os.environ.get ('REL_V', -1)
    majV = os.environ.get ('MAJ_V', -1)
    minV = os.environ.get ('MIN_V', -1)
    subminV = os.environ.get ('SUBMIN_V', -1)

    if not -1 in [relV, majV, minV, subminV]:
        return (relV, majV, minV, subminV)

    # Next, look in stdversion.mki for default versions (Dev build)
    try:
        stdversionFileName = GetSharedMkiFile ('stdversion.mki')
        if os.path.exists (stdversionFileName):
            svlines = []
            with open (stdversionFileName, 'rt') as svfile:
                svlines = svfile.readlines ()

            for line in svlines:
                if re.match (r"^[ \t]*REL_V=([0-9]+)", line):
                    relV = re.match (r"^[ \t]*REL_V=([0-9]+)", line).group (1)
                elif re.match (r"^[ \t]*MAJ_V=([0-9]+)", line):
                    majV = re.match (r"^[ \t]*MAJ_V=([0-9]+)", line).group (1)
                elif re.match (r"^[ \t]*MIN_V=([0-9]+)", line):
                    minV = re.match (r"^[ \t]*MIN_V=([0-9]+)", line).group (1)
                elif re.match (r"^[ \t]*SUBMIN_V=([0-9]+)", line):
                    subminV = re.match (r"^[ \t]*SUBMIN_V=([0-9]+)", line).group (1)

    except BuildError: # No bsicommon dir?
        pass

    if not -1 in [relV, majV, minV, subminV]:
        return (relV, majV, minV, subminV)

    # All else failed; use bogus number.  999 is intentional; with 8.64 already in the pipeline, 99 may not be high enough.
    return globalvars.defaultVersion

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def copyFile (source, target):
    if compareFileTimes (target, source):
        return 0

    symlinks.makeSureBaseDirectoryExists(target)

    showInfoMsg ("Copying '{0}' to '{1}'\n".format (source, target), INFO_LEVEL_VeryInteresting)
    shutil.copy2 (source, target)
    return 0

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def mirrorDirWithSymlinks (source, target):
    if not os.path.exists(source):
        raise BuildError ("Directory mirroring failed: source directory '{0}' could not be found".format (source))

    showInfoMsg ("Mirroring directory '{0}' to '{1}'\n".format (source, target), INFO_LEVEL_VeryInteresting)
    if not os.path.exists(target):
        os.makedirs(target)

    for root, dirs, files in os.walk(source):
        for fileName in files:
            fileSource = os.path.join(root, fileName)
            fileTarget = os.path.join(target, os.path.relpath(fileSource, source))
            if not compareFileTimes (fileTarget, fileSource):
                if symlinks.isSymbolicLink (fileSource):
                    symlinks.createFileSymLink (fileTarget, symlinks.getSymlinkTarget (fileSource), checkSame=True, checkTargetExists=True, skipIntermediateLinks=True)
                else:
                    shutil.copy2(fileSource, fileTarget)

        for dirName in dirs:
            dirSource = os.path.join(root, dirName)
            dirTarget = os.path.join(target, os.path.relpath(dirSource, source))
            if not os.path.exists(dirTarget):
                if symlinks.isSymbolicLink (dirSource):
                    symlinks.createDirSymLink (dirTarget, symlinks.getSymlinkTarget (dirSource), checkSame=True, checkTargetExists=True, skipIntermediateLinks=True)
                else:
                    os.makedirs(dirTarget)

#--------------------------------------------------------------------------------
# @bsimethod
#--------------------------------------------------------------------------------
def queryForRegistryEntry (regPath, regEntry, force32bitRegistry=False, force64bitRegistry=False):
    entry, regtype, msg = compat.getRegistryEntry (regPath, regEntry, force32bitRegistry, force64bitRegistry)
    if msg:
        showInfoMsg (msg, INFO_LEVEL_RarelyUseful)

    return (entry, regtype)

#--------------------------------------------------------------------------------
# @bsimethod
#--------------------------------------------------------------------------------
def pathRemainder (path, start):
    if not start.lower() in path.lower():
        return path

    remPath = path[len(start):]
    if remPath[0] == '\\':
        remPath = remPath[1:]
    return remPath

#--------------------------------------------------------------------------------
# @bsiclass
#--------------------------------------------------------------------------------
def _getlines (filename):
    with open (filename, "rt") as curfile:
        lines = curfile.readlines ()
    return lines

#--------------------------------------------------------------------------------
# @bsiclass
#--------------------------------------------------------------------------------
class FileDiffer (object):
    def __init__ (self, filenameA, filenameB):
        self.m_filenameA = filenameA
        self.m_filenameB = filenameB
        self.m_diffResult = None

#--------------------------------------------------------------------------------
# @bsimethod
#--------------------------------------------------------------------------------
    def Diff (self):
        alines = _getlines (self.m_filenameA)
        blines = _getlines (self.m_filenameB)
        self.m_diffResult = difflib.unified_diff (alines, blines)

#--------------------------------------------------------------------------------
# @bsimethod
#--------------------------------------------------------------------------------
    def HasDifferences (self):
        #  m_diffResult is a generator so can't check the length
        for _ in self.m_diffResult:
            return True
        return False

#--------------------------------------------------------------------------------
# @bsimethod
#--------------------------------------------------------------------------------
    def GetDifferenceString (self):
        if not self.HasDifferences ():
            return "{0} and {1} are equivalent.".format (self.m_filenameA, self.m_filenameB)
        msg = "{0} and {1} are different:\n".format (self.m_filenameA, self.m_filenameB)
        for line in self.m_diffResult:
            msg += line
        return msg

#--------------------------------------------------------------------------------
# @bsimethod
#--------------------------------------------------------------------------------
def zipDir (dirToZip, outputFilename, infoLevel=INFO_LEVEL_VeryInteresting):
    """
    zip dirToZip as the root level folder in the created .zip.
    Generates outputFilename.
    """
    # We want the root directory in the zip to be the directory we are zipping.
    if "\\" == dirToZip[-1]:
        dirToZip = dirToZip[:-1]

    dstDir = os.path.split (outputFilename)[0]
    symlinks.makeSureDirectoryExists (dstDir)

    showInfoMsg ("\n\nZipping {0} into {1}.\n".format (dirToZip, outputFilename), infoLevel)

    zf = zipfile.ZipFile (outputFilename, "w", allowZip64=True)
    for curfile in getContentsOfDirectory (dirToZip, getRelPathFromRoot=False):
        relPath = curfile[len (os.path.split(dirToZip)[0])+1:]
        showStatusMsg ("  {0}\n".format (relPath), infoLevel)
        zf.write (curfile, arcname=relPath, compress_type=zipfile.ZIP_DEFLATED)
    zf.close ()

#--------------------------------------------------------------------------------
# @bsimethod
#--------------------------------------------------------------------------------
def unzipToDir (zipFilename, dstDir):
    if not os.path.isfile (zipFilename):
        msg = "{0} doesn't exist.\n".format (zipFilename)
        showInfoMsg (msg, INFO_LEVEL_VeryInteresting)
        return 1

    #cleanDirectory (dstDir, deleteFiles=True)
    symlinks.makeSureDirectoryExists (dstDir)

    zf = zipfile.ZipFile (zipFilename, "r", allowZip64=True)
    zf.extractall (dstDir)
    zf.close()
    return 0

#--------------------------------------------------------------------------------
# @bsimethod
#--------------------------------------------------------------------------------
def getContentsOfDirectory (dirName, getRelPathFromRoot=False):
    if os.path.split (dirName)[1] == '':
        dirName = os.path.split (dirName)[0]

    allFiles = []
    for root, _, files in os.walk (dirName):
        for name in files:
            fname = os.path.join (root, name)
            if getRelPathFromRoot:
                allFiles.append (fname[len(dirName)+1:])
            else:
                allFiles.append (fname)
    return allFiles


#--------------------------------------------------------------------------------
# @bsimethod
#--------------------------------------------------------------------------------
def IsVersionGreater (version1, version2):
    verQuad1 = (version1 + ".0.0.0").split ('.')
    verQuad2 = (version2 + ".0.0.0").split ('.')
    for i in range (4):
        if int (verQuad1 [i]) > int (verQuad2 [i]):
            return True
        elif int (verQuad1 [i]) < int (verQuad2 [i]):
            return False
        else:
            continue


    return False

#--------------------------------------------------------------------------------
# @bsimethod
#--------------------------------------------------------------------------------
def getLKGVersionStr (versionStr):
    versionSplit = versionStr.split (".")
    def lZerostrip (inStr):
        outStr = inStr.lstrip ('0')
        return outStr if len (outStr) > 0 else "0"

    return "-".join ([
                        lZerostrip (versionSplit [0]),
                        lZerostrip (versionSplit [1]),
                        lZerostrip (versionSplit [2]),
                        lZerostrip (versionSplit [3])
                    ])

#--------------------------------------------------------------------------------
# @bsimethod
#--------------------------------------------------------------------------------
def createVersionToDirMappingFromDirectory (path, listOfItems, onInvalidCallback):
    versionToDirDict = {}
    for item in listOfItems:
        b = item.replace ('b', '-')  # convert rebuild number to dash - sort will treat it as version number.
        b = b.replace ('.', '-')  # convert dot version numbers (mainly for Nuget)
        versionStrNums = b.split ('-')

        msg = "{0} has invalid named file/folder '{1}'. Must be four values 'xx-xx-xx-xx' or five values 'xx-xx-xx-xxbxx'.\n  Not all versions of bentleybuild handle this. It should be removed.\n".format (path, item)
        if len (versionStrNums) < 4 or len (versionStrNums) > 5:
            onInvalidCallback (item, msg)
            continue
        try:
            t = tuple (map(int, versionStrNums))
        except:
            onInvalidCallback (item, msg)
            continue

        versionToDirDict[t] = item
    return versionToDirDict

#--------------------------------------------------------------------------------
# @bsimethod
#--------------------------------------------------------------------------------
def copyToClipboard (inbuffer):
    from Tkinter import Tk
    r = Tk()
    r.withdraw()
    r.clipboard_clear()
    r.clipboard_append (inbuffer)
    r.destroy()

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def GetPullTime ():
    global g_pullDate
    if g_pullDate == -1:
        dt = datetime.datetime.utcnow()
        # This is approximated ISO 8601 time, and something CVS will accept.
        g_pullDate = dt.strftime ('%Y-%m-%d %H:%M:%S GMT')
    return g_pullDate

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def FindMaxVersionDirectory (parentDirPath, versionHint):
    if not os.path.isdir (parentDirPath):
        return None

    childDirs = glob.glob (os.path.join (parentDirPath, versionHint))
    if len(childDirs) == 0:
        return None

    childDirWithVersionMax = None
    maxFoundVersion = "0.0.0.0"
    for childDir in childDirs:
        baseName = os.path.basename (childDir)
        versionStr = ".".join (baseName.split ("-"))
        if IsVersionGreater (versionStr, maxFoundVersion):
            maxFoundVersion = versionStr
            childDirWithVersionMax = baseName


    if childDirWithVersionMax:
        return os.path.join (parentDirPath, childDirWithVersionMax)

    return None

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def SplitStringAtVersionVariable (stringWithVersionVar):
    versionVarLocation = str(stringWithVersionVar).find ("$(version)")
    if  versionVarLocation == -1:
        return (stringWithVersionVar, "")

    return (stringWithVersionVar [:versionVarLocation], stringWithVersionVar [versionVarLocation + 10:])

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def ReplaceCaseInsensitive (oldStr, newStr, strData):
    idx = 0
    if len (oldStr) == 0 or len (strData) == 0:
        return strData

    while idx < len(strData):
        index_l = strData.lower().find(oldStr.lower(), idx)
        if index_l == -1:
            return strData
        strData = strData [:index_l] + newStr + strData [index_l + len(oldStr):]
        idx = index_l + len(oldStr)
    return strData

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def CompressPathToPathMacro (pathMacro, completePath):
    expandedPathMacro = symlinks.normalizePathName (pathMacro)
    completePath = symlinks.normalizePathName (completePath)
    return ReplaceCaseInsensitive (expandedPathMacro, pathMacro, completePath)

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def GetBsicommon ():
    if 'SrcBsiCommon' in os.environ:
        return os.environ ['SrcBsiCommon']
    elif 'SrcRoot' in os.environ:
        return os.path.join (os.environ ['SrcRoot'], 'bsicommon')
    else:
        raise BuildError ("Cannot find file SrcRoot in the environment")

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def GetSharedMkiFile (mkiFile):
    itemPath = os.path.join (GetBsicommon(), 'sharedmki', mkiFile)
    if os.path.exists (itemPath):
        return itemPath
    raise BuildError ("Cannot find file {0} ({1})".format (mkiFile, itemPath))

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def GetToolFile (toolName, fileName):
    itemPath = os.path.join (buildpaths.GetToolsOutputRoot(), toolName, fileName)
    if os.path.exists (itemPath):
        return itemPath
    raise BuildError ("Cannot find file {0} ({1})".format (fileName, itemPath))

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def GetBsiToolsFile (fileName):
    return GetToolFile(globalvars.TOOLS_BSITOOLS, fileName)

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def GetBsiScriptsFile (fileName):
    return GetToolFile(globalvars.TOOLS_BSISCRIPTS, fileName)

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def GetCpuCount ():
    import multiprocessing
    return multiprocessing.cpu_count()

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
s_natSortRe=re.compile('([0-9]+)')
def NaturalSortKey (text):
    return [int(c) if c.isdigit() else c.lower() for c in re.split(s_natSortRe, text)]

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def NameMatch (itemName, pattern):
    # When not matching filenames (common) we really want to always be case insenstive.
    # Fnmatch is case-insensitive based on the file system but that isn't what we want
    #   since we are comparing part names, build contexts, etc. and we want it consistent
    #   across all platforms.
    # Fnmatch is also slow in our most common cases ('*' and 'name', not 'name*')
    #  so skip the regex creation most of the time.
    
    # if it is just * then it matches
    if pattern == '*':
        return True

    # if no wildcards use simple compare
    if not '*' in pattern and not '?' in pattern:
        return itemName.lower() == pattern.lower()
        
    # One or the other is a partially wildcarded item; fall back to fnmatch
    return fnmatch.fnmatch (itemName.lower(), pattern.lower())

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def GetNewestBentleyBuildFileTime ():
    # We use the time of any bb files as a surragate for any of the stored data structures changing.
    newestTime = 0
    for bbfile in glob.glob (os.path.join (os.path.dirname(os.path.realpath(__file__)), '*.py')):
        st = os.stat (bbfile)
        if st.st_mtime > newestTime:
            newestTime = st.st_mtime
    return newestTime

#-------------------------------------------------------------------------------------------
# bsimethod
# Use the below in a "with" statement to preserve the cwd before and after the block
#-------------------------------------------------------------------------------------------
@contextmanager
def pushd(newDir):
    previousDir = os.getcwd()
    os.chdir(newDir)
    yield
    os.chdir(previousDir)

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def _GetInitialRootPath(inputPath, rootEnv):
    rootPath = os.environ.get (rootEnv)
    if rootPath and inputPath.lower().startswith(rootPath.lower()):
        return rootPath
    return None
        
#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def GetExactCaseFilePath (inputPath):
    srcRoot = _GetInitialRootPath(inputPath, 'SrcRoot')
    outRoot = _GetInitialRootPath(inputPath, 'OutRoot')
    # initialize with matching part of root path
    if srcRoot:
        name = srcRoot
    elif outRoot:
        name = outRoot
    else:
        name = ''
    start = len(name)
    # then extend with lower/upper bracketed pairs for remainder of full path
    glob_list = [ ''.join(['[', ch.lower(), ch.upper(), ']']) if ch.lower() != ch.upper() else ch for ch in inputPath[start:] ]
    # convert back from list to string
    name += ''.join(glob_list)
    result = glob.glob(name)
    showInfoMsg("\nPosixDebug: GetExactCaseFilePath name {0} glob result {1} inputPath {2}\n".format(name, str(result), inputPath), INFO_LEVEL_TemporaryDebugging)
    return result

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def resolveBindingMacros (filename, platform, part, forceUseApiNumber=False, ignoreApiNumber=False):
    # Moved to utils to get past circular references
    replaceMap = platform.GetExtensionMap(part.IsStatic())

    if part.IsStatic() and not forceUseApiNumber:
        replaceMap[globalvars.API_NUMBER_MACRO] = ''
    elif not ignoreApiNumber and part.m_apiNumber:
        replaceMap[globalvars.API_NUMBER_MACRO] = part.m_apiNumber

    for key in replaceMap.keys():
        filename = filename.replace (key, replaceMap[key])

    return filename

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def createSeededGuid (seed, seedGuid="58D26D5C-4984-44E5-BE8E-36EE0CDF84CD"):
    seed = str (''.join([i if ord(i) < 128 else str (ord(i)) for i in seed])).lower ().strip ()
    return  "{" + str (uuid.uuid3( uuid.UUID(seedGuid), str (seed))).upper() + "}"

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def GetRetryInfo ():
    global g_retryInfo
    if not g_retryInfo:
        g_retryInfo = [5, 7]  # NumRetries, Max Seconds
        if "BB_RETRY_INFO" in os.environ:
            retryEnv = os.environ["BB_RETRY_INFO"]
            showInfoMsg("Setting retries based on environment {0}\n".format(retryEnv), INFO_LEVEL_SomewhatInteresting)
            retryList = retryEnv.split(';')
            for item in retryList:
                splitItem = item.split('=')
                if len(splitItem) != 2: 
                    raise BuildError ("Bad item '{0}' in BB_RETRY_INFO '{1}'".format(item, retryEnv))
                if splitItem[0].upper() == 'RETRIES':
                    g_retryInfo[0] = int(splitItem[1])
                    showInfoMsg("Setting retries to {0} from environment\n".format(g_retryInfo[0]), INFO_LEVEL_SomewhatInteresting)
                if splitItem[0].upper() == 'MAXDELAY':
                    g_retryInfo[1] = int(splitItem[1])
                    showInfoMsg("Setting max duration to {0} from environment\n".format(g_retryInfo[1]), INFO_LEVEL_SomewhatInteresting)
                    
    return g_retryInfo[0], g_retryInfo[1]

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def GetNextSleepTime (curSleepTime, maxDelay):
    return curSleepTime + 1 if curSleepTime < maxDelay else curSleepTime # Try sleepging a little longer each time, but not too long.

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def getUrlWithJsonWithRetries (url, jsonData, proxyDict=None):
    retries, maxDelay = GetRetryInfo ()
    sleepTime = 0
    attempt = 0
    while attempt < retries:
        attempt += 1
        httpCode, jsonContent = compat.getUrlWithJson (url, jsonData, proxyDict)
        if httpCode == 200:
            break

        showInfoMsg ("getUrlWithJson failed for {0} with code {1}; Retry ({2}) after sleep ({3})\n".format (url, httpCode, attempt, sleepTime), INFO_LEVEL_RarelyUseful)
        time.sleep (sleepTime)
        sleepTime = GetNextSleepTime(sleepTime, maxDelay)
    return httpCode, jsonContent

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def getUrlWithDataWithRetries (url, data, statusheaders, unredirectedAuthHeader = None, proxyDict=None):
    retries, maxDelay = GetRetryInfo ()
    sleepTime = 0
    attempt = 0
    while attempt < retries:
        attempt += 1
        httpCode, httpContent = compat.getUrlWithData(url, data, statusheaders, 
            unredirectedAuthHeader, proxyDict, showInfoMsg=showInfoMsg, LogLevel=INFO_LEVEL_RarelyUseful)
        if httpCode == 200:
            break

        showInfoMsg ("getUrlWithData failed for {0} with code {1}; Retry ({2}) after sleep ({3})\n".format (url, httpCode, attempt, sleepTime), INFO_LEVEL_RarelyUseful)
        time.sleep (sleepTime)
        sleepTime = GetNextSleepTime(sleepTime, maxDelay)
    return httpCode, httpContent
