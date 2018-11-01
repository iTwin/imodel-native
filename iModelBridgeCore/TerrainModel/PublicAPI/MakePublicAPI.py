#--------------------------------------------------------------------------------------
#
#     $Source: PublicAPI/MakePublicAPI.py $
#    $RCSfile: MakePublicAPI.py,v $
#   $Revision: 1.2 $
#       $Date: 2010/02/18 17:04:03 $
#     $Author: Daryl.Holmwood $
#
#  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
#
#--------------------------------------------------------------------------------------

import os
import string
import stat
import sys
import getopt

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def openForWrite(filename):
    dirname = os.path.dirname(filename)
    if not os.path.exists(dirname):
        os.mkdir(dirname)
    
    if os.path.exists (filename):
        os.chmod(filename, stat.S_IWRITE)
        os.remove(filename)
        
    oOutFile = open(filename, 'w')
    print 'Generating Published Header File: ', filename
    return oOutFile

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def getLastModificationTime(filename):
    # We purposefully return 0 if the file does not exist - rather than throw an error
    try:
        st = os.stat(filename)
    except os.error:
        return 0
    return st[stat.ST_MTIME]

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def setLastModificationTime(filename, lastModTime):
    st = os.stat(filename)
    os.utime(filename, (lastModTime, lastModTime))
    os.chmod(filename, stat.S_IREAD)

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def checkForNonpublishedExports (fileLine, privateExportFile, originalFile):
    if not privateExportFile:
        return

    if -1 != string.find (fileLine, 'DTMCORE_EXPORT'):
        fileBase = os.path.split (originalFile)[1]
        privateExportFile.write (fileBase + ': ' + fileLine)

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def examineLine (fileLine, outputSection, fileName, fileLineNumber):
    # outputSection is true if we are in a PUBLISH_SECTION part, and false otherwise.

    foundDelimiter = -1 != string.find (fileLine, '/*__PUBLISH_SECTION_START__*/')

    if foundDelimiter and outputSection:
        # Found START tag within PUBLISH_SECTION segment.
        print "Error: " + fileName + " : Line " + `fileLineNumber` + \
            " : __PUBLISH_SECTION_START__ found within __PUBLISH_SECTION segment."
        sys.exit(1)
    if foundDelimiter:
        return True
            
    foundDelimiter = -1 != string.find (fileLine, '/*__PUBLISH_SECTION_END__*/')

    if foundDelimiter and not outputSection:
        # Found END tag outside PUBLISH_SECTION segment.
        print "Error: " + fileName + " : Line " + `fileLineNumber` + \
            " : __PUBLISH_SECTION_END__ found outside PUBLISH_SECTION segment."
        sys.exit(1)
    if foundDelimiter:
        return False
           
    return outputSection                        

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def writeToFile (fileLine, outputFile):

    start = string.find (fileLine, '@bsimethod')
    if -1 != start: 
        fileLine = fileLine[:start] + '@bsimethod                                                    Bentley Systems\n'
    else:
        start = string.find (fileLine, '@bsiclass')
        if -1 != start:
            fileLine = fileLine[:start] + '@bsiclass                                                     Bentley Systems\n'
        
    fileLine = string.replace (fileLine, '/*__PUBLISH_ABSTRACT__*/', 'abstract')
    fileLine = string.replace (fileLine, '/*__PUBLISH_SEALED__*/', 'sealed')
    fileLine = string.replace (fileLine, '/*__PUBLISH_HEADER__*/', 'Bentley.')
    fileLine = string.replace (fileLine, '/*__PUBLISH_CLASS_VIRTUAL__*/', 'private:\n    virtual void MakeClassAbstract() = 0;\n\n')

    outputFile.write (fileLine)

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def generatePublicHeaderBody (originalFile, outputFile, privateExportFile):

    """--------Read original header file as a single string---------"""
    oFile = open(originalFile, 'r')
    fileInAStringList = oFile.readlines()
    oFile.close()

    delimitersFound = False
    outputSection = False
    fileLineCount = 0
    for fileLine in fileInAStringList:
        fileLineCount += 1
        string.strip (fileLine)

        if outputSection:
            outputSection = examineLine (fileLine, outputSection, originalFile, fileLineCount)
            if outputSection:
                delimitersFound = True
                writeToFile (fileLine, outputFile)
            # For non-published sections, look for exported methods
        else:
            if -1 != string.find (fileLine, '/*__PUBLISH_CLASS_VIRTUAL__*/'):
                outputFile.write ('private:\n    virtual void MakeClassAbstract() = 0;\n\n')
            else:
                checkForNonpublishedExports (fileLine, privateExportFile, originalFile)
            outputSection = examineLine (fileLine, outputSection, originalFile, fileLineCount)
    if not delimitersFound:
        print '\nNo public file generation statements found in', originalFile
        sys.exit (1)

    
