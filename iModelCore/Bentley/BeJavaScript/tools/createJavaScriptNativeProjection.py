#!Python
#--------------------------------------------------------------------------------------
#
#  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
#
#--------------------------------------------------------------------------------------
import getopt, sys, os, codecs, re, tempfile, subprocess, shlex

nativeOutput = []
nativeHeaderOutputScoped = []
nativeHeaderOutputUnscoped = []
nativeHeaderOutputUnscopedHelper = []
nativeSources = {}
tsOutput = []
globalModule = None
classesPendingInitialize = {}
forceBuiltInTsTypes = False
suppressTypeAliases = False
suppressCoreTypeAliases = False
suppressingTypeAliases = False
lastTypeId = -1
initializedTypes = []
projectionTypeName = ""
exportMacroName = ""
projectionIndex = 0
refCountedBaseType = ""
bentleyName = "Bentley"
modulesForTsOutput = {}
forwardDeclaresRegistry = {}
parseTargetIsExternal = False

nativeLib = '''

'''

#---------------------------------------------------------------------------------------
#  @bsimethod                                  Bentley.Systems
#---------------------------------------------------------------------------------------
def getNativeName (declarationBody, declaredName):
    aliases = re.finditer (r"/\*\*\*\s*NATIVE_TYPE_NAME\s*=\s*([^\s\*]+)\s*\*\*\*/", declarationBody)
    for a in aliases:
        return a.group (1)

    return declaredName

#---------------------------------------------------------------------------------------
#  @bsimethod                                  Bentley.Systems
#---------------------------------------------------------------------------------------
def getFileContents (path):
    return open (path).read().replace (codecs.BOM_UTF8, '')

#---------------------------------------------------------------------------------------
#  @bsimethod                                  Bentley.Systems
#---------------------------------------------------------------------------------------
def createGeneratorMessage (message):
    return "/*********** " + message + " ***********/"

#---------------------------------------------------------------------------------------
#  @bsimethod                                  Bentley.Systems
#---------------------------------------------------------------------------------------
def dispatchTsTypeLookup (name, source):
    if name in ("void", "number", "string", "boolean", "any"):
        return name

    resolvedName = name

    if '.' in name:
        if '<' in name:
            nameTokens = re.search (r"([^<]+)<([^>]+)>", name)

            namespace, localName = nameTokens.group (2).rsplit ('.', 1)
            localName = nameTokens.group (1) + '<' + localName + '>'
        else:
            namespace, localName = name.rsplit ('.', 1)

        resolver = Module.registry [namespace][-1]

        for m in reversed (Module.registry [namespace]):
            if m.canEnhanceTsTypeRepresentation (localName):
                resolver = m
                break

        resolvedName = resolver.createTsTypeRepresentation (localName)
    else:
        resolvedName = source.holder.createTsTypeRepresentation (name)

    if forceBuiltInTsTypes:
        return resolvedName

    if '<' in name:
        if "Bentley_BeJsArray" in name:
            return resolvedName + '>'

        return resolvedName + " /*" + name + "*/"
    else:
        return name

#---------------------------------------------------------------------------------------
#  @bsimethod                                  Bentley.Systems
#---------------------------------------------------------------------------------------
def dispatchConversionLookup (name, source):
    if '.' in name:
        if '<' in name:
            nameTokens = re.search (r"([^<]+)<([^>]+)>", name)

            namespace, localName = nameTokens.group (2).rsplit ('.', 1)
            localName = nameTokens.group (1) + '<' + localName + '>'
        else:
            namespace, localName = name.rsplit ('.', 1)
        
        converter = Module.registry [namespace][-1]

        for m in reversed (Module.registry [namespace]):
            if m.canEnhanceConversion (localName):
                converter = m
                break

        return converter.getConversion (localName)
    else:
        return source.holder.getConversion (name)

#---------------------------------------------------------------------------------------
#  @bsiclass                                  Bentley.Systems
#---------------------------------------------------------------------------------------
class FunctionArgument:
    def __init__ (self, name, type, holder):
        self.name = name
        self.type = type
        self.holder = holder
        self.external = parseTargetIsExternal

    def createTsRepresentation (self):
        return self.name + " : " + dispatchTsTypeLookup (self.type, self.holder)

    def getConvertToCxxRepresentation (self, params):
        c = dispatchConversionLookup (self.type, self.holder)

        return c.resolveCxx (self.type, params, self.holder)

#---------------------------------------------------------------------------------------
#  @bsiclass                                  Bentley.Systems
#---------------------------------------------------------------------------------------
class Property:
    def __init__ (self, name, returnType, comment, static, holder):
        self.name = name
        self.returnType = returnType
        self.static = static
        self.holder = holder
        self.comment = comment
        self.external = parseTargetIsExternal

        self.getter = Function ("Get" + self.name, '', self.returnType, '', self.static, self.holder, False)
        self.setter = Function ("Set" + self.name, "value : " + self.returnType, "void", '', self.static, self.holder, False)

        self.getter.suppressTs = True
        self.getter.isPropertyCallback = True
        self.setter.suppressTs = True
        self.setter.isPropertyCallback = True

    def generateStaticCode (self):
        if self.external:
            return

        self.getter.generateStaticCode()
        self.setter.generateStaticCode()

        isClassProperty = isinstance (self.holder, Class)

        tsPrefix = self.static and isClassProperty and "static " or ""

        if not isClassProperty:
            tsPrefix = "declare property "

            if isinstance (self.holder, Module) and self.holder.name:
                tsPrefix = "export declare property "

        tsReturn = dispatchTsTypeLookup (self.returnType, self)
        tsIndent = isClassProperty and "    " or ""
        if self.comment and 0 != len(self.comment):
            tsOutput.append (tsIndent + self.comment)
        tsOutput.append (tsIndent + tsPrefix + self.name + " : " + tsReturn + ";")

        if not isClassProperty:
            tsOutput.append ("")

    def generateRuntimeCode (self):
        if self.external:
            return

        self.getter.generateRuntimeCode()
        self.setter.generateRuntimeCode()

        if self.static:
            nativeOutput.append ("    " + self.holder.getCallbacksObjectName() + ".SetProperty (\"Static GetProperty " + self.name + "\", BeJsNativeFunction (m_context, &" + self.getter.getNativeCallbackName() + "));")
            nativeOutput.append ("    " + self.holder.getCallbacksObjectName() + ".SetProperty (\"Static SetProperty " + self.name + "\", BeJsNativeFunction (m_context, &" + self.setter.getNativeCallbackName() + "));")
        else:
            nativeOutput.append ("    " + self.holder.getCallbacksObjectName() + ".SetProperty (\"Instance GetProperty " + self.name + "\", BeJsNativeFunction (m_context, &" + self.getter.getNativeCallbackName() + "));")
            nativeOutput.append ("    " + self.holder.getCallbacksObjectName() + ".SetProperty (\"Instance SetProperty " + self.name + "\", BeJsNativeFunction (m_context, &" + self.setter.getNativeCallbackName() + "));")

