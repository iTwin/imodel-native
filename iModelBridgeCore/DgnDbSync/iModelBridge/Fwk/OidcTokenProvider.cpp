/*--------------------------------------------------------------------------------------+
|
|     $Source: iModelBridge/Fwk/OidcTokenProvider.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "OidcTokenProvider.h"
#include <Logging/bentleylogging.h>
#include <BeHttp/Http.h>
#include <Bentley/Tasks/AsyncTask.h>
#include <rapidjson/document.h>

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_HTTP

#define IMODELHUB_ClientId            "imodel-hub-integration-tests-2485"
#define IMODELHUB_Scope               "openid profile email imodelhub"

/*--------------------------------------------------------------------------------------+
* @bsiclass                                     Algirdas.Mikoliunas             08/2018
+---------------+---------------+---------------+---------------+---------------+------*/
OidcTokenProvider::OidcTokenProvider(Utf8StringCR callBackUrl)
    :m_callBackUrl(callBackUrl)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Tasks::AsyncTaskPtr<WebServices::ISecurityTokenPtr> OidcTokenProvider::UpdateToken()
    {
    NativeLogging::LoggingManager::GetLogger("iModelBridge")->errorv("OidcTokenProvider::UpdateToken(0)");

    Http::Request request = m_client.CreateGetJsonRequest(m_callBackUrl);
    return request.PerformAsync()->Then <WebServices::ISecurityTokenPtr>([this](Http::Response& httpResponse)
        {
        if (!httpResponse.IsSuccess())
            {
            NativeLogging::LoggingManager::GetLogger("iModelBridge")->errorv("OidcTokenProvider::UpdateToken(1)");

            return m_token;
            }
        rapidjson::Document document;
        document.Parse<0>(httpResponse.GetBody().AsString().c_str());
        auto& tokenJson = document["access_token"];
        auto& timeJson = document["expires_in"];
        if (tokenJson.IsNull())
            return m_token;
            
        Utf8String bearerString("Bearer ");
        bearerString.append(tokenJson.GetString());

        NativeLogging::LoggingManager::GetLogger("iModelBridge")->errorv("OidcTokenProvider::UpdateToken(2) bearerString = %s", bearerString.c_str());


        m_token = std::make_shared<OidcToken>(tokenJson.GetString());
        
        m_tokenValidUntil = BeTimePoint::FromNow(BeDuration::FromSeconds(timeJson.GetInt() - 10));//Reduce the expiration by 10 s.
        
        NativeLogging::LoggingManager::GetLogger("iModelBridge")->errorv("OidcTokenProvider::UpdateToken(3)");

        return m_token;
    });
    }

/*--------------------------------------------------------------------------------------+
* @bsiclass                                     Algirdas.Mikoliunas             08/2018
+---------------+---------------+---------------+---------------+---------------+------*/
WebServices::ISecurityTokenPtr OidcTokenProvider::GetToken()
    {
    NativeLogging::LoggingManager::GetLogger("iModelBridge")->errorv("OidcTokenProvider::GetToken(0)");

    if (m_tokenValidUntil.IsInPast()) {
        NativeLogging::LoggingManager::GetLogger("iModelBridge")->errorv("OidcTokenProvider::GetToken(1)");
        m_token = nullptr;   
    }

    NativeLogging::LoggingManager::GetLogger("iModelBridge")->errorv("OidcTokenProvider::GetToken(2)");
    return m_token;
    }
