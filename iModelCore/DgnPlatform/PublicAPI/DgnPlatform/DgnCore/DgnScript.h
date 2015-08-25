/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/DgnScript.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <DgnPlatform/DgnPlatform.h>
#include <DgnPlatform/DgnCore/DgnElement.h>

BEGIN_BENTLEY_DGN_NAMESPACE

//=======================================================================================
//! Enables JavaScript programs to access the DgnPlatform API.
//! DgnScript creates a set of JavaScript types and functions that expose native Dgn and BentleyApi types to JavaScript functions. These types and functions are collectively known as the DgnPlatform API for JavaScript.
//! DgnScript also provides functions to allow native code to invoke JavaScript functions. See DgnScript::ExecuteJavaScriptEga.
//! @section JavaScriptLibrary The JavaScript Library
//! JavaScript programs are loaded from the JavaScript library. The \a myNamespace portion of the myNamespace.myEgaPublicName EGA identifier string must identify a program in the library.
//! <p>The JavaScript library is a virtual storage. An application may use the DgnJavaScriptLibrary class to store a JavaScript program inside a DgnDb. 
//! Or, an application may override the Dgn::DgnPlatformLib::Host::ScriptAdmin::_FetchJavaScript method in order to locate and supply the text of JavaScript programs from some other source.
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

    <h2>Specifying an EGA in an ECClass.</h2>
    An ECClass that is derived from dgn.ElementItem that wants to specify an EGA must identify the JavaScript function in a custom attribute, like this:
    @verbatim
    <ECClass typeName="SomeItem" isDomainClass="True">
        <BaseClass>dgn.ElementItem</BaseClass>
        ...
        <!-- Here is where I specify what kind of EGA I implement and where to find it. -->
        <ECCustomAttributes xmlns="...">
            <EGASpecifier>
                <Type>JavaScript</Type>
                <Name>myNamespace.myEgaPublicName</Name>
                <!-- These are the properties that I want the handler to look up and pass to my EGA. -->
                <Inputs>prop1, prop2, ...</Inputs>
            </EGASpecifier>
        </ECCustomAttributes>
    </ECClass>
    @endverbatim

    Note that the EGASpecifier identifies the EGA JavaScript function using the same string as the JavaScript program used to register it.
    Also note that the EGASpecifier specifies additional inputs by name. In the example above, they are prop1, prop2, and so forth.
    The EGA JavaScript function expects to find properties by these names on the \a parms object that is passed in as its 4th argument.

    @param[out] functionReturnStatus    The function's integer return value. 0 means success.
    @param[in] el           The element to update
    @param[in] jsEgaFunctionName   Identifies the JavaScript function to be executed. Must be of the form namespace.functionname. 
    @param[in] origin       The placement origin
    @param[in] angles       The placement angles
    @param[in] parms        Any additional parameters to pass to the EGA function. 
    @return non-zero if the EGA is not in JavaScript, if the egaInstance properties are invalid, or if the JavaScript function could not be found or failed to execute.
    **/
    DGNPLATFORM_EXPORT static DgnDbStatus ExecuteEga(int& functionReturnStatus, Dgn::DgnElementR el, Utf8CP jsEgaFunctionName, DPoint3dCR origin, YawPitchRollAnglesCR angles, Json::Value const& parms);

    /**
    Call a DgnModel validation solver function that is implemented in JavaScript.

    <h2>Signature of a JavaScript model solver function.</h2>
    A model solver function written in TypeScript must have the following signature:
    @verbatim
    function myModelSolverFunction(model: BentleyApi.Dgn.JsDgnModel, params: any) : number
    @endverbatim
    Or, in JavaScript:
    @verbatim
    function myModelSolverFunction(model, params)
    @endverbatim

    <h2>Registering a JavaScript model solver function.</h2>
    The JavaScript program must register a model solver function in its start-up logic like this:
    @verbatim
    BentleyApi.Dgn.RegisterModelSolver('myNamespace.myModelSolverPublicName', myModelSolverFunction);
    @endverbatim
    The \a myModelSolverPublicName parameter must match the name used to register a JavaScript model solver.

    <h2>Specifying a JavaScript model solver in an model solver.</h2>
    Every DgnModel can have a model solver. To specify a JavaScript model solver, an application must supply a Solver object in the model's CreateParams, and 
    the Solver object must be set up as follows: type = ModelSolverDef::Type::Script, identifer = full name of the script function in the script library.
    For example,
    @verbatim
    Json::Value parameters(Json::objectValue);
    ... define parameters, as expected by the solver function ...
    ModelSolverDef solver(ModelSolverDef::Type::Script, myNamespace.myModelSolverPublicName", parameters);
    @endverbatim

    Note the use of the ModelSolverDef::Type::Script type. 
    Also note that the solver identifier "myNamespace.myModelSolverPublicName" must match the string passed to BentleyApi.Dgn.RegisterModelSolver.
    And, the namespace part of the identifer "myNamespace" must match the identifier of a JavaScript program in the script library.

    @param[out] functionReturnStatus    The function's integer return value. 0 means success.
    @param[in] model           The model to validate
    @param[in] jsFunctionName   Identifies the Script function to be executed. Must be of the form namespace.functionname
    @param[in] parms        The parameters to pass to the solver. 
    @return non-zero if the specified namespace is not found in the JavaScript library or if the specified function could not be found or failed to execute.
    **/
    DGNPLATFORM_EXPORT static DgnDbStatus ExecuteModelSolver(int& functionReturnStatus, Dgn::DgnModelR model, Utf8CP jsFunctionName, Json::Value const& parms);
}; 

END_BENTLEY_DGN_NAMESPACE
