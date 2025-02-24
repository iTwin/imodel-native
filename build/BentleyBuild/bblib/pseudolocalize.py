#---------------------------------------------------------------------------------------------
#  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
#  See LICENSE.md in the repository root for full copyright notice.
#---------------------------------------------------------------------------------------------
import argparse, os, subprocess, re, sys

from . import utils, symlinks, languagesettings

PREFIX = u'\u2039'  # looks like <
SUFFIX = u'\u203a'  # looks like >
FORMAT_REPLACEMENT = u'\u2045{0}\u2261{{{0}}}\u2046'    # looks like [%s={%s}], only with fancy characters

def __make_translation_table():
    """Create mapping for unicode.translate to add diacritics to vowels."""

    translate_from = u"AEIOU"
    translate_to = u"\u0104\u0228\u012e\u01ea\u0172"        # A with ogonek; E with cedilla; I, O, U with ogonek

    translate_from += translate_from.lower()
    translate_to += translate_to.lower()
    
    assert len(translate_from) == len(translate_to)
    
    # unicode.translate requires mapping from unicode *ordinals* to unicode ordinals, unicode strings, or None
    translate_from_ord = (ord(c) for c in translate_from)
    translate_to_ord = (ord(c) for c in translate_to)

    translation_table = dict(list(zip(translate_from_ord, translate_to_ord)))
    
    # special case, y with ogonek, needs combining
    translation_table[ord(u"Y")] = u"Y\u0328"
    translation_table[ord(u"y")] = u"y\u0328"
    
    return translation_table


TRANSLATION_TABLE = __make_translation_table ()     


def pseudotranslate (inputStr, format_placeholders=False):
    """Pseudo-localize given string."""
    
    if utils.py3:
        assert isinstance(inputStr, str)
    else:
        assert isinstance(inputStr, unicode) # pylint: disable=undefined-variable
        

    result_parts = [PREFIX]
    
    # strings between curly brackets should not be translated
    # usually they are format strings (placeholders)
    
    start_idx = inputStr.find('{')
    end_idx = -1
    while start_idx != -1:
        result_parts.append(inputStr[end_idx+1:start_idx].translate(TRANSLATION_TABLE))
        
        end_idx = inputStr.find('}', start_idx+1)
        placeholder_name = inputStr[start_idx+1:end_idx]
        
        if format_placeholders:
            result_parts.append(FORMAT_REPLACEMENT.format(placeholder_name))
        else:
            result_parts.append("{{{0}}}".format(placeholder_name))

        start_idx = inputStr.find('{', end_idx+1)

    result_parts.append(inputStr[end_idx+1:].translate(TRANSLATION_TABLE))
    
    result_parts.append(SUFFIX)

    return u"".join(result_parts)


NonTranslatableType = [
    "System.Drawing.Bitmap",
    "System.Drawing.Icon",
    "System.Drawing.Point",
    "System.Drawing.Size",
    "System.Resources.ResXFileRef",
    "System.UInt32" ]

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def isTranslatable (node):

    # The incorrect "istranslateable" spelling was what Leon originally added.
    # I couldn't stand telling people to about how to mark things non-translatable
    # anymore so I added support for the correct spelling too.  BernMcCarty 8/2006 (copied from dntranskit.cs)
    attr = None
    if 'istranslateable' in node.attributes.keys():
        attr = node.attributes['istranslateable']
    elif 'istranslatable' in node.attributes.keys():
        attr = node.attributes['istranslatable']
    if attr and attr.value.lower() == 'false':
        return False

    # Check the type as well
    if 'type' in node.attributes.keys():
        types = node.attributes["type"].value.replace(',', '  ').split()
        for trtype in types:
            trtype = trtype.strip()
            if trtype in NonTranslatableType:
                return False

    # Finally check the name to see if it's valid.
    if 'name' in node.attributes.keys():
        name = node.attributes["name"].value.lower()
        if -1 == name.find('.') or name.endswith(".text") or name.endswith(".tooltiptext") or name.endswith(".title"):
            return True
        if isSpecialScenarioForSelectSever(node):
            return True
    return False

