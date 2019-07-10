/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
// #include <cstdio>
// #include <json/value.h>
// #include <Napi/napi.h>
// #include <iModelBridge/iModelBridgeFwk.h>

// #include "OidcTokenProvider.h"

// USING_NAMESPACE_BENTLEY_HTTP
// USING_NAMESPACE_BENTLEY_DGN
// using namespace Napi;

#include "BridgeAddon.h"

static intptr_t s_mainThreadId;
static Napi::ObjectReference s_logger;
static Napi::ObjectReference s_jobUtility;
static Napi::Env *s_env;

Napi::String toJsString(Napi::Env env, Utf8CP val, size_t len) { return Napi::String::New(env, val, len); }
Napi::String toJsString(Napi::Env env, Utf8CP val) { return toJsString(env, val, std::strlen(val)); }
Napi::String toJsString(Napi::Env env, Utf8StringCR str) { return toJsString(env, str.c_str(), str.length()); }
Napi::String toJsString(Napi::Env env, BeInt64Id id) { return toJsString(env, id.ToHexStr()); }


static void logMessageToJobUtility(Utf8CP msg) 
    {
    auto env = s_jobUtility.Env();
    Napi::HandleScope scope(env);

    Utf8CP fname = "logMessage";

    auto method = s_jobUtility.Get(fname).As<Napi::Function>();
    if (method == env.Undefined())
        {
        //Napi::Error::New(IModelJsNative::JsInterop::Env(), "Invalid Logger").ThrowAsJavaScriptException();
        return;
        }

  
    auto msgJS = toJsString(env, msg);

    method({ msgJS });
    }

static void setBriefcaseIdJobUtility(uint32_t bcid) 
    {
    auto env = s_jobUtility.Env();
    Napi::HandleScope scope(env);

    Utf8CP fname = "setBriefcaseId";

    auto method = s_jobUtility.Get(fname).As<Napi::Function>();
    if (method == env.Undefined())
        {
        //Napi::Error::New(IModelJsNative::JsInterop::Env(), "Invalid Logger").ThrowAsJavaScriptException();
        return;
        }

    WPrintfString str(L"%u", bcid);        
    Utf8String tempsUtf8String(str);
    Utf8CP msg = tempsUtf8String.c_str();
  
    auto msgJS = toJsString(env, msg);

    method({ msgJS });
    }    

static void logMessageToJs(Utf8CP category, NativeLogging::SEVERITY sev, Utf8CP msg)
    {
    auto env = s_logger.Env();
    Napi::HandleScope scope(env);

    Utf8CP fname = (sev == NativeLogging::LOG_TRACE)?   "logTrace":
                (sev == NativeLogging::LOG_DEBUG)?   "logTrace": // Logger does not distinguish between trace and debug
                (sev == NativeLogging::LOG_INFO)?    "logInfo":
                (sev == NativeLogging::LOG_WARNING)? "logWarning":
                                                        "logError"; // Logger does not distinguish between error and fatal

    auto method = s_logger.Get(fname).As<Napi::Function>();
    if (method == env.Undefined())
        {
        // Napi::Error::New(JsInterop::Env(), Utf8PrintfString("Invalid Logger -- missing %s function", fname).c_str()).ThrowAsJavaScriptException();
        return;
        }

    auto catJS = toJsString(env, category);
    auto msgJS = toJsString(env, msg);

    method({catJS, msgJS});            
    } 

static void LogTrace(Utf8CP msg)         
    {
    logMessageToJs("iModelBridgeApiServer", NativeLogging::LOG_TRACE, msg);
    } 
/*
static void LogInfo(Utf8CP msg)         
    {
        logMessageToJs("iModelBridgeApiServer", NativeLogging::LOG_INFO, msg);
    }        
*/
static void LogError(Utf8CP msg)         
    {
    logMessageToJs("iModelBridgeApiServer", NativeLogging::LOG_WARNING, msg);
    }      

namespace BridgeNative {

    /*---------------------------------------------------------------------------------**/ /**
    * @bsimethod                                    Sam.Wilson                      06/18
    +---------------+---------------+---------------+---------------+---------------+------*/
    bool JsInterop::IsMainThread()
    {
        return (BeThreadUtilities::GetCurrentThreadId() == s_mainThreadId);
    }

    /*---------------------------------------------------------------------------------**/ /**
    * @bsimethod                                    Sam.Wilson                      01/18
    +---------------+---------------+---------------+---------------+---------------+------*/
    void JsInterop::ThrowJsException(Utf8CP msg)
    {
        Napi::Error::New(*s_env, msg).ThrowAsJavaScriptException();
    }

    //=======================================================================================
    // @bsistruct                                   Sam.Wilson                  02/18
    //=======================================================================================
    static Napi::Value GetLogger(Napi::CallbackInfo const& info) { return s_logger.Value(); }

    Napi::ObjectReference &jsInterop_getLogger() { return s_logger; }

    //=======================================================================================
    // @bsistruct                                   Sam.Wilson                  02/18
    //=======================================================================================
    static void SetLogger(Napi::CallbackInfo const& info)
        {
        s_logger = Napi::ObjectReference::New(info[0].ToObject());
        s_logger.SuppressDestruct();
        }

