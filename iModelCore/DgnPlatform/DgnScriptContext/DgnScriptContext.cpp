/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnScriptContext/DgnScriptContext.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#ifndef BENTLEYCONFIG_OS_WINRT
#include <BeJavaScript/BeJavaScript.h>
#endif
#include <DgnPlatform/DgnCore/DgnScriptContext.h>
#include <Bentley/BeFileListIterator.h>
#include <ECObjects/ECObjectsAPI.h>
#include <DgnPlatform/DgnPlatformApi.h>
#include <DgnPlatform/DgnPlatformLib.h>

extern Utf8CP dgnJavaDgnScriptContextImpl_GetBootstrappingSource();

#ifndef BENTLEYCONFIG_OS_WINRT

BEJAVASCRIPT_EMIT_CUSTOM_TYPESCRIPT_DECLARATION ("export declare class BeJsNativePointer {}")

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      06/15
//---------------------------------------------------------------------------------------
static void deleteThis(void* b) {delete (BeJsNativePointer*)b;}

//---------------------------------------------------------------------------------------
// Type conversion helpers
// @bsimethod                                   Sam.Wilson                      06/15
//---------------------------------------------------------------------------------------
static Utf8CP getUtf8CP(Utf8StringCR s) {return s.c_str();}

//=======================================================================================
// A wrapper class wraps a JsObject around a native value.
// Every wrapper class is-a BeJsNativePointer, and it holds the corresponding native value as its data.
// The lifetime of a wrapper class instance is controlled by the lifetime of the JS object.
// The JS object invokes the supplied BeJsNativePointer::DisposeCallback on the wrapper object when
// the JS object is garbage-collected. Therefore, all wrapper instances must be created using new and must
// not be deleted from the native side. Wrapper instances must not be allocated on the stack!
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
struct JsDgnElementPrototype;
BEJAVASCRIPT_EXPORT_CLASS struct JsDgnElement : BeJsNativePointer
{
    // Every wrapper class will hold the native value
    DgnElementPtr m_el;

    //  Interface methods - These are specific to each kind of native type

    //! Get the element's DgnElementId
    Utf8String GetElementId();
    BEJAVASCRIPT_DECLARE_INSTANCE_CALLBACK(JsDgnElement, GetElementId)

    //  The constructor for every wrapper class will be the same
    JsDgnElement(DgnElementR el, BeJsContext& jsctx, BeJsObject& jsprototype) : BeJsNativePointer(jsctx, this, deleteThis, &jsprototype), m_el(&el) {;}
    };

typedef JsDgnElement* JsDgnElementP;

// Implement the instance functions and the corresponding callback marshalling functions
Utf8String JsDgnElement::GetElementId() {return Utf8PrintfString("%lld", (long long)m_el->GetElementId().GetValueUnchecked());}
BEJAVASCRIPT_DEFINE_INSTANCE_CALLBACK(JsDgnElement, GetElementId, String, getUtf8CP)

//  Every wrapper class will have a Prototype class associated with it.
//  The prototype is paired with a JS object that has properties that are the get/set and other instance functions.
struct JsDgnElementPrototype
{
    BeJsContext& m_ctx;
    BeJsContext& GetJsContext() {return m_ctx;}

    BEJAVASCRIPT_DECLARE_JS_CALLBACKS_HOLDER(JsDgnElement)  // this just declares a member variable of type JsObject
    BeJsObject& GetJsObject() {return BEJAVASCRIPT_JS_CALLBACKS_HOLDER(JsDgnElement);}

    JsDgnElementPrototype(BeJsContext& ctx) : m_ctx(ctx),
        BEJAVASCRIPT_INSTANTIATE_JS_CALLBACKS_HOLDER(JsDgnElement)  // this constructs the JsObject
        {
        //  Set the function properties of the JS prototype object, one for each callback method
        BEJAVASCRIPT_INSTANTIATE_INSTANCE_FUNCTION_CALLBACK(JsDgnElement, GetElementId);

        //  Finish the initialization of the JS prototype object.
        BEJAVASCRIPT_INITIALIZE_CLASS("BentleyApi.Dgn", JsDgnElement)
        }

    //  Make an instance
    JsDgnElement* Create(DgnElementR el) {return new JsDgnElement(el, m_ctx, GetJsObject());}
};

