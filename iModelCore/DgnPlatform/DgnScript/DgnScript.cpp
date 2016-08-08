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
#include <DgnPlatform/ScriptDomain.h>
#include <regex>
#include <iostream>

extern Utf8CP dgnScriptContext_GetBootstrappingSource();

DOMAIN_DEFINE_MEMBERS(ScriptDomain)
HANDLER_DEFINE_MEMBERS(ScriptDefinitionElementHandler)
HANDLER_DEFINE_MEMBERS(ScriptLibraryModelHandler)

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

    DgnDbStatus LoadProgram(Dgn::DgnDbR db, Utf8CP tsFunctionSpec, bool forceReload);
    DgnDbStatus ExecuteEga(int& functionReturnStatus, Dgn::DgnElementR el, Utf8CP jsEgaFunctionName, DPoint3dCR origin, YawPitchRollAnglesCR angles, Json::Value const& parms);
    DgnDbStatus ExecuteComponentGenerateElements(int& functionReturnStatus, Dgn::ComponentModelR componentModel, Dgn::DgnModelR destModel, ECN::IECInstanceR instance, Dgn::ComponentDefR cdef, Utf8StringCR functionName);
    DgnDbStatus ExecuteDgnDbScript(int& functionReturnStatus, Dgn::DgnDbR db, Utf8StringCR jsFunctionName, Json::Value const& parms);
};
END_BENTLEY_DGNPLATFORM_NAMESPACE


USING_NAMESPACE_BENTLEY_DGNPLATFORM

