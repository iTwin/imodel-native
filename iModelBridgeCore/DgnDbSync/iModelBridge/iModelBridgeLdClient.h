/*--------------------------------------------------------------------------------------+
|
|     $Source: iModelBridge/iModelBridgeLdClient.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <Bentley/WString.h>
#include <unordered_map>
#include <WebServices/Configuration/UrlProvider.h>

typedef struct LDConfig_i LDConfig;
typedef struct LDUser_i LDUser;

class LDClient;

struct iModelBridgeLdClient
    {
    private:
        std::unordered_map <std::string, bool> m_featureFlags;
        LDConfig*   m_config;
        LDUser*     m_user;
        LDClient*   m_client;
        
        iModelBridgeLdClient();
        BentleyStatus   Init(CharCP authKey);
        static CharCP   GetSDKKey(WebServices::UrlProvider::Environment environment);
    public:
        static iModelBridgeLdClient& GetInstance(WebServices::UrlProvider::Environment environment);
        
        BentleyStatus Close();
        ~iModelBridgeLdClient();
        BentleyStatus IsFeatureOn(bool &, CharCP featureName);
    };