#---------------------------------------------------------------------------------------
#  @bsiclass                                  Bentley.Systems
#---------------------------------------------------------------------------------------
class Function:
    def __init__ (self, name, arguments, returnType, comment, static, holder, public):
        if name == "constructor" and not static and isinstance (holder, Class):
            if "BeJsProjection_RefCounted" in holder.implements:
                returnType = "Bentley_RefCountedPtr<" + holder.name + ">"
            else:
                returnType = "cxx_owner_pointer<" + holder.name + ">"

        self.name = name
        self.arguments = arguments
        self.returnType = returnType
        self.static = static
        self.holder = holder
        self.suppressTs = False
        self.suppressNative = False
        self.isPropertyCallback = False
        self.comment = comment
        self.external = parseTargetIsExternal
        self.public = public

        if self.public:
            pass

        if name in ["OnDispose"]:
            self.suppressNative = True

    def getQualifiedName (self):
        return self.holder.getQualifiedName() + '.' + self.name

    def getQualifiedNameIdentifier (self):
        return self.getQualifiedName().replace ('.', '_')

    def createTsTypeRepresentation (self, type):
        return self.holder.createTsTypeRepresentation (type)

    def getConversion (self, type):
        return self.holder.getConversion (type)

    def resolveTypeName (self, type):
        return self.holder.resolveTypeName (type)

    def getNativeCallbackName (self):
        return self.getQualifiedName().replace ('.', '_') + "JsCallback"

    def isConstructor (self):
        return self.name == "constructor" and not self.static and isinstance (self.holder, Class)

    def generateStaticCode (self):
        if self.external:
            return

        global refCountedBaseType, projectionTypeName

        arguments = []
        declaredArgs = self.arguments and [a.strip() for a in self.arguments.split (',')] or []
        for a in declaredArgs:
            name, type = [t.strip() for t in a.split (':')]
            arguments.append (FunctionArgument (name, type, self))
        
        isClassFunction = isinstance (self.holder, Class) or isinstance (self.holder, Interface)

        if not self.suppressTs:
            tsPrefix = self.static and isClassFunction and "static " or ""

            if not isClassFunction:
                tsPrefix = "declare function "

                if isinstance (self.holder, Module) and self.holder.name:
                    tsPrefix = "export declare function "
        
            tsArguments = []
            for a in arguments:
                tsArguments.append (a.createTsRepresentation())

            tsReturn = dispatchTsTypeLookup (self.returnType, self)
            tsIndent = isClassFunction and "    " or ""
            tsArgumentsSpace = len (arguments) and " " or ""

            if tsReturn != '':
                tsReturn = " : " + tsReturn

            if self.isConstructor():
                tsReturn = ""
                tsArguments.append ("_customPrototype ?: any")

            if self.comment and 0 != len(self.comment):
                tsOutput.append (tsIndent + self.comment);
            tsOutput.append (tsIndent + tsPrefix + self.name + tsArgumentsSpace + "(" + ', '.join (tsArguments) + ")" + tsReturn + ";")

            if not isClassFunction:
                tsOutput.append ("")

        if not self.suppressNative and not self.public:
            nativeOutput.append ("static BentleyApi::BeJsValue " + self.getNativeCallbackName() + " (BeJsNativeFunction::CallbackInfoR _callbackInfo)")
            nativeOutput.append ("    {")

            output = []

            nativeReturn = self.returnType == "void" and "void" or dispatchConversionLookup (self.returnType, self)
            isDispose = not self.static and nativeReturn == "void" and self.name == "Dispose"

            if not self.static:
                output.append ("    BentleyApi::BeJsNativePointer _jsObject = _callbackInfo.GetCallContext();")
                holderName = self.holder.getQualifiedNameNative()

                if self.isConstructor():
                    output.append ("\n    BentleyApi::BeJsObject _customPrototype;")
                    output.append ("    if (_callbackInfo.GetArgumentCount() > " + str (len (arguments)) + ")")
                    output.append ("        _customPrototype = _callbackInfo.GetObjectArgument (" + str (len (arguments)) + ");\n")

                if not self.isConstructor() and not isDispose:
                    output.append ("    " + holderName + "* _nativeObject = _jsObject.GetValueTyped<" + holderName + ">();\n")
                    output.append ("    if (_nativeObject == nullptr)\n        return BentleyApi::BeJsValue::Null (_context);\n")

            for i, a in enumerate (arguments):
                params = ConvertParams (a.name, "_callbackInfo.Get__type__Argument (" + str (i) + ")", "_context")
                output.append ("    " + a.getConvertToCxxRepresentation (params).replace ('\n', "\n    "))

            nativeArguments = ", ".join ([a.name for a in arguments])
            nativeArgumentsSpace = len (arguments) and " " or ""
            nativeCall = self.name + nativeArgumentsSpace + "(" + nativeArguments + ")"
            
            if nativeReturn == "void":
                if isDispose:
                    output.append ("    _jsObject.Dispose();\n");
                else:
                    if self.static:
                        output.append ("    " + self.holder.getQualifiedNameNative() + "::" + nativeCall + ";\n")
                    else:
                        output.append ("    _nativeObject->" + nativeCall + ";\n")

                output.append ("    return BentleyApi::BeJsValue::Undefined (_context);")
            else:
                resolvedReturnType = self.resolveTypeName (self.returnType)

                if self.static:
                    output.append ("    " + nativeReturn.resolveCxxName (resolvedReturnType, self) + " _nativeValue = " + self.holder.getQualifiedNameNative() + "::" + nativeCall + ";")
                else:
                    call = "_nativeObject->" + nativeCall
                    if self.isConstructor():
                        if "BeJsProjection_RefCounted" in self.holder.implements:
                            call = refCountedBaseType + "::Create<" + self.holder.getQualifiedNameNative() + ">" + nativeArgumentsSpace + "(" + nativeArguments + ")"
                        else:
                            call = "new " + self.holder.getQualifiedNameNative() + nativeArgumentsSpace + "(" + nativeArguments + ")"

                    output.append ("    " + nativeReturn.resolveCxxName (resolvedReturnType, self) + " _nativeValue = " + call + ";")

                params = ConvertParams ("_jsValue", "_nativeValue", "_context")
                output.append ("    " + nativeReturn.resolveJs (self.returnType, params, self).replace ('\n', "\n    ") + '\n')
                output.append ("    return _jsValue;")

            output.append ("    }\n")

            output = '\n'.join (output)

            if self.isConstructor():
                output = output.replace ("__customPrototype__", "_customPrototype.IsValid() ? &_customPrototype : nullptr")
            else:
                output = output.replace ("__customPrototype__", "nullptr")

            if "_context" in output:
                output= "    BentleyApi::BeJsContextCR _context = _callbackInfo.GetContext();\n" + output

            nativeOutput.append (output)

    def generateRuntimeCode (self):
        if self.external:
            return

        if self.public:
            return

        if not self.isPropertyCallback:
            if self.static:
                nativeOutput.append ("    " + self.holder.getCallbacksObjectName() + ".SetProperty (\"Static Function " + self.name + "\", BeJsNativeFunction (m_context, &" + self.getNativeCallbackName() + "));")
            else:
                if self.name in ["OnDispose"]:
                    nativeOutput.append ("    " + self.holder.getCallbacksObjectName() + ".SetProperty (\"Instance Function " + self.name + "\", BeJsValue::Null (m_context));")
                else:
                    nativeOutput.append ("    " + self.holder.getCallbacksObjectName() + ".SetProperty (\"Instance Function " + self.name + "\", BeJsNativeFunction (m_context, &" + self.getNativeCallbackName() + "));")

#---------------------------------------------------------------------------------------
#  @bsiclass                                  Bentley.Systems
#---------------------------------------------------------------------------------------
class ConvertParams:
    def __init__ (self, name, object, context):
        self.name = name
        self.object = object
        self.context = context

