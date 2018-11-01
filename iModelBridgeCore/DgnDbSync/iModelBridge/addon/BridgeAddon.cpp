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

    // printf("BridgeAddon.cpp: _setArgV() argc = %d member %s command %s\n", argc, MEMBER, COMMAND);      \
    // printf("BridgeAddon.cpp: _setArgV() isMember = %s\n", (json.isMember(MEMBER)?"TRUE":"FALSE"));      \
    // printf("BridgeAddon.cpp: _setArgV() value-0  = %s\n", json.get(MEMBER, "" ).asString().c_str());    \
    // printf("BridgeAddon.cpp: _setArgV() value-1  = %s\n", json[MEMBER].asString().c_str());             \
    // printf("BridgeAddon.cpp: _setArgV() empty    = %s\n", (json.get(MEMBER, "" ).asString().empty()?"TRUE":"FALSE"));    \    

#define _setArgV(json, argc, argv, MEMBER, COMMAND) \
{ \
    if(json.isMember(MEMBER) && (!json.get(MEMBER, "" ).asString().empty())) {                          \
        Utf8String strUtf8 = Utf8PrintfString("--%s=%s", COMMAND, json.get(MEMBER, "" ).asString());    \
        if (!Utf8String::IsNullOrEmpty(strUtf8.c_str())) {                                              \
            WString tempWString;                                                                        \
            BeStringUtilities::Utf8ToWChar(tempWString, strUtf8.c_str());                               \
            argv[argc] = tempWString.c_str();                                                           \
            ++argc;                                                                                     \
        }                                                                                               \
    }                                                                                                   \
}

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod                                    John.Majerle                      06/18
+---------------+---------------+---------------+---------------+---------------+------*/
int RunBridge(const char* jsonString)
    {
    int status = 1;  // Assume failure

    printf("BridgeAddon.cpp: RunBridge() jsonString = %s\n", jsonString);

    Json::Value json;
    Json::Reader::Parse(jsonString, json); 

    printf("BridgeAddon.cpp: List of json members:\n");
    bvector<Utf8String> memberNames = json.getMemberNames();
    for(unsigned int ii = 0; ii < memberNames.size(); ++ii) {
        BentleyB0200::Utf8String memberName = memberNames[ii];
        Json::Value value = json[memberName];
        printf("BridgeAddon.cpp: RunBridge() json[%s] = %s\n", memberName.c_str(), value.toStyledString().c_str());
    }

    // Convert json to argv/argc
    int argc = 0;
    WCharCP argv[14];

    try {
        _setArgV(json, argc, argv, "server_user", "server-user");
        _setArgV(json, argc, argv, "server_password", "server-password");
        _setArgV(json, argc, argv, "server_repository", "server-repository");
        _setArgV(json, argc, argv, "server_project", "server-project");
        _setArgV(json, argc, argv, "server_environment", "server-environment");
 
        _setArgV(json, argc, argv, "fwk_bridge_library", "fwk-bridge-library");
        _setArgV(json, argc, argv, "fwk_bridge_regsubkey", "fwk-bridge-regsubkey");
        _setArgV(json, argc, argv, "fwk_staging_dir", "fwk-staging-dir");
        _setArgV(json, argc, argv, "fwk_input", "fwk-input");
        _setArgV(json, argc, argv, "fwk_input_sheet", "fwk-input-sheet");
        _setArgV(json, argc, argv, "fwk_revision_comment", "fwk-revision-comment");
        _setArgV(json, argc, argv, "fwk_logging_config_file", "fwk-logging-config-file");
        _setArgV(json, argc, argv, "fwk_argsJson", "fwk-argsJson");
        _setArgV(json, argc, argv, "fwk_max_wait", "fwk_max-wait");
        _setArgV(json, argc, argv, "fwk_jobrun_guid", "fwk-jobrun-guid");
        _setArgV(json, argc, argv, "fwk_assetsDir", "fwk-assetsDir");
        _setArgV(json, argc, argv, "fwk_bridgeAssetsDir", "fwk-bridgeAssetsDir");        
        _setArgV(json, argc, argv, "fwk_imodelbank_url", "fwk-imodelbank-url");    
        _setArgV(json, argc, argv, "fwk_jobrequest_guid", "fwk-jobrequest-guid");           
    } catch (...) {
        printf("BridgeAddon.cpp: RunBridge() exception occurred processing json. argc = %d\n", argc);
        return status;
    }

    if(0 == argc) {
        printf("BridgeAddon.cpp: RunBridge() No valid members passed in json\n");
        return status;    
    }

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
    } catch (...) {
        printf("BridgeAddon.cpp: RunBridge() exception occurred processing argv[]\n");
        return status;
    }

    return status;
    }

NODE_API_MODULE(NODE_GYP_MODULE_NAME, RegisterModule)