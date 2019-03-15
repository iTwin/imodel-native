/*--------------------------------------------------------------------------------------+
|
|     $Source: iModelBridge/addon/BridgeAddon.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <cstdio>
#include <json/value.h>
#include <Napi/napi.h>
#include <iModelBridge/iModelBridgeFwk.h>

#include "OidcTokenProvider.h"

USING_NAMESPACE_BENTLEY_HTTP
USING_NAMESPACE_BENTLEY_DGN
using namespace Napi;

#include "BridgeAddon.h"

namespace BridgeNative {

    static Napi::ObjectReference s_logger;
    static Napi::ObjectReference s_JobUtility;

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Sam.Wilson                      02/18
    +---------------+---------------+---------------+---------------+---------------+------*/
    static Napi::Value GetLogger(Napi::CallbackInfo const& info) { return s_logger.Value(); }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Sam.Wilson                      02/18
    +---------------+---------------+---------------+---------------+---------------+------*/
    static void SetLogger(Napi::CallbackInfo const& info)
        {
        s_logger = Napi::ObjectReference::New(info[0].ToObject());
        s_logger.SuppressDestruct();
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Sam.Wilson                      02/18
    +---------------+---------------+---------------+---------------+---------------+------*/
    static void logMessageToSEQ(Utf8CP category, Utf8CP msg)
       {
       auto env = BridgeNative::s_logger.Env();
       Napi::HandleScope scope(env);

       Utf8CP fname = "logError";

       auto method = BridgeNative::s_logger.Get(fname).As<Napi::Function>();
       if (method == env.Undefined())
           {
           //Napi::Error::New(IModelJsNative::JsInterop::Env(), "Invalid Logger").ThrowAsJavaScriptException();
           return;
           }

       auto catJS = Napi::String::New(env, category);
       auto msgJS = Napi::String::New(env, msg);

       method({catJS, msgJS});
       }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    John.Majerle                      02/18
    +---------------+---------------+---------------+---------------+---------------+------*/
    static Napi::Value GetJobUtility(Napi::CallbackInfo const& info) { return s_JobUtility.Value(); }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    John.Majerle                      02/18
    +---------------+---------------+---------------+---------------+---------------+------*/
    static void SetJobUtility(Napi::CallbackInfo const& info)
        {
        s_JobUtility = Napi::ObjectReference::New(info[0].ToObject());
        s_JobUtility.SuppressDestruct();
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    John.Majerle                      02/18
    +---------------+---------------+---------------+---------------+---------------+------*/
    static void logMessage(Utf8CP msg) 
        {
        auto env = BridgeNative::s_JobUtility.Env();
        Napi::HandleScope scope(env);

        Utf8CP fname = "logMessage";

        auto method = BridgeNative::s_JobUtility.Get(fname).As<Napi::Function>();
        if (method == env.Undefined())
            {
            //Napi::Error::New(IModelJsNative::JsInterop::Env(), "Invalid Logger").ThrowAsJavaScriptException();
            return;
            }

        auto msgJS = Napi::String::New(env, msg);

        method({ msgJS });
        }
    }  // End namespace BridgeNative

Value _RunBridge(const CallbackInfo& info) 
    {
    Env env = info.Env();
    Napi::String str = info[0].As<Napi::String>();
    std::string strVal = str.Utf8Value();

    int result = RunBridge(env, strVal.c_str());
    // return String::New(env, strVal.c_str());

    return String::New(env, std::to_string(result).c_str());
    }

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod                                    Sam.Wilson                      06/18
+---------------+---------------+---------------+---------------+---------------+------*/
static Napi::Object RegisterModule(Napi::Env env, Napi::Object exports)
    {
    Napi::HandleScope scope(env);


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
    auto env = BridgeNative::s_JobUtility.Env();
    Napi::HandleScope scope(env);

    Utf8CP fname = "requestToken";

    auto method = BridgeNative::s_JobUtility.Get(fname).As<Napi::Function>();
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

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod                                    John.Majerle                      10/18
+---------------+---------------+---------------+---------------+---------------+------*/
int RunBridge(Env env, const char* jsonString)
    {
    int status = 1;  // Assume failure

    printf("BridgeAddon.cpp: RunBridge() jsonString = %s\n", jsonString);           // [NEEDSWORK] This line just for testing.  Remove later.
 
    // Disable asserts since we may be running as a service (and perhaps on the cloud)
    BeAssertFunctions::SetBeAssertHandler(justLogAssertionFailures);

    BridgeNative::logMessageToSEQ("INFO", "BridgeAddon.cpp: RunBridge() BEGIN");        
    BridgeNative::logMessage("BridgeAddon.cpp: RunBridge() BEGIN");                 // [NEEDSWORK] This line just for testing.  Remove later.
    
    // Convert JSON string into a JSON object
    Json::Value json;
    Json::Reader::Parse(jsonString, json); 

    bvector<WString> args;

    args.push_back(L"BridgeAddon");    

    // Required
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
    SET_ARG("fwk_bridge_regsubkey", L"fwk-bridge-regsubkey");
    SET_ARG("fwk_input_sheet", L"fwk-input-sheet");
    SET_ARG("fwk_revision_comment", L"fwk-revision-comment");
    SET_ARG("fwk_logging_config_file", L"fwk-logging-config-file");
    SET_ARG("fwk_argsJson", L"fwk-argsJson");
    SET_ARG("fwk_max_wait", L"fwk_max-wait");
    SET_ARG("fwk_jobrun_guid", L"fwk-jobrun-guid");
    SET_ARG("fwk_imodelbank_url", L"fwk-imodelbank-url");    
    SET_ARG("fwk_jobrequest_guid", L"fwk-jobrequest-guid");  
    SET_ARG("fwk_create_repository_if_necessary", L"fwk-create-repository-if-necessary"); 

    SET_ARG_NO_VALUE("fwk_skip_assignment_check", L"fwk-skip-assignment-check");
    
    bvector<WCharCP> argptrs;
    MAKE_ARGC_ARGV(argptrs, args);

    if(1 == argc) {
        BridgeNative::logMessage("BridgeAddon.cpp: RunBridge() No valid members passed in json");
        BridgeNative::logMessageToSEQ("ERROR", "BridgeAddon.cpp: RunBridge() No valid members passed in json"); 
        return status;    
    }

    // [NEEDSWORK] This block just for testing.  Remove b4fore deployment!!!.
    BridgeNative::logMessage("BridgeAddon.cpp: RunBridge() argv[]:");
    for(int ii = 0; ii < argc; ++ii) {
        char buffer [MAX_PATH];
        sprintf(buffer, "BridgeAddon.cpp: argv[%d] = %S", ii, argv[ii]);
        BridgeNative::logMessage(buffer);
    }
   
    try {
        Dgn::iModelBridgeFwk app;

        // Initialze the bridge processor
        if (BentleyApi::BSISUCCESS != app.ParseCommandLine(argc, argv)) {
            BridgeNative::logMessage("BridgeAddon.cpp: RunBridge() ParseCommandLine failure");
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

        {
            char buffer [MAX_PATH];
            sprintf(buffer, "BridgeAddon.cpp: RunBridge() Run completed with status = %d", status);
            BridgeNative::logMessage(buffer);            
        }

    } catch (...) {
        BridgeNative::logMessage("BridgeAddon.cpp: RunBridge() exception occurred during bridging");
        BridgeNative::logMessageToSEQ("ERROR", "BridgeAddon.cpp: RunBridge() exception occurred during bridging"); 
        return status;
    }

    BridgeNative::logMessage("BridgeAddon.cpp: RunBridge() END"); // [NEEDSWORK] This line just for testing.  Remove later.
    BridgeNative::logMessageToSEQ("INFO", "BridgeAddon.cpp: RunBridge() END");       

    return status;
    }

NODE_API_MODULE(NODE_GYP_MODULE_NAME, RegisterModule)