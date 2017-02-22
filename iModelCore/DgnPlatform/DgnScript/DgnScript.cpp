/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnScript/DgnScript.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
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
#include <sstream> 

extern Utf8CP dgnScriptContext_GetBootstrappingSource();

namespace {
struct ArgMarshallInfo
    {
    typedef ScriptDefinitionElement::ArgValueUnion::Type Type;
    Type m_type;
    bool m_grabOutput;
    DgnPlatformLib::Host::ScriptAdmin::INativePointerMarshaller* m_nativePtrMarshaller;
    ArgMarshallInfo(Type t, DgnPlatformLib::Host::ScriptAdmin::INativePointerMarshaller* pm, bool grabOutput = false) : m_type(t), m_nativePtrMarshaller(pm), m_grabOutput(grabOutput) { ; }
    };
}

static bmap<DgnClassId, bvector<ArgMarshallInfo>> s_argMarshalling;
static intptr_t s_homeThreadId;
static size_t s_nonHomeThreadCount;
static thread_local BeJsEnvironmentP s_jsenv;
static thread_local BeJsContextP s_jscontext;
static thread_local DgnPlatformLib::Host::ScriptAdmin::ScriptNotificationHandler* s_jsnotificationHandler;
static DgnPlatformLib::Host::ScriptAdmin::ScriptNotificationHandler* s_defaultNotificationHandler;

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

    if (nullptr == el.ToGeometrySource3dP() && nullptr == el.ToGeometrySource2dP())
        return DgnDbStatus::WrongClass;

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
    BeJsNativePointer jsel; // try to get the best fit for the element type
    auto g3d = dynamic_cast<GeometricElement3d*>(&el);
    if (nullptr != g3d)
        jsel = ObtainProjectedClassInstancePointer(new JsGeometricElement3d(*g3d), true);
    else
        {
        auto g2d = dynamic_cast<GeometricElement2d*>(&el);
        if (nullptr != g2d)
            jsel = ObtainProjectedClassInstancePointer(new JsGeometrySource2d(*g2d), true);
        else
            jsel = ObtainProjectedClassInstancePointer(new JsDgnElement(el), true); // It's some kind of geometry source. We don't know the concrete class.
        }
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
// @bsimethod                                   Sam.Wilson                      07/15
//---------------------------------------------------------------------------------------
DgnPlatformLib::Host::ScriptAdmin::ScriptAdmin()
    {
    s_homeThreadId = BeThreadUtilities::GetCurrentThreadId();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      07/15
//---------------------------------------------------------------------------------------
void DgnPlatformLib::Host::ScriptAdmin::CheckCleanup()
    {
    bool anyLeft = false;
        {
        BeSystemMutexHolder _thread_safe_;
        anyLeft = (s_nonHomeThreadCount != 0);
        }

    if (!anyLeft)
        return;
    LOG.error("Some thread initialized script support on a thread and then forgot to call TerminateOnThread");
    BeAssert(false && "Some thread initialized script support on a thread and then forgot to call TerminateOnThread");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      07/15
//---------------------------------------------------------------------------------------
DgnPlatformLib::Host::ScriptAdmin::~ScriptAdmin()
    {
    CheckCleanup();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      07/15
//---------------------------------------------------------------------------------------
void DgnPlatformLib::Host::ScriptAdmin::InitializeOnThread()
    {
    //  Already initialized?
    if (nullptr != s_jsenv)
        {
        if (BeThreadUtilities::GetCurrentThreadId() == s_homeThreadId)      // To make this a little more foolproof, tolerate redundant calls to InitializeOnThread on home thread
/*<=*/      return;

        BeAssert(false && "Call ScriptAdmin::InitializeOnThread only once on a given thread");
        return;
        }

    //  Try to detect missing calls to Terminate
    bool tooMany = false;
        {
        BeSystemMutexHolder _thread_safe_;
        if (BeThreadUtilities::GetCurrentThreadId() != s_homeThreadId)
            {
            ++s_nonHomeThreadCount;
            tooMany = (s_nonHomeThreadCount > 20);
            }
        }

    if (tooMany)
        {
        LOG.warningv("ScriptAdmin::InitializeOnThread called %lld times. Did you forget to call TerminateOnThread?", s_nonHomeThreadCount);
        BeDataAssert(false && "ScriptAdmin::InitializeOnThread called many times. Did you forget to call TerminateOnThread?");
        }

    //  *********************************************
    //  Initialize the BeJsEnvironment
    s_jsenv = new BeJsEnvironment;

    //  *********************************************
    //  Initialize the DgnScriptContext
    auto dgnScriptContext = new DgnScriptContext(*s_jsenv);
    s_jscontext = dgnScriptContext;

    // First, register DgnJsApi's projections
    RegisterScriptLibraryImporter("Bentley.Dgn-Core", *new DgnJsApi(*dgnScriptContext));
    ImportScriptLibrary("Bentley.Dgn-Core");

    // Next, allow DgnScriptContext to do some one-time setup work. This will involve evaluating some JS expressions.
    // This is a little tricky. We must wait until we have registered the first projection that defines Bentley.Dgn 
    // before we attempt to add more properties to the Bentley.Dgn global object. This tricky sequencing is only needed 
    // here, where we are actually defining Bentley.Dgn. Other projections can add to Bentley.Dgn and can contain a
    // combination of native code projections and pure TS/JS code without needing any special sequencing.
    dgnScriptContext->Initialize();


    //  Register other core projections here
    //  v v v v v v v v v v v v v v v v v v v v v v v v v v v v v v v
    RegisterScriptLibraryImporter("Bentley.Dgn-Geom", *new GeomJsApi(*dgnScriptContext));
    ImportScriptLibrary("Bentley.Dgn-Geom");     // we also auto-load the geom types, as they are used everywhere


    //  ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^ ^
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      07/15
//---------------------------------------------------------------------------------------
void DgnPlatformLib::Host::ScriptAdmin::TerminateOnThread()
    {
    if (BeThreadUtilities::GetCurrentThreadId() == s_homeThreadId)  // To make this a little more foolproof, don't terminate the JS environment on the home thread.
        return;

    if (nullptr == s_jsenv)
        {
        BeAssert(false && "Call ScriptAdmin::TerminateOnThread only once on a given thread");
        return;
        }

    if (true)
        {
        BeSystemMutexHolder _thread_safe_;
        --s_nonHomeThreadCount;
        }

    // Free the BeJsEnvironment and BeJsContext that were set up on this thread
    if (nullptr != s_jscontext)
        {
        delete s_jscontext;
        s_jscontext = nullptr;
        }

    delete s_jsenv;
    s_jsenv = nullptr;

    if (nullptr != s_jsnotificationHandler)
        s_jsnotificationHandler = nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      07/15
//---------------------------------------------------------------------------------------
BeJsEnvironmentR DgnPlatformLib::Host::ScriptAdmin::GetBeJsEnvironment()
    {
    if ((nullptr == s_jsenv) && (BeThreadUtilities::GetCurrentThreadId() == s_homeThreadId))        // only the home thread auto-initializes
        InitializeOnThread();

    if (nullptr == s_jsenv)
        throw "InitializeOnThread was not called";

    return *s_jsenv;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      07/15
//---------------------------------------------------------------------------------------
BeJsContextR DgnPlatformLib::Host::ScriptAdmin::GetDgnScriptContext()
    {
    if ((nullptr == s_jsenv) && (BeThreadUtilities::GetCurrentThreadId() == s_homeThreadId))        // only the home thread auto-initializes
        InitializeOnThread();

    if (nullptr == s_jscontext)
        throw "InitializeOnThread was not called";

    return *s_jscontext;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      07/17
//---------------------------------------------------------------------------------------
DgnPlatformLib::Host::ScriptAdmin::ScriptNotificationHandler* DgnPlatformLib::Host::ScriptAdmin::RegisterScriptNotificationHandler(ScriptNotificationHandler& h)
    {
    ScriptNotificationHandler* was = s_jsnotificationHandler;
    s_jsnotificationHandler = &h;
    return was;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      07/17
//---------------------------------------------------------------------------------------
DgnPlatformLib::Host::ScriptAdmin::ScriptNotificationHandler* DgnPlatformLib::Host::ScriptAdmin::GetScriptNotificationHandler()
    {
    if (nullptr != s_jsnotificationHandler)
        return s_jsnotificationHandler;

    BeSystemMutexHolder _thread_safe_;

    if (nullptr == s_defaultNotificationHandler)
        s_defaultNotificationHandler = new ScriptNotificationHandler; // we should always have a default notification handler

    return s_defaultNotificationHandler;
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
    auto nh = GetScriptNotificationHandler();
    if (nullptr == nh)
        return;
    nh->_HandleScriptError(GetDgnScriptContext(), category, description, details);
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
    auto nh = GetScriptNotificationHandler();
    if (nullptr == nh)
        return;
    nh->_HandleLogMessage(category, sev, message);
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

    delete this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      07/15
//---------------------------------------------------------------------------------------
DgnPlatformLib::Host::ScriptAdmin::INativePointerMarshaller* DgnPlatformLib::Host::ScriptAdmin::GetINativePointerMarshaller(Utf8StringCR typeScriptTypeName)
    {
    for (auto const& entry : m_importers)
        {
        auto m = entry.second.first->_GetMarshallerForType(typeScriptTypeName);
        if (nullptr != m)
            return m;
        }
    return nullptr;
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
static bvector<Utf8String> parseCDL(Utf8StringCR cdlIn)
    {
    bvector<Utf8String> items;

    Utf8String cdl = cdlIn.substr(0, cdlIn.find(')'));

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
static BentleyStatus checkEntryPoint(Utf8CP entryPoint, Utf8CP textIn, Utf8StringCR signature, Utf8StringCR args)
    {
    //                           1           2
    std::regex re("function\\s+(\\w+)\\s*[(](.*)[)]\\s*[{]", std::regex_constants::ECMAScript);
    std::smatch sm;
    std::string s (textIn);
    while (std::regex_search(s, sm, re))
        {
        if (3 != sm.size())
            {
            s = sm.suffix();
            continue;
            }

        auto name = sm[1].str();
        if (name != entryPoint)
            {
            s = sm.suffix();
            continue;
            }

        auto haveArgs = parseCDL(sm[2].str().c_str());
        auto wantArgs = parseCDL(args);
        if (haveArgs.size() != wantArgs.size())
            return BSIERROR;

        return BSISUCCESS;
        }

    return BSIERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      08/16
//---------------------------------------------------------------------------------------
static void findLastFunction(Utf8StringR entryPoint, Utf8CP textIn, Utf8StringCR signature)
    {
    std::regex re("function\\s+(\\w+)\\s*[(](.*)[)]", std::regex_constants::ECMAScript);
    std::smatch sm;
    std::string s(textIn);
    while (std::regex_search(s, sm, re))
        {
        if (3 == sm.size())
            entryPoint = sm[1].str().c_str();

        s = sm.suffix().str();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/16
+---------------+---------------+---------------+---------------+---------------+------*/
struct ScriptDefinitionElementPropertyValidators
    {
    ECSqlClassInfo::T_ElementPropValidator text, ecmaVersion, entryPoint;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ScriptDefinitionElementHandler::_RegisterPropertyAccessors(ECSqlClassInfo& params, ECN::ClassLayoutCR layout)
    {
    T_Super::_RegisterPropertyAccessors(params, layout);

    params.RegisterPropertyValidator(layout, SCRIPT_DOMAIN_PROPERTY_Script_Text, 
         [](DgnElementR el, ECValueCR value)
            {
            if (!isNonEmptyString(value))
                return DgnDbStatus::BadArg;
            // *** TBD: Is there any way to check that this is valid script (without evaluating it)?
            return DgnDbStatus::Success;
            });

    params.RegisterPropertyValidator(layout, SCRIPT_DOMAIN_PROPERTY_Script_EcmaScriptVersionRequired, 
        [](DgnElementR el, ECValueCR value)
            {
            if (!isNonEmptyString(value))
                return DgnDbStatus::BadArg;

            auto const& str = value.ToString();
            if (!str.StartsWithI("ES"))
                return DgnDbStatus::BadArg;

            return DgnDbStatus::Success;
            });

    params.RegisterPropertyValidator(layout, SCRIPT_DOMAIN_PROPERTY_Script_EntryPoint, 
        [](DgnElementR elIn, ECValueCR value)
            {
            ScriptDefinitionElement& el = (ScriptDefinitionElement&)elIn; // I KNOW that el is-a ScriptDefinitionElement.

            if (!isNonEmptyString(value))
                return DgnDbStatus::BadArg;

            ECN::ECValue text;
            el.GetPropertyValue(text, SCRIPT_DOMAIN_PROPERTY_Script_Text);
            
            Utf8String rt, args;
            el.GetSignature(rt, args);
            if (BSISUCCESS != checkEntryPoint(value.ToString().c_str(), text.ToString().c_str(), rt, args))
                return DgnDbStatus::BadArg;

            return DgnDbStatus::Success;
            });
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      08/16
//---------------------------------------------------------------------------------------
ScriptDefinitionElementPtr ScriptDefinitionElement::Create(DgnDbStatus* statOut, ScriptLibraryModel& smodel, Utf8CP className, 
    Utf8CP text, Utf8CP entryPoint, Utf8CP desc, Utf8CP url, Utf8CP esv)
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
    
    if (nullptr == text || 0 == *text)
        {
        stat = DgnDbStatus::BadArg;
        return nullptr;
        }
    stat = el->SetPropertyValue(SCRIPT_DOMAIN_PROPERTY_Script_Text, ECN::ECValue(text));
    if (DgnDbStatus::Success != stat)
        return nullptr;

    if (nullptr == esv || 0 == *esv)
        esv = "ES5";
    stat = el->SetPropertyValue(SCRIPT_DOMAIN_PROPERTY_Script_EcmaScriptVersionRequired, ECN::ECValue(esv));
    if (DgnDbStatus::Success != stat)
        return nullptr;

    Utf8String defaultEntryPoint;
    if (nullptr == entryPoint || 0 == *entryPoint)
        {
        findLastFunction(defaultEntryPoint, text, "");
        entryPoint = defaultEntryPoint.c_str();
        }

    stat = el->SetPropertyValue(SCRIPT_DOMAIN_PROPERTY_Script_EntryPoint, ECN::ECValue(entryPoint));
    if (DgnDbStatus::Success != stat)
        return nullptr;

    if (nullptr != desc && 0 != *desc)
        {
        stat = el->SetPropertyValue(SCRIPT_DOMAIN_PROPERTY_Script_Description, ECN::ECValue(desc));
        if (DgnDbStatus::Success != stat)
            return nullptr;
        }

    if (nullptr != url && 0 != *url)
        {
        stat = el->SetPropertyValue(SCRIPT_DOMAIN_PROPERTY_Script_SourceUrl, ECN::ECValue(url));
        if (DgnDbStatus::Success != stat)
            return nullptr;
        }

    return el;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      08/16
//---------------------------------------------------------------------------------------
Utf8String ScriptDefinitionElement::GetText() const
    {
    ECN::ECValue v;
    GetPropertyValue(v, SCRIPT_DOMAIN_PROPERTY_Script_Text);
    return v.ToString();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      08/16
//---------------------------------------------------------------------------------------
Utf8String ScriptDefinitionElement::GetEntryPoint() const
    {
    ECN::ECValue v;
    GetPropertyValue(v, SCRIPT_DOMAIN_PROPERTY_Script_EntryPoint);
    return v.ToString();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      08/16
//---------------------------------------------------------------------------------------
Utf8String ScriptDefinitionElement::GetSourceUrl() const
    {
    ECN::ECValue v;
    GetPropertyValue(v, SCRIPT_DOMAIN_PROPERTY_Script_SourceUrl);
    return v.IsNull() ? "" : v.ToString();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      08/16
//---------------------------------------------------------------------------------------
Utf8String ScriptDefinitionElement::GetDescription() const
    {
    ECN::ECValue v;
    GetPropertyValue(v, SCRIPT_DOMAIN_PROPERTY_Script_Description);
    return v.IsNull()? "": v.ToString();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      08/16
//---------------------------------------------------------------------------------------
Utf8String ScriptDefinitionElement::GetEcmaScriptVersionRequired() const
    {
    ECN::ECValue v;
    GetPropertyValue(v, SCRIPT_DOMAIN_PROPERTY_Script_EcmaScriptVersionRequired);
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
    if (!GetElementId().IsValid())
        return DgnDbStatus::MissingId;

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
static DgnDbStatus compileScriptSignature(bvector<ArgMarshallInfo>& marshallers, Utf8StringCR argsCdl)
    {
    auto& scriptAdmin = T_HOST.GetScriptAdmin();

    auto argTypes = parseCDL(argsCdl);

    Utf8String _plainArgType;

    for (auto const& decoratedArgType : argTypes)
        {
        Utf8StringCP plainArgType = &decoratedArgType;
        Utf8String modifiers;
        auto iModifier = decoratedArgType.find('[');
        if (iModifier != Utf8String::npos)
            {
            _plainArgType = decoratedArgType.substr(0, iModifier);
            plainArgType = &_plainArgType;
//            _plainArgType.Strip();
            modifiers = decoratedArgType.substr(iModifier, -1);
            }

        if (plainArgType->EqualsIAscii("DgnObjectId"))
            marshallers.push_back(ArgMarshallInfo(ArgMarshallInfo::Type::ObjectId, nullptr));
        else if (plainArgType->EqualsIAscii("boolean") || plainArgType->EqualsIAscii("cxx_bool"))
            marshallers.push_back(ArgMarshallInfo(ArgMarshallInfo::Type::Bool, nullptr));
        else if (plainArgType->EqualsIAscii("integer") || plainArgType->EqualsIAscii("cxx_int32_t"))
            marshallers.push_back(ArgMarshallInfo(ArgMarshallInfo::Type::Int32, nullptr));
        else if (plainArgType->EqualsIAscii("double") || plainArgType->EqualsIAscii("cxx_double"))
            marshallers.push_back(ArgMarshallInfo(ArgMarshallInfo::Type::Double, nullptr));
        else if (plainArgType->EqualsIAscii("string") || plainArgType->EqualsIAscii("cxx_Utf8String"))
            marshallers.push_back(ArgMarshallInfo(ArgMarshallInfo::Type::Utf8CP, nullptr));
        else
            {
            auto m = scriptAdmin.GetINativePointerMarshaller(*plainArgType);
            if (nullptr == m)
                return DgnDbStatus::UnknownFormat;
                
            marshallers.push_back(ArgMarshallInfo(ArgMarshallInfo::Type::Pointer, m));
            }

        if (modifiers.find("[out]") != Utf8String::npos)
            marshallers.back().m_grabOutput = true;
        }
    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      08/16
//---------------------------------------------------------------------------------------
DgnDbStatus ScriptDefinitionElement::Execute(Utf8StringR result, std::initializer_list<ArgValueUnion> const& argValues) const
    {
    DgnDbStatus status = LoadScript();
    if (DgnDbStatus::Success != status)
        return status;
    
    auto imarshallers = s_argMarshalling.find(GetElementClassId());
    if (imarshallers == s_argMarshalling.end())
        {
        Utf8String returnType, argumentTypes;
        GetSignature(returnType, argumentTypes);
        status = compileScriptSignature(s_argMarshalling[GetElementClassId()], argumentTypes);
        if (DgnDbStatus::Success != status)
            return status;
        imarshallers = s_argMarshalling.find(GetElementClassId());
        }

    bvector<ArgMarshallInfo> const& marshallers = imarshallers->second;

    //  Check inputs
    if (argValues.size() != marshallers.size())
        {
        return DgnDbStatus::BadArg;
        }

    size_t i=0;
    for (ArgValueUnion const& argValue : argValues)
        {
        ArgMarshallInfo const& marshaller = marshallers[i];
        ArgMarshallInfo::Type mtype = marshaller.m_type;
        ArgMarshallInfo::Type atype = argValue.m_type;
        if (mtype != atype)
            {
            return DgnDbStatus::BadArg;
            }
        ++i;
        }

    struct OutputVar
        {
        BeJsNativePointer m_jsptr;
        void* m_nativeptr;
        DgnPlatformLib::Host::ScriptAdmin::INativePointerMarshaller* m_marshaller;
        OutputVar(void* native, BeJsNativePointer const& js, DgnPlatformLib::Host::ScriptAdmin::INativePointerMarshaller& m) : m_nativeptr(native), m_jsptr(js), m_marshaller(&m) {;}
        };

    DgnScriptContext& ctx = static_cast<DgnScriptContext&>(T_HOST.GetScriptAdmin().GetDgnScriptContext());

    /* !@!!@!!@!!@!!@! */       ctx.BeginCallContext();         /* !@!!@!!@!!@!!@!!@! */
        
    //  !@!!@!!@! DO NOT RETURN EARLY before calling EndCallContext !@!!@!!@!

    bvector<OutputVar> outputVars;
    bvector<BeJsValue> jsValues;

    i = 0;
    for (auto const& argValue : argValues)
        {
        auto const& marshaller = marshallers[i];
        ++i;

        switch(argValue.m_type)
            {
            case ArgMarshallInfo::Type::Pointer:
                {
                if (nullptr != argValue.m_ptr)
                    {
                    BeJsNativePointer jsptr;
                    marshaller.m_nativePtrMarshaller->_MarshallNativePointerToJs(jsptr, ctx, argValue.m_ptr);
                    jsValues.push_back(jsptr);
                    if (marshaller.m_grabOutput)
                        outputVars.push_back(OutputVar(argValue.m_ptr, jsptr, *marshaller.m_nativePtrMarshaller));
                    }
                else
                    {
                    jsValues.push_back(BeJsNativePointer::Null(ctx));
                    }
                break;
                }
            case ArgMarshallInfo::Type::ObjectId:
                jsValues.push_back(ctx.ObtainProjectedClassInstancePointer(new JsDgnObjectId(argValue.m_objectId.GetValueUnchecked())));
                break;

            case ArgMarshallInfo::Type::Bool:
                jsValues.push_back(BeJsBoolean(ctx, argValue.m_bool));
                break;

            case ArgMarshallInfo::Type::Int32:
                jsValues.push_back(BeJsNumber(ctx, argValue.m_int32));
                break;

            case ArgMarshallInfo::Type::Double:
                jsValues.push_back(BeJsNumber(ctx, argValue.m_double));
                break;
            
            case ArgMarshallInfo::Type::Utf8CP:
                jsValues.push_back(BeJsString(ctx, argValue.m_utf8cp));
                break;
            }
        }

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

        if (retVal.IsValid() && !retVal.IsEmpty() && !retVal.IsUndefined() && !retVal.IsNull())
            result = retVal.Serialize();

        status = DgnDbStatus::Success; // *** NEEDS WORK: Detect JS exception

        for (auto const& outputVar : outputVars)
            {
            outputVar.m_marshaller->_MarshallNativePointerFromJs(outputVar.m_nativeptr, outputVar.m_jsptr);
            }
        }

    /* !@!!@!!@!!@!!@! */       ctx.EndCallContext();       /* !@!!@!!@!!@!!@! */

    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      08/16
//---------------------------------------------------------------------------------------
ScriptLibraryModelPtr ScriptLibraryModel::Create(DefinitionPartitionCR partition, Utf8CP sourceUrl)
    {
    DgnDbR db = partition.GetDgnDb();
    DgnClassId classId(db.Schemas().GetECClassId(SCRIPT_DOMAIN_NAME, SCRIPT_DOMAIN_CLASSNAME_ScriptLibraryModel));
    CreateParams mcparams(db, classId, partition.GetElementId());
    auto model = new ScriptLibraryModel(mcparams);
    if (nullptr == model)
        return nullptr;
    // *** TBD: set sourceUrl ... where?
    return model;
    }