#--------------------------------------------------------------------------------
# @bsimethod
#--------------------------------------------------------------------------------
def isSpecialScenarioForSelectSever (node):
    #If the type is definded for the resex field make sure it is a string otherwise ignore it.
    if 'type' in node.attributes.keys() :
        if -1 == node.attributes["type"].value.find("system.string"):
            return False

    if 'name' in node.attributes.keys() and node.attributes["name"].value.endswith(".Value") :
        return True
            
    #Logic to ensure we psuedo any field with .Text or .Tooltip or .*Text or .*Tooltip in it.
    name = node.attributes["name"].value.lower()
    prog = re.compile (r".*\..*text")
    result = prog.match (name)
    if None != result:
        return True    

    prog = re.compile (r".*\..*tooltip")
    result = prog.match (name)
    if None != result:
        return True    

    return False
        
#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def fixDomSchemanNode (dom):
    # This works around a bug in the minidom that causes it to crash when a blank attribute is used.  This version of xmlns is common in our resx files.
    #    <xsd:schema id="root" xmlns="" xmlns:xsd="http://www.w3.org/2001/XMLSchema" xmlns:msdata="urn:schemas-microsoft-com:xml-msdata">
    schemaNode = dom.getElementsByTagName('xsd:schema')
    if schemaNode and schemaNode[0]:
        for attr in schemaNode[0].attributes.keys():
            if schemaNode[0].attributes[attr].value == None:
                schemaNode[0].attributes[attr].value = ''

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def getMartianizerToolDir ():
    return os.path.join (os.environ["TRANSKIT_ROOT"], "Tools" ,"martiantool")