    //=======================================================================================
    // @bsistruct                                   John.Majerle                  02/18
    //=======================================================================================
    static Napi::Value GetJobUtility(Napi::CallbackInfo const& info) { return s_jobUtility.Value(); }

    //=======================================================================================
    // @bsistruct                                   John.Majerle                  02/18
    //=======================================================================================
    static void SetJobUtility(Napi::CallbackInfo const& info)
        {
        s_jobUtility = Napi::ObjectReference::New(info[0].ToObject());
        s_jobUtility.SuppressDestruct();
        }
    }  // End namespace BridgeNative


// ***********************************************************************************
// Functions outside of namespace BridgeNative
// ***********************************************************************************

Value _RunBridge(const CallbackInfo& info) 
    {
    Env env = info.Env();
    Napi::String str = info[0].As<Napi::String>();
    std::string strVal = str.Utf8Value();

    int result = RunBridge(env, strVal.c_str());
    // return String::New(env, strVal.c_str());

    return String::New(env, std::to_string(result).c_str());
    }

//=======================================================================================
// @bsistruct                                   Sam.Wilson                  06/18
//=======================================================================================
static Napi::Object RegisterModule(Napi::Env env, Napi::Object exports)
    {
    s_env = new Napi::Env(env);

    Napi::HandleScope scope(env);

    s_mainThreadId = BeThreadUtilities::GetCurrentThreadId();

    exports.Set(String::New(env, "RunBridge"), Napi::Function::New(env, _RunBridge));

    exports.DefineProperties(
        {
        Napi::PropertyDescriptor::Accessor(env, exports, "logger", &BridgeNative::GetLogger, &BridgeNative::SetLogger),
        Napi::PropertyDescriptor::Accessor(env, exports, "JobUtility", &BridgeNative::GetJobUtility, &BridgeNative::SetJobUtility),
        });

    return exports;
    }

#define MAKE_ARGC_ARGV(argptrs, args)\
for (auto& arg: args)\
   argptrs.push_back(arg.c_str());\
int argc = (int)argptrs.size();\
wchar_t const** argv = argptrs.data();\

#define SET_ARG(MEMBER, COMMAND) \
{\
    if(json.isMember(MEMBER)) { \
        Utf8String tempsUtf8String = json.get(MEMBER, "").asString();\
        Utf8CP t = tempsUtf8String.c_str();\
        args.push_back(WPrintfString(L"--%s=%S", COMMAND, t));\
    }\
}

#define SET_ARG_NO_VALUE(MEMBER, COMMAND) \
{\
    if(json.isMember(MEMBER)) { \
        args.push_back(WPrintfString(L"--%s", COMMAND));\
    }\
}

// Note: Used by OidcTokenProvider.cpp.
Napi::Function RequestTokenFunction() 
    {
    auto env = s_jobUtility.Env();
    Napi::HandleScope scope(env);

    Utf8CP fname = "requestToken";

    auto method = s_jobUtility.Get(fname).As<Napi::Function>();
    if (method == env.Undefined())
        {
        Napi::Error::New(env, "Invalid JobUtility function: requestToken").ThrowAsJavaScriptException();
        }

    return method;
    }

static void justLogAssertionFailures(WCharCP message, WCharCP file, uint32_t line, BeAssertFunctions::AssertType atype)
    {
    WPrintfString str(L"BridgeAddon.cpp: ASSERT: (%ls) @ %ls:%u\n", message, file, line);
    NativeLogging::LoggingManager::GetLogger("iModelBridge")->errorv(str.c_str());
    }    

