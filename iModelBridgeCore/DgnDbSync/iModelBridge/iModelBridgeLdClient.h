/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#pragma once
#include <iModelBridge/iModelBridge.h>
#include <Bentley/WString.h>
#include <unordered_map>
#include <WebServices/Configuration/UrlProvider.h>

typedef struct LDConfig_i LDConfig;
typedef struct LDUser_i LDUser;
struct LDClient_i;


struct iModelBridgeLdClient
    {
    private:
        std::unordered_map <std::string, bool> m_featureFlags;
        LDConfig*   m_config;
        LDUser*     m_user;
        LDClient_i* m_client;
        
        iModelBridgeLdClient();
        BentleyStatus   Init(CharCP authKey);
        static CharCP   GetSDKKey(WebServices::UrlProvider::Environment environment);
    public:
        IMODEL_BRIDGE_EXPORT static iModelBridgeLdClient& GetInstance(WebServices::UrlProvider::Environment environment);
        IMODEL_BRIDGE_EXPORT BentleyStatus SetUserName(CharCP userName);
        IMODEL_BRIDGE_EXPORT BentleyStatus SetProjectDetails(CharCP iModelName, CharCP guid);
        IMODEL_BRIDGE_EXPORT BentleyStatus InitClient();
        IMODEL_BRIDGE_EXPORT BentleyStatus Close();
        IMODEL_BRIDGE_EXPORT ~iModelBridgeLdClient();
        IMODEL_BRIDGE_EXPORT BentleyStatus IsFeatureOn(bool &, CharCP featureName);
        IMODEL_BRIDGE_EXPORT BentleyStatus GetFeatureValue(Utf8StringR value, CharCP featureName);
    };