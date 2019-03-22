#!Python
#--------------------------------------------------------------------------------------
#
#     $Source: BeJavaScript/tools/mergeJsSourceDeliveryFiles.py $
#
#  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
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
        chars = [repr (c).replace ('"\'"', "\'\\\'\'") for c in jsSource]
        chunks = [','.join (chars [x:x + 50]) for x in range (0, len (chars), 50)]
        jsSources [identifier] = '{' + ',\n    '.join (chunks) + (',', '')[len (chunks) == 0] + '\'\\0\'}'

    nativeSource = readSourceFile (nativeSourceFile)

    for identifier, jsSource in jsSources.iteritems():
        nativeSource = nativeSource.replace (identifier, jsSource)
    
    if os.path.isfile (outputPath):
        o = open (outputPath, 'r')
        if o.read() == nativeSource:
            return

    o = open (outputPath, 'w')
    o.write (nativeSource)
    o.close()
        
if __name__ == "__main__":
    main()
