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
Value Fn(const CallbackInfo& info) {
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

    exports.Set(String::New(env, "fn"), Napi::Function::New(env, Fn));

    return exports;
}


int RunBridge(const char* json){
    auto value = Json::Value(json);
    printf("%s", value.ToString().c_str());
    return 0;
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, RegisterModule)