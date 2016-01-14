/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnScript/DgnScript.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <BeJavaScript/BeJavaScript.h>
#include <DgnPlatform/DgnScript.h>
#include <Bentley/BeFileListIterator.h>
#include <ECObjects/ECObjectsAPI.h>
#include <DgnPlatform/DgnPlatformApi.h>
#include <DgnPlatform/DgnPlatformLib.h>
#include <DgnPlatform/GeomJsApi.h>
#include <DgnPlatform/DgnJsApi.h>
#include <DgnPlatform/DgnJsApiProjection.h>

extern Utf8CP dgnScriptContext_GetBootstrappingSource();

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE
struct DgnScriptContext : BeJsContext
{
    BeJsObject m_egaRegistry;
    BeJsObject m_modelSolverRegistry;
    BeJsObject m_dgndbScriptRegistry;
    bset<Utf8String> m_jsScriptsExecuted;

    DgnScriptContext(BeJsEnvironmentR jsenv) : BeJsContext(jsenv, "DgnScript")
        {
        }

    void Initialize()
        {
        EvaluateScript(dgnScriptContext_GetBootstrappingSource(), "file:///DgnJsApi.js");
        
        m_egaRegistry = EvaluateScript("Bentley.Dgn.GetEgaRegistry()");
        m_modelSolverRegistry = EvaluateScript("Bentley.Dgn.GetModelSolverRegistry()");
        m_dgndbScriptRegistry = EvaluateScript("Bentley.Dgn.GetDgnDbScriptRegistry()");

        BeAssert(!m_egaRegistry.IsUndefined() && m_egaRegistry.IsObject());
        BeAssert(!m_modelSolverRegistry.IsUndefined() && m_modelSolverRegistry.IsObject());
        BeAssert(!m_dgndbScriptRegistry.IsUndefined() && m_dgndbScriptRegistry.IsObject());
        }

    DgnDbStatus LoadProgram(Dgn::DgnDbR db, Utf8CP tsFunctionSpec);
    DgnDbStatus ExecuteEga(int& functionReturnStatus, Dgn::DgnElementR el, Utf8CP jsEgaFunctionName, DPoint3dCR origin, YawPitchRollAnglesCR angles, Json::Value const& parms);
    DgnDbStatus ExecuteComponentGenerateElements(int& functionReturnStatus, Dgn::ComponentModelR componentModel, Dgn::DgnModelR destModel, ECN::IECInstanceR instance, Dgn::ComponentDefR cdef, Utf8StringCR functionName);
    DgnDbStatus ExecuteDgnDbScript(int& functionReturnStatus, Dgn::DgnDbR db, Utf8StringCR jsFunctionName, Json::Value const& parms);
};
END_BENTLEY_DGNPLATFORM_NAMESPACE


USING_NAMESPACE_BENTLEY_DGNPLATFORM

