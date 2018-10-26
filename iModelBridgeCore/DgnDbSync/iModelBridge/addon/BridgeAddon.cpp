/*--------------------------------------------------------------------------------------+
|
|     $Source: iModelBridge/addon/BridgeAddon.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "BridgeAddon.h"
#include <cstdio>
#include <iModelBridge/iModelBridgeFwk.h>
#include <json/value.h>
#include <node-addon-api/napi.h>

using namespace Napi;

Value _RunBridge(const CallbackInfo& info) 
    {
    Env env = info.Env();
    auto str = info[0].As<Napi::String>().Utf8Value().c_str();
    RunBridge(str);
    return String::New(env, str);
    }

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod                                    Sam.Wilson                      06/18
+---------------+---------------+---------------+---------------+---------------+------*/
static Napi::Object RegisterModule(Napi::Env env, Napi::Object exports)
    {
    Napi::HandleScope scope(env);

    exports.Set(String::New(env, "RunBridge"), Napi::Function::New(env, _RunBridge));

    return exports;
    }

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod                                    John.Majerle                      06/18
+---------------+---------------+---------------+---------------+---------------+------*/
int RunBridge(const char* jsonString)
    {
    auto json = Json::Value(jsonString);
    printf("%s", json.ToString().c_str());

    // Convert json to argv/argc
    int argc = 0;
    WCharCP argv[14];

    // [NEEDSWORK] Perhaps should use json.isNull("membername") instead?

    if(json.isMember("fwk_bridge_library")) {
        argv[argc] = Utf8PrintfString("--fwk-bridge-library=%s", json.get("fwk_bridge_library", "" ).asString());
        ++argc;
    }

    if(json.isMember("fwk_bridge_regsubkey")) {
        argv[argc] = Utf8PrintfString("--fwk-bridge-regsubkey=%s)", json.get("fwk_bridge_regsubkey", "" ).asString());         
        ++argc;
    }
    
    if(json.isMember("fwk_staging_dir")) {
        argv[argc] = Utf8PrintfString("--fwk-staging-dir=%s)", json.get("fwk_staging_dir", "" ).asString());    
        ++argc;
    }

    if(json.isMember("fwk_input")) {
        argv[argc] = Utf8PrintfString("--fwk-input=%s)", json.get("fwk_input", "" ).asString());        
        ++argc;
    }

    if(json.isMember("fwk_input_sheet")) {
        argv[argc] = Utf8PrintfString("--fwk-input-sheet=%s)", json.get("fwk_input_sheet", "" ).asString());        
        ++argc;
    }

    if(json.isMember("fwk_revision_comment")) {
        argv[argc] = Utf8PrintfString("--fwk-revision-comment=%s)", json.get("fwk_revision_comment", "" ).asString());              
        ++argc;
    }

    if(json.isMember("fwk_logging_config_file")) {
        argv[argc] = Utf8PrintfString("--fwk-logging-config-file=%s)", json.get("fwk_logging_config_file", "" ).asString());         
        ++argc;
    }

    if(json.isMember("fwk_argsJson")) {
        argv[argc] = Utf8PrintfString("--fwk-argsJson=%s)", json.get("fwk_argsJson", "" ).asString());                
        ++argc;
    }

    if(json.isMember("fwk_max_wait")) {
        argv[argc] = Utf8PrintfString("--fwk_max-wait=%s)", json.get("fwk_max_wait", "" ).asString());           
        ++argc;
    }    

    if(json.isMember("fwk_jobrun_guid")) {
        argv[argc] = Utf8PrintfString("--fwk-jobrun-guid=%s)", json.get("fwk_jobrun_guid", "" ).asString()); 
        ++argc;
    }    
    
    if(json.isMember("fwk_assetsDir")) {
        argv[argc] = Utf8PrintfString("--fwk-assetsDir=%s)", json.get("fwk_assetsDir", "" ).asString());            
        ++argc;
    }       

    if(json.isMember("fwk_bridgeAssetsDir")) {
        argv[argc] = Utf8PrintfString("--fwk-bridgeAssetsDir=%s)", json.get("fwk_bridgeAssetsDir", "" ).asString()); 
        ++argc;
    }      
    
    if(json.isMember("fwk_imodelbank_url")) {
        argv[argc] = Utf8PrintfString("--fwk-imodelbank-url=%s)", json.get("fwk_imodelbank_url", "" ).asString());               
        ++argc;
    }      

    if(json.isMember("fwk_jobrequest_guid")) {
        argv[argc] = Utf8PrintfString("--fwk-jobrequest-guid=%s)", json.get("fwk_jobrequest_guid", "" ).asString());  
        ++argc;
    }
    
    Dgn::iModelBridgeFwk app;

    if (BentleyApi::BSISUCCESS != app.ParseCommandLine(argc, argv))
        return 1;

    return app.Run(argc, argv);

    return 0;
    }

NODE_API_MODULE(NODE_GYP_MODULE_NAME, RegisterModule)