//=======================================================================================
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
BEJAVASCRIPT_EXPORT_CLASS struct JsDPoint3d : BeJsNativePointer
{
    DPoint3d m_pt;

    //! Get the X coordinate
    double GetX();
    BEJAVASCRIPT_DECLARE_INSTANCE_CALLBACK(JsDPoint3d, GetX)
    //! Get the Y coordinate
    double GetY();
    BEJAVASCRIPT_DECLARE_INSTANCE_CALLBACK(JsDPoint3d, GetY)
    //! Get the Z coordinate
    double GetZ();
    BEJAVASCRIPT_DECLARE_INSTANCE_CALLBACK(JsDPoint3d, GetZ)
    //! Set the X coordinate
    void SetX(double meters);
    BEJAVASCRIPT_DECLARE_INSTANCE_CALLBACK(JsDPoint3d, SetX)
    //! Set the Y coordinate
    void SetY(double meters);
    BEJAVASCRIPT_DECLARE_INSTANCE_CALLBACK(JsDPoint3d, SetY)
    //! Set the Z coordinate
    void SetZ(double meters);
    BEJAVASCRIPT_DECLARE_INSTANCE_CALLBACK(JsDPoint3d, SetZ)

    JsDPoint3d(DPoint3dCR pt, BeJsContext& jsctx, BeJsObject& jsprototype) : BeJsNativePointer(jsctx, this, deleteThis, &jsprototype), m_pt(pt) {;}
};

typedef JsDPoint3d* JsDPoint3dP;

BEJAVASCRIPT_DEFINE_INSTANCE_CALLBACK(JsDPoint3d, GetX, Number, )
BEJAVASCRIPT_DEFINE_INSTANCE_CALLBACK(JsDPoint3d, GetY, Number, )
BEJAVASCRIPT_DEFINE_INSTANCE_CALLBACK(JsDPoint3d, GetZ, Number, )
BEJAVASCRIPT_DEFINE_INSTANCE_CALLBACK_VOID_1ARG(JsDPoint3d, SetX, double, Number, GetValue)
BEJAVASCRIPT_DEFINE_INSTANCE_CALLBACK_VOID_1ARG(JsDPoint3d, SetY, double, Number, GetValue)
BEJAVASCRIPT_DEFINE_INSTANCE_CALLBACK_VOID_1ARG(JsDPoint3d, SetZ, double, Number, GetValue)

double JsDPoint3d::GetX() {return m_pt.x;}
double JsDPoint3d::GetY() {return m_pt.y;}
double JsDPoint3d::GetZ() {return m_pt.z;}
void JsDPoint3d::SetX(double v) {m_pt.x = v;}
void JsDPoint3d::SetY(double v) {m_pt.y = v;}
void JsDPoint3d::SetZ(double v) {m_pt.z = v;}

//  Every wrapper class will have a Prototype class associated with it.
//  The prototype is paired with a JS object that has properties that are the get/set and other instance functions.
struct JsDPoint3dPrototype
    {
    BeJsContext& m_ctx;
    BeJsContext& GetJsContext() {return m_ctx;}

    BEJAVASCRIPT_DECLARE_JS_CALLBACKS_HOLDER(JsDPoint3d)  // this just declares a member variable of type JsObject
    BeJsObject& GetJsObject() {return BEJAVASCRIPT_JS_CALLBACKS_HOLDER(JsDPoint3d);}

    JsDPoint3dPrototype(BeJsContext& ctx) : m_ctx(ctx),
        BEJAVASCRIPT_INSTANTIATE_JS_CALLBACKS_HOLDER(JsDPoint3d)  // this constructs the JsObject
        {
        //  Set the function properties of the JS prototype object, one for each callback method
        BEJAVASCRIPT_INSTANTIATE_INSTANCE_PROPERTY_CALLBACKS(JsDPoint3d, X);
        BEJAVASCRIPT_INSTANTIATE_INSTANCE_PROPERTY_CALLBACKS(JsDPoint3d, Y);
        BEJAVASCRIPT_INSTANTIATE_INSTANCE_PROPERTY_CALLBACKS(JsDPoint3d, Z);

        //  Finish the initialization of the JS prototype object.
        BEJAVASCRIPT_INITIALIZE_CLASS("BentleyApi.Dgn", JsDPoint3d)
        }

    JsDPoint3d* Create(DPoint3dCR pt) {return new JsDPoint3d(pt, m_ctx, GetJsObject());}
    };

