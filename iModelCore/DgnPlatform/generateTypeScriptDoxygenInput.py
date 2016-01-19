#!Python
#--------------------------------------------------------------------------------------
#
#     $Source: generateTypeScriptDoxygenInput.py $
#
#  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
#
#--------------------------------------------------------------------------------------
import getopt, sys, os, codecs, re



#  constructor(fn: (onFulfilled: (result: any) => void, onRejected: (result: any) => void) => void);

functionTypeDeclaration = r"((new\s)?\([^\(\)]*\))\s*=>\s*([^\(\);,{}]+)"
functionInterfaceDeclaration = r"^\s*\(([^\(\)]*)\)\s*:"

memberLines = r"^\s*(private|protected|)\s+(m_|__|private static|protected static).*$"
typeIds = r"^\s*private\s+.+TypeId\s*;$"

declareType = r"^\s*declare\s+type\s+.*$"
declareVar = r"^\s*declare\s+var\s+.*$"
declareFunction = r"^\s*declare\s+function\s+.*$"
privateLines = r"^\s*private\s+(.*)$"
buildCondLines = r"^\s*__buildcond__.*"
protectedLines = r"^\s*protected\s+(.*)$"
declareModule = r"declare\s+module\s"
typeLines = r"^\s*type\s+.*$"
varLines = r"^\s*var\s+.*$"
extendsImplements = r"\s+(extends|implements)\s+"
classDeclarations = r"^\s*(?:class|interface)\s+([^}]+)\s*{"
bracedDeclarations = r"{[^{}]+}[;\)]"
truncatedLines = r":\s*$"
defaultVoidLines = r"^(\s*[^\*\s].+\))\s*;$"
typeDeclaration = r"([^\(\){}\[\];,:\s\?]+)\s*(\?)?\s*:\s*([^\(\){};,:\s]+)"
functionLines = r"[\t ]*([^:\r\n]+)\s*:\s*([^:;\s]+)\s*;"
lambdas = r":\s*\([^\)]*\)\s*=>\s*[^\s,;\(\){}\[\]]*"
typedefs = r"^\s*type\s+(\S+)\s*=\s*(\S+)\s*;$"

#---------------------------------------------------------------------------------------
#  @bsimethod                                  Bentley.Systems
#---------------------------------------------------------------------------------------
def _transformDeclarationsToDoxygenList (declarations, indentLevel=0):
    indent = '    ' * indentLevel

    doxygenList = []

    commentsByParam = {}
    paramOrder = []

    declarationMatches = re.findall ('(.+?)?(/[/\*].+)', declarations)

    for declarationMatch in declarationMatches:
        directive, comment = declarationMatch
        directive = directive.strip()
            
        if directive:
            paramName = directive.split ('(', 1) [0].split() [-1].strip ('{}()\'";')
            
        if paramName not in commentsByParam:
            commentsByParam [paramName] = []

        comment = re.sub ('[@\\\\][^\s]+\s.+', '', comment)
        commentsByParam [paramName].append (comment.lstrip ('/*!< '))
        paramOrder.append (paramName)

    for param in paramOrder:
        comments = commentsByParam [param]
        isOptional = False
        isRequired = False

        for c, comment in enumerate (comments):
            if comment.lower().startswith ('optional:'):
                comments [c] = comment [9:].strip()
                isOptional = True

            if comment.lower().startswith ('required:'):
                comments [c] = comment [9:].strip()
                isRequired = True

        prefix = ''

        if isOptional:
            prefix = '`[Optional]` '

        if isRequired:
            prefix = '`[Required]` '

        doxygenList.append (indent + '- ' + prefix + '**' + param + '** ' + ('\n  ' + indent).join (comments))

    return '\n          * '.join (doxygenList) + '\n          *'

#---------------------------------------------------------------------------------------
#  @bsimethod                                  Bentley.Systems
#---------------------------------------------------------------------------------------
def transformDeclarationsToDoxygenList (source):
    transformDeclarationsMatches = re.findall ('( \# transformDeclarationsToDoxygenList \s+ (.+) )', source, re.X)
    for transformDeclarationsMatch in transformDeclarationsMatches:
        transformDirective, declarationsFileName = transformDeclarationsMatch
        declarationsFileName = declarationsFileName.strip()
        declarationsFileContent = readSourceFile (declarationsFileName)
        source = source.replace (transformDirective, _transformDeclarationsToDoxygenList (declarationsFileContent))

    transformDeclarationsMatches = re.findall ('( \# transformDeclarationsToDoxygenSubList \s+ (.+) )', source, re.X)
    for transformDeclarationsMatch in transformDeclarationsMatches:
        transformDirective, declarationsFileName = transformDeclarationsMatch
        declarationsFileName = declarationsFileName.strip()
        declarationsFileContent = readSourceFile (declarationsFileName)
        source = source.replace (transformDirective, _transformDeclarationsToDoxygenList (declarationsFileContent, 1))

    return source