//=======================================================================================
// @bsistruct                                   John.Majerle                  10/18
//=======================================================================================
int RunBridge(Env env, const char* jsonString)
    {
    int status = 1;  // Assume failure

    // Disable asserts since we may be running as a service (and perhaps on the cloud)
    BeAssertFunctions::SetBeAssertHandler(justLogAssertionFailures);

    // TBI-149402-DisableSeqLogging
    //      We have a working theory that SEQ logging inside a single threaded node.exe process
    //      is the cause of our STATUS_STACK_BUFFER_OVERRUN error.  When this error occcurs, 
    //      the process exists and no SEQ messages are sent out.
    // BridgeNative::JsInterop::InitLogging();

    LogTrace("BridgeAddon.cpp: RunBridge() BEGIN");    
    logMessageToJobUtility("BridgeAddon.cpp: RunBridge() BEGIN");                 // [NEEDSWORK] This line just for testing.  Remove later.
    
    // Convert JSON string into a JSON object
    Json::Value json;
    Json::Reader::Parse(jsonString, json); 

    bvector<WString> args;

    args.push_back(L"BridgeAddon");    

    SET_ARG("server_user", L"server-user")                  // [NEEDSWORK] Deprecated but still supported.  Use server_oidcCallBackUrl instead.
    SET_ARG("server_password", L"server-password")          // [NEEDSWORK] Deprecated but still supported.  Use server_oidcCallBackUrl instead.
    SET_ARG("server_project", L"server-project") 
    SET_ARG("server_oidcCallBackUrl", L"server-oidcCallBackUrl") 
    SET_ARG("server_repository", L"server-repository") 
    SET_ARG("server_environment", L"server-environment") 

    // These may need to be quoted?
    SET_ARG("fwk_assetsDir", L"fwk-assetsDir") 
    SET_ARG("fwk_bridgeAssetsDir", L"fwk-bridgeAssetsDir") 
    SET_ARG("fwk_bridge_library", L"fwk-bridge-library") 
    SET_ARG("fwk_staging_dir", L"fwk-staging-dir") 
    SET_ARG("fwk_input", L"fwk-input") 

    // For files located in a DMS (Document Management System)
    SET_ARG("dms_library", L"dms-library") 
    SET_ARG("dms_inputFileUrn", L"dms-inputFileUrn")
    SET_ARG("dms_type", L"dms-type")  

    // Optional 
    SET_ARG("server_project_guid", L"server-project-guid"); 
    SET_ARG("server_briefcaseId", L"server-briefcaseId") 

    SET_ARG("fwk_bridge_regsubkey", L"fwk-bridge-regsubkey");
    SET_ARG("fwk_input_sheet", L"fwk-input-sheet");
    SET_ARG("fwk_revision_comment", L"fwk-revision-comment");
    SET_ARG("fwk_logging_config_file", L"fwk-logging-config-file");
    SET_ARG("fwk_argsJson", L"fwk-argsJson");
    SET_ARG("fwk_max_wait", L"fwk_max-wait");
    SET_ARG("fwk_jobrun_guid", L"fwk-jobrun-guid");

    SET_ARG("imodel_bank_imodel_id", L"imodel-bank-imodel-id");
    SET_ARG("imodel_bank_url", L"imodel-bank-url");
    SET_ARG("imodel_bank_access_token", L"imodel-bank-access-token");

    SET_ARG("fwk_jobrequest_guid", L"fwk-jobrequest-guid");  

    SET_ARG_NO_VALUE("fwk_create_repository_if_necessary", L"fwk-create-repository-if-necessary"); 
    SET_ARG_NO_VALUE("fwk_skip_assignment_check", L"fwk-skip-assignment-check");
    
    bvector<WCharCP> argptrs;
    MAKE_ARGC_ARGV(argptrs, args);

    if(1 == argc) {
        logMessageToJobUtility("BridgeAddon.cpp: RunBridge() No valid members passed in json");
        LogError("BridgeAddon.cpp: RunBridge() No valid members passed in json"); 
        return status;    
    }

    // [NEEDSWORK] This block just for testing.  Remove b4fore deployment!!!.
    // logMessageToJobUtility("BridgeAddon.cpp: RunBridge() argv[]:");
    // for(int ii = 0; ii < argc; ++ii) {
    //     char buffer [MAX_PATH];
    //     sprintf(buffer, "BridgeAddon.cpp: argv[%d] = %S", ii, argv[ii]);
    //     logMessageToJobUtility(buffer);
    // }

    LogTrace("BridgeAddon.cpp: RunBridge() argv[]:"); 
    for(int ii = 0; ii < argc; ++ii) {
        char buffer [MAX_PATH];
        sprintf(buffer, "BridgeAddon.cpp: argv[%d] = %S", ii, argv[ii]);
        LogTrace(buffer);
    }
    
    try {
        Dgn::iModelBridgeFwk app;

        // Initialze the bridge processor
        if (BentleyApi::BSISUCCESS != app.ParseCommandLine(argc, argv)) {
            logMessageToJobUtility("BridgeAddon.cpp: RunBridge() ParseCommandLine failure");
            LogError("BridgeAddon.cpp: RunBridge() ParseCommandLine failure"); 
            return status;
        }

        // Establish a bridge token provider 
        // if(json.isMember("encodedToken")) { 
        //     // Pull the initial token value from the json object
        //     Utf8String encodedToken = json.get("encodedToken", "").asString();

        //     std::shared_ptr<OidcTokenProvider> provider = std::make_shared<OidcTokenProvider>(encodedToken);
        //     app.SetTokenProvider(provider);
        // }

        // Executed the bridge process
        status = app.Run(argc, argv);

        // Relay the briefcaseId to the calling node.js TypeScript
        uint32_t bcid = app.GetBriefcaseId().GetValue();

        setBriefcaseIdJobUtility(bcid);

        {
            char buffer [MAX_PATH];
            sprintf(buffer, "BridgeAddon.cpp: RunBridge() Run completed with status = %d briefcaseId = %d", status, bcid);
            logMessageToJobUtility(buffer);   
            LogTrace(buffer);          
        }

    } catch (...) {
        logMessageToJobUtility("BridgeAddon.cpp: RunBridge() exception occurred during bridging");
        LogError("BridgeAddon.cpp: RunBridge() exception occurred during bridging"); 
        return status;
    }

    logMessageToJobUtility("BridgeAddon.cpp: RunBridge() END"); // [NEEDSWORK] This line just for testing.  Remove later.
    LogTrace("BridgeAddon.cpp: RunBridge() END");       

    return status;
    }

NODE_API_MODULE(NODE_GYP_MODULE_NAME, RegisterModule)