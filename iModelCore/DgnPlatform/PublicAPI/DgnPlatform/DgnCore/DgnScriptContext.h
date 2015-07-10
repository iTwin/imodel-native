/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/DgnScriptContext.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <DgnPlatform/DgnPlatform.h>
#include <DgnPlatform/DgnCore/DgnElement.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

struct DgnScriptContextImpl;

//=======================================================================================
//! Enables JavaScript programs to access the DgnPlatform API.
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
struct DgnScriptContext
{
private:
    friend struct DgnScriptContextImpl;
    DgnScriptContextImpl* m_pimpl;

public:
    DGNPLATFORM_EXPORT DgnScriptContext(BeJsEnvironmentR);
    DGNPLATFORM_EXPORT ~DgnScriptContext();
   
    /**
    Execute an Element Generation Algorithm (EGA) that is implemented in JavaScript. The \a jsEgaFunctionName identifies the EGA function. The namespace portion of the name
    must identify a JavaScript program that was previously registered in the DgnDb. See BentleyApi::DgnPlatform::DgnJavaScriptLibrary.
    @param[out] functionReturnStatus    The function's integer return value. 0 means success.
    @param[in] el           The element to update
    @param[in] jsEgaFunctionName   Identifies the JavaScript function to be executed. Must be of the form namespace.functionname
    @param[in] origin       The placement origin
    @param[in] angles       The placement angles
    @param[in] parms        Any additional parameters to pass to the EGA function. 
    @return non-zero if the EGA is not in JavaScript, if the egaInstance properties are invalid, or if the JavaScript function could not be found or failed to execute.
    @remarks Signature of a JavaScript EGA function.
    An EGA function written in TypeScript must have the following signature:
    <p>    function myEgaFunction(element: BentleyApi.Dgn.JsDgnElement, origin: BentleyApi.Dgn.JsDPoint3d, angles: BentleyApi.Dgn.JsYawPitchRollAngles, params: any) : number
    <p>Or, in JavaScript:
    <p>    function myEgaFunction(element, origin, angles, params)
    <p> The JavaScript \em program must register an EGA function in its start-up logic like this:
        BentleyApi.Dgn.RegisterEGA('myNamespace.myEgaPublicName', myEgaFunction);
    <p>
    The string myNamespace.myEgaPublicName is meant to be the way that an ECClass can identify the EGA by name.
    An EGA is specified by an ECClass by naming it in a custom attribute, like this:
    @verbatim
    <ECClass typeName="SomeItem" isDomainClass="True">
        <BaseClass>some item base class</BaseClass>
        ...
        <!-- Here is where is specify what kind of EGA I implement and where to find it. -->
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
    **/
    DGNPLATFORM_EXPORT DgnDbStatus ExecuteJavaScriptEga(int& functionReturnStatus, Dgn::DgnElementR el, Utf8CP jsEgaFunctionName, DPoint3dCR origin, YawPitchRollAnglesCR angles, Json::Value const& parms);

}; 

END_BENTLEY_DGNPLATFORM_NAMESPACE