//=======================================================================================
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
BEJAVASCRIPT_EXPORT_CLASS struct JsYawPitchRollAngles : BeJsNativePointer
{
    YawPitchRollAngles m_angles;

    //! Get the Yaw angle
    double GetYaw();
    BEJAVASCRIPT_DECLARE_INSTANCE_CALLBACK(JsYawPitchRollAngles, GetYaw)
    //! Get the Pitch angle
    double GetPitch();
    BEJAVASCRIPT_DECLARE_INSTANCE_CALLBACK(JsYawPitchRollAngles, GetPitch)
    //! Get the Roll angle
    double GetRoll();
    BEJAVASCRIPT_DECLARE_INSTANCE_CALLBACK(JsYawPitchRollAngles, GetRoll)
    //! Set the Yaw angle
    void SetYaw(double degrees);
    BEJAVASCRIPT_DECLARE_INSTANCE_CALLBACK(JsYawPitchRollAngles, SetYaw)
    //! Set the Pitch angle
    void SetPitch(double degrees);
    BEJAVASCRIPT_DECLARE_INSTANCE_CALLBACK(JsYawPitchRollAngles, SetPitch)
    //! Set the Roll angle
    void SetRoll(double degrees);
    BEJAVASCRIPT_DECLARE_INSTANCE_CALLBACK(JsYawPitchRollAngles, SetRoll)

    JsYawPitchRollAngles(YawPitchRollAnglesCR angles, BeJsContext& jsctx, BeJsObject& jsprototype) : BeJsNativePointer(jsctx, this, deleteThis, &jsprototype), m_angles(angles) {;}
};

typedef JsYawPitchRollAngles* JsYawPitchRollAnglesP;

BEJAVASCRIPT_DEFINE_INSTANCE_CALLBACK(JsYawPitchRollAngles, GetYaw  , Number, )
BEJAVASCRIPT_DEFINE_INSTANCE_CALLBACK(JsYawPitchRollAngles, GetPitch, Number, )
BEJAVASCRIPT_DEFINE_INSTANCE_CALLBACK(JsYawPitchRollAngles, GetRoll , Number, )
BEJAVASCRIPT_DEFINE_INSTANCE_CALLBACK_VOID_1ARG(JsYawPitchRollAngles, SetYaw  , double, Number, GetValue)
BEJAVASCRIPT_DEFINE_INSTANCE_CALLBACK_VOID_1ARG(JsYawPitchRollAngles, SetPitch, double, Number, GetValue)
BEJAVASCRIPT_DEFINE_INSTANCE_CALLBACK_VOID_1ARG(JsYawPitchRollAngles, SetRoll , double, Number, GetValue)

double JsYawPitchRollAngles::GetYaw  () {return m_angles.GetYaw().Degrees();}
double JsYawPitchRollAngles::GetPitch() {return m_angles.GetPitch().Degrees();}
double JsYawPitchRollAngles::GetRoll () {return m_angles.GetRoll().Degrees();}
void JsYawPitchRollAngles::SetYaw  (double v) {m_angles.FromDegrees(v, GetPitch(), GetRoll());}
void JsYawPitchRollAngles::SetPitch(double v) {m_angles.FromDegrees(GetYaw(), v, GetRoll());}
void JsYawPitchRollAngles::SetRoll (double v) {m_angles.FromDegrees(GetYaw(), GetPitch(), v);}