#-------------------------------------------------------------------------------------------
# bsiclass
#-------------------------------------------------------------------------------------------
class PseudolocalizeOptions:
    def __init__(self):
        self.m_targetLanguage = 'ru-RU'
        self.m_forceWixEnUs = True
        self.m_addBrackets = False
        self.m_addPostfix = False

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def processFile (inputFileName, outputFileName, pseudolocalizeOptions, _handleSpecialCases=True, martianizerToolDir=None):
    if martianizerToolDir is None:
        martianizerToolDir = getMartianizerToolDir()
    martianizer = os.path.join(martianizerToolDir,'BentleyFileConverter.exe')
    currdir = os.path.split(martianizer)[0]
    outputFileName = os.path.split(outputFileName)[0]

    cmd = [martianizer, '-m', inputFileName, 'en-US', pseudolocalizeOptions.m_targetLanguage, '-o', outputFileName]

    if not pseudolocalizeOptions.m_forceWixEnUs:
        cmd.append("-NoForceWixEnUs")

    if pseudolocalizeOptions.m_addBrackets:
        cmd.append("-ab")
    else:
        cmd.append("-nb")

    if pseudolocalizeOptions.m_addPostfix:
        cmd.append("-ap")
    p = subprocess.Popen(cmd, cwd= currdir, shell = False, stdout = subprocess.PIPE, stderr=subprocess.PIPE, universal_newlines=True)
    p.communicate()    

    if p.returncode != 0:
        msg = "WARNING {0} FAILED: returned {1}. Check if correct arguments are passed to BentleyFileConverter.exe: \r\n    {0} ".format(cmd, repr(p.returncode))
        utils.showInfoMsg (msg, utils.INFO_LEVEL_Essential, utils.YELLOW)

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def processDirectory (inputDirectory, outputDirectory, pseudolocalizeOptions, handleSpecialCases=True, martianizerToolDir=None):
    utils.showInfoMsg ("processing directory {0}\r\n".format (inputDirectory), utils.INFO_LEVEL_VeryInteresting)
    for item in os.listdir (inputDirectory):
        if os.path.isdir (os.path.join (inputDirectory, item)):
            outDir = os.path.join (outputDirectory, item)
            symlinks.makeSureDirectoryExists (outDir)
            processDirectory (os.path.join (inputDirectory, item), outDir, pseudolocalizeOptions, handleSpecialCases, martianizerToolDir)
        else:
            outputFileName = os.path.join (outputDirectory, item)
            inputFileName = os.path.join (inputDirectory, item)
            processFile (inputFileName, outputFileName, pseudolocalizeOptions, handleSpecialCases, martianizerToolDir)

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def getOutputFile (inputFileName, outputIsDir, output):
    if not output:
        return inputFileName
    elif not outputIsDir:
        return output
    else:
        return os.path.join (output, os.path.basename(inputFileName))

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def main():
    parser = argparse.ArgumentParser ("usage: %(prog)s [options] [fileToMartianize] [dirToMartianize]")
    parser.add_argument("-o", "--output",     action="store",         dest="output",  help="Output location")
    parser.add_argument("-r", "--recurse",    action="store_true",    dest="recurse", help="Recurse subdirectories")
    parser.add_argument("-d", "--disableSpecialCases",    action="store_false",    dest="handleSpecialCases", help="Check SpecialCases : data name ends in \\.Value, then psuedo it")
    parser.add_argument("-l", "--targetLanguage",    action="store",    dest="target_language", default="ru-RU",  help="Pseudolocalize to target language. Format: xx-XX. Default: ru-RU")
    parser.add_argument("-l", "--targetLanguage",    action="store",    dest="target_language", default="mr",  help="Pseudolocalize to target bb language. Defaults to 'mr' (uses 'ru-RU' pseudolocalization language with WIXculture set to en-US)")
    parser.add_argument("-b", "--addBrackets",    action="store_true",    dest="add_brackets", help="Add brackets [] to pseudolocalized string")
    parser.add_argument("-p", "--addPostfix",    action="store_true",    dest="add_postfix", help="Add postfix to pseudolocalized string")
    parser.add_argument("--failOnErrors",    action="store_true",    dest="fail_on_errors", help="Fail on pseudolocalize errors")
    parser.add_argument ('args', nargs=argparse.REMAINDER, help="Files and directories to martianize")  # Collects up the rest of the arguments to pass to command

    parser.set_defaults (output=None, recurse=False, handleSpecialCases=True)
    options = parser.parse_args()

    argDirs = False
    for item in options.args:
        if os.path.isdir (item):
            argDirs = True
            
    if argDirs and options.output and os.path.exists (options.output) and not os.path.isdir (options.output):
        sys.stderr.write ('Output must be a directory if input is a directory or multiple files\n')
        sys.exit (1)
        
    # If there are multiple files and the output is specified, then it must be a directory.  Create if necessary.
    outputIsDir = False
    if options.output:
        if options.args or argDirs:
            if not os.path.exists (options.output):
                os.makedirs (options.output)
                outputIsDir = True
        elif os.path.isdir (options.output):
            outputIsDir = True
    
    pseudoOptions = PseudolocalizeOptions()
    pseudoOptions.m_addBrackets = options.add_brackets
    pseudoOptions.m_addPostfix = options.add_postfix

    if options.target_language == 'mr':
        pseudoOptions.m_targetLanguage = 'ru-RU'
        pseudoOptions.m_forceWixEnUs = True
    else:
        pseudoOptions.m_targetLanguage = languagesettings.LanguageSettings(options.target_language).m_wixCultureCode
        pseudoOptions.m_forceWixEnUs = False
    
    if options.fail_on_errors:
        os.environ['BentleyFileConverterStrictMode'] = '1'

    for item in options.args:
        if os.path.isfile (item):
            outName = getOutputFile (item, outputIsDir, options.output)
            processFile (item, outName, pseudoOptions, True, None)
        else:
            item = os.path.normpath (item)
            outputDirectory = options.output if options.output else item
            processDirectory (item, outputDirectory, pseudoOptions, options.handleSpecialCases, None)

    sys.exit (0)

#--------------------------------------------------------------------------------
# @bsimethod
#--------------------------------------------------------------------------------
if __name__ == '__main__':
    main()
