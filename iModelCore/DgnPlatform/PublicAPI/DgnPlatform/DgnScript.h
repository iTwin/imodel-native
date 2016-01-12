/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnScript.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <DgnPlatform/DgnPlatform.h>
#include <DgnPlatform/DgnElement.h>
#include <DgnPlatform/DgnModel.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
//! Enables JavaScript programs to access the DgnPlatform API.
//! DgnScript creates a set of JavaScript types and functions that expose native Dgn and BentleyApi types to JavaScript functions. These types and functions are collectively known as the DgnPlatform API for JavaScript.
//! DgnScript also provides functions to allow native code to invoke JavaScript functions. See DgnScript::ExecuteJavaScriptEga.
//! @section JavaScriptLibrary The JavaScript Library
//! JavaScript programs are loaded from the JavaScript library. The \a myNamespace portion of the myNamespace.myEgaPublicName EGA identifier string must identify a program in the library.
//! <p>The JavaScript library is a virtual storage. An application may use the DgnJavaScriptLibrary class to store a JavaScript program inside a DgnDb. 
//! Or, an application may override the DgnPlatformLib::Host::ScriptAdmin::_FetchJavaScript method in order to locate and supply the text of JavaScript programs from some other source.
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
struct DgnScript
{
    /**
    Execute an Element Generation Algorithm (EGA) that is implemented in JavaScript. 
    An EGA is identified by a two-part name, of the form namespace.function. 
    The JavaScript program that defines an EGA must use this identifier to register the function. 
    The caller of this function must pass in this identifier in the \a jsEgaFunctionName parameter.
    Ordinarily, an EGA is specified by an ECClass.

    <h2>Signature of a JavaScript EGA function.</h2>
    An EGA function written in TypeScript must have the following signature:
    @verbatim
    function myEgaFunction(element: BentleyApi.Dgn.JsDgnElement, origin: BentleyApi.Dgn.JsDPoint3d, angles: BentleyApi.Dgn.JsYawPitchRollAngles, params: any) : number
    @endverbatim
    Or, in JavaScript:
    @verbatim
    function myEgaFunction(element, origin, angles, params)
    @endverbatim

    <h2>Registering a JavaScript EGA function.</h2>
    The JavaScript program must register an EGA function in its start-up logic like this:
    @verbatim
    BentleyApi.Dgn.RegisterEGA('myNamespace.myEgaPublicName', myEgaFunction);
    @endverbatim
    The \a myEgaPublicName parameter must match the name used to register a JavaScript EGA.

    @param[out] functionReturnStatus    The function's integer return value. 0 means success.
    @param[in] el           The element to update
    @param[in] jsEgaFunctionName   Identifies the JavaScript function to be executed. Must be of the form namespace.functionname. 
    @param[in] origin       The placement origin
    @param[in] angles       The placement angles
    @param[in] parms        Any additional parameters to pass to the EGA function. 
    @return non-zero if the EGA is not in JavaScript, if the egaInstance properties are invalid, or if the JavaScript function could not be found or failed to execute.
    **/
    DGNPLATFORM_EXPORT static DgnDbStatus ExecuteEga(int& functionReturnStatus, DgnElementR el, Utf8CP jsEgaFunctionName, DPoint3dCR origin, YawPitchRollAnglesCR angles, Json::Value const& parms);

    /**
    Call a ComponentModel element generator function that is implemented in JavaScript.

    @verbatim
        export function GenerateElements(componentModel: be.ComponentModel, destModel: be.DgnModel, instance: be.ECInstance, cdef: be.ComponentDef): number
    @endverbatim

    @return non-zero if the specified namespace is not found in the JavaScript library or if the specified function could not be found or failed to execute.
    **/
    DGNPLATFORM_EXPORT static DgnDbStatus ExecuteComponentGenerateElements(int& functionReturnStatus, ComponentModelR componentModel, DgnModelR destModel, ECN::IECInstanceR instance, ComponentDefR cdef, Utf8StringCR functionName);

    DGNPLATFORM_EXPORT static DgnDbStatus ExecuteDgnDbScript(int& functionReturnStatus, DgnDbR db, Utf8StringCR functionName, Json::Value const& parms);

    //! Make sure that the script referenced by \a tsFunctionSpec is loaded.
    //! @param db   The DgnDb that is in use. The script does not have to be stored in the db.
    //! @param tsFunctionSpec Specifies a script in the library, optionally followed by the name of a function, separated with '.'  
    DGNPLATFORM_EXPORT static DgnDbStatus LoadScript(Dgn::DgnDbR db, Utf8CP tsFunctionSpec);

}; 

END_BENTLEY_DGNPLATFORM_NAMESPACE
