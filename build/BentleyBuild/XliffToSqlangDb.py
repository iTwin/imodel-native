#---------------------------------------------------------------------------------------------
#  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
#  See LICENSE.md in the repository root for full copyright notice.
#---------------------------------------------------------------------------------------------
#   Based partially on what is being done in Graphite to localize resources.  This
#   version does not support validating the xliff as that requires lxml. Requiring
#   all Vancouver developer to install lxml was too big of a hassle.
#--------------------------------------------------------------------------------------
import sys
import argparse
import bblib.utils as bbutils
import os

from xml.etree import ElementTree as etree
import sqlite3
import locale

language, output_encoding = locale.getdefaultlocale()

def enum(*sequential, **named):
    enums = dict(list(zip(sequential, list(range(len(sequential))))), **named)
    reverse = dict((value, key) for key, value in enums.items())
    enums['reverse_mapping'] = reverse
    return type('Enum', (), enums)

DbMod = enum('KeepNothing', 'KeepDb', 'KeepRows')

class ArgError (Exception):
    def __init__(self, message):
        self.errmessage = message

#---------------------------------------------------------------------------------------
#  bsimethod
#--------------+---------------+---------------+---------------+---------------+--------
def checkVal (val, name):
    if None != val:
        return val

    raise ArgError ("{0} not defined.".format(name))

PREFIX = u'\u2039'  # looks like <
SUFFIX = u'\u203a'  # looks like >

#---------------------------------------------------------------------------------------
#  bsimethod
#--------------+---------------+---------------+---------------+---------------+--------
def pseudotranslate (input):
    """Pseudo-localize given string."""
    
    result_parts = [PREFIX]
    result_parts.append(input)
    result_parts.append(SUFFIX)

    return u"".join(result_parts)

#---------------------------------------------------------------------------------------
#  bsimethod
#--------------+---------------+---------------+---------------+---------------+--------
def getDefaultTextFromSource (inputText):
    newString = inputText.replace ("_[", "")
    newString = newString.replace ("]_", "")
    return newString



#=======================================================================================
#  bsiclass
#=======================================================================================
class LocaleDb:
    #---------------------------------------------------------------------------------------
    #  bsimethod
    #--------------+---------------+---------------+---------------+---------------+--------
    def __init__(self, dbFilename, dbMod):
        self.dbFilename = os.path.expandvars(dbFilename)
        if dbMod == DbMod.KeepNothing and os.path.exists(dbFilename):
            os.remove(dbFilename)
        self.conn = sqlite3.connect(self.dbFilename, isolation_level=None) # Internet says to use isolateion_level as a workaround for a python bug giving the error "cannot VACUUM from within a transaction" 

        self.cursor = self.conn.cursor()
        self.cursor.execute(r'PRAGMA encoding = "UTF-8";')
        if dbMod == DbMod.KeepDb:
            self.cursor.execute(r'DROP TABLE IF EXISTS l10n_strings')
        self.cursor.execute("""CREATE TABLE IF NOT EXISTS l10n_strings (
            Id CHAR NOT NULL CHECK(Id!=''), 
            Value CHAR,
            Note CHAR,
            PRIMARY KEY (Id)
            )
        """)

    #---------------------------------------------------------------------------------------
    #  bsimethod
    #--------------+---------------+---------------+---------------+---------------+--------
    def Commit(self):
        self.conn.commit()

    #---------------------------------------------------------------------------------------
    #  bsimethod
    #--------------+---------------+---------------+---------------+---------------+--------
    def CommitAndClose(self, verbose):
        self.cursor.execute('VACUUM')
        self.conn.commit()

        if verbose:
            self.cursor.execute ('SELECT Id,Value,Note FROM l10n_strings')
            for colValues in self.cursor: 
                print("\nId=" + colValues[0].encode(output_encoding))
                print("Value=" + colValues[1].encode('utf-8'))
            
        self.conn.close()

    #---------------------------------------------------------------------------------------
    #  bsimethod
    #--------------+---------------+---------------+---------------+---------------+--------
    def UpdateFromXliff(self, xliffT, verbose, shouldPseudoLocalize):
        xliff = os.path.expandvars(xliffT)
        xliffEtree = etree.parse(xliff)

        sourceLocale = None
        targetLocale = None

        for fileElement in xliffEtree.findall(".//{urn:oasis:names:tc:xliff:document:1.2}file"):
            sourceLocale = fileElement.get("source-language")
            targetLocale = fileElement.get("target-language")
            if not targetLocale:
                targetLocale = sourceLocale

            #print ("sourceLocale="+sourceLocale+"   targetLocale="+targetLocale);

            for transunit in fileElement.findall("*/*/{urn:oasis:names:tc:xliff:document:1.2}trans-unit"):
                textId = transunit.get('id')
                textToInsert = None
                inputText = None
                noteToInsert = ""

                targetElement = transunit.find("{urn:oasis:names:tc:xliff:document:1.2}target")
                if targetElement is not None:
                    inputText = targetElement.text
                else:  
                    sourceElement = transunit.find("{urn:oasis:names:tc:xliff:document:1.2}source")
                    if sourceElement is not None:
                        inputText = getDefaultTextFromSource (sourceElement.text)

                if shouldPseudoLocalize:
                    textToInsert = pseudotranslate (inputText)
                else:
                    textToInsert = inputText

                if textToInsert is not None:
                    try:
                        self.cursor.execute ('INSERT INTO l10n_strings(Id,Value,Note) VALUES(?,?,?)', [textId, textToInsert, noteToInsert])
                    except sqlite3.IntegrityError as err:
                        if (verbose):
                            if shouldPseudoLocalize:
                                sys.stderr.write ("Error: Inserting values for Id={0}".format (textId))
                            else:
                                sys.stderr.write ("Error: Inserting values \n Id={0}\n Value={1}\n Note={2}\n".format (textId, textToInsert, noteToInsert))

                            sys.stderr.write ("{0}\n".format (err.message))