//  Every wrapper class will have a Prototype class associated with it.
//  The prototype is paired with a JS object that has properties that are the get/set and other instance functions.
struct JsYawPitchRollAnglesPrototype
    {
    BeJsContext& m_ctx;
    BeJsContext& GetJsContext() {return m_ctx;}

    BEJAVASCRIPT_DECLARE_JS_CALLBACKS_HOLDER(JsYawPitchRollAngles)  // this just declares a member variable of type JsObject
    BeJsObject& GetJsObject() {return BEJAVASCRIPT_JS_CALLBACKS_HOLDER(JsYawPitchRollAngles);}

    JsYawPitchRollAnglesPrototype(BeJsContext& ctx) : m_ctx(ctx),
        BEJAVASCRIPT_INSTANTIATE_JS_CALLBACKS_HOLDER(JsYawPitchRollAngles)  // this constructs the JsObject
        {
        //  Set the function properties of the JS prototype object, one for each callback method
        BEJAVASCRIPT_INSTANTIATE_INSTANCE_PROPERTY_CALLBACKS(JsYawPitchRollAngles, Yaw);
        BEJAVASCRIPT_INSTANTIATE_INSTANCE_PROPERTY_CALLBACKS(JsYawPitchRollAngles, Pitch);
        BEJAVASCRIPT_INSTANTIATE_INSTANCE_PROPERTY_CALLBACKS(JsYawPitchRollAngles, Roll);

        //  Finish the initialization of the JS prototype object.
        BEJAVASCRIPT_INITIALIZE_CLASS("BentleyApi.Dgn", JsYawPitchRollAngles)
        }

    JsYawPitchRollAngles* Create(YawPitchRollAnglesCR a) {return new JsYawPitchRollAngles(a, m_ctx, GetJsObject());}
    };

//=======================================================================================
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
BEJAVASCRIPT_EXPORT_CLASS struct JsElementGeometryBuilder : BeJsNativePointer
{
    ElementGeometryBuilderPtr m_builder;

    JsElementGeometryBuilder(DgnElement3dR el, DPoint3dCR o, YawPitchRollAnglesCR angles, BeJsContext& jsctx, BeJsObject& jsprototype) 
        : BeJsNativePointer(jsctx, this, deleteThis, &jsprototype),
        m_builder (ElementGeometryBuilder::Create(el, o, angles)) 
        {;}
    JsElementGeometryBuilder(DgnElement2dR el, DPoint2dCR o, AngleInDegrees angle, BeJsContext& jsctx, BeJsObject& jsprototype)
        : BeJsNativePointer(jsctx, this, deleteThis, &jsprototype),
        m_builder (ElementGeometryBuilder::Create(el, o, angle)) 
        {;}

    BeJsObjectR GetJsObject() {return *this;}

    static JsElementGeometryBuilder* Create(JsDgnElementP el, JsDPoint3dP o, JsYawPitchRollAnglesP angles);
    BEJAVASCRIPT_DECLARE_STATIC_CALLBACK(JsElementGeometryBuilder, Create)

    void AppendBox(double h, double w, double l);
    BEJAVASCRIPT_DECLARE_INSTANCE_CALLBACK(JsElementGeometryBuilder, AppendBox);

    BentleyStatus SetGeomStreamAndPlacement (JsDgnElementP element);
    BEJAVASCRIPT_DECLARE_INSTANCE_CALLBACK(JsElementGeometryBuilder, SetGeomStreamAndPlacement);
};
typedef JsElementGeometryBuilder* JsElementGeometryBuilderP;

BEJAVASCRIPT_DEFINE_STATIC_CALLBACK_NULLABLEOBJECT_3ARGS(JsElementGeometryBuilder, Create,
    JsElementGeometryBuilderP,
    JsDgnElementP, NativePointer, GetValue,
    JsDPoint3dP, NativePointer, GetValue,
    JsYawPitchRollAnglesP, NativePointer, GetValue)

BEJAVASCRIPT_DEFINE_INSTANCE_CALLBACK_VOID_3ARGS(JsElementGeometryBuilder, AppendBox, 
    double, Number, GetValue, 
    double, Number, GetValue, 
    double, Number, GetValue)

BEJAVASCRIPT_DEFINE_INSTANCE_CALLBACK_1ARG(JsElementGeometryBuilder, SetGeomStreamAndPlacement,
    Number, BentleyStatus,
    JsDgnElementP, NativePointer, GetValue)

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      06/15
//---------------------------------------------------------------------------------------
BentleyStatus JsElementGeometryBuilder::SetGeomStreamAndPlacement(JsDgnElementP el) {return m_builder->SetGeomStreamAndPlacement(*el->m_el->ToGeometricElementP());}

