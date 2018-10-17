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

int RunBridge(char* json){
    auto value = Json::Value(json);
    printf("%s", value.ToString().c_str());
    return 0;
}