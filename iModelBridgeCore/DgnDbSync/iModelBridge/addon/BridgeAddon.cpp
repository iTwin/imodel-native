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
    // Hardcoded values work.
    // int _argc = 10;
    // WCharCP _argv[10] = {
    //     L"BridgeAddon",
    //     L"--server-user=imv-test@be-mailinator.cloudapp.net",
    //     L"--server-password=X$c9Fy3u!TRBVxFz",
    //     L"--server-project=AT-JOHN-TEST",
    //     L"--server-repository=Foo2",        
    //     L"--server-environment=DEV",
    //     L"--fwk-assetsDir=C:\\Program Files\\Bentley\\iModelBridgeMstn\\Assets",
    //     L"--fwk-bridge-library=C:\\Program Files\\Bentley\\iModelBridgeMstn\\Dgnv8BridgeB02.dll",
    //     L"--fwk-staging-dir=D:\\junk\\stagingDir",
    //     L"--fwk-input=D:\\junk\\input\\Foo.i.dgn"
    // };

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