//---------------------------------------------------------------------------------------
// @bsimethod                                                   BentleySystems
//---------------------------------------------------------------------------------------
DgnDbStatus DgnScriptContext::LoadProgram(Dgn::DgnDbR db, Utf8CP jsFunctionSpec, bool forceReload)
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

    if (!forceReload && (m_jsScriptsExecuted.find(jsProgramName) != m_jsScriptsExecuted.end()))
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
DgnDbStatus DgnScript::LoadScript(Dgn::DgnDbR db, Utf8CP jsFunctionSpec, bool forceReload)
    {
    DgnScriptContext& ctx = static_cast<DgnScriptContext&>(T_HOST.GetScriptAdmin().GetDgnScriptContext());
    return ctx.LoadProgram(db, jsFunctionSpec, forceReload);
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

    DgnDbStatus status = LoadProgram(el.GetDgnDb(), jsEgaFunctionName, false);
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
        DgnDbStatus status = LoadProgram(db, functionName.c_str(), false);
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

    DgnDbStatus status = LoadProgram(componentModel.GetDgnDb(), jsFunctionName.c_str(), false);
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
    if (nullptr != m_jsContext)
        delete m_jsContext;

    if (nullptr != m_jsenv)
        delete m_jsenv;
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
        ON_HOST_TERMINATE(entry.second.first, px);
        }

    if (nullptr != m_notificationHandler)
        ON_HOST_TERMINATE(m_notificationHandler, px);

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
    /*auto res = */GetDgnScriptContext().EvaluateScript(script, "file:///DgnScriptContext.js", &evstatus, &evexception);
    //m_jsContext->
    if (BeJsContext::EvaluateStatus::Success != evstatus)
        {
        if (BeJsContext::EvaluateStatus::ParseError==evstatus)
            HandleScriptError(ScriptNotificationHandler::Category::ParseError, "", "");
        else
            HandleScriptError(ScriptNotificationHandler::Category::Exception, evexception.message.c_str(), evexception.trace.c_str());
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      02/16
//---------------------------------------------------------------------------------------
void DgnPlatformLib::Host::ScriptAdmin::_ThrowException(Utf8CP exname, Utf8CP details)
    {
#ifdef WIP_BEJAVASCRIPT // *** BeJsContext is not quite ready to return values while JsRT is in an exception state
    BeJsContext::EvaluateStatus status; // We do have to ask for the status. Otherwise BeJsContext will assert.
    // BeJsContext::EvaluateException exception; *** NB: Don't request the exception info. If you do, then BeJsContext will clear the exception, prevent it from being propagated to the caller.
    GetDgnScriptContext().EvaluateScript(Utf8PrintfString("Bentley.Dgn.ThrowException('%s', '%s')", exname, details? details: ""),
                                            "file://DgnJsApi.js", &status, nullptr);
#else
    T_HOST.GetScriptAdmin().HandleScriptError(DgnPlatformLib::Host::ScriptAdmin::ScriptNotificationHandler::Category::Exception, exname, details);
#endif
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      08/16
//---------------------------------------------------------------------------------------
DgnDbStatus ScriptDomain::ImportSchema(DgnDbR db)
    {
    Register(); // make sure it's registered

    BeFileName domainSchemaFile = T_HOST.GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory();
    domainSchemaFile.AppendToPath(SCRIPT_DOMAIN_ECSCHEMA_PATH);
    BeAssert(domainSchemaFile.DoesPathExist());

    DgnDomainR domain = ScriptDomain::GetDomain();
    DgnDbStatus importSchemaStatus = domain.ImportSchema(db, domainSchemaFile);
    BeAssert(DgnDbStatus::Success == importSchemaStatus);
    return importSchemaStatus;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      08/16
//---------------------------------------------------------------------------------------
ScriptDefinitionElement::ScriptDefinitionElement(CreateParams const& params) : T_Super(params) {}

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      08/16
//---------------------------------------------------------------------------------------
static bool isNonEmptyString(ECN::ECValueCR value)
    {
    return !value.IsNull() && value.IsString() && !value.ToString().empty();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      08/16
//---------------------------------------------------------------------------------------
static bvector<Utf8String> parseCDL(Utf8StringCR cdl)
    {
    bvector<Utf8String> items;

    size_t offset = 0;
    Utf8String arg;
    while ((offset = cdl.GetNextToken(arg, "\t\n ,", offset)) != Utf8String::npos)
        {
        items.push_back(arg.c_str());
        }
    return items;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      08/16
//---------------------------------------------------------------------------------------
static BentleyStatus checkEntryPoint(Utf8CP entryPoint, Utf8CP textIn, Utf8StringCR signature)
    {
    std::regex re("function\\s+(\\w+)\\s*[(](.*)[)]", std::regex_constants::ECMAScript);
    std::smatch sm;
    std::string s (textIn);
    while (std::regex_search(s, sm, re))
        {
        s = sm.suffix().str();

        if (3 != sm.size())
            continue;

        auto name = sm[1].str();
        if (name != entryPoint)
            continue;


        // *** NEEDS WORK: how to validate arguments??
        //auto haveArgs = parseCDL(sm[2].str().c_str());
        //auto wantArgs = parseCDL(signature);

        return BSISUCCESS;
        }

    return BSIERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      08/16
//---------------------------------------------------------------------------------------
DgnDbStatus ScriptDefinitionElement::_SetProperty(Utf8CP name, ECN::ECValueCR value)
    {
    if (0 == strcmp(SCRIPT_DOMAIN_PROPERTY_Script_Text, name))
        {
        if (!isNonEmptyString(value))
            return DgnDbStatus::BadArg;
        // *** TBD: Is there any way to check that this is valid script (without evaluating it)?
        }
    else if (0 == strcmp(SCRIPT_DOMAIN_PROPERTY_Script_EcmaScriptVersionRequired, name))
        {
        if (!isNonEmptyString(value))
            return DgnDbStatus::BadArg;

        auto const& str = value.ToString();
        if (!str.StartsWithI("ES"))
            return DgnDbStatus::BadArg;
        }
    else if (0 == strcmp(SCRIPT_DOMAIN_PROPERTY_Script_EntryPoint, name))
        {
        if (!isNonEmptyString(value))
            return DgnDbStatus::BadArg;

        ECN::ECValue text;
        GetProperty(text, SCRIPT_DOMAIN_PROPERTY_Script_Text);
        Utf8String rt, args;
        GetSignature(rt, args);
        if (BSISUCCESS != checkEntryPoint(value.ToString().c_str(), text.ToString().c_str(), rt))
            return DgnDbStatus::BadArg;
        }

    return T_Super::_SetProperty(name, value);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      08/16
//---------------------------------------------------------------------------------------
ScriptDefinitionElementPtr ScriptDefinitionElement::CreateElement(DgnDbStatus* statOut, ScriptLibraryModel& smodel, Utf8CP className, Definition const& scparams)
    {
    DgnDbStatus ALLOW_NULL_OUTPUT(stat, statOut);
    auto& db = smodel.GetDgnDb();

    DgnClassId classId(db.Schemas().GetECClassId(SCRIPT_DOMAIN_NAME, className));
    if (!classId.IsValid())
        {
        stat = DgnDbStatus::MissingDomain;
        return nullptr;
        }

    CreateParams ecparams(db, smodel.GetModelId(), classId);
    ScriptDefinitionElementPtr el = new ScriptDefinitionElement(ecparams);
    stat = el->SetProperty(SCRIPT_DOMAIN_PROPERTY_Script_Text, ECN::ECValue(scparams.m_text));
    if (DgnDbStatus::Success != stat)
        return nullptr;
    stat = el->SetProperty(SCRIPT_DOMAIN_PROPERTY_Script_EcmaScriptVersionRequired, ECN::ECValue(scparams.m_esv));
    if (DgnDbStatus::Success != stat)
        return nullptr;
    stat = el->SetProperty(SCRIPT_DOMAIN_PROPERTY_Script_EntryPoint, ECN::ECValue(scparams.m_entryPoint));
    if (DgnDbStatus::Success != stat)
        return nullptr;
    stat = el->SetProperty(SCRIPT_DOMAIN_PROPERTY_Script_Description, ECN::ECValue(scparams.m_desc));
    if (DgnDbStatus::Success != stat)
        return nullptr;
    stat = el->SetProperty(SCRIPT_DOMAIN_PROPERTY_Script_SourceUrl, ECN::ECValue(scparams.m_url));
    if (DgnDbStatus::Success != stat)
        return nullptr;
    return el;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      08/16
//---------------------------------------------------------------------------------------
Utf8String ScriptDefinitionElement::GetText() const
    {
    ECN::ECValue v;
    GetProperty(v, SCRIPT_DOMAIN_PROPERTY_Script_Text);
    return v.ToString();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      08/16
//---------------------------------------------------------------------------------------
Utf8String ScriptDefinitionElement::GetEntryPoint() const
    {
    ECN::ECValue v;
    GetProperty(v, SCRIPT_DOMAIN_PROPERTY_Script_EntryPoint);
    return v.ToString();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      08/16
//---------------------------------------------------------------------------------------
Utf8String ScriptDefinitionElement::GetSourceUrl() const
    {
    ECN::ECValue v;
    GetProperty(v, SCRIPT_DOMAIN_PROPERTY_Script_SourceUrl);
    return v.ToString();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      08/16
//---------------------------------------------------------------------------------------
Utf8String ScriptDefinitionElement::GetDescription() const
    {
    ECN::ECValue v;
    GetProperty(v, SCRIPT_DOMAIN_PROPERTY_Script_Text);
    return v.ToString();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      08/16
//---------------------------------------------------------------------------------------
void ScriptDefinitionElement::GetSignature(Utf8StringR returnType, Utf8StringR arguments) const
    {
    ECN::ECClassCP caClass = GetDgnDb().Schemas().GetECClass(SCRIPT_DOMAIN_NAME, "ScriptSignature");
    if (nullptr == caClass)
        {
        BeAssert(false);
        return;
        }
    auto caInstance = GetElementClass()->GetCustomAttribute(*caClass);
    if (!caInstance.IsValid())
        {
        BeAssert(false);
        return;
        }
    ECN::ECValue value;
    auto status = caInstance->GetValue(value, "ReturnType");
    BeAssert(ECN::ECObjectsStatus::Success == status);
    returnType = value.ToString();
    status = caInstance->GetValue(value, "Arguments");
    BeAssert(ECN::ECObjectsStatus::Success == status);
    arguments = value.ToString();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      08/16
//---------------------------------------------------------------------------------------
DgnDbStatus ScriptDefinitionElement::LoadScript(bool forceReload) const
    {
    static bset<DgnElementId> s_loaded;

    if (!forceReload)
        {
        if (s_loaded.find(GetElementId()) != s_loaded.end())
            return DgnDbStatus::Success;
        }

    s_loaded.erase(GetElementId());

    Utf8String text = GetText();

    DgnScriptContext& ctx = static_cast<DgnScriptContext&>(T_HOST.GetScriptAdmin().GetDgnScriptContext());

    Utf8String fileUrl("file:///"); // This does not really identify a file. It is something tricky that is needed to get JS to accept the script that we pass in as a script to be evaluated.
    fileUrl.append(Utf8PrintfString("%lld", GetElementId().GetValue()).c_str());
    fileUrl.append(".js");

    BeJsContext::EvaluateStatus jsStatus = BeJsContext::EvaluateStatus::Success;
    BeJsContext::EvaluateException jsException;
    ctx.EvaluateScript(text.c_str(), fileUrl.c_str(), &jsStatus, &jsException);   // evaluate the whole script, allowing it to define objects and their properties. 

    if (BeJsContext::EvaluateStatus::Success != jsStatus)
        {
        if (BeJsContext::EvaluateStatus::RuntimeError == jsStatus)
            T_HOST.GetScriptAdmin().HandleScriptError(DgnPlatformLib::Host::ScriptAdmin::ScriptNotificationHandler::Category::Exception, jsException.message.c_str(), jsException.trace.c_str());
        else if (BeJsContext::EvaluateStatus::ParseError == jsStatus)
            T_HOST.GetScriptAdmin().HandleScriptError(DgnPlatformLib::Host::ScriptAdmin::ScriptNotificationHandler::Category::ParseError, fileUrl.c_str(), jsException.message.c_str());
        else
            T_HOST.GetScriptAdmin().HandleScriptError(DgnPlatformLib::Host::ScriptAdmin::ScriptNotificationHandler::Category::Other, fileUrl.c_str(), "");

        return DgnDbStatus::BadRequest;
        }

    s_loaded.insert(GetElementId());
    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      08/16
//---------------------------------------------------------------------------------------
DgnDbStatus ScriptDefinitionElement::Execute(Utf8String argTypeCdl, ...) const
    {
    DgnScriptContext& ctx = static_cast<DgnScriptContext&>(T_HOST.GetScriptAdmin().GetDgnScriptContext());
    
    DgnDbStatus status = DgnDbStatus::Success;
    
    // VVVVVVV CallContext VVVVVVVV         *** DO NOT RETURN EARLY ***
    ctx.BeginCallContext();

    Utf8String returnType, argumentTypes;
    GetSignature(returnType, argumentTypes);
    if (argTypeCdl.empty())
        argTypeCdl = argumentTypes;

    auto argTypes = parseCDL(argTypeCdl);

    bvector<BeJsValue> jsValues;

    va_list va;
    va_start(va, argTypeCdl);
    for (auto const& argType : argTypes)
        {
        if (argType.EqualsI("DgnDb"))
            {
            DgnDbP db = va_arg(va, DgnDbP);
            if (nullptr != db)
                jsValues.push_back(ctx.ObtainProjectedClassInstancePointer(new JsDgnDb(*db), true));
            else
                jsValues.push_back(BeJsNativePointer());
            }
        else if (argType.EqualsI("Viewport"))
            {
            DgnViewportP vp = va_arg(va, DgnViewportP);
            if (nullptr != vp)
                jsValues.push_back(ctx.ObtainProjectedClassInstancePointer(new JsViewport(*vp), true));
            else
                jsValues.push_back(BeJsNativePointer());
            }
        else if (argType.EqualsI("DgnObjectId"))
            {
            BeInt64Id id = va_arg(va, BeInt64Id);
            jsValues.push_back(ctx.ObtainProjectedClassInstancePointer(new JsDgnObjectId(id.GetValue())));
            }
        else if (argType.EqualsI("DgnElement"))
            {
            DgnElementP el = va_arg(va, DgnElementP);
            if (nullptr != el)
                jsValues.push_back(ctx.ObtainProjectedClassInstancePointer(new JsDgnElement(*el)));
            else
                jsValues.push_back(BeJsNativePointer());
            }
        }

    va_end(va);
    
    BeJsObject callScope = ctx.GetGlobalObject();   // *** WIP_DGNSCRIPT - embed scriptlets in some kind of namespace object
    auto entryPoint = callScope.GetFunctionProperty(GetEntryPoint().c_str());
    if (!entryPoint.IsValid() || !entryPoint.IsFunction() || entryPoint.IsNull())
        {
        BeAssert(false);
        status = DgnDbStatus::NotFound;
        }
    else
        {
        auto retVal = entryPoint.CallWithList(&callScope, false, jsValues.data(), jsValues.size());
        }

    ctx.EndCallContext();

    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      08/16
//---------------------------------------------------------------------------------------
ScriptLibraryModelPtr ScriptLibraryModel::Create(DgnDbR db, DgnCode code, Utf8CP sourceUrl)
    {
    DgnClassId classId(db.Schemas().GetECClassId(SCRIPT_DOMAIN_NAME, SCRIPT_DOMAIN_CLASSNAME_ScriptLibraryModel));
    CreateParams mcparams(db, classId, code);
    auto model = new ScriptLibraryModel(mcparams);
    if (nullptr == model)
        return nullptr;
    // *** TBD: set sourceUrl ... where?
    return model;
    }
