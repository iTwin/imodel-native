#!Python
#--------------------------------------------------------------------------------------
#
#     $Source: DgnScriptContext/createNativeClassTypeScriptDeclaration.py $
#
#  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
#
#--------------------------------------------------------------------------------------
import getopt, sys, os, codecs, re

exportedClasses = r"BEJAVASCRIPT_EXPORT_CLASS\s+(?:struct|class)\s+([^{]+)"
exportedEnumClasses = r"BEJAVASCRIPT_EXPORT_CLASS\s+enum\s+class\s+([^\s:]+)\s*:\s*([^\s{]+)\s*([^;]+)"
declares = r"\(([^\)]*)\)\s*(?:const)?\s*(?:{[^}]*})?\s*;\s*BEJAVASCRIPT_DECLARE_(\S+)\s*\(\s*([^\)]+)\s*\)"
defines = r"BEJAVASCRIPT_DEFINE_(\S+)\s*\(\s*([^\)]+)\s*\)"
initializers = r"BEJAVASCRIPT_INSTANTIATE_(\S+)\s*\(\s*([^\)]+)\s*\)"
nativeLookups = r"BEJAVASCRIPT_GET_JS_OBJECT_FUNCTION_PROPERTY\s*\(([^\,]+),([^\)]+)"
customDeclarationsPattern = r"BEJAVASCRIPT_EMIT_CUSTOM_TYPESCRIPT_DECLARATION\s*\(\s*\"([^\"]+)"
customMemberDeclarationsPattern = r"BEJAVASCRIPT_EMIT_CUSTOM_TYPESCRIPT_MEMBER_DECLARATION\s*\(([^\,]+),([^;]+)"
jsDefines = r"BeJavaScript.Define(Instance|Static)Property[^\"]+\"([^\"]+)\"[^\"]+\"([^\"]+)\"\s*([^{;]+)"
literalFunction = r"function\s*\(([^\)]*)\s*(.+)"
literalValue = r"^(\S+)\s*/\*\s*:\s*([^\s\*]+)"
enumLeftShiftedValue = r"(\S+)\s*<<\s*(\S+)\s*[,}]"

output = []
declarations = {}
declarationSignatures = {}
customMemberDeclarations = {}
customDeclarations = []

declaredNames = []
definedNames = []
initializedNames = []
jsInitializedNames = []
queriedNames = []

#---------------------------------------------------------------------------------------
#  @bsimethod                                  Bentley.Systems
#---------------------------------------------------------------------------------------
def getJsName (nativeName):
    if nativeName.endswith ("Ptr"):
        nativeName = nativeName [:-3]

    return nativeName.replace ("const ", "").replace (" const", "").rstrip ('RCP&* ')

