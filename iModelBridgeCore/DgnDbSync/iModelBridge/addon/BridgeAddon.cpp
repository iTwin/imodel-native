/*--------------------------------------------------------------------------------------+
|
|     $Source: iModelBridge/addon/BridgeAddon.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
// #include "BridgeAddon.h"
#include <cstdio>
#include <atlbase.h> 

#include <iModelBridge/iModelBridgeFwk.h>
#include <json/value.h>
#include <node-addon-api/napi.h>

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
    //static void logMessageToSEQ(Utf8CP category, Utf8CP msg)
    //    {
    //    auto env = BridgeNative::s_logger.Env();
    //    Napi::HandleScope scope(env);

    //    Utf8CP fname = "logError";

    //    auto method = BridgeNative::s_logger.Get(fname).As<Napi::Function>();
    //    if (method == env.Undefined())
    //        {
    //        //Napi::Error::New(IModelJsNative::JsInterop::Env(), "Invalid Logger").ThrowAsJavaScriptException();
    //        return;
    //        }

    //    auto catJS = Napi::String::New(env, category);
    //    auto msgJS = Napi::String::New(env, msg);

    //    method({catJS, msgJS});
    //    }

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

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    John.Majerle                      02/18
    +---------------+---------------+---------------+---------------+---------------+------*/
    static Napi::String RequestToken() 
        {
        Napi::String result;

        auto env = BridgeNative::s_JobUtility.Env();
        Napi::HandleScope scope(env);

        Utf8CP fname = "requestToken";

        auto method = BridgeNative::s_JobUtility.Get(fname).As<Napi::Function>();
        if (method == env.Undefined())
            {
            //Napi::Error::New(env, "Invalid JobUtility").ThrowAsJavaScriptException();
            result = Napi::String::New(env, "");
            return result;
            }

        result = method({ }).ToString();
        return result;
        }

    // static Napi::Promise RequestToken() 
    //     {
    //     auto env = BridgeNative::s_JobUtility.Env();
    //     Napi::HandleScope scope(env);

    //     Utf8CP fname = "requestToken";

    //     auto method = BridgeNative::s_JobUtility.Get(fname).As<Napi::Function>();
    //     if (method == env.Undefined())
    //         {
    //         Napi::Error::New(env, "Invalid JobUtility").ThrowAsJavaScriptException();
    //         }

    //     // [NEEDSWORK] method() returns a Napi::Value that is really a Promise.  How to get at it?
    //     //                      ...try studying Napi::AsyncWorker: https://github.com/nodejs/node-addon-api/blob/master/doc/async_worker.md
    //     Napi::Promise result = method({ }).As<Promise>();;

    //     return result;
    //     }
    }  

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

    exports.Set(String::New(env, "RunBridge"), Napi::Function::New(env, _RunBridge));

    exports.DefineProperties(
        {
        Napi::PropertyDescriptor::Accessor("logger", &BridgeNative::GetLogger, &BridgeNative::SetLogger),
        Napi::PropertyDescriptor::Accessor("JobUtility", &BridgeNative::GetJobUtility, &BridgeNative::SetJobUtility),
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
        LPCTSTR t = static_cast<LPCTSTR> (tempsUtf8String.c_str());\
        args.push_back(WPrintfString(L"--%s=%S", COMMAND, t));\
    }\
}

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod                                    John.Majerle                      10/18
+---------------+---------------+---------------+---------------+---------------+------*/
int RunBridge(Env env, const char* jsonString)
    {
    int status = 1;  // Assume failure

    // [NEEDSWORK] Convert all printf statements to  BridgeNative::logMessageToSEQ() after refactor to pass correct category string (see imodel02 source)
    printf("printf: BridgeAddon.cpp: RunBridge() jsonString = %s\n", jsonString);   // [NEEDSWORK] This line just for testing.  Remove later.
 
    //BridgeNative::logMessageToSEQ("INFO", "BridgeAddon.cpp: RunBridge()");        // [NEEDSWORK] This line just for testing.  Remove later.
    BridgeNative::logMessage("BridgeAddon.cpp: RunBridge() BEGIN");                 // [NEEDSWORK] This line just for testing.  Remove later.
    
    // Convert JSON string into a JSON object
    Json::Value json;
    Json::Reader::Parse(jsonString, json); 

    // [NEEDSWORK] This block just for testing.  Remove later.
    // BridgeNative::logMessage("BridgeAddon.cpp: List of json members:");
    // bvector<Utf8String> memberNames = json.getMemberNames();
    // for(unsigned int ii = 0; ii < memberNames.size(); ++ii) {
    //     BentleyB0200::Utf8String memberName = memberNames[ii];
    //     Json::Value value = json[memberName];
    //     printf("printf: BridgeAddon.cpp: RunBridge() json[%s] = %s\n", memberName.c_str(), value.toStyledString().c_str());
    // }

    bvector<WString> args;

    args.push_back(L"BridgeAddon");    

    // Required
    SET_ARG("server_user", L"server-user")                  
    SET_ARG("server_password", L"server-password") 
    SET_ARG("server_project", L"server-project") 
    SET_ARG("server_repository", L"server-repository") 
    SET_ARG("server_environment", L"server-environment") 

    // These may need to be quoted?
    SET_ARG("fwk_assetsDir", L"fwk-assetsDir") 
    SET_ARG("fwk_bridgeAssetsDir", L"fwk-bridgeAssetsDir") 
    SET_ARG("fwk_bridge_library", L"fwk-bridge-library") 
    SET_ARG("fwk_staging_dir", L"fwk-staging-dir") 
    SET_ARG("fwk_input", L"fwk-input") 

    // Optional 
    SET_ARG("server_project_guid", "server-project-guid"); 
    SET_ARG("fwk_bridge_regsubkey", "fwk-bridge-regsubkey");
    SET_ARG("fwk_input_sheet", "fwk-input-sheet");
    SET_ARG("fwk_revision_comment", "fwk-revision-comment");
    SET_ARG("fwk_logging_config_file", "fwk-logging-config-file");
    SET_ARG("fwk_argsJson", "fwk-argsJson");
    SET_ARG("fwk_max_wait", "fwk_max-wait");
    SET_ARG("fwk_jobrun_guid", "fwk-jobrun-guid");
    SET_ARG("fwk_imodelbank_url", "fwk-imodelbank-url");    
    SET_ARG("fwk_jobrequest_guid", "fwk-jobrequest-guid");  
    SET_ARG("fwk_create_repository_if_necessary", "fwk-create-repository-if-necessary");  
    
    bvector<WCharCP> argptrs;
    MAKE_ARGC_ARGV(argptrs, args);

    if(1 == argc) {
        BridgeNative::logMessage("BridgeAddon.cpp: RunBridge() No valid members passed in json");
        return status;    
    }

    // [NEEDSWORK] This block just for testing.  Remove later.
    BridgeNative::logMessage("BridgeAddon.cpp: RunBridge() argv[]:");
    for(int ii = 0; ii < argc; ++ii) {
        char buffer [MAX_PATH];
        sprintf(buffer, "BridgeAddon.cpp: argv[%d] = %S", ii, argv[ii]);
        BridgeNative::logMessage(buffer);
    }

    // [NEEDSWORK] Simulate a long running process by sleeping for 30 seconds
    //             Note: BridgeNative::RequestToken() returns a Promise, not a string.  I don't know how to 'await' its execution.
    // BeThreadUtilities::BeSleep(10000);
    // Napi::String encodedToken = BridgeNative::RequestToken();  
    // BridgeNative::logMessage(encodedToken.Utf8Value().c_str());                                
    // BeThreadUtilities::BeSleep(10000);
    //status = 0;     // Indicate success
    
    try {
        Dgn::iModelBridgeFwk app;

        if (BentleyApi::BSISUCCESS != app.ParseCommandLine(argc, argv)) {
            BridgeNative::logMessage("BridgeAddon.cpp: RunBridge() ParseCommandLine failure");
            return status;
        }

        status = app.Run(argc, argv);

        char buffer [MAX_PATH];
        sprintf(buffer, "BridgeAddon.cpp: RunBridge() Run completed with status = %d", status);
        BridgeNative::logMessage(buffer);
    } catch (...) {
        BridgeNative::logMessage("BridgeAddon.cpp: RunBridge() exception occurred bridging the file");
        return status;
    }

    BridgeNative::logMessage("BridgeAddon.cpp: RunBridge() END"); // [NEEDSWORK] This line just for testing.  Remove later.

    return status;
    }

NODE_API_MODULE(NODE_GYP_MODULE_NAME, RegisterModule)