//---------------------------------------------------------------------------------------
// *** TEMPORARY METHOD *** 
// @bsimethod                                   Sam.Wilson                      06/15
//---------------------------------------------------------------------------------------
void JsElementGeometryBuilder::AppendBox(double h, double w, double l)
    {
    // *** TEMPORARY METHOD *** 
    DPoint3d localOrigin;
    localOrigin.x = 0.0;
    localOrigin.y = 0.0;
    localOrigin.z = 0.0;

    DPoint3d localTop (localOrigin);
    localTop.z = h;

    DVec3d localX = DVec3d::From(1,0,0);
    DVec3d localY = DVec3d::From(0,1,0);

    DgnBoxDetail boxd(localOrigin, localTop, localX, localY, l, w, l, w, true);
    ISolidPrimitivePtr solid = ISolidPrimitive::CreateDgnBox(boxd);

    m_builder->Append(*solid);
    }

//  Every wrapper class will have a Prototype class associated with it.
//  The prototype is paired with a JS object that has properties that are the get/set and other instance functions.
struct JsElementGeometryBuilderPrototype
    {
    BeJsContext& m_ctx;
    BeJsContext& GetJsContext() {return m_ctx;}

    BEJAVASCRIPT_DECLARE_JS_CALLBACKS_HOLDER(JsElementGeometryBuilder)  // this just declares a member variable of type JsObject
    BeJsObject& GetJsObject() {return BEJAVASCRIPT_JS_CALLBACKS_HOLDER(JsElementGeometryBuilder);}

    JsElementGeometryBuilderPrototype(BeJsContext& ctx) : m_ctx(ctx),
        BEJAVASCRIPT_INSTANTIATE_JS_CALLBACKS_HOLDER(JsElementGeometryBuilder)  // this constructs the JsObject
        {
        //  Set the function properties of the JS prototype object, one for each callback method
        BEJAVASCRIPT_INSTANTIATE_INSTANCE_FUNCTION_CALLBACK(JsElementGeometryBuilder, AppendBox);
        BEJAVASCRIPT_INSTANTIATE_INSTANCE_FUNCTION_CALLBACK(JsElementGeometryBuilder, SetGeomStreamAndPlacement);
        BEJAVASCRIPT_INSTANTIATE_STATIC_FUNCTION_CALLBACK(JsElementGeometryBuilder, Create);
        
        //  Finish the initialization of the JS prototype object.
        BEJAVASCRIPT_INITIALIZE_CLASS("BentleyApi.Dgn", JsElementGeometryBuilder)
        }

    JsElementGeometryBuilder* Create(DgnElement3dR el, DPoint3dCR o, YawPitchRollAnglesCR angles) {return new JsElementGeometryBuilder(el, o, angles, m_ctx, GetJsObject());}
    JsElementGeometryBuilder* Create(DgnElement2dR el, DPoint2dCR o, AngleInDegrees angle) {return new JsElementGeometryBuilder(el, o, angle, m_ctx, GetJsObject());}
    };

//=======================================================================================
// This class holds the one and only BeJsEnvironment and BeJsContext for the session.
// It defines all of the DgnJavaScript Object Model proxies and registers them in JavaScript.
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
struct DgnScriptContextImpl : BeJsContext
{
    struct Prototypes
        {
        JsDgnElementPrototype         m_dgnElement;
        JsDPoint3dPrototype           m_dpoint3d;
        JsYawPitchRollAnglesPrototype m_angles;
        JsElementGeometryBuilderPrototype m_builder;

        Prototypes(BeJsContext& ctx) : m_dgnElement(ctx), m_dpoint3d(ctx), m_angles(ctx), m_builder(ctx)
            {;}
        };

    // -------------------------------
    // Member variables
    // -------------------------------
    BeJsObject m_egaRegistry;
    bset<Utf8String> m_jsScriptsExecuted;
    Prototypes m_prototypes;

    // -------------------------------
    // Member functions
    // -------------------------------
    DgnScriptContextImpl(BeJsEnvironmentR);
    ~DgnScriptContextImpl();
    DgnDbStatus LoadProgram(Dgn::DgnDbR db, Utf8CP tsFunctionSpec);
    DgnDbStatus ExecuteJavaScriptEga(int& functionReturnStatus, Dgn::DgnElementR el, Utf8CP jsEgaFunctionName, DPoint3dCR origin, YawPitchRollAnglesCR angles, Json::Value const& parms);
};

END_BENTLEY_DGNPLATFORM_NAMESPACE