#---------------------------------------------------------------------------------------
#  @bsimethod                                  Bentley.Systems
#---------------------------------------------------------------------------------------
def parseSourceFile (path):
    global output

    source = open (path).read().replace (codecs.BOM_UTF8, '')
    
    classMatches = re.finditer (exportedClasses, source)
    for match in classMatches:
        className = match.group (1)
        superName = ""
        
        if ':' in className:
            if ',' in className:
                raise Exception ("Multiple inheritance is not supported (" + className + ")")

            classNameTokens = className.split()
            className = classNameTokens [0]
            superName = classNameTokens [-1]
        else:
            className = className.strip()

        if className not in declarations:
            declarations [className] = { "type": "class",
                                         "super": superName,
                                         "staticFunctions": [],
                                         "instanceFunctions": [],
                                         "staticPropertyGetters": [],
                                         "staticPropertySetters": [],
                                         "instancePropertyGetters": [],
                                         "instancePropertySetters": [],
                                         "staticProperties": [],
                                         "instanceProperties": [],
                                         "signatures": {} }

    enumMatches = re.finditer (exportedEnumClasses, source)
    for match in enumMatches:
        className = match.group (1)
        values = match.group (3)

        for match in re.finditer (enumLeftShiftedValue, values):
            values = values.replace (match.group (0), str (int (match.group (1)) << int (match.group (2))) + ',')

        if '}' not in values:
            values += '\n    }'

        declarations [className] = { "type": "enum", "values": values }

    declareMatches = re.finditer (declares, source)
    for match in declareMatches:
        descriptor = match.group (2)
        params = [p.strip() for p in match.group (3).split (',')]
        className = params [0]
        memberName = params [1]
        declaredNames.append (className + "::" + memberName)
        declarationSignatures [className + "::" + memberName] = match.group (1)
    
    defineMatches = re.finditer (defines, source)
    for match in defineMatches:
        descriptor = match.group (1)
        params = [p.strip() for p in match.group (2).split (',')]
        className = params [0]
        memberName = params [1]
        isVoid = "_VOID" in descriptor
        isObject = "_OBJECT" in descriptor or "_NULLABLEOBJECT" in descriptor
        hasArgs = "ARG" in descriptor
        returnType = isVoid and "void" or params [2]
        arguments = isVoid and params [2:] or params [4:]

        if isObject:
            arguments = params [3:]
            returnType = getJsName (returnType)
        else:
            returnType = returnType.lower()

        argCount = len (arguments) / 3

        argumentTypes = []
        for a in range (argCount):
            jsType = arguments [a * 3 + 1].lower()
            
            nativeType = arguments [a * 3]

            if jsType == "nativepointer" or nativeType in declarations:
                jsType = getJsName (nativeType)

            argumentTypes.append (jsType)

        declaredNativeSignature = declarationSignatures [className + "::" + memberName]
        argumentNames = []
        if declaredNativeSignature:
            nativeArguments = [a.strip() for a in declaredNativeSignature.split(',')]
            for a in nativeArguments:
                argumentNames.append (a.split() [1])

        declarations [className] ["signatures"] [memberName] = { "return": returnType, "argumentTypes": argumentTypes, "argumentNames": argumentNames }

        definedNames.append (className + "::" + memberName)

    initializerMatches = re.finditer (initializers, source)
    for match in initializerMatches:
        descriptor = match.group (1)
        
        if descriptor in ("JS_CALLBACKS_HOLDER"):
            continue

        params = [p.strip() for p in match.group (2).split (',')]
        className = params [0]
        memberName = params [1]
        isStatic = "STATIC_" in descriptor

        if "GET_PROPERTY_CALLBACK" in descriptor:
            declarations [className] [isStatic and "staticPropertyGetters" or "instancePropertyGetters"].append (memberName)
            initializedNames.append (className + '::Get' + memberName)
        elif "SET_PROPERTY_CALLBACK" in descriptor:
            declarations [className] [isStatic and "staticPropertySetters" or "instancePropertySetters"].append (memberName)
            initializedNames.append (className + '::Set' + memberName)
        elif "PROPERTY_CALLBACKS" in descriptor:
            declarations [className] [isStatic and "staticPropertyGetters" or "instancePropertyGetters"].append (memberName)
            declarations [className] [isStatic and "staticPropertySetters" or "instancePropertySetters"].append (memberName)
            initializedNames.append (className + '::Get' + memberName)
            initializedNames.append (className + '::Set' + memberName)
        elif "FUNCTION_CALLBACK" in descriptor:
            declarations [className] [isStatic and "staticFunctions" or "instanceFunctions"].append (memberName)
            initializedNames.append (className + "::" + memberName)
    
    jsDefineMatches = re.finditer (jsDefines, source)
    for match in jsDefineMatches:
        className = match.group (2)
        memberName = match.group (3)
        value = match.group (4).rstrip (')').lstrip (', \r\n\t')
        isStatic = match.group (1) == "Static"
        argumentTypes = []
        argumentValues = []
        functionMatch = re.search (literalFunction, value)
        valueMatch = re.search (literalValue, value)

        if functionMatch is not None:
            returnType = functionMatch.group (2).strip ('\r\n\t /\*():')
            argumentPairs = [[k.strip ('\r\n\t /\*():') for k in a.split (':')] for a in [a.strip() for a in functionMatch.group (1).split(',')]]

            argumentTypes = []
            argumentNames = []

            if functionMatch.group (1):
                argumentTypes = [a [1] for a in argumentPairs]
                argumentNames = [a [0] for a in argumentPairs]

            declarations [className] [isStatic and "staticFunctions" or "instanceFunctions"].append (memberName)
            declarations [className] ["signatures"] [memberName] = { "return": returnType, "argumentTypes": argumentTypes, "argumentNames": argumentNames }
        elif valueMatch is not None:
            declarations [className] [isStatic and "staticProperties" or "instanceProperties"].append (memberName)
            declarations [className] ["signatures"] [memberName] = { "return": valueMatch.group (2) }

        jsInitializedNames.append (className + "::" + memberName)

    nativeLookupMatches = re.finditer (nativeLookups, source)
    for match in nativeLookupMatches:
        className = match.group (1).strip ('\r\n\t \'"')
        memberName = match.group (2).strip ('\r\n\t \'"')
        queriedNames.append (className + "::" + memberName)

    customMemberDeclarationMatches = re.finditer (customMemberDeclarationsPattern, source)
    for match in customMemberDeclarationMatches:
        className = match.group (1).strip ('\r\n\t \'"')
        declaration = match.group (2).strip ('\r\n\t \'"')

        if className not in customMemberDeclarations:
            customMemberDeclarations [className] = []

        customMemberDeclarations [className].append (declaration)

    customDeclarationMatches = re.finditer (customDeclarationsPattern, source)
    for match in customDeclarationMatches:
        declaration = match.group (1)

        customDeclarations.append (declaration)