#---------------------------------------------------------------------------------------
#  @bsiclass                                  Bentley.Systems
#---------------------------------------------------------------------------------------
class Class:
    nativeNameRegistry = {}

    def __init__ (self, name, comment, body, holder):
        self.functions = {}
        self.properties = {}
        self.initialized = False
        self.implements = []
        self.typeId = -1
        self.descendants = []

        self.name = name
        self.comment = comment
        self.holder = holder
        self.external = parseTargetIsExternal

        self.nativeName = getNativeName (body, name)
        Class.nativeNameRegistry [self.getQualifiedName().replace ('$', "::").replace ('.', "::")] = self.getQualifiedNameNative()

        extendsMatch = re.search (r"extends\s+([^{]+)", body, re.S)
        self.extends = extendsMatch and extendsMatch.group (1).strip() or None

        if self.extends is not None and re.search (r"\simplements\s", self.extends) is not None:
            self.extends, implements = [t.strip() for t in re.split (r"\simplements\s", self.extends)]
            self.implements = [t.strip() for t in implements.split (',')]

        if extendsMatch is None:
            implementsMatch = re.search (r"implements\s+([^{]+)", body, re.S)
            if implementsMatch is not None:
                self.implements = [t.strip() for t in implementsMatch.group (1).split (',')]

        self.isInterface = "BeJsProjection_IsInterface" in self.implements

        functions = re.finditer (r"^\s*(/\*\*\s(?:.(?!\*/))*.\*/\s*)?(public\s)?\s*(static\s)?\s*(\S+)\s*\(([^\)]*)\)\s*:\s*(\S+)\s*;", body, re.M|re.S)
        for match in functions:
            if match.group (3) is None:
                self.functions [match.group (4)] = Function (match.group (4), match.group (5), match.group (6), match.group(1), False, self, match.group (2) is not None)
            else:
                self.functions [match.group (4)] = Function (match.group (4), match.group (5), match.group (6), match.group(1), True, self, match.group (2) is not None)

        properties = re.finditer (r"^\s*(/\*\*\s(?:.(?!\*/))*.\*/\s*)?(?:public\s)?\s*(static\s)?\s*([^\s\(\):]+)\s*:\s*(\S+)\s*;", body, re.M|re.S)
        for match in properties:
            if match.group (2) is None:
                self.properties [match.group (3)] = Property (match.group (3), match.group (4), match.group(1), False, self)
            else:
                self.properties [match.group (3)] = Property (match.group (3), match.group (4), match.group(1), True, self)

        constructorMatch = re.search (r"^\s*(?:public\s)?\s*constructor\s*\(([^\)]*)\)\s*;", body, re.M)
        if constructorMatch is not None:
            self.functions ["constructor"] = Function ("constructor", constructorMatch.group (1), "", "", False, self, False)

        if "BeJsProjection_SuppressConstructor" not in self.implements and "constructor" not in self.functions:
            self.functions ["constructor"] = Function ("constructor", "", "", "", False, self, False)

    def getQualifiedName (self):
        return self.holder.getQualifiedName() + '.' + self.name

    def getQualifiedNameNative (self):
        return self.holder.getQualifiedNameNative() + '::' + self.nativeName.replace ('$', "::")

    def getQualifiedNameIdentifier (self):
        return self.getQualifiedName().replace ('.', '_')

    def getBaseQualifiedName (self):
        base = self.extends or ""
        if base and '.' not in base:
            base = self.holder.getQualifiedName() + '.' + base
        return base

    def createTsTypeRepresentation (self, type):
        return self.holder.createTsTypeRepresentation (type)

    def getConversion (self, type):
        return self.holder.getConversion (type)

    def resolveTypeName (self, type):
        return self.holder.resolveTypeName (type)

    def generateFirstPassStaticCode (self):
        if self.external:
            return

        nativeName = self.getQualifiedNameNative()

        nativeOutput.append ("static void " + self.getQualifiedNameIdentifier() + "DisposeCallback (void* o, BentleyApi::BeJsObject* jsObject)\n    {")
            
        if "BeJsProjection_RefCounted" in self.implements:
            nativeOutput.append ("    if (o == nullptr)\n        return;\n")
            nativeOutput.append ("    " + nativeName + "* object = static_cast<" + nativeName + "*>(o);\n");

            if "BeJsProjection_SuppressBeProjectedCasts" not in self.implements:
                nativeOutput.append ("    if (jsObject != nullptr)\n        {")
                nativeOutput.append ("        jsObject->CallMemberFunction (\"OnDispose\");\n");
                nativeOutput.append ("        auto objectAsProjected = dynamic_cast<BentleyApi::BeProjected*>(object);\n        if (objectAsProjected != nullptr)\n            objectAsProjected->OnDispose();")
                nativeOutput.append ("        }\n")

            nativeOutput.append ("    object->Release();");

        nativeOutput.append ("    }\n");

        nativeOutput.append ("static void " + self.getQualifiedNameIdentifier() + "DisposeCallbackOwner (void* o, BentleyApi::BeJsObject* jsObject)\n    {")
        nativeOutput.append ("    if (o == nullptr)\n        return;\n")

        if "BeJsProjection_RefCounted" in self.implements or "BeJsProjection_SuppressDisposeDelete" not in self.implements:
            nativeOutput.append ("    " + nativeName + "* object = static_cast<" + nativeName + "*>(o);\n");

        if "BeJsProjection_SuppressBeProjectedCasts" not in self.implements:
            nativeOutput.append ("    if (jsObject != nullptr)\n        {")
            nativeOutput.append ("        jsObject->CallMemberFunction (\"OnDispose\");\n");
            nativeOutput.append ("        auto objectAsProjected = dynamic_cast<BentleyApi::BeProjected*>(object);\n        if (objectAsProjected != nullptr)\n            objectAsProjected->OnDispose();")
            nativeOutput.append ("        }\n")

        if "BeJsProjection_RefCounted" in self.implements:
            nativeOutput.append ("    object->Release();");
        elif "BeJsProjection_SuppressDisposeDelete" not in self.implements:
            nativeOutput.append ("    delete object;");

        nativeOutput.append ("    }\n");

        nativeOutput.append ("static void " + self.getQualifiedNameIdentifier() + "InstanceCreatedCallback (BentleyApi::BeJsContext const& context, void* o)\n    {")
            
        if "BeJsProjection_RefCounted" in self.implements:
            nativeOutput.append ("    if (o == nullptr)\n        return;\n")
            nativeOutput.append ("    " + nativeName + "* object = static_cast<" + nativeName + "*>(o);");
            nativeOutput.append ("    object->AddRef();");

        nativeOutput.append ("    }\n");

    def generateStaticCode (self):
        if self.external:
            return

        global modulesForTsOutput

        name = self.name
        for e in self.holder.exports.itervalues():
            if e.target == name:
                name = e.name

        output = []
        moduleName = None

        if self.comment and 0 != len(self.comment):
            output.append (self.comment)

        if '$' in name:
            moduleName, name = name.rsplit ('$', 1)
            if moduleName not in modulesForTsOutput:
                modulesForTsOutput [moduleName] = []

        tsExtends = self.extends and (" extends " + self.extends) or ""
        output.append ("export declare class " + name + tsExtends + "\n    {")

        if not self.isInterface:
            output.append ("    private " + name + "TypeId : any;")

        tsOutputMarker = len (tsOutput)

        for f in self.functions.itervalues():
            f.generateStaticCode()

        for p in self.properties.itervalues():
            p.generateStaticCode()

        if len (tsOutput) != tsOutputMarker:
            output.append ('\n'.join (tsOutput [tsOutputMarker:]))
            del tsOutput [tsOutputMarker:]

        output.append ("    };\n")

        if moduleName is not None:
            modulesForTsOutput [moduleName].append ('\n'.join (output))
        else:
            tsOutput.append ('\n'.join (output))

    def getCallbacksObjectName (self):
        return self.getQualifiedNameIdentifier()

    def getPrototypeObjectName (self):
        return self.getQualifiedNameIdentifier() + "Prototype"

    def initialize (self):
        global classesPendingInitialize, lastTypeId, initializedTypes

        if self.initialized:
            return

        base = self.getBaseQualifiedName()
        if base in classesPendingInitialize:
            classesPendingInitialize [base].initialize()

        lastTypeId = lastTypeId + 1
        self.typeId = lastTypeId;

        if not self.external:
            nativeHeaderOutputScoped.append ("    BeJsObject " + self.getQualifiedNameIdentifier() + ";")
            #nativeHeaderOutputScoped.append ("    static void " + self.getQualifiedNameIdentifier() + "DisposeCallback (void* object);")
            #nativeHeaderOutputScoped.append ("    static void " + self.getQualifiedNameIdentifier() + "DisposeCallbackOwner (void* object);")
            #nativeHeaderOutputScoped.append ("    static void " + self.getQualifiedNameIdentifier() + "InstanceCreatedCallback (BentleyApi::BeJsContext const& context, void* instance);")
            nativeOutput.append ("    " + self.getQualifiedNameIdentifier() + " = _initializeClass (BeJsString (m_context, \"" + self.getQualifiedName().replace ('$', '.') + "\"), " + self.getCallbacksObjectName() + ");");
        
        initializedTypes.append (self)

        self.initialized = True

    def generateRuntimeCode (self):
        global classesPendingInitialize

        if not self.external:
            nativeOutput.append ("    _declareClass (BeJsString (m_context, \"" + self.getQualifiedName().replace ('$', '.') + "\"), BeJsString (m_context, \"" + self.getBaseQualifiedName().replace ('$', '.') + "\"));");
        
        classesPendingInitialize [self.getQualifiedName()] = self

        for f in self.functions.itervalues():
            f.generateRuntimeCode()

        for p in self.properties.itervalues():
            p.generateRuntimeCode()

        if not self.external:
            nativeOutput.append ("")

    def generateHelperCode (self):
        if self.external:
            return

        global projectionIndex, projectionTypeName, exportMacroName

        #nativeOutput.append ("template<> uint32_t _getProjectionId<" + self.getQualifiedName().replace ('.', "::") + ">() { return " + str (self.typeId) + "; };")
        #nativeOutput.append ("template uint32_t _getProjectionId<" + self.getQualifiedName().replace ('.', "::") + ">();")

        nativeHeaderOutputUnscoped.append ("template<> " + exportMacroName + " BentleyApi::BeJsProjection& BentleyApi::BeJsContext::GetProjection<" + self.getQualifiedNameNative() + ">() const;")
        nativeHeaderOutputUnscoped.append ("template<> " + exportMacroName + " BentleyApi::BeJsNativePointer BentleyApi::BeJsProjection::ObtainInstancePointer<" + self.getQualifiedNameNative() + "> (" + self.getQualifiedNameNative() + "* instance, bool owner, BentleyApi::BeJsObjectP customPrototype) const;")
        nativeHeaderOutputUnscoped.append ("template<> " + exportMacroName + " BentleyApi::BeJsNativePointer BentleyApi::BeProjected::GetJsObject<" + self.getQualifiedNameNative() + "> (BentleyApi::BeJsProjection const& projection, BentleyApi::BeJsObjectP customPrototype);")
        
        nativeOutput.append ("template<>\nBentleyApi::BeJsProjection& BentleyApi::BeJsContext::GetProjection<" + self.getQualifiedNameNative() + ">() const\n    {")
        nativeOutput.append ("    return *m_projections [" + projectionTypeName + "::Id + BeJsProjection::s_idOffset - 1];")
        nativeOutput.append ("    }\n")

        nativeOutput.append ("template<>\nBentleyApi::BeJsNativePointer BentleyApi::BeProjected::GetJsObject<" + self.getQualifiedNameNative() + "> (BentleyApi::BeJsProjection const& projection, BentleyApi::BeJsObjectP customPrototype)\n    {")

        nativeOutput.append ("    if (!m_object.IsValid())\n        {")

        for d in self.descendants:
            nativeOutput.append ("        if (dynamic_cast<" + d.getQualifiedNameNative() + "*>(this) != nullptr)\n            return GetJsObject<" + d.getQualifiedNameNative() + "> (projection, customPrototype);\n")

        nativeOutput.append ("        " + self.getQualifiedNameIdentifier() + "InstanceCreatedCallback (projection.m_context, this);")
        nativeOutput.append ("        BentleyApi::BeJsObject const& prototype = static_cast<" + projectionTypeName + " const&>(projection)." + self.getQualifiedNameIdentifier() + ";")
        nativeOutput.append ("        m_object = BentleyApi::BeJsNativePointer (projection.m_context, this, " + self.getQualifiedNameIdentifier() + "DisposeCallbackOwner, customPrototype != nullptr ? customPrototype : &prototype, customPrototype != nullptr);")
        nativeOutput.append ("        }\n")
        nativeOutput.append ("    return m_object;")
        nativeOutput.append ("    }\n")

        nativeOutput.append ("template<>\nBentleyApi::BeJsNativePointer BentleyApi::BeJsProjection::ObtainInstancePointer<" + self.getQualifiedNameNative() + "> (" + self.getQualifiedNameNative() + "* instance, bool owner, BentleyApi::BeJsObjectP customPrototype) const\n    {")
        nativeOutput.append ("    if (instance == nullptr)\n        return BentleyApi::BeJsValue::Null (m_context);\n")

        if "BeJsProjection_SuppressBeProjectedCasts" not in self.implements:
            nativeOutput.append ("    auto instanceAsProjected = dynamic_cast<BentleyApi::BeProjected*>(instance);\n    if (instanceAsProjected != nullptr)\n        return instanceAsProjected->GetJsObject<" + self.getQualifiedNameNative() + "> (*this, customPrototype);\n")

        nativeOutput.append ("    " + self.getQualifiedNameIdentifier() + "InstanceCreatedCallback (m_context, instance);\n")
        nativeOutput.append ("    " + projectionTypeName + " const& projection = static_cast<" + projectionTypeName + " const&>(*this);")
        nativeOutput.append ("    return BentleyApi::BeJsNativePointer (m_context, instance, owner ? " + self.getQualifiedNameIdentifier() + "DisposeCallbackOwner : " + self.getQualifiedNameIdentifier() + "DisposeCallback, customPrototype != nullptr ? customPrototype : &projection." + self.getQualifiedNameIdentifier() + ", customPrototype != nullptr);")

        nativeOutput.append ("    }\n")

        '''
        nativeHeaderOutputUnscoped.append ("template<> BentleyApi::BeJsObjectP BentleyApi::BeJsContext::GetProjectedClassPrototype<" + self.getQualifiedName().replace ('.', "::") + "> (BentleyApi::BeJsContext::Projection& projection) const;")

        nativeOutput.append ("template<>\nBentleyApi::BeJsObjectP BentleyApi::BeJsContext::GetProjectedClassPrototype<" + self.getQualifiedName().replace ('.', "::") + "> (BentleyApi::BeJsContext::Projection& projection) const\n    {")
        nativeOutput.append ("    " + projectionTypeName + "& typedProjection = static_cast<" + projectionTypeName + "&>(projection);\n")
        nativeOutput.append ("    return &typedProjection." + self.getQualifiedNameIdentifier() + ";")
        nativeOutput.append ("    }\n")

        nativeHeaderOutputUnscoped.append ("template<> BentleyApi::BeJsNativePointer::DisposeCallback BentleyApi::BeJsContext::GetProjectedClassDisposeCallback<" + self.getQualifiedName().replace ('.', "::") + "> (BentleyApi::BeJsContext::Projection& projection, bool owner) const;")

        nativeHeaderOutputUnscoped.append ("template<> BentleyApi::BeJsContext::ProjectedClassInstancePointerCreatedCallback BentleyApi::BeJsContext::GetProjectedClassInstancePointerCreatedCallback<" + self.getQualifiedName().replace ('.', "::") + "> (BentleyApi::BeJsContext::Projection& projection) const;")

        nativeOutput.append ("template<>\nBentleyApi::BeJsContext::ProjectedClassInstancePointerCreatedCallback BentleyApi::BeJsContext::GetProjectedClassInstancePointerCreatedCallback<" + self.getQualifiedName().replace ('.', "::") + "> (BentleyApi::BeJsContext::Projection& projection) const\n    {")
        nativeOutput.append ("    return " + self.getQualifiedNameIdentifier() + "InstanceCreatedCallback;")
        nativeOutput.append ("    }\n")

        nativeOutput.append ("template<>\nBentleyApi::BeJsNativePointer::DisposeCallback BentleyApi::BeJsContext::GetProjectedClassDisposeCallback<" + self.getQualifiedName().replace ('.', "::") + "> (BentleyApi::BeJsContext::Projection& projection, bool owner) const\n    {")
        nativeOutput.append ("    if (owner)")
        nativeOutput.append ("        return " + self.getQualifiedNameIdentifier() + "DisposeCallbackOwner;\n")
        nativeOutput.append ("    return " + self.getQualifiedNameIdentifier() + "DisposeCallback;")
        nativeOutput.append ("    }\n")
        '''

