#!Python
#--------------------------------------------------------------------------------------
#
#     $Source: DgnScriptContext/mergeJsSourceDeliveryFiles.py $
#
#  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
#
#--------------------------------------------------------------------------------------
import getopt, sys, os, codecs, re

#---------------------------------------------------------------------------------------
#  @bsimethod                                  Bentley.Systems
#---------------------------------------------------------------------------------------
def readSourceFile (path):
    return open (path).read().replace (codecs.BOM_UTF8, '')

#---------------------------------------------------------------------------------------
#  @bsimethod                                  Bentley.Systems
#---------------------------------------------------------------------------------------
def main():
    shortArgs = "j:n:o:"
    longArgs = ["jsSourceFiles=", "nativeSourceFile=", "outputPath="]

    try:
        opts, args = getopt.getopt (sys.argv [1:], shortArgs, longArgs)
    except getopt.GetoptError as err:
        print err
        sys.exit (2)

    jsSourceFiles = ""
    nativeSourceFile = ""
    outputPath = ""

    for o, a in opts:
        if o in ("-j", "--jsSourceFiles"):
            jsSourceFiles = a
        elif o in ("-n", "--nativeSourceFile"):
            nativeSourceFile = a
        elif o in ("-o", "--outputPath"):
            outputPath = a

    jsSources = {}
    for sourceDescriptor in jsSourceFiles.split():
        identifier, sourcePath = [t.strip() for t in sourceDescriptor.split ('=')]

        jsSource = readSourceFile (sourcePath)
        chunks = [jsSource [x:x + 1024] for x in range (0, len (jsSource), 1024)]

        jsSourceChunked = 'R"***(' + ')***" R"***('.join (chunks) + ')***"'

        jsSources [identifier] = jsSourceChunked

    nativeSource = readSourceFile (nativeSourceFile)

    for identifier, jsSource in jsSources.iteritems():
        nativeSource = nativeSource.replace (identifier, jsSource)
    
    o = open (outputPath, 'w')
    o.write (nativeSource)
    o.close()
        
if __name__ == "__main__":
    main()