USING_NAMESPACE_BENTLEY_DGNPLATFORM

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      06/15
//---------------------------------------------------------------------------------------
JsElementGeometryBuilder* JsElementGeometryBuilder::Create(JsDgnElementP e, JsDPoint3dP o, JsYawPitchRollAnglesP a)
    {
    DgnScriptContextImpl* holder = static_cast<DgnScriptContextImpl*>(const_cast<BeJsContext*>(&e->GetContext()));

    DgnElement3dP e3d = dynamic_cast<DgnElement3dP>(e->m_el.get());
    if (nullptr != e3d)
        return holder->m_prototypes.m_builder.Create(*e3d, o->m_pt, a->m_angles);

    DgnElement2dP e2d = dynamic_cast<DgnElement2dP>(e->m_el.get());
    if (nullptr != e2d)
        return holder->m_prototypes.m_builder.Create(*e2d, DPoint2d::From(o->GetX(), o->GetY()), AngleInDegrees::FromDegrees(a->GetYaw()));

    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      06/15
//---------------------------------------------------------------------------------------
DgnScriptContextImpl::DgnScriptContextImpl(BeJsEnvironmentR jsenv)
    : 
    BeJsContext(jsenv, "DgnScriptContext", dgnJavaDgnScriptContextImpl_GetBootstrappingSource()),
    m_egaRegistry(EvaluateScript("BentleyApi.Dgn.GetEgaRegistry()")),
    m_prototypes(*this)
    {
    BeAssert(!m_egaRegistry.IsUndefined() && m_egaRegistry.IsObject());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      06/15
//---------------------------------------------------------------------------------------
DgnScriptContextImpl::~DgnScriptContextImpl()
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   BentleySystems
//---------------------------------------------------------------------------------------
DgnDbStatus DgnScriptContextImpl::LoadProgram(Dgn::DgnDbR db, Utf8CP tsFunctionSpec)
    {
    Utf8String tsProgramName;
    Utf8CP dot = strrchr(tsFunctionSpec, '.');
    if (nullptr == dot)
        {
        NativeLogging::LoggingManager::GetLogger("DgnScriptContext")->errorv ("[%s] is an illegal JavaScript function spec. Must be of the form program.function", tsFunctionSpec);
        BeAssert(false && "illegal JavaScript function spec");
        return DgnDbStatus::BadArg;
        }

    tsProgramName.assign(tsFunctionSpec, dot);

    if (m_jsScriptsExecuted.find(tsProgramName) != m_jsScriptsExecuted.end())
        return DgnDbStatus::Success;

    Utf8String tsprog;
    DgnJavaScriptLibrary jslib(db);
    if (jslib.QueryJavaScript(tsprog, tsProgramName.c_str()) != BSISUCCESS)
        {
        NativeLogging::LoggingManager::GetLogger("DgnScriptContext")->infov ("JavaScript program %s is not registered", tsProgramName.c_str());
        return DgnDbStatus::NotFound;
        }

    m_jsScriptsExecuted.insert(tsProgramName);

    Utf8String fileUrl("file:///"); // This does not really identify a file. It is something tricky that is needed to get JS to accept the script that we pass in tsprog.
    fileUrl.append(tsProgramName);
    fileUrl.append(".js");

    NativeLogging::LoggingManager::GetLogger("DgnScriptContext")->tracev ("Evaluating %s", tsProgramName.c_str());

    EvaluateScript(tsprog.c_str(), fileUrl.c_str());   // evaluate the whole script, allowing it to define objects and their properties. 
    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   BentleySystems
//---------------------------------------------------------------------------------------
DgnDbStatus DgnScriptContextImpl::ExecuteJavaScriptEga(int& functionReturnStatus, Dgn::DgnElementR el, Utf8CP jsEgaFunctionName, DPoint3dCR origin, YawPitchRollAnglesCR angles, Json::Value const& parms)
    {
    functionReturnStatus = -1;

    DgnDbStatus status = LoadProgram(el.GetDgnDb(), jsEgaFunctionName);
    if (DgnDbStatus::Success != status)
        return status;

    BeJsFunction jsfunc = m_egaRegistry.GetFunctionProperty(jsEgaFunctionName);
    if (jsfunc.IsUndefined() || !jsfunc.IsFunction())
        {
        NativeLogging::LoggingManager::GetLogger("DgnScriptContext")->errorv ("[%s] is not registered as an EGA", jsEgaFunctionName);
        BeAssert(false && "EGA not registered");
        return DgnDbStatus::NotEnabled;
        }

    BeJsObject parmsObj = EvaluateJson(parms);
    JsDgnElement* jsel = m_prototypes.m_dgnElement.Create(el);
    JsDPoint3d* jsorigin = m_prototypes.m_dpoint3d.Create(origin);
    JsYawPitchRollAngles* jsangles = m_prototypes.m_angles.Create(angles);
    BeJsValue retval = jsfunc(*jsel, *jsorigin, *jsangles, parmsObj);

    if (!retval.IsNumber())
        {
        NativeLogging::LoggingManager::GetLogger("DgnScriptContext")->errorv ("[%s] does not have the correct signature - must return an int", jsEgaFunctionName);
        BeAssert(false && "EGA has incorrect return type");
        return DgnDbStatus::NotEnabled;
        }

    BeJsNumber num(retval);
    functionReturnStatus = num.GetIntegerValue();
    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      06/15
//---------------------------------------------------------------------------------------
DgnScriptContext::DgnScriptContext(BeJsEnvironmentR jsenv)
    {
    m_pimpl = new DgnScriptContextImpl(jsenv);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      06/15
//---------------------------------------------------------------------------------------
DgnScriptContext::~DgnScriptContext()
    {
    delete m_pimpl;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      07/15
//---------------------------------------------------------------------------------------
DgnDbStatus DgnScriptContext::ExecuteJavaScriptEga(int& functionReturnStatus, Dgn::DgnElementR el, Utf8CP jsEgaFunctionName, DPoint3dCR origin, YawPitchRollAnglesCR angles, Json::Value const& parms)
    {
    return m_pimpl->ExecuteJavaScriptEga(functionReturnStatus, el, jsEgaFunctionName, origin, angles, parms);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      07/15
//---------------------------------------------------------------------------------------
DgnPlatformLib::Host::ScriptingAdmin::ScriptingAdmin()
    {
    m_jsenv = nullptr;
    m_dgnContext = nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      07/15
//---------------------------------------------------------------------------------------
DgnPlatformLib::Host::ScriptingAdmin::~ScriptingAdmin()
    {
    if (nullptr != m_dgnContext)
        delete m_dgnContext;
    if (nullptr != m_jsenv)
        delete m_jsenv;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      07/15
//---------------------------------------------------------------------------------------
BeJsEnvironmentR DgnPlatformLib::Host::ScriptingAdmin::GetBeJsEnvironment()
    {
    if (nullptr == m_jsenv)
        m_jsenv = new BeJsEnvironment;
    return *m_jsenv;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      07/15
//---------------------------------------------------------------------------------------
DgnScriptContextR DgnPlatformLib::Host::ScriptingAdmin::GetDgnScriptContext()
    {
    if (nullptr == m_dgnContext)
        m_dgnContext = new DgnScriptContext(GetBeJsEnvironment());
    return *m_dgnContext;
    }

#else

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      07/15
//---------------------------------------------------------------------------------------
DgnDbStatus DgnScriptContext::ExecuteJavaScriptEga(int& functionReturnStatus, Dgn::DgnElementR el, Utf8CP jsEgaFunctionName, DPoint3dCR origin, YawPitchRollAnglesCR angles, Json::Value const& parms)
    {
    return DgnDbStatus::NotEnabled;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      07/15
//---------------------------------------------------------------------------------------
DgnPlatformLib::Host::ScriptingAdmin::ScriptingAdmin()
    {
    m_jsenv = nullptr;
    m_dgnContext = nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      07/15
//---------------------------------------------------------------------------------------
DgnPlatformLib::Host::ScriptingAdmin::~ScriptingAdmin()
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      07/15
//---------------------------------------------------------------------------------------
BeJsEnvironmentR DgnPlatformLib::Host::ScriptingAdmin::GetBeJsEnvironment()
    {
    BeAssert(false);
    return *m_jsenv;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      07/15
//---------------------------------------------------------------------------------------
DgnScriptContextR DgnPlatformLib::Host::ScriptingAdmin::GetDgnScriptContext()
    {
    BeAssert(false);
    return *m_dgnContext;
    }

#endif