#---------------------------------------------------------------------------------------
#  @bsiclass                                  Bentley.Systems
#---------------------------------------------------------------------------------------
class Interface:
    def __init__ (self, name, comment, body, holder, isClass):
        self.name = name
        self.body = body
        self.holder = holder
        self.properties = {}
        self.functions = {}
        self.jsName = "Object"
        self.comment = comment
        self.external = parseTargetIsExternal
        self.isClass = isClass
        self.initialized = False
        self.descendants = []

        self.nativeName = getNativeName (body, name)
        Class.nativeNameRegistry [self.getQualifiedName().replace ('$', "::").replace ('.', "::")] = self.getQualifiedNameNative()

        self.extends = None
        self.implements = None

        if self.isClass:
            extendsMatch = re.search (r"extends\s+([^{]+)", body, re.S)
            self.extends = extendsMatch and extendsMatch.group (1).strip() or None

            if self.extends is not None and re.search (r"\simplements\s", self.extends) is not None:
                self.extends, implements = [t.strip() for t in re.split (r"\simplements\s", self.extends)]
                self.implements = [t.strip() for t in implements.split (',')]

            if extendsMatch is None:
                implementsMatch = re.search (r"implements\s+([^{]+)", body, re.S)
                if implementsMatch is not None:
                    self.implements = [t.strip() for t in implementsMatch.group (1).split (',')]

            properties = re.finditer (r"^\s*private\s+([^\s\(]+)\s*:\s*(\S+)\s*;", body, re.M)
            for match in properties:
                self.properties [match.group (1)] = (match.group (1), match.group (2))

            if self.extends is not None and self.extends in self.holder.interfaces:
                parentProperties = self.holder.interfaces [self.extends].properties
                
                for k, v in parentProperties.iteritems():
                    if k not in self.properties:
                        self.properties [k] = v

            functions = re.finditer (r"^\s*(/\*\*\s(?:.(?!\*/))*.\*/\s*)?(public\s)?\s*(static\s)?\s*(\S+)\s*\(([^\)]*)\)\s*:\s*(\S+)\s*;", body, re.M|re.S)
            for match in functions:
                if match.group (3) is None:
                    self.functions [match.group (4)] = Function (match.group (4), match.group (5), match.group (6), match.group(1), False, self, match.group (2) is not None)
                else:
                    self.functions [match.group (4)] = Function (match.group (4), match.group (5), match.group (6), match.group(1), True, self, match.group (2) is not None)

            constructorMatch = re.search (r"^\s*(?:public\s)?\s*constructor\s*\(([^\)]*)\)\s*;", body, re.M)
            if constructorMatch is not None:
                self.functions ["constructor"] = Function ("constructor", constructorMatch.group (1), "", "", False, self, True)
        else:
            properties = re.finditer (r"^\s*([^\s\(]+)\s*:\s*(\S+)\s*;", body, re.M)
            for match in properties:
                self.properties [match.group (1)] = (match.group (1), match.group (2))

    def getQualifiedName (self):
        return self.holder.getQualifiedName() + '.' + self.name

    def getQualifiedNameNative (self):
        return self.holder.getQualifiedNameNative() + '::' + self.nativeName.replace ('$', "::")

    def getQualifiedNameIdentifier (self):
        return self.getQualifiedName().replace ('.', '_')

    def getBaseQualifiedName (self):
        base = self.extends or ""
        if base and '.' not in base:
            base = self.holder.getQualifiedName() + '.' + base
        return base

    def getCallbacksObjectName (self):
        return self.getQualifiedNameIdentifier()

    def generateFirstPassStaticCode (self):
        if self.external:
            return

        global projectionIndex, projectionTypeName, exportMacroName

        nativeHeaderOutputUnscoped.append ("template<> " + exportMacroName + " BentleyApi::BeJsProjection& BentleyApi::BeJsContext::GetProjection<" + self.getQualifiedNameNative() + ">() const;")
        nativeHeaderOutputUnscoped.append ("template<> " + exportMacroName + " BentleyApi::BeJsObject BentleyApi::BeJsProjection::CopyInterfaceInstanceObject<" + self.getQualifiedNameNative() + "> (" + self.getQualifiedNameNative() + " const* instance, BentleyApi::BeJsObjectP objectToReuse) const;")
        nativeHeaderOutputUnscoped.append ("template<> " + exportMacroName + " " + self.getQualifiedNameNative() + " BentleyApi::BeJsProjection::CreateInterfaceInstance<" + self.getQualifiedNameNative() + "> (BentleyApi::BeJsObject const& data) const;")

        nativeOutput.append ("template<>\nBentleyApi::BeJsObject BentleyApi::BeJsProjection::CopyInterfaceInstanceObject<" + self.getQualifiedNameNative() + "> (" + self.getQualifiedNameNative() + " const* instance, BentleyApi::BeJsObjectP objectToReuse) const\n    {")
        nativeOutput.append ("    if (instance == nullptr)\n        return BentleyApi::BeJsValue::Null (m_context);\n")
        nativeOutput.append ("    BentleyApi::BeJsObject jsValue = objectToReuse == nullptr ? BentleyApi::BeJsObject::New (m_context) : *objectToReuse;")

        if self.isClass:
            nativeOutput.append ("    if (objectToReuse == nullptr)\n        jsValue.SetPrototype (static_cast<" + projectionTypeName + " const*>(this)->" + self.getCallbacksObjectName() + ");\n")

        for p in self.properties.itervalues():
            conversion = self.holder.getConversion (p [1])
            propParams = ConvertParams (p [0], "instance->" + p [0], "m_context")
            resolvedJs = conversion.resolveJs (p [1], propParams, self.holder)

            #if 'MakeJsObject (' not in resolvedJs:
            resolvedJsTokens = resolvedJs.split (' ', 2)
            resolvedJs = resolvedJsTokens [0] + ' ' + resolvedJsTokens [2]

            resolvedJs = resolvedJs.replace ("__customPrototype__", "nullptr")
            resolvedJs = resolvedJs.replace (");", ')')

            if '=' in resolvedJs:
                resolvedJs = resolvedJs.replace ('=', '(').replace (' ( ', ' (') + ')'

            nativeOutput.append ("    jsValue.SetProperty (\"" + p [0] + "\", " + resolvedJs.rstrip (';') + ");")

        nativeOutput.append ('')
        nativeOutput.append ("    return jsValue;")
        nativeOutput.append ("    }\n")

        nativeOutput.append ("template<>\n" + self.getQualifiedNameNative() + " BentleyApi::BeJsProjection::CreateInterfaceInstance<" + self.getQualifiedNameNative() + "> (BentleyApi::BeJsObject const& data) const\n    {")
        nativeOutput.append ("    " + self.getQualifiedNameNative() + " nativeValue;");

        for p in self.properties.itervalues():
            conversion = self.holder.getConversion (p [1])
            propParams = ConvertParams (p [0], "data.Get" + conversion.jsName + "Property (\"" + p [0] + "\")", "m_context")
            resolvedCxx = conversion.resolveCxx (p [1], propParams, self.holder)

            #if 'MakeNativeObject (' not in resolvedCxx:
            resolvedCxx = resolvedCxx [resolvedCxx.index (' '):].strip()

            nativeOutput.append ("    nativeValue." + resolvedCxx)

        nativeOutput.append ('')
        nativeOutput.append ("    return nativeValue;")
        nativeOutput.append ("    }\n")

    def generateStaticCode (self):
        if self.external:
            return

        global projectionIndex, projectionTypeName, modulesForTsOutput

        name = self.name
        for e in self.holder.exports.itervalues():
            if e.target == name:
                name = e.name

        output = []
        moduleName = None

        if self.comment and 0 != len(self.comment):
            output.append (self.comment)

        if '$' in name:
            moduleName, name = name.rsplit ('$', 1)
            if moduleName not in modulesForTsOutput:
                modulesForTsOutput [moduleName] = []

        if self.isClass:
            iExtends = self.extends
            if iExtends and '$' in iExtends:
                iExtendsNamespace, iExtendsName = iExtends.rsplit ('$', 1)
                iExtends = iExtendsNamespace + "$I" + iExtendsName

            tsExtends = iExtends and (" extends " + iExtends) or ""
            output.append ("export interface I" + name + tsExtends + "\n    {")
        else:
            output.append ("export interface " + name + " {")

        if self.isClass:
            tsOutputMarker = len (tsOutput)

            for f in self.functions.itervalues():
                f.generateStaticCode()

            if len (tsOutput) != tsOutputMarker:
                output.append ('\n'.join (tsOutput [tsOutputMarker:]))
                del tsOutput [tsOutputMarker:]

        for p in self.properties.itervalues():
            tsType = dispatchTsTypeLookup (p [1], self)
            output.append ("    " + p [0] + ' : ' + tsType + ';')

        output.append ("    };\n")

        if moduleName is not None:
            modulesForTsOutput [moduleName].append ('\n'.join (output))
        else:
            tsOutput.append ('\n'.join (output))

        nativeOutput.append ("template<>\nBentleyApi::BeJsProjection& BentleyApi::BeJsContext::GetProjection<" + self.getQualifiedNameNative() + ">() const\n    {")
        nativeOutput.append ("    return *m_projections [" + projectionTypeName + "::Id + BeJsProjection::s_idOffset - 1];")
        nativeOutput.append ("    }\n")

    def initialize (self):
        global classesPendingInitialize, lastTypeId, initializedTypes

        if self.initialized:
            return

        base = self.getBaseQualifiedName()
        if base in classesPendingInitialize:
            classesPendingInitialize [base].initialize()

        lastTypeId = lastTypeId + 1
        self.typeId = lastTypeId;

        if not self.external:
            nativeHeaderOutputScoped.append ("    BeJsObject " + self.getQualifiedNameIdentifier() + ";")
            nativeOutput.append ("    " + self.getQualifiedNameIdentifier() + " = _initializeClass (BeJsString (m_context, \"" + self.getQualifiedName().replace ('$', '.') + "\"), " + self.getCallbacksObjectName() + ");");
        
        initializedTypes.append (self)

        self.initialized = True

    def generateRuntimeCode (self):
        global classesPendingInitialize

        if not self.isClass:
            return

        if not self.external:
            nativeOutput.append ("    _declareClass (BeJsString (m_context, \"" + self.getQualifiedName().replace ('$', '.') + "\"), BeJsString (m_context, \"" + self.getBaseQualifiedName().replace ('$', '.') + "\"));");
        
        classesPendingInitialize [self.getQualifiedName()] = self

        if not self.external:
            nativeOutput.append ("")

    def createTsTypeRepresentation (self, type):
        return self.holder.createTsTypeRepresentation (type)

    def getConversion (self, type):
        return self.holder.getConversion (type)

    def resolveTypeName (self, type):
        return self.holder.resolveTypeName (type)

    def resolveCxx (self, type, params, context):
        return self.getQualifiedNameNative() + ' ' + params.name + " = " + params.context+ ".CreateProjectedInterfaceInstance<" + self.getQualifiedNameNative() + "> (" + params.object.replace ("__type__", "Object") + ");"

        '''output = [
            'BentleyApi::BeJsObject _' + params.name + " = " + params.object.replace ("__type__", "Object") + ';',
            self.getQualifiedNameNative() + ' ' + params.name + ';'
            ]

        for p in self.properties.itervalues():
            conversion = self.holder.getConversion (p [1])

            propParams = ConvertParams (p [0], '_' + params.name + ".Get" + conversion.jsName + "Property (\"" + p [0] + "\")", params.context)
            resolvedCxx = conversion.resolveCxx (type, propParams, context)
            output.append (params.name + '.' + resolvedCxx [resolvedCxx.index (' '):].strip())

        return '\n'.join (output)'''

    def resolveCxxName (self, type, context):
        return self.getQualifiedNameNative()

    def resolveJs (self, type, params, context):
        return "BentleyApi::BeJsObject " + params.name + " = " + params.context + ".CopyProjectedInterfaceInstanceObject<" + self.getQualifiedNameNative() + "> (&" + params.object + ");"

        '''output = [
            'BentleyApi::BeJsObject ' + params.name + " = BentleyApi::BeJsObject::New (" + params.context + ');'
            ]

        for p in self.properties.itervalues():
            conversion = self.holder.getConversion (p [1])
            propParams = ConvertParams (p [0], "_nativeValue." + p [0], params.context)
            resolvedJsTokens = conversion.resolveJs (type, propParams, context).split (' ', 2)
            resolvedJs = resolvedJsTokens [0] + ' ' + resolvedJsTokens [2]
            output.append (params.name + ".SetProperty (\"" + p [0] + "\", " + resolvedJs.rstrip(';') + ");")

        return '\n'.join (output)'''

    def generateHelperCode (self):
        if self.external:
            return

        pass

