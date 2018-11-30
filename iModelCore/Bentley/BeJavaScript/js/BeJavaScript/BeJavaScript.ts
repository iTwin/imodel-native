/*--------------------------------------------------------------------------------------+
|
|     $Source: BeJavaScript/js/BeJavaScript/BeJavaScript.ts $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
module BentleyApi {

//=======================================================================================
// @bsiclass                                                    Steve.Wilson    56/15
//=======================================================================================
export class BeJavaScript {

private BeJavaScriptTypeId : any;

private static s_classDeclarations : { [index : string] : string } = {};
private static s_initializedClasses : { [index : string] : boolean } = {};
private static s_classAliases : { [index : string] : string } = {};
private static s_classConstructors : { [index : string] : any } = {};
private static s_global : any = null;
private static s_generatedTsConstructor = /\{\s*\\?n?\s*_super\s*\.\s*apply\s*\(\s*this\s*,\s*arguments\s*\)\s*;\s*\\?n?\s*\}\s*$/;

private static s_projectedOnDispose = function() { };

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/15
//---------------------------------------------------------------------------------------
private static GetLocalNameFromQualifiedName (name : string, followAlias : boolean = true) : string
    {
    var nameTokens = BeJavaScript.GetNameTokensFromQualifiedName (name, followAlias);
    return nameTokens [nameTokens.length - 1];
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/15
//---------------------------------------------------------------------------------------
private static GetNameTokensFromQualifiedName (name : string, followAlias : boolean = true) : string[]
    {
    if (!followAlias)
        return name.split ('.');

    var resolvedName = BeJavaScript.s_classAliases.hasOwnProperty (name) ? BeJavaScript.s_classAliases [name] : name;
    return resolvedName.split ('.');
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/15
//---------------------------------------------------------------------------------------
private static GetObjectScopeFromQualifiedName (name : string, followAlias : boolean = true) : any
    {
    var nameTokens = BeJavaScript.GetNameTokensFromQualifiedName (name, followAlias);
    var classScope = BeJavaScript.s_global;
    for (var i = 0; i != nameTokens.length - 1; ++i)
        {
        if (!classScope.hasOwnProperty (nameTokens [i]))
            classScope [nameTokens [i]] = {};

        classScope = classScope [nameTokens [i]];
        }

    return classScope;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/15
//---------------------------------------------------------------------------------------
private static GetClassBaseName (name : string) : string
    {
    if (BeJavaScript.s_classDeclarations.hasOwnProperty (name))
        return BeJavaScript.s_classDeclarations [name];

    return "";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/15
//---------------------------------------------------------------------------------------
private static GetObjectFromQualifiedName (name : string, followAlias : boolean = true, nativeInstanceProperties : any = null) : any
    {
    var classScope = BeJavaScript.GetObjectScopeFromQualifiedName (name, followAlias);
    var className = BeJavaScript.GetLocalNameFromQualifiedName (name, followAlias);

    if (!classScope.hasOwnProperty (className))
        {
        var constructorFunction : any;
        if (nativeInstanceProperties.hasOwnProperty ("constructor"))
            constructorFunction = nativeInstanceProperties.constructor.value;
        else
            constructorFunction = function() {};
         
        if (typeof (constructorFunction.prototype) === "undefined")
            constructorFunction.prototype = Object.create ({});

        var base = BeJavaScript.GetClassBaseName (name);
        if (base !== "")
            {
            var baseConstructorFunction = BeJavaScript.GetObjectFromQualifiedName (base);
            constructorFunction.prototype = Object.create (baseConstructorFunction.prototype);
            }

        if (nativeInstanceProperties !== null && nativeInstanceProperties.hasOwnProperty ("constructor"))
            constructorFunction.prototype.constructor = nativeInstanceProperties ["constructor"].value;
        else
            constructorFunction.prototype.constructor = null;

        classScope [className] = constructorFunction;
        }
    
    return classScope [className];
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/15
//---------------------------------------------------------------------------------------
private static OnProjectionInitialized() : void
    {
    for (var name in BeJavaScript.s_classDeclarations)
        {
        if (BeJavaScript.s_classDeclarations.hasOwnProperty (name) && !BeJavaScript.s_initializedClasses.hasOwnProperty (name))
            {
            var nativeClassScope = BeJavaScript.GetObjectScopeFromQualifiedName (name);
            var nativeClassName = BeJavaScript.GetLocalNameFromQualifiedName (name);
            var nativeClassConstructor = nativeClassScope [nativeClassName];
            var nativeClassPrototype = nativeClassConstructor.prototype;

            var jsClassScope = BeJavaScript.GetObjectScopeFromQualifiedName (name, false);
            var jsClassName = BeJavaScript.GetLocalNameFromQualifiedName (name, false);
            var jsClassConstructor = jsClassScope [jsClassName];
            var jsClassPrototype = jsClassConstructor.prototype;

            for (var p in jsClassPrototype)
                if (p !== "constructor")
                    nativeClassPrototype [p] = jsClassPrototype [p];

            if (BeJavaScript.s_generatedTsConstructor.test (jsClassConstructor.toString()))
                jsClassScope [jsClassName] = nativeClassConstructor;

            BeJavaScript.s_initializedClasses [name] = true;
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/15
//---------------------------------------------------------------------------------------
public static DeclareClass (name : string, base : string = "") : void
    {
    BeJavaScript.s_classDeclarations [name] = base;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/15
//---------------------------------------------------------------------------------------
public static RegisterClassAlias (name : string, alias : string) : any
    {
    BeJavaScript.s_classAliases [name] = alias;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Steve.Wilson                    6/15
//---------------------------------------------------------------------------------------
public static InitializeClass (name : string, callbacks : { [index : string] : any }) : any
    {
    var instanceProperties : PropertyDescriptorMap = {};
    var staticProperties : PropertyDescriptorMap = {};
    
    for (var c in callbacks)
        {
        if (!callbacks.hasOwnProperty (c))
            continue;
        
        var callback = callbacks [c];
        var tokens : string[] = c.split (' ', 3);
        var definitionTarget = (tokens [0] === "Instance") ? instanceProperties : staticProperties;
        var action = tokens [1];

        if (action === "GetProperty" || action === "SetProperty")
            {
            var property = tokens [2];
            if (!definitionTarget.hasOwnProperty (property))
                definitionTarget [property] = {};
            
            var descriptor : PropertyDescriptor = definitionTarget [property];
            if (action === "GetProperty")
                descriptor.get = callback;
            else
                descriptor.set = callback;
            }
        else
            {
            var property = tokens[2];

            if (property === "OnDispose")
                callback = BeJavaScript.s_projectedOnDispose;

            definitionTarget [property] = { value: callback, writable: true };
            }
        }

    var classConstructor = BeJavaScript.GetObjectFromQualifiedName (name, true, instanceProperties);
    if (typeof (classConstructor.prototype) === "undefined")
        classConstructor.prototype = {};

    Object.defineProperties (classConstructor.prototype, instanceProperties);
    Object.defineProperties (classConstructor, staticProperties);

    return classConstructor.prototype;
    }

};

}

(function (global : any) {

"use strict";
(<any>BentleyApi.BeJavaScript).s_global = global;

}(this));