#---------------------------------------------------------------------------------------
#  @bsimethod                                  Bentley.Systems
#---------------------------------------------------------------------------------------
def createTsFunctionSignature (d):
    s = []
    for n, t in zip (d ["argumentNames"], d ["argumentTypes"]):
        s.append (n + " : " + t)
    
    return (len(s) and ' ' or '') + '(' + ", ".join (s) + ') : ' + d ["return"]

#---------------------------------------------------------------------------------------
#  @bsimethod                                  Bentley.Systems
#---------------------------------------------------------------------------------------
def main():
    shortArgs = "s:o:n:"
    longArgs = ["sources=", "output=", "namespace="]

    try:
        opts, args = getopt.getopt (sys.argv [1:], shortArgs, longArgs)
    except getopt.GetoptError as err:
        print err
        sys.exit (2)

    sourcePaths = ""
    outputPath = ""
    namespace = ""

    for o, a in opts:
        if o in ("-o", "--output"):
            outputPath = a
        elif o in ("-s", "--sources"):
            sourcePaths = a
        elif o in ("-n", "--namespace"):
            namespace = a

    output.append ("module " + namespace + " {\n")

    for sourcePath in sourcePaths.strip().split():
        parseSourceFile (sourcePath)

    for name in declaredNames:
        if name not in definedNames:
            raise Exception (name + " callback is not defined.")

    for name in definedNames:
        if name not in declaredNames:
            raise Exception (name + " callback is not declared.")

        if name not in initializedNames:
            raise Exception (name + " callback is not initialized.")

    for name in initializedNames:
        if name not in definedNames:
            raise Exception (name + " callback is not defined.")

    for name in queriedNames:
        if name not in initializedNames and name not in jsInitializedNames:
            raise Exception (name + " callback is not initialized.")

    for declaration in customDeclarations:
        output.append (declaration + "\n")

    for className, classDefinition in declarations.iteritems():
        if classDefinition ["type"] == "enum":
            output.append ("export enum " + className)
            output.append ("    " + classDefinition ["values"])
            output.append ('')
        elif classDefinition ["type"] == "class":
            superDeclaration = ""
            if classDefinition ["super"] != "":
                superDeclaration = " extends " + classDefinition ["super"]

            output.append ("export declare class " + className + superDeclaration)
            output.append ("    {")
            output.append ("    private " + className + "TypeId : any;")

            if className in customMemberDeclarations:
                for declaration in customMemberDeclarations [className]:
                    output.append ("    " + declaration + ';')

            for f in classDefinition ["staticFunctions"]:
                d = classDefinition ["signatures"] [f]
                output.append ("    public static " + f + createTsFunctionSignature (d) + ";")

            for f in classDefinition ["instanceFunctions"]:
                d = classDefinition ["signatures"] [f]
                output.append ("    public " + f + createTsFunctionSignature (d) + ";")

            for f in classDefinition ["staticPropertyGetters"]:
                d = classDefinition ["signatures"] ["Get" + f]
                output.append ("    public static " + f + " : " + d ["return"] + ";")

            #for f in classDefinition ["staticPropertySetters"]:
            #    d = classDefinition ["signatures"] ["Set" + f]
            #    output.append ("    public static set " + f + " (" + d ["argumentNames"] [0] + " : " + d ["argumentTypes"] [0] + ");")

            for f in classDefinition ["instancePropertyGetters"]:
                d = classDefinition ["signatures"] ["Get" + f]
                output.append ("    public " + f + " : " + d ["return"] + ";")

            #for f in classDefinition ["instancePropertySetters"]:
            #    d = classDefinition ["signatures"] ["Set" + f]
            #    output.append ("    public set " + f + " (" + d ["argumentNames"] [0] + " : " + d ["argumentTypes"] [0] + ");")

            for f in classDefinition ["staticProperties"]:
                d = classDefinition ["signatures"] [f]
                output.append ("    public static " + f + " : " + d ["return"] + ";")

            for f in classDefinition ["instanceProperties"]:
                d = classDefinition ["signatures"] [f]
                output.append ("    public " + f + " : " + d ["return"] + ";")

            output.append ("    }\n")

            declareSuper = classDefinition ["super"] and (namespace + "." + classDefinition ["super"]) or ""
            output.append ("BeJavaScript.DeclareClass (\"" + namespace + "." + className + "\", \"" + declareSuper + "\");\n")

    output.append ("\n}")
    
    o = open (outputPath, 'w')
    o.write ('\n'.join (output))
    o.close()
        
if __name__ == "__main__":
    main()