#---------------------------------------------------------------------------------------
#  @bsiclass                                  Bentley.Systems
#---------------------------------------------------------------------------------------
class Enum:
    def __init__ (self, name, holder):
        self.name = name
        self.holder = holder
        self.external = parseTargetIsExternal
        Class.nativeNameRegistry [holder.getQualifiedName().replace ('.', "::") + "::" + name.replace ('$', "::")] = holder.getQualifiedNameNative() + "::" + name.replace ('$', "::")

    def generateStaticCode (self):
        if self.external:
            return

        global nativeSources, modulesForTsOutput

        name = self.name
        moduleName = None
        output = []

        if '$' in name:
            moduleName, name = name.rsplit ('$', 1)
            if moduleName not in modulesForTsOutput:
                modulesForTsOutput [moduleName] = []

        output.append ("export enum " + name)

        namespace = self.holder.getQualifiedName()

        if moduleName is not None:
            namespace = namespace + '.' + moduleName

        namespace = namespace.replace ('$', '.')

        foundNativeValues = False
        for s in nativeSources.itervalues():
            matches = re.finditer (r"BEJAVASCRIPT_EXPORT_CLASS\s*\(([^\)]*)\)\s*enum\s+class\s+([^\s:]+)\s*:\s*([^\s{]+)\s*([^;]+)", s)
            for match in matches:
                matchNamespace = match.group (1)
                matchClass = match.group (2)

                if namespace == matchNamespace and name == matchClass:
                    values = match.group (4)

                    for match in re.finditer (r"(\S+)\s*<<\s*(\S+)\s*[,}]", values):
                        values = values.replace (match.group (0), str (int (match.group (1)) << int (match.group (2).rstrip (','))) + ', //' + match.group (1) + ' << ' + match.group (2))

                    if '}' not in values:
                        values += '\n    }'

                    output.append (values.replace ('{', "    {") + ";\n")
                    foundNativeValues = True

        if not foundNativeValues:
            raise Exception ("Enum values not found: " + name)

        if moduleName is not None:
            modulesForTsOutput [moduleName].append ('\n'.join (output))
        else:
            tsOutput.append ('\n'.join (output))

#---------------------------------------------------------------------------------------
#  @bsiclass                                  Bentley.Systems
#---------------------------------------------------------------------------------------
class Export:
    def __init__ (self, name, target, holder):
        self.name = name
        self.target = target
        self.holder = holder
        self.external = parseTargetIsExternal

#---------------------------------------------------------------------------------------
#  @bsiclass                                  Bentley.Systems
#---------------------------------------------------------------------------------------
class Type:
    def __init__ (self, name, target, holder):
        self.name = name
        self.target = target
        self.holder = holder
        self.suppress = suppressingTypeAliases
        self.external = parseTargetIsExternal

    def canResolveTs (self, type):
        return type == self.name

    def resolveTs (self, type):
        return self.target

    def canResolveCxx (self, type):
        return type == self.name

    def resolveCxx (self, type):
        return self.target

    def getQualifiedName (self):
        return self.holder.getQualifiedName() + '.' + self.name

    def getQualifiedNameIdentifier (self):
        return self.getQualifiedName().replace ('.', '_')

    def generateStaticCode (self):
        if self.external:
            return

        if self.suppress:
            return

        if self.name in ("CxxCodeTemplate"):
            return

        declarePrefix = ""
        if self.holder.name:
            declarePrefix = "export "

        tsOutput.append (declarePrefix + "declare type " + self.name + " = " + dispatchTsTypeLookup (self.target, self) + ";\n")

#---------------------------------------------------------------------------------------
#  @bsiclass                                  Bentley.Systems
#---------------------------------------------------------------------------------------
class Conversion:
    def __init__ (self, name, body, holder):
        self.name = name
        self.holder = holder
        self.suppress = suppressingTypeAliases
        self.external = parseTargetIsExternal

        self.jsType = re.search (r"CxxType<([^,]+)", body).group (1).strip()
        self.cxxName, self.jsName, self.convertToJs, self.convertToCxx = [m.group (1).strip() for m in re.finditer (r"CxxCodeTemplate\s*/\*\*\*((?:.(?!\*\*\*/))*)", body, re.S)]
        self.baseName = self.name.replace ('<' + self.jsType + '>', '')

    def canResolveTs (self, type):
        if '<' in type and '<' in self.name:
            baseName = re.search (r"([^<]+)", type).group (1)
            return self.baseName == baseName

        return type == self.name

    def resolveTs (self, type):
        if '<' in type and '<' in self.name:
            return re.search (r"<([^>]+)>", type).group (1)

        return self.jsType

    def canResolveCxx (self, type, context):
        if '<' in type and '<' in self.name:
            baseName = re.search (r"([^<]+)", context.resolveTypeName (type)).group (1)
            return self.baseName == baseName

        return type == self.name

    def getQualifiedTypeForCxx (self, type, context):
        if type in forwardDeclaresRegistry:
            return forwardDeclaresRegistry [type]

        if '.' in type:
            return Class.nativeNameRegistry [type.replace ('.', "::")]

        c = context
        while not isinstance (c, Module):
            c = c.holder

        if "::" not in type:
            matched = False

            for m in reversed (Module.registry [c.name]):
                if type in m.classes or type in m.enums:
                    matched = True
                    c = m
                    break

            if not matched:
                return type

        return Class.nativeNameRegistry [c.getQualifiedName().replace ('.', "::") + "::" + type]

    def resolveCxx (self, type, params, context):
        output = self.convertToCxx
        output = output.replace ("__name__", params.name)
        output = output.replace ("__object__", params.object)
        output = output.replace ("__context__", params.context)
        output = output.replace ("__type__", self.jsName)

        typeR = context.resolveTypeName (type)
        if '<' in typeR and '<' in self.name:
            t = re.search (r"<([^>]+)>", typeR).group (1)
            output = output.replace ("__" + self.jsType + "__", self.getQualifiedTypeForCxx (t.replace ("$", "::"), context))

        return output

    def resolveJs (self, type, params, context):
        output = self.convertToJs
        output = output.replace ("__name__", params.name)
        output = output.replace ("__object__", params.object)
        output = output.replace ("__context__", params.context)
        output = output.replace ("__type__", self.cxxName)

        typeR = context.resolveTypeName (type)
        if '<' in typeR and '<' in self.name:
            t = re.search (r"<([^>]+)>", typeR).group (1)
            output = output.replace ("__" + self.jsType + "__", self.getQualifiedTypeForCxx (t.replace ("$", "::"), context))

        return output

    def resolveCxxName (self, type, context):
        typeR = context.resolveTypeName (type)

        if '<' in typeR and '<' in self.name:
            t = re.search (r"<([^>]+)>", typeR).group (1)
            return self.cxxName.replace ("__" + self.jsType + "__", self.getQualifiedTypeForCxx (t.replace ("$", "::"), context))
        
        return self.cxxName

    def getQualifiedName (self):
        return self.holder.getQualifiedName() + '.' + self.name

    def getQualifiedNameIdentifier (self):
        return self.getQualifiedName().replace ('.', '_')

    def generateStaticCode (self):
        if self.external:
            return

        if self.suppress:
            return

        declarePrefix = ""
        if self.holder.name:
            declarePrefix = "export "

        if '<' not in self.name:
            tsOutput.append (declarePrefix + "declare type " + self.name + " = " + dispatchTsTypeLookup (self.jsType, self) + ";\n")
        else:
            tsOutput.append ("//" + declarePrefix + "declare type " + self.name + " = " + dispatchTsTypeLookup (self.jsType, self) + ";\n")

#---------------------------------------------------------------------------------------
#  @bsiclass                                  Bentley.Systems
#---------------------------------------------------------------------------------------
class Module:
    registry = {}

    def __init__ (self, name, body=""):
        if name not in Module.registry:
            Module.registry [name] = []
        
        Module.registry [name].append (self)

        self.types = {}
        self.enums = {}
        self.functions = {}
        self.classes = {}
        self.exports = {}
        self.conversions = {}
        self.interfaces = {}
        self.typeId = -1
        self.extends = None

        self.name = name
        self.external = parseTargetIsExternal

        self.nativeName = getNativeName (body, name)

        types = re.finditer (r"^\s*type\s+(\S+)\s*=\s*(\S+)\s*;", body, re.M)
        for match in types:
            if re.match (r"^=\s+CxxType", match.group (2)) is not None:
                self.conversions [match.group (1)] = Conversion (match.group (1), match.group (2), self)
            else:
                self.types [match.group (1)] = Type (match.group (1), match.group (2).strip ("=;\r\n\t "), self)

        enums = re.finditer (r"^\s*enum\s+(\S+)\s*\{\s*\}\s*$", body, re.M)
        for match in enums:
            self.enums [match.group (1)] = Enum (match.group (1), self)

        functions = re.finditer (r"^\s*function\s+(\S+)\s*\(([^\)]*)\)\s*:\s*(\S+)\s*;", body, re.M)
        for match in functions:
            self.functions [match.group (1)] = Function (match.group (1), match.group (2), match.group (3), "", True, self, False)

        classes = re.finditer (r"^\s*(/\*\*\s(?:.(?!\*/))*.\*/\s*)?class\s+(\S+)\s*((?:.(?!\}))*)", body, re.M | re.S)
        for match in classes:
            if 'BeJsProjection_ValueType' in match.group(3):
                self.interfaces [match.group (2)] = Interface (match.group (2), match.group(1), match.group (3), self, True)
            else:
                self.classes [match.group (2)] = Class (match.group (2), match.group(1), match.group (3), self)

        interfaces = re.finditer (r"^\s*(/\*\*\s(?:.(?!\*/))*.\*/\s*)?interface\s+(\S+)\s*((?:.(?!\}))*)", body, re.M | re.S)
        for match in interfaces:
            self.interfaces [match.group (2)] = Interface (match.group (2), match.group(1), match.group (3), self, False)

        exports = re.finditer (r"^\s*export\s+type\s+(\S+)\s*=\s*(\S+)\s*;", body, re.M)
        for match in exports:
            self.exports [match.group (1)] = Export (match.group (1), match.group (2), self)

    def getQualifiedName (self):
        return self.name

    def getQualifiedNameNative (self):
        return self.nativeName.replace ('.', '::')

    def getQualifiedNameIdentifier (self):
        if self.name == "":
            return "global"

        return self.getQualifiedName().replace ('.', '_')

    def createTsTypeRepresentation (self, type):
        nsPrefix = ""
        if self.external:
            nsPrefix = self.getQualifiedName() + '.'

        for t in self.types.itervalues():
            if t.canResolveTs (type):
                type = t.resolveTs (type)

        for c in self.conversions.itervalues():
            if c.canResolveTs (type):
                return nsPrefix + c.resolveTs (type)

        if self.name:
            return nsPrefix + globalModule.createTsTypeRepresentation (type)

        return nsPrefix + type

    def canEnhanceTsTypeRepresentation (self, type):
        for t in self.types.itervalues():
            if t.canResolveTs (type):
                return True

        for c in self.conversions.itervalues():
            if c.canResolveTs (type):
                return True

        return False

    def getConversion (self, type):
        if self.name and type.startswith (self.name):
            type = type [len(self.name) + 1:]

        for t in self.types.itervalues():
            if t.canResolveCxx (type):
                type = t.resolveCxx (type)

        if type == "void":
            return type

        for c in self.conversions.itervalues():
            if c.canResolveCxx (type, self):
                return c

        for i in self.interfaces.itervalues():
            if i.name == type:
                return i

        if self.name:
            return globalModule.getConversion (type)

        msg = "Cannot convert type - {0}".format(type)
        raise Exception (msg)

    def canEnhanceConversion (self, type):
        if self.name and type.startswith (self.name):
            type = type [len(self.name) + 1:]

        for t in self.types.itervalues():
            if t.canResolveCxx (type):
                return True

        if type == "void":
            return False

        for c in self.conversions.itervalues():
            if c.canResolveCxx (type, self):
                return True

        for i in self.interfaces.itervalues():
            if i.name == type:
                return True

        return False

    def resolveTypeName (self, type):
        if self.name and type.startswith (self.name):
            type = type [len(self.name) + 1:]

        for t in self.types.itervalues():
            if t.canResolveCxx (type):
                return t.resolveCxx (type)

        if self.name:
            return globalModule.resolveTypeName (type)

        return type

    def generateFirstPassStaticCode (self):
        for i in self.interfaces.itervalues():
            i.generateFirstPassStaticCode()

        for c in self.classes.itervalues():
            c.generateFirstPassStaticCode()

    def generateStaticCode (self):
        global modulesForTsOutput

        if not self.external:
            if self.name:
                tsOutput.append ("module " + self.name + " {\n")

        if not suppressTypeAliases:
            for t in self.types.itervalues():
                t.generateStaticCode()

            for c in self.conversions.itervalues():
                c.generateStaticCode()

        for f in self.functions.itervalues():
            f.generateStaticCode()

        for e in self.enums.itervalues():
            e.generateStaticCode()

        for c in self.classes.itervalues():
            c.generateStaticCode()

        for i in self.interfaces.itervalues():
            i.generateStaticCode()

        if not self.external:
            for m, c in modulesForTsOutput.iteritems():
                tsOutput.append ("export module " + m + " {\n")
                tsOutput.append ('\n'.join (c))
                tsOutput.append ("}\n")

        modulesForTsOutput = {}

        if not self.external:
            if self.name:
                tsOutput.append ("}\n")

    def getCallbacksObjectName (self):
        return self.getQualifiedNameIdentifier()

    def getPrototypeObjectName (self):
        return self.getQualifiedNameIdentifier() + "Prototype"

    def initialize (self):
        global initializedTypes, lastTypeId

        lastTypeId = lastTypeId + 1
        self.typeId = lastTypeId;

        if not self.external:
            nativeHeaderOutputScoped.append ("    BeJsObject " + self.getQualifiedNameIdentifier() + ";")
            #nativeHeaderOutputScoped.append ("    static void " + self.getQualifiedNameIdentifier() + "DisposeCallback (void* object);")
            #nativeHeaderOutputScoped.append ("    static void " + self.getQualifiedNameIdentifier() + "DisposeCallbackOwner (void* object);")
            #nativeHeaderOutputScoped.append ("    static void " + self.getQualifiedNameIdentifier() + "InstanceCreatedCallback (BentleyApi::BeJsContext const& context, void* instance);")
            nativeOutput.append ("    " + self.getQualifiedNameIdentifier() + " = _initializeClass (BeJsString (m_context, \"" + self.getQualifiedName() + "\"), " + self.getCallbacksObjectName() + ");");

        initializedTypes.append (self)

    def generateRuntimeCode (self):
        if not self.external:
            nativeOutput.append ("    _declareClass (BeJsString (m_context, \"" + self.getQualifiedName().replace ('$', '.') + "\"));");

        for f in self.functions.itervalues():
            f.generateRuntimeCode()

        for c in self.classes.itervalues():
            c.generateRuntimeCode()

        for i in self.interfaces.itervalues():
            i.generateRuntimeCode()

        if not self.external:
            for e in self.exports.itervalues():
                name = e.name
                if '.' not in name:
                    name = self.name + '.' + name

                target = e.target
                if '.' not in target:
                    target = self.name + '.' + target

                nativeOutput.append ("    _registerClassAlias (BeJsString (m_context, \"" + target + "\"), BeJsString (m_context, \"" + name + "\"));");

            nativeOutput.append ("")

    def generateHelperCode (self):
        if self.external:
            return

        pass
        #nativeOutput.append ("template<> uint32_t _getProjectionId<" + self.getQualifiedName().replace ('.', "::") + ">() { return " + str (self.typeId) + "; };")
        #nativeOutput.append ("template uint32_t _getProjectionId<" + self.getQualifiedName().replace ('.', "::") + ">();")

#---------------------------------------------------------------------------------------
#  @bsimethod                                  Bentley.Systems
#---------------------------------------------------------------------------------------
def parseDeclaration (type, name, value):
    if type == "module":
        Module (name, value)
    elif type == "type":
        if re.match (r"^=\s+CxxType", value) is not None:
            globalModule.conversions [name] = Conversion (name, value, globalModule)
        else:
            globalModule.types [name] = Type (name, value.strip ("=;\r\n\t "), globalModule)
    elif type == "class":
        if re.match (r"^implements\s+CxxType", value) is not None:
            globalModule.conversions [name] = Conversion (name, value, globalModule)
        else:
            globalModule.classes [name] = Class (name, value, globalModule)
    elif type == "enum":
        globalModule.enums [name] = Enum (name, globalModule)
    elif type == "function":
        nameTokens = name.split ('(', 1)
        extractedName = nameTokens [0]
        arguments = nameTokens [1].strip ("()\r\n\t ")
        returnType = value.strip (":;\r\n\t ")
        globalModule.functions [extractedName] = Function (extractedName, arguments, returnType, "", True, globalModule)

#---------------------------------------------------------------------------------------
#  @bsimethod                                  Bentley.Systems
#---------------------------------------------------------------------------------------
def getClassByName (name, holder):
    if '.' not in name:
        if name in holder.interfaces:
            return holder.interfaces [name]

        return holder.classes [name]

    namespace, localName = name.rsplit ('.', 1)

    for m in reversed (Module.registry [namespace]):
        if localName in m.classes:
            return m.classes [localName]

    raise Exception (name + " not found.")

#---------------------------------------------------------------------------------------
#  @bsimethod                                  Bentley.Systems
#---------------------------------------------------------------------------------------
def main():
    global nativeSources, globalModule, classesPendingInitialize, initializedTypes, forceBuiltInTsTypes, suppressTypeAliases, suppressCoreTypeAliases, suppressingTypeAliases, lastTypeId, nativeLib, nativeHeaderOutputScoped, nativeHeaderOutputUnscoped, projectionTypeName, projectionIndex, refCountedBaseType, bentleyName, modulesForTsOutput, forwardDeclaresRegistry, exportMacroName, parseTargetIsExternal

    shortArgs = "d:s:t:n:"
    longArgs = ["declarationPath=", "nativeSourcePaths=", "typeScriptOutputPath=", "nativeOutputPath=", "nativeOutputHeaderPath=", "platformTsLibPath=", "nativeOutputTemplatePath=", "nativeOutputHeaderTemplatePath=",
                "projectionTypeName=", "refCountedBaseType=", "tscCommand=", "bentleyName=","exportMacroName=", 
                "forceBuiltInTsTypes", "suppressTypeAliases", "suppressCoreTypeAliases", "externalDeclarationPaths="]

    try:
        if '.rsp' in sys.argv [1]:
            opts, args = getopt.getopt (shlex.split (getFileContents (sys.argv [1])), shortArgs, longArgs)
        else:
            opts, args = getopt.getopt (sys.argv [1:], shortArgs, longArgs)
    except getopt.GetoptError as err:
        print err
        sys.exit (2)

    declarationPath = ""
    nativeSourcePaths = ""
    typeScriptOutputPath = ""
    nativeOutputPath = ""
    nativeOutputHeaderPath = ""
    nativeOutputTemplatePath = ""
    nativeOutputHeaderTemplatePath = ""
    platformTsLibPath = ""
    refCountedBaseType = ""
    tscCommand = ""
    exportMacroName = ""
    externalDeclarationPaths = ""

    for o, a in opts:
        if o in ("-d", "--declarationPath"):
            declarationPath = a
        elif o in ("-s", "--nativeSourcePaths"):
            nativeSourcePaths = a
        elif o in ("-t", "--typeScriptOutputPath"):
            typeScriptOutputPath = a
        elif o in ("-n", "--nativeOutputPath"):
            nativeOutputPath = a
        elif o in ("-n", "--nativeOutputHeaderPath"):
            nativeOutputHeaderPath = a
        elif o in ("--platformTsLibPath"):
            platformTsLibPath = a
        elif o in ("--nativeOutputTemplatePath"):
            nativeOutputTemplatePath = a
        elif o in ("--nativeOutputHeaderTemplatePath"):
            nativeOutputHeaderTemplatePath = a
        elif o in ("--projectionTypeName"):
            projectionTypeName = a
        elif o in ("--exportMacroName"):
            exportMacroName = a
        elif o in ("--refCountedBaseType"):
            refCountedBaseType = a
        elif o in ("--tscCommand"):
            tscCommand = a
        elif o in ("--bentleyName"):
            bentleyName = a
        elif o in ("--forceBuiltInTsTypes"):
            forceBuiltInTsTypes = True
        elif o in ("--suppressTypeAliases"):
            suppressTypeAliases = True
        elif o in ("--suppressCoreTypeAliases"):
            suppressCoreTypeAliases = True
        elif o in ("--externalDeclarationPaths"):
            externalDeclarationPaths = a

    if nativeSourcePaths != "":
        for p in re.split (r"\s+", nativeSourcePaths):
            nativeSources [p] = getFileContents (p)

    globalModule = Module ("")

    declarationPaths = [(platformTsLibPath, {"external": False})]

    if externalDeclarationPaths != "":
        for p in re.split (r"\s+", externalDeclarationPaths):
            declarationPaths.append ((p, {"external": True}))

    declarationPaths.append ((declarationPath, {"external": False}))

    tmpFile, tmpFilePath = tempfile.mkstemp()
    tscStatus = os.system (tscCommand + ' ' + ' '.join ([p [0] for p in declarationPaths]) + " --target ES5 --noImplicitAny --out " + tmpFilePath)
    os.close (tmpFile)
    os.remove (tmpFilePath)
    if tscStatus != 0:
        raise Exception ("TypeScript compile failed.")

    for pathDescriptor in declarationPaths:
        path, pathInfo = pathDescriptor

        parseTargetIsExternal = pathInfo ["external"]

        if suppressCoreTypeAliases and path == platformTsLibPath:
            suppressingTypeAliases = True

        declaration = getFileContents (path)

        forwardDeclarations = re.finditer (r"/\*\*\*\s*BEGIN_FORWARD_DECLARATIONS\s*\*\*\*/((?:.(?!/\*\*\*\s*END_FORWARD_DECLARATIONS\s*\*\*\*/))*)", declaration, re.S)
        for declaresMatch in forwardDeclarations:
            for classMatch in re.finditer (r"class\s+(\S+).*?/\*\*\*\s*NATIVE_TYPE_NAME\s*=\s*([^\s\*]+)\s*\*\*\*/", declaresMatch.group (1)):
                forwardDeclaresRegistry [classMatch.group (1)] = classMatch.group (2)

        declaration = re.sub (r"/\*\*\*\s*BEGIN_FORWARD_DECLARATIONS\s*\*\*\*/(?:.(?!/\*\*\*\s*END_FORWARD_DECLARATIONS\s*\*\*\*/))*", "", declaration, 0, re.S)

        declares = re.finditer (r"declare\s+(\S+)\s+(\S+)((?:.(?!^\s*declare\s))*)", declaration, re.M | re.S)
        for match in declares:
            parseDeclaration (match.group (1).strip(), match.group (2).strip(), match.group (3).strip())

        suppressingTypeAliases = False

    if nativeOutputTemplatePath != "":
        nativeOutput.append (getFileContents (nativeOutputTemplatePath))

    nativeOutput.append (createGeneratorMessage ("GENERATED CONTENT -- DO NOT MODIFY") + '\n')
    nativeHeaderOutputScoped.append (createGeneratorMessage ("GENERATED CONTENT -- DO NOT MODIFY") + '\n')
    nativeHeaderOutputUnscoped.append (createGeneratorMessage ("GENERATED CONTENT -- DO NOT MODIFY") + '\n')
    tsOutput.append (createGeneratorMessage ("GENERATED CONTENT -- DO NOT MODIFY") + '\n')

    nativeHeaderOutputUnscoped.append ("#include <BeJavaScript/BeJavaScript.h>")

    nativeHeaderOutputUnscoped.append ("BEGIN_" + bentleyName.upper() + "_NAMESPACE")

    if "::" in projectionTypeName:
        projectionTypeNamespace, projectionTypeLocalName = projectionTypeName.rsplit ("::", 1)
    else:
        projectionTypeNamespace = ""
        projectionTypeLocalName = projectionTypeName

    nativeHeaderOutputScoped.append ("struct " + projectionTypeLocalName + " : public BentleyApi::BeJsProjection\n    {\n    friend struct BentleyApi::BeJsContext;\n\nprivate:\n    static size_t Id;\n\npublic:")
    nativeHeaderOutputScoped.append ("    " + projectionTypeLocalName + " (BentleyApi::BeJsContextR context);")

    nativeOutput.append (nativeLib)

    for ml in Module.registry.itervalues():
        for m in ml:
            m.generateFirstPassStaticCode()

    for ml in Module.registry.itervalues():
        for m in ml:
            m.generateStaticCode()

    nativeOutput.append ("size_t " + projectionTypeName + "::Id = 0;\n")

    nativeOutput.append (projectionTypeName + "::" + projectionTypeLocalName + " (BentleyApi::BeJsContextR _jsContext)\n    : BentleyApi::BeJsProjection (_jsContext),")
    nativeOutput.append ('')
    constructorLine = len (nativeOutput) - 1

    nativeOutput.append ("    {")

    nativeOutput.append ("    BeJsObject _beJavaScript = m_context.GetGlobalObject().GetObjectProperty (\"BentleyApi\").GetObjectProperty (\"BeJavaScript\");")
    nativeOutput.append ("    BeJsFunction _initializeClass = _beJavaScript.GetFunctionProperty (\"InitializeClass\");")
    nativeOutput.append ("    BeJsFunction _declareClass = _beJavaScript.GetFunctionProperty (\"DeclareClass\");")
    nativeOutput.append ("    BeJsFunction _registerClassAlias = _beJavaScript.GetFunctionProperty (\"RegisterClassAlias\");\n")

    for ml in Module.registry.itervalues():
        for m in ml:
            m.generateRuntimeCode()

    for ml in Module.registry.itervalues():
        for m in ml:
            m.initialize()

    for c in classesPendingInitialize.itervalues():
        if '$' not in c.name:
            c.initialize()

    for c in classesPendingInitialize.itervalues():
        if '$' in c.name:
            c.initialize()

    nativeOutput.append ("    }\n")

    for t in initializedTypes:
        if t.extends is not None:
            getClassByName (t.extends, t.holder).descendants.append (t)

    for t in initializedTypes:
        t.generateHelperCode()

    nativeHeaderOutputScoped.append ("    };")

    initializerOutput = []
    for i, t in enumerate (initializedTypes):
        if not t.external:
            initializerOutput.append ("      " + t.getQualifiedNameIdentifier() + " (BentleyApi::BeJsObject::New (m_context)),")

    initializerOutput [-1] = initializerOutput [-1].rstrip (',')

    nativeOutput [constructorLine] = '\n'.join (initializerOutput)

    nativeHeaderOutputUnscoped.append ("END_" + bentleyName.upper() + "_NAMESPACE")

    nativeOutput.append ('\n' + createGeneratorMessage ("END GENERATED CONTENT") + '\n')
    nativeHeaderOutputScoped.append ('\n' + createGeneratorMessage ("END GENERATED CONTENT") + '\n')
    nativeHeaderOutputUnscoped.append ('\n' + createGeneratorMessage ("END GENERATED CONTENT") + '\n')
    tsOutput.append ('\n' + createGeneratorMessage ("END GENERATED CONTENT") + '\n')
    
    tsOutputString = '\n'.join (tsOutput)
    tsOutputString = tsOutputString.replace ('$', '.')
    tsOutputIsStale = True
    if os.path.isfile (typeScriptOutputPath):
        o = open (typeScriptOutputPath, 'r')
        if o.read() == tsOutputString:
            tsOutputIsStale = False

    if tsOutputIsStale:
        ot = open (typeScriptOutputPath, 'w')
        ot.write (tsOutputString)
        ot.close()
    
    nativeOutputString = '\n'.join (nativeOutput)
    nativeOutputIsStale = True
    if os.path.isfile (nativeOutputPath):
        o = open (nativeOutputPath, 'r')
        if o.read() == nativeOutputString:
            nativeOutputIsStale = False

    if nativeOutputIsStale:
        on = open (nativeOutputPath, 'w')
        on.write (nativeOutputString)
        on.close()

    nativeHeaderGeneratedOutputScoped = '\n'.join (nativeHeaderOutputScoped)
    nativeHeaderGeneratedOutputUnscoped = '\n'.join (nativeHeaderOutputUnscoped)
    nativeHeaderGeneratedOutputUnscopedHelper = '\n'.join (nativeHeaderOutputUnscopedHelper)
    nativeHeaderOutput = getFileContents (nativeOutputHeaderTemplatePath)
    nativeHeaderOutput = nativeHeaderOutput.replace ("___GENERATED_SCOPED_CODE__", nativeHeaderGeneratedOutputScoped)
    nativeHeaderOutput = nativeHeaderOutput.replace ("___GENERATED_UNSCOPED_CODE__", nativeHeaderGeneratedOutputUnscoped)
    nativeHeaderOutput = nativeHeaderOutput.replace ("___GENERATED_UNSCOPED_HELPER_CODE__", nativeHeaderGeneratedOutputUnscopedHelper)

    nativeHeaderOutputIsStale = True
    if os.path.isfile (nativeOutputHeaderPath):
        o = open (nativeOutputHeaderPath, 'r')
        if o.read() == nativeHeaderOutput:
            nativeHeaderOutputIsStale = False

    if nativeHeaderOutputIsStale:
        on = open (nativeOutputHeaderPath, 'w')
        on.write (nativeHeaderOutput)
        on.close()
        
if __name__ == "__main__":
    main()