#---------------------------------------------------------------------------------------
#  @bsimethod                                  Bentley.Systems
#---------------------------------------------------------------------------------------
def readSourceFile (path):
    return open (path).read().replace (codecs.BOM_UTF8, '')

#---------------------------------------------------------------------------------------
#  @bsimethod                                  Bentley.Systems
#---------------------------------------------------------------------------------------
def main():
    shortArgs = "i:o:e:"
    longArgs = ["inputPath=", "outputPath=", "extractionsPath="]

    try:
        opts, args = getopt.getopt (sys.argv [1:], shortArgs, longArgs)
    except getopt.GetoptError as err:
        print err
        sys.exit (2)

    inputPath = ""
    outputPath = ""
    extractionsPath = ""

    for o, a in opts:
        if o in ("-i", "--inputPath"):
            inputPath = a
        elif o in ("-o", "--outputPath"):
            outputPath = a
        elif o in ("-e", "--extractionsPath"):
            extractionsPath = a

    source = readSourceFile (inputPath)

    #matches = re.finditer (functionTypeDeclaration, source)
    #for m in matches:
    #    print m.groups()

    source = re.sub ("typeof ", "", source, 0, re.M)

    while '=>' in source:
        source = re.sub (functionTypeDeclaration, "FUNCTION_TYPE", source, 0, re.M)

    while re.search (bracedDeclarations, source, re.M) is not None:
        source = re.sub (bracedDeclarations, "", source, 0, re.M)

    source = re.sub (truncatedLines, ": any;", source, 0, re.M)

    source = re.sub (declareType, "", source, 0, re.M)
    source = re.sub (declareVar, "", source, 0, re.M)
    source = re.sub (declareFunction, "", source, 0, re.M)
    source = re.sub (varLines, "", source, 0, re.M)
    source = re.sub (memberLines, "", source, 0, re.M)
    source = re.sub (typeIds, "", source, 0, re.M)
    
    source = re.sub (functionInterfaceDeclaration, "operator() (\g<1>) :", source, 0, re.M)

    source = re.sub ("\?:", "=DEFAULT_VALUE:", source, 0, re.M)

    source = re.sub (typeDeclaration, "\g<3> \g<1>", source, 0, re.M)

    source = re.sub (defaultVoidLines, "\g<1>: void;", source, 0, re.M)
    
    source = re.sub (privateLines, "private:\n\g<1>\npublic:", source, 0, re.M)
    source = re.sub (protectedLines, "protected:\n\g<1>\npublic:", source, 0, re.M)
    
    source = re.sub (functionLines, "    \g<2> \g<1>;", source, 0, re.M)
    
    
    
    source = re.sub (extendsImplements, " : public ", source, 0, re.M)
    source = re.sub (classDeclarations, "class \g<1> {\npublic:\n", source, 0, re.M)

    source = re.sub (declareModule, "namespace ", source)

    source = re.sub (typedefs, "typedef \g<2> \g<1>;", source, 0, re.M)

    source = re.sub (buildCondLines, "", source, 0, re.M)

    source = re.sub ("}", "};", source, 0, re.M)

    source = re.sub ("bentley\.dgnclientfx", "BentleyApi.DgnClientFx.Js", source, 0, re.M)
    source = re.sub ("Bentley\.DgnClientFx", "BentleyApi.DgnClientFx", source, 0, re.M)
    source = re.sub ("namespace\s+bentley\s+", "namespace BentleyApi.DgnClientFx.Js ", source, 0, re.M)
    source = re.sub ("namespace\s+\S+\s+{\s*};", "", source, 0, re.M)

    source = re.sub ("declare function ", "", source, 0, re.M)

    source = re.sub ("/\*\*\s*___DOC_ONLY___", "", source, 0, re.M)
    source = re.sub ("___END_DOC_ONLY___\s*\*/", "", source, 0, re.M)

    source = source.replace ("DGNCLIENTFX_EXTRACTIONS", extractionsPath)
    source = transformDeclarationsToDoxygenList (source)

    source = re.sub (r"^\s*class\s*(\S+)(.*?)(void\s+constructor)", "class \g<1> \g<2> \g<1>", source, 0, re.S | re.M)

    source = re.sub (r"(\s*[\(,]\s*)\[(\w+)\]", "\g<1>\g<2>[]", source)

    output = source

    o = open (outputPath, 'w')
    o.write (output)
    o.close()
        
if __name__ == "__main__":
    main()
