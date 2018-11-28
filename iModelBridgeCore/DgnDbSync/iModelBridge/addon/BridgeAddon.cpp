/*--------------------------------------------------------------------------------------+
|
|     $Source: iModelBridge/addon/BridgeAddon.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
// #include "BridgeAddon.h"
#include <cstdio>
#include <iModelBridge/iModelBridgeFwk.h>
#include <json/value.h>
#include <node-addon-api/napi.h>

using namespace Napi;
#include "BridgeAddon.h"

namespace BridgeNative {

    static Napi::ObjectReference s_logger;
    static Napi::ObjectReference s_WebSocketUtility;

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
    //static void logMessageToJs(Utf8CP category, Utf8CP msg)
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
    static Napi::Value GetWebSocketUtility(Napi::CallbackInfo const& info) { return s_WebSocketUtility.Value(); }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    John.Majerle                      02/18
    +---------------+---------------+---------------+---------------+---------------+------*/
    static void SetWebSocketUtility(Napi::CallbackInfo const& info)
        {
        s_WebSocketUtility = Napi::ObjectReference::New(info[0].ToObject());
        s_WebSocketUtility.SuppressDestruct();
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    John.Majerle                      02/18
    +---------------+---------------+---------------+---------------+---------------+------*/
    static void SendMessage(Utf8CP msg) 
        {
        auto env = BridgeNative::s_WebSocketUtility.Env();
        Napi::HandleScope scope(env);

        Utf8CP fname = "SendMessage";

        auto method = BridgeNative::s_WebSocketUtility.Get(fname).As<Napi::Function>();
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

        auto env = BridgeNative::s_WebSocketUtility.Env();
        Napi::HandleScope scope(env);

        Utf8CP fname = "RequestToken";

        auto method = BridgeNative::s_WebSocketUtility.Get(fname).As<Napi::Function>();
        if (method == env.Undefined())
            {
            //Napi::Error::New(IModelJsNative::JsInterop::Env(), "Invalid WebSocketUtility").ThrowAsJavaScriptException();
            result = Napi::String::New(env, "");
            return result;
            }

        result = method({ }).ToString();

        return result;
        }
}

Value _RunBridge(const CallbackInfo& info) 
    {
    Env env = info.Env();
    Napi::String str = info[0].As<Napi::String>();
    std::string strVal = str.Utf8Value();
    RunBridge(env, strVal.c_str());
    return String::New(env, "Worked");
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
        Napi::PropertyDescriptor::Accessor("WebSocketUtility", &BridgeNative::GetWebSocketUtility, &BridgeNative::SetWebSocketUtility),
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
        WString tempWString; \
        BeStringUtilities::Utf8ToWChar(tempWString, tempsUtf8String.c_str());\
        args.push_back(WPrintfString(L"--%s=%s", COMMAND, tempWString));\
    }\
}


/*---------------------------------------------------------------------------------**/ /**
* @bsimethod                                    John.Majerle                      10/18
+---------------+---------------+---------------+---------------+---------------+------*/
int RunBridge(Env env, const char* jsonString)
    {
    // [NEEDSWORK] Convert all printf statements to logs using BridgeNative::logMessageToJs() after refactor to pass correct category string (see imodel02 source)
    
    int status = 1;  // Assume failure

    printf("BridgeAddon.cpp: RunBridge() jsonString = %s\n", jsonString);                       // [NEEDSWORK] This line just for testing.  Remove later.

    //BridgeNative::logMessageToJs("INFO", "BridgeAddon.cpp: RunBridge()");                       // [NEEDSWORK] This line just for testing.  Remove later.
    //BridgeNative::SendMessage("BridgeAddon.cpp: RunBridge()");                                  // [NEEDSWORK] This line just for testing.  Remove later.

    Napi::String encodedToken = BridgeNative::RequestToken();                                   // [NEEDSWORK] This line just for testing.  Remove later.
    BridgeNative::SendMessage(encodedToken.Utf8Value().c_str());                                // [NEEDSWORK] This line just for testing.  Remove later.
    
    printf("BridgeAddon.cpp: RunBridge() new token = %s\n", encodedToken.Utf8Value().c_str());  // [NEEDSWORK] This line just for testing.  Remove later.

    Json::Value json;
    Json::Reader::Parse(jsonString, json); 

    // [NEEDSWORK] This block just for testing.  Remove later.
    printf("BridgeAddon.cpp: List of json members:\n");
    bvector<Utf8String> memberNames = json.getMemberNames();
    for(unsigned int ii = 0; ii < memberNames.size(); ++ii) {
        BentleyB0200::Utf8String memberName = memberNames[ii];
        Json::Value value = json[memberName];
        printf("BridgeAddon.cpp: RunBridge() json[%s] = %s\n", memberName.c_str(), value.toStyledString().c_str());
    }

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
        printf("BridgeAddon.cpp: RunBridge() No valid members passed in json\n");
        return status;    
    }

    // [NEEDSWORK] This block just for testing.  Remove later.
    printf("BridgeAddon.cpp: RunBridge() argv[]:\n");
    for(int ii = 0; ii < argc; ++ii) {
       printf("BridgeAddon.cpp: argv[%d] = %S\n", ii, argv[ii]); 
    }
   
    try {
        Dgn::iModelBridgeFwk app;

        if (BentleyApi::BSISUCCESS != app.ParseCommandLine(argc, argv)) {
            printf("\n");
            printf("BridgeAddon.cpp: RunBridge() ParseCommandLine failure\n");
            return status;
        }

        status = app.Run(argc, argv);

        printf("BridgeAddon.cpp: RunBridge() Run completed with status = %d\n", status);
    } catch (...) {
        printf("BridgeAddon.cpp: RunBridge() exception occurred bridging the file\n");
        return status;
    }

    return status;
    }

NODE_API_MODULE(NODE_GYP_MODULE_NAME, RegisterModule)