//---------------------------------------------------------------------------------------
// @bsimethod                                                   BentleySystems
//---------------------------------------------------------------------------------------
DgnDbStatus DgnScriptContext::LoadProgram(Dgn::DgnDbR db, Utf8CP jsFunctionSpec)
    {
    Utf8String jsProgramName;
    Utf8CP dot = strrchr(jsFunctionSpec, '.');
    if (nullptr == dot || 0==BeStringUtilities::Stricmp(".js", dot) || 0 == BeStringUtilities::Stricmp(".ts", dot))
        {
        jsProgramName = jsFunctionSpec;
        }
    else
        {
        jsProgramName.assign(jsFunctionSpec, dot);
        }

    if (m_jsScriptsExecuted.find(jsProgramName) != m_jsScriptsExecuted.end())
        return DgnDbStatus::Success;

    DgnScriptType sTypePreferred = DgnScriptType::JavaScript;
    DgnScriptType sTypeFound;
    Utf8String jsprog;
    DateTime lastModifiedTime;
    DgnDbStatus status = T_HOST.GetScriptAdmin()._FetchScript(jsprog, sTypeFound, lastModifiedTime, db, jsProgramName.c_str(), sTypePreferred);
    if (DgnDbStatus::Success != status)
        {
        NativeLogging::LoggingManager::GetLogger("DgnScript")->infov ("Script program %s is not registered in the script library", jsProgramName.c_str());
        return status;
        }

    if (sTypeFound != DgnScriptType::JavaScript)
        {
        switch (sTypeFound)
            {
            case DgnScriptType::TypeScript:
                BeAssert(false && "*** TBD: compile script type on the fly");
                return DgnDbStatus::NotEnabled;
                break;
            default:
                BeAssert(false && "Unsupported script type");
                return DgnDbStatus::NotEnabled;
                break;
            }
        }

    m_jsScriptsExecuted.insert(jsProgramName);

    Utf8String fileUrl("file:///"); // This does not really identify a file. It is something tricky that is needed to get JS to accept the script that we pass in jsprog.
    fileUrl.append(jsProgramName);
    fileUrl.append(".js");

    NativeLogging::LoggingManager::GetLogger("DgnScript")->tracev ("Evaluating %s", jsProgramName.c_str());

    EvaluateStatus jsStatus = EvaluateStatus::Success;
    EvaluateException jsException;
    EvaluateScript(jsprog.c_str(), fileUrl.c_str(), &jsStatus, &jsException);   // evaluate the whole script, allowing it to define objects and their properties. 

    if (EvaluateStatus::Success != jsStatus)
        {
        if (EvaluateStatus::RuntimeError == jsStatus)
            T_HOST.GetScriptAdmin().HandleScriptError(DgnPlatformLib::Host::ScriptAdmin::ScriptNotificationHandler::Category::Exception, jsException.message.c_str(), jsException.trace.c_str());
        else if (EvaluateStatus::ParseError   == jsStatus)
            T_HOST.GetScriptAdmin().HandleScriptError(DgnPlatformLib::Host::ScriptAdmin::ScriptNotificationHandler::Category::ParseError, jsProgramName.c_str(), jsException.message.c_str());
        else
            T_HOST.GetScriptAdmin().HandleScriptError(DgnPlatformLib::Host::ScriptAdmin::ScriptNotificationHandler::Category::Other, jsProgramName.c_str(), "");

        return DgnDbStatus::BadRequest;
        }

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   BentleySystems
//---------------------------------------------------------------------------------------
DgnDbStatus DgnScript::LoadScript(Dgn::DgnDbR db, Utf8CP jsFunctionSpec)
    {
    DgnScriptContext& ctx = static_cast<DgnScriptContext&>(T_HOST.GetScriptAdmin().GetDgnScriptContext());
    return ctx.LoadProgram(db, jsFunctionSpec);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   BentleySystems
//---------------------------------------------------------------------------------------
DgnDbStatus DgnScript::ExecuteEga(int& functionReturnStatus, Dgn::DgnElementR el, Utf8CP jsEgaFunctionName, DPoint3dCR origin, YawPitchRollAnglesCR angles, Json::Value const& parms)
    {
    DgnScriptContext& ctx = static_cast<DgnScriptContext&>(T_HOST.GetScriptAdmin().GetDgnScriptContext());
    return ctx.ExecuteEga(functionReturnStatus, el, jsEgaFunctionName, origin, angles, parms);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   BentleySystems
//---------------------------------------------------------------------------------------
DgnDbStatus DgnScriptContext::ExecuteEga(int& functionReturnStatus, Dgn::DgnElementR el, Utf8CP jsEgaFunctionName, DPoint3dCR origin, YawPitchRollAnglesCR angles, Json::Value const& parms)
    {
    functionReturnStatus = -1;

    DgnDbStatus status = LoadProgram(el.GetDgnDb(), jsEgaFunctionName);
    if (DgnDbStatus::Success != status)
        return status;

    BeJsFunction jsfunc = m_egaRegistry.GetFunctionProperty(jsEgaFunctionName);
    if (jsfunc.IsUndefined() || !jsfunc.IsFunction())
        {
        NativeLogging::LoggingManager::GetLogger("DgnScript")->errorv ("[%s] is not registered as an EGA", jsEgaFunctionName);
        BeAssert(false && "EGA not registered");
        return DgnDbStatus::NotEnabled;
        }

    BeginCallContext();
    BeJsObject parmsObj = EvaluateJson(parms);
    BeJsNativePointer jsel = ObtainProjectedClassInstancePointer(new JsDgnElement(el), true);
    BeJsNativePointer jsorigin = ObtainProjectedClassInstancePointer(new JsDPoint3d(origin), true);
    BeJsNativePointer jsangles = ObtainProjectedClassInstancePointer(new JsYawPitchRollAngles(angles), true);
    BeJsValue retval = jsfunc(jsel, jsorigin, jsangles, parmsObj);
    EndCallContext();

    if (!retval.IsNumber())
        {
        NativeLogging::LoggingManager::GetLogger("DgnScript")->errorv ("[%s] does not have the correct signature for an EGA - must return an int", jsEgaFunctionName);
        BeAssert(false && "EGA has incorrect return type");
        return DgnDbStatus::NotEnabled;
        }

    BeJsNumber num(retval);
    functionReturnStatus = num.GetIntegerValue();
    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   BentleySystems
//---------------------------------------------------------------------------------------
DgnDbStatus DgnScript::ExecuteDgnDbScript(int& functionReturnStatus, Dgn::DgnDbR db, Utf8StringCR functionName, Json::Value const& parms)
    {
    DgnScriptContext& ctx = static_cast<DgnScriptContext&>(T_HOST.GetScriptAdmin().GetDgnScriptContext());
    return ctx.ExecuteDgnDbScript(functionReturnStatus, db, functionName, parms);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   BentleySystems
//---------------------------------------------------------------------------------------
DgnDbStatus DgnScriptContext::ExecuteDgnDbScript(int& functionReturnStatus, Dgn::DgnDbR db, Utf8StringCR functionName, Json::Value const& parms)
    {
    functionReturnStatus = -1;

    BeJsFunction jsfunc = m_dgndbScriptRegistry.GetFunctionProperty(functionName.c_str());
    if (jsfunc.IsUndefined())
        {
        DgnDbStatus status = LoadProgram(db, functionName.c_str());
        if (DgnDbStatus::Success != status)
            return status;
        
        jsfunc = m_dgndbScriptRegistry.GetFunctionProperty(functionName.c_str());
        }

    if (jsfunc.IsUndefined() || !jsfunc.IsFunction())
        {
        NativeLogging::LoggingManager::GetLogger("DgnScript")->errorv ("[%s] is not registered as a DgnDbScript", functionName.c_str());
        BeAssert(false && "DgnDbScript not found");
        return DgnDbStatus::NotEnabled;
        }

    BeginCallContext();
    BeJsObject parmsObj = EvaluateJson(parms);
    BeJsNativePointer jsdb = ObtainProjectedClassInstancePointer(new JsDgnDb(db), true);
    BeJsValue retval = jsfunc(jsdb, parmsObj);
    EndCallContext();

    if (!retval.IsNumber())
        {
        NativeLogging::LoggingManager::GetLogger("DgnScript")->errorv ("[%s] does not have the correct signature for an DgnDbScript - must return an int", functionName.c_str());
        BeAssert(false && "DgnDbScript has incorrect return type");
        return DgnDbStatus::NotEnabled;
        }

    BeJsNumber num(retval);
    functionReturnStatus = num.GetIntegerValue();
    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   BentleySystems
//---------------------------------------------------------------------------------------
DgnDbStatus DgnScript::ExecuteComponentGenerateElements(int& functionReturnStatus, Dgn::ComponentModelR componentModel, Dgn::DgnModelR destModel, ECN::IECInstanceR instance, Dgn::ComponentDefR cdef, Utf8StringCR functionName)
    {
    DgnScriptContext& ctx = static_cast<DgnScriptContext&>(T_HOST.GetScriptAdmin().GetDgnScriptContext());
    return ctx.ExecuteComponentGenerateElements(functionReturnStatus, componentModel, destModel, instance, cdef, functionName);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   BentleySystems
//---------------------------------------------------------------------------------------
DgnDbStatus DgnScriptContext::ExecuteComponentGenerateElements(int& functionReturnStatus, Dgn::ComponentModelR componentModel, Dgn::DgnModelR destModel, ECN::IECInstanceR instance, Dgn::ComponentDefR cdef, Utf8StringCR jsFunctionName)
    {
    functionReturnStatus = -1;

    DgnDbStatus status = LoadProgram(componentModel.GetDgnDb(), jsFunctionName.c_str());
    if (DgnDbStatus::Success != status)
        return status;

    BeJsFunction jsfunc = m_modelSolverRegistry.GetFunctionProperty(jsFunctionName.c_str());
    if (jsfunc.IsUndefined() || !jsfunc.IsFunction())
        {
        NativeLogging::LoggingManager::GetLogger("DgnScript")->errorv ("[%s] is not registered as a model solver", jsFunctionName.c_str());
        BeAssert(false && "model solver not registered");
        return DgnDbStatus::NotEnabled;
        }

    BeginCallContext();
    BeJsObject jsInstance = ObtainProjectedClassInstancePointer(new JsECInstance(const_cast<ECN::IECInstanceR>(instance)), true);
    BeJsNativePointer jsCompDef = ObtainProjectedClassInstancePointer(new JsComponentDef(cdef), true);
    BeJsNativePointer jsCompModel = ObtainProjectedClassInstancePointer(new JsComponentModel(componentModel), true);
    BeJsNativePointer jsDestModel = ObtainProjectedClassInstancePointer(new JsDgnModel(destModel), true);
    // export function GenerateElements(componentModel: be.ComponentModel, destModel: be.DgnModel, instance: be.ECInstance, cdef: be.ComponentDef): number
    BeJsValue retval = jsfunc(jsCompModel, jsDestModel, jsInstance, jsCompDef);
    EndCallContext();

    if (!retval.IsNumber())
        {
        NativeLogging::LoggingManager::GetLogger("DgnScript")->errorv ("[%s] does not have the correct signature for a model solver - must return an int", jsFunctionName.c_str());
        BeAssert(false && "model solver has incorrect return type");
        return DgnDbStatus::NotEnabled;
        }

    BeJsNumber num(retval);
    functionReturnStatus = num.GetIntegerValue();
    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      07/15
//---------------------------------------------------------------------------------------
DgnPlatformLib::Host::ScriptAdmin::ScriptAdmin()
    {
    m_jsenv = nullptr;
    m_jsContext = nullptr;
    m_notificationHandler = new ScriptNotificationHandler;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      07/15
//---------------------------------------------------------------------------------------
DgnPlatformLib::Host::ScriptAdmin::~ScriptAdmin()
    {
#ifdef WIP_BEJAVASCRIPT // *** This triggers an assertion failure . . EDL 
    if (nullptr != m_jsContext)
        delete m_jsContext;
#endif
#ifdef WIP_BEJAVASCRIPT // *** This triggers an assertion failure because JsDisposeRuntime returns JsErrorRuntimeInUse
    if (nullptr != m_jsenv)
        delete m_jsenv;
#endif
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      07/15
//---------------------------------------------------------------------------------------
BeJsEnvironmentR DgnPlatformLib::Host::ScriptAdmin::GetBeJsEnvironment()
    {
    if (nullptr == m_jsenv)
        m_jsenv = new BeJsEnvironment;
    return *m_jsenv;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      07/15
//---------------------------------------------------------------------------------------
BeJsContextR DgnPlatformLib::Host::ScriptAdmin::GetDgnScriptContext()
    {
    if (nullptr != m_jsContext)
        return *m_jsContext;

    //  *************************************************************
    //  Bootstrap the Bentley.Dgn "namespace"
    //  *************************************************************
    //  Initialize the DgnScriptContext
    auto dgnScriptContext = new DgnScriptContext(GetBeJsEnvironment());
    m_jsContext = dgnScriptContext;

    // First, register DgnJsApi's projections
    RegisterScriptLibraryImporter("Bentley.Dgn-Core", *new DgnJsApi(*m_jsContext));
    ImportScriptLibrary("Bentley.Dgn-Core");

    // Next, allow DgnScriptContext to do some one-time setup work. This will involve evaluating some JS expressions.
    // This is a little tricky. We must wait until we have registered the first projection that defines Bentley.Dgn 
    // before we attempt to add more properties to the Bentley.Dgn global object. This tricky sequencing is only needed 
    // here, where we are actually defining Bentley.Dgn. Other projections can add to Bentley.Dgn and can contain a
    // combination of native code projections and pure TS/JS code without needing any special sequencing.
    dgnScriptContext->Initialize();


    //  *************************************************************
    //  Register other core projections here
    //  vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
    RegisterScriptLibraryImporter("Bentley.Dgn-Geom", *new GeomJsApi(*m_jsContext));
    ImportScriptLibrary("Bentley.Dgn-Geom");     // we also auto-load the geom types, as they are used everywhere


    //  ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

    return *m_jsContext;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      07/15
//---------------------------------------------------------------------------------------
void DgnPlatformLib::Host::ScriptAdmin::RegisterScriptLibraryImporter(Utf8CP libName, ScriptLibraryImporter& importer)
    {
    m_importers[libName] = make_bpair(&importer, false);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      07/15
//---------------------------------------------------------------------------------------
BentleyStatus DgnPlatformLib::Host::ScriptAdmin::ImportScriptLibrary(Utf8CP libName)
    {
    auto ilib = m_importers.find(libName);
    if (ilib == m_importers.end())
        {
        HandleScriptError(ScriptNotificationHandler::Category::Other, "Missing library", libName);
        return BSIERROR;
        }
    if (ilib->second.second)
        return BSISUCCESS;

    ilib->second.first->_ImportScriptLibrary(GetDgnScriptContext(), libName);
    ilib->second.second = true;
    return BSISUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      07/15
//---------------------------------------------------------------------------------------
void DgnPlatformLib::Host::ScriptAdmin::HandleScriptError (ScriptNotificationHandler::Category category, Utf8CP description, Utf8CP details)
    {
    if (nullptr == m_notificationHandler)
        return;
    m_notificationHandler->_HandleScriptError(GetDgnScriptContext(), category, description, details);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      07/15
//---------------------------------------------------------------------------------------
void DgnPlatformLib::Host::ScriptAdmin::ScriptNotificationHandler::_HandleScriptError(BeJsContextR, Category category, Utf8CP description, Utf8CP details)
    {
    NativeLogging::LoggingManager::GetLogger("DgnScript")->errorv("Script error category: %x, description: %s, details: %s", (int)category, description, details);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      07/15
//---------------------------------------------------------------------------------------
void DgnPlatformLib::Host::ScriptAdmin::HandleLogMessage(Utf8CP category, LoggingSeverity sev, Utf8CP message)
    {
    if (nullptr == m_notificationHandler)
        return;
    m_notificationHandler->_HandleLogMessage(category, sev, message);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      07/15
//---------------------------------------------------------------------------------------
void DgnPlatformLib::Host::ScriptAdmin::ScriptNotificationHandler::_HandleLogMessage(Utf8CP category, LoggingSeverity severity, Utf8CP message)
    {
    NativeLogging::LoggingManager::GetLogger(category)->message(ToNativeLoggingSeverity(severity), message);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      07/15
//---------------------------------------------------------------------------------------
void DgnPlatformLib::Host::ScriptAdmin::_OnHostTermination(bool px)
    {
    for (auto &entry : m_importers)
        {
        TERMINATE_HOST_OBJECT(entry.second.first, px);
        }

    if (nullptr != m_notificationHandler)
        TERMINATE_HOST_OBJECT(m_notificationHandler, px);

    delete this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      07/15
//---------------------------------------------------------------------------------------
DgnDbStatus DgnPlatformLib::Host::ScriptAdmin::_FetchScript(Utf8StringR sText, DgnScriptType& stypeFound, DateTime& lastModifiedTime, DgnDbR db, Utf8CP sName, DgnScriptType stypePreferred)
    {
    DgnScriptLibrary jslib(db);
    return jslib.QueryScript(sText, stypeFound, lastModifiedTime, sName, stypePreferred);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      07/15
//---------------------------------------------------------------------------------------
void/*Json::Value*/ DgnPlatformLib::Host::ScriptAdmin::EvaluateScript(Utf8CP script)
    {
    BeJsContext::EvaluateStatus evstatus;
    BeJsContext::EvaluateException evexception;
    /*auto res = */GetDgnScriptContext().EvaluateScript(script, "file:///DgnScriptContex", &evstatus, &evexception);
    //m_jsContext->
    if (BeJsContext::EvaluateStatus::Success != evstatus)
        {
        if (BeJsContext::EvaluateStatus::ParseError==evstatus)
            HandleScriptError(ScriptNotificationHandler::Category::ParseError, "", "");
        else
            HandleScriptError(ScriptNotificationHandler::Category::Exception, evexception.message.c_str(), evexception.trace.c_str());
        }
    }
