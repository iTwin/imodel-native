#!Python
#--------------------------------------------------------------------------------------
#
#      $Source: DgnHandlers/WordBreakDataGenerator.py $
#
#   $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
#
#--------------------------------------------------------------------------------------

import getopt
import os
import sys

#----------------------------------------------------------------------------------------------------------------------------------------------------
# Print usage
#----------------------------------------------------------------------------------------------------------------------------------------------------
def printUsageAndExit ():
    print """
Usage: WordBreakDataGenerator [-i|--input=]INPUT [-o|--output=]OUTPUT
    
    Arguments:
    INPUT   path to input word break property data file (see http://www.unicode.org/Public/UNIDATA/auxiliary/WordBreakProperty.txt)
    OUTPUT  path to a header file with data structures representing the input data
    
    Summary:
    This program reads a published WordBreakProperty.txt file, and generates
    a header file containing hard-coded data structures that represent said
    data. This is deemed preferrable to embedding the plain text file and
    parsing it during runtime.
"""
    sys.exit(-1)

#----------------------------------------------------------------------------------------------------------------------------------------------------
#
#----------------------------------------------------------------------------------------------------------------------------------------------------
def emitCharRangesForClass (outputFile, charClasses, className):
    
    structName = "WORD_BREAK_RANGES_{0}s".format (className)
    
    outputFile.write ("static const WordBreakRangeInfo {0}[] = {{ ".format (structName));
    
    numCharRanges = 0    
    
    for charRange in (potentialCharRange for potentialCharRange in charClasses if className == charClasses[potentialCharRange]):
        if numCharRanges > 0:
            outputFile.write (", ")
        
        if ".." in charRange:
            (begin, sep, end) = charRange.partition ("..")
            outputFile.write ("{{ 0x{0}, 0x{1} }}".format (begin, end))
        else:
            outputFile.write ("{{ 0x{0}, 0x{0} }}".format (charRange))
        
        numCharRanges += 1

    outputFile.write (" };\n")
    outputFile.write ("static const size_t {0}_Count = {1};\n\n".format (structName, numCharRanges))

#----------------------------------------------------------------------------------------------------------------------------------------------------
#
#----------------------------------------------------------------------------------------------------------------------------------------------------
def emitHelperTestMethod (outputFile, className):
    outputFile.write ("    public: static bool Is{0} (WChar codePoint)\n".format (className))
    outputFile.write ("        {\n")
    outputFile.write ("        for (size_t iRange = 0; iRange < WORD_BREAK_RANGES_{0}s_Count; ++iRange)\n".format (className))
    outputFile.write ("            if ((codePoint >= WORD_BREAK_RANGES_{0}s[iRange].m_rangeStart) && (codePoint <= WORD_BREAK_RANGES_{0}s[iRange].m_rangeEnd))\n".format (className))
    outputFile.write ("                return true;\n\n")
    
    outputFile.write ("        return false;\n")
    outputFile.write ("        }\n\n")

#----------------------------------------------------------------------------------------------------------------------------------------------------
# Run the app
#----------------------------------------------------------------------------------------------------------------------------------------------------
def main ():
    
    if 3 != len (sys.argv):
        printUsageAndExit ()
    
    #..............................................................................................
    # Process arguments
    
    inputFilePath   = ""
    outputFilePath  = ""
    
    try:
        opts, args = getopt.getopt (sys.argv[1:], "i:o:", ["input=", "output="])
    except:
        print ("ERROR: Failed to extract arguments")
        printUsageAndExit ()
    
    for opt, arg in opts:
        if opt in ("-i", "--input"):
            inputFilePath = arg.strip ()
        
        elif opt in ("-o", "--output"):
            outputFilePath = arg.strip ()
        
        else:
            print ("ERROR: Unknown argument '{0}'".format (opt))
            printUsageAndExit ()
    
    #..............................................................................................
    # Further validate arguments
    
    if "" == inputFilePath or not os.path.exists (inputFilePath):
        print ("ERROR: INPUT file path '{0}' does not exist".format (inputFilePath))
        sys.exit (-1)
    
    if "" == outputFilePath:
        print ("ERROR: OUTPUT file path is blank")
        sys.exit (-1)
    
    (outputFileDir, outputFileName) = os.path.split (outputFilePath)
    
    if "" == outputFileName:
        print ("ERROR: OUTPUT file name is blank")
        sys.exit (-1)
    
    if not os.path.exists (outputFileDir):
        try:
            os.makedirs (outputFileDir)
        except:
            print ("ERROR: OUTPUT directory '{0}' does not exist, and could not be created".format (outputFileDir))
            sys.exit (-1)
    
    #..............................................................................................
    # Process input file
    
    try:
        inputFile = open (inputFilePath, "r")
    except IOError:
        print ("ERROR: Could not open INPUT '{0}' for text read".format (inputFilePath))
        sys.exit (-1)
    
    charClasses = dict ()
    uniqueClassNames = list ()
    
    for inputFileLine in (rawLine.strip () for rawLine in inputFile):
        if "" == inputFileLine or inputFileLine.startswith ("#"):
            continue
        
        lineTokens = inputFileLine.split (";")
        
        if len(lineTokens) < 2:
            print ("WARNING: Could not parse line '{0}'".format (inputFileLine))
            continue
        
        charCodes = lineTokens[0].strip ()
        charClass = lineTokens[1].split ()[0]
        
        charClasses[charCodes] = charClass
        
        if charClass not in uniqueClassNames:
            uniqueClassNames.append (charClass)
    
    inputFile.close ()
    
    #..............................................................................................
    # Generate output file
    
    try:
        outputFile = open (outputFilePath, 'w')
    except IOError:
        print ("ERROR: Could not open OUTPUT '{0}' for text write".format (outputFilePath))
        sys.exit (-1)
    
    outputFile.write ("// This is a generated file; do not edit.\n")
    outputFile.write ("// The purpose of this file is to wrap the Unicode data file WordBreakProperty.txt in data structures, so that we don't have to parse it at runtime.\n")
    outputFile.write ("// This data comes from the Unicode consortium; at last writing, it came from http://www.unicode.org/Public/UNIDATA/auxiliary/WordBreakProperty.txt\n")
    outputFile.write ("// Generator arguments: {0}\n\n".format (sys.argv))
    
    outputFile.write ("#pragma once\n\n")
    
    outputFile.write ("/*__BENTLEY_INTERNAL_ONLY__*/\n\n")
    
    outputFile.write ("#include <DgnPlatform/DgnPlatform.h>\n\n")
    
    outputFile.write ("BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE\n\n")
    
    outputFile.write ("struct WordBreakRangeInfo\n")
    outputFile.write ("    {\n")
    outputFile.write ("    public: UInt32  m_rangeStart;\n")
    outputFile.write ("    public: UInt32  m_rangeEnd;\n\n")

    outputFile.write ("    }; // WordBreakRangeInfo\n\n")

    for className in uniqueClassNames:
        emitCharRangesForClass (outputFile, charClasses, className)
    
    outputFile.write ("struct WordBreakRangeUtilities\n")
    outputFile.write ("    {\n")
    outputFile.write ("    private: WordBreakRangeUtilities () { }\n")
    outputFile.write ("    private: WordBreakRangeUtilities (WordBreakRangeUtilities const&) { }\n")
    outputFile.write ("    private: ~WordBreakRangeUtilities () { }\n")
    outputFile.write ("    private: WordBreakRangeUtilities& operator= (WordBreakRangeUtilities const&) { return *this; }\n\n")
    
    for className in uniqueClassNames:
        emitHelperTestMethod (outputFile, className)
    
    outputFile.write ("    }; // WordBreakRangeUtilities\n\n")
    
    outputFile.write ("END_BENTLEY_DGNPLATFORM_NAMESPACE\n")
    
    outputFile.close ()
    
if __name__ == '__main__':
    main()