#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def printUsageAndExit(customErrorMessage = None):
    if None != customErrorMessage:
        sys.stderr.write('\n' + customErrorMessage + '\n')
    sys.stderr.write('\nUsage: %s [options] publicFile1.h [publicFile2.h,]\n' % os.path.basename(sys.argv[0]))
                
    usageString = """
    
    Options include:
    \t--help                    (-h)  Print this help message.
    \t--sourcedir   soureDirname    Source directory name. This is replaced in every filename with outdir filename
    \t--outdir      outdirname      Base directory to write output to.
    \t--template    templatefile    Filename for template for the copyright message and #pragma once 
    \t--privexports privExportFile  Filename to write exported functions that are private
        
    """                        
                    
    sys.stderr.write (usageString)            
    sys.stderr.write('\t\n')
    sys.exit (1)

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def writePublicHeaderFileHeaderSection (outFile, templateFile):
    oFile = open(templateFile, 'r')
    fileInAStringList = oFile.readlines()
    oFile.close()
    
    for fileLine in fileInAStringList:
        outFile.write (fileLine)

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def checkForFileExistence (fileName, customErrorMessage = None):
    fileExists = os.path.exists (fileName)
    if not fileExists:
        if None != customErrorMessage:
            sys.stderr.write('\n' + customErrorMessage + '\n') 
        print '\n%s does not exist' % fileName
        sys.exit (1)

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def checkForDirectoryExistence (fileName, customErrorMessage = None):
    dirExists = os.path.exists (fileName)
    dirName = os.path.dirname (fileName)
    if not dirExists:
        if None != customErrorMessage:
            sys.stderr.write('\n' + customErrorMessage + '\n')    
        print '\n%s does not exist' % dirName
        sys.exit (1)
        
#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def main():

    try:
        optTupleList, args = getopt.getopt (sys.argv[1:], 'h', ['help', 'sourcedir=', 'outdir=', 'template=', 'privexports='])
    except getopt.GetoptError, errIns:
        if errIns.msg:                        
            sys.stderr.write(errIns.msg + '\n')
        printUsageAndExit ()

    sourceDir = ''
    outDir = ''
    template = ''
    privateExports = None
    privateExportFile = None
    
    for optTuple in optTupleList:
        if optTuple[0] == '--sourcedir':
            sourceDir = optTuple[1]
        elif optTuple[0] == '--outdir':
            outDir = optTuple[1]
        elif optTuple[0] == '--template':
            template = optTuple[1]
        elif optTuple[0] == '--privexports':
            privateExports = optTuple[1]
        elif optTuple[0] == '--help' or optTuple[0] == '-h':
            printUsageAndExit()

    checkForFileExistence (template, 'Template File Does Not Exist')
        
    if privateExports:
        privateExportFile = openForWrite (privateExports)

    fileList = args
    try:
        for srcfilename in fileList:
            outfilename = string.replace (srcfilename, sourceDir, outDir)

            oOutFile = openForWrite(outfilename)
            writePublicHeaderFileHeaderSection (oOutFile, template)
            generatePublicHeaderBody (srcfilename, oOutFile, privateExportFile)
            oOutFile.close()
            os.chmod(outfilename, stat.S_IREAD)
            
    except:
        raise                   # reraise whatever exception it was

    if privateExports:
        privateExportFile.close()

if __name__ == '__main__':
    main()