#---------------------------------------------------------------------------------------
#  bsimethod
#--------------+---------------+---------------+---------------+---------------+--------
def ImportXliffIntoDb(xliff, output, dbMod=DbMod.KeepNothing, shouldPseudoLocalize=False, verbose=False):
    db = LocaleDb(output, dbMod)
    db.UpdateFromXliff(xliff, verbose, shouldPseudoLocalize)
    db.CommitAndClose(verbose)

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def main():
    usage="usage: XliffToSqlangDb.py [options] "
    parser = bbutils.ArgumentParser(usage=usage)

    parser.add_argument ("-x", "--xliff",          action="store",       dest="xliff",                help="the name of the xliff file to convert into sqlang.db3")
    parser.add_argument ("-o", "--output",         action="store",       dest="output",               help="the name of the sqlite3 file to create, typically with .mui extension")
    parser.add_argument ("-v", "--verbose",        action="store_true",  dest="verbose",              help="If True then dump database rows.")
    parser.add_argument ("-c", "--clean",          action="store_true",  dest="clean",                help="Delete all target files")
    parser.add_argument ("-p", "--PseudoLocalize", action="store_true",  dest="shouldPseudoLocalize", help="Generate PseudoLocalize strings")

    parser.set_defaults (clean=False, shouldPseudoLocalize=False)

    #global options
    parserOpts = parser.parse_args()

    parserOpts.logFile = None

    if parserOpts.clean:
        exit (0)

    # When a *.mke invokes this script, they currently are not supplying the verbosity level from 
    # bentleybuild and the output for some things doesn't specify which command line args are being used which makes looking at the logs useless.
    bbutils.g_verbosity = bbutils.INFO_LEVEL_SomewhatInteresting

    try:
        checkVal (parserOpts.xliff, "xliff File Name")
        checkVal (parserOpts.output, "output mui File Name")
        ImportXliffIntoDb(parserOpts.xliff, parserOpts.output, DbMod.KeepNothing, parserOpts.shouldPseudoLocalize, parserOpts.verbose)
    except ArgError as err:
        sys.stderr.write ("Error: {0}\n".format (err.errmessage))
        # parser.formatter = optparse.IndentedHelpFormatter(2, 70, 130)
        parser.print_help()
        exit (1)

    exit (0)

#---------------------------------------------------------------------------------------
#  bsimethod
#--------------+---------------+---------------+---------------+---------------+--------
if __name__ == '__main__':
    main()
