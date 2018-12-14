/*--------------------------------------------------------------------------------------+
|
|     $Source: iModelBridge/Fwk/OidcTokenProvider.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "OidcTokenProvider.h"
#include <BeHttp/Http.h>
#include <Bentley/Tasks/AsyncTask.h>
#include <rapidjson/document.h>
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

    Http::Request request = m_client.CreateGetJsonRequest(m_callBackUrl);
    return request.PerformAsync()->Then <WebServices::ISecurityTokenPtr>([this](Http::Response& httpResponse)
        {
        if (!httpResponse.IsSuccess())
            {
            return m_token;
            }
        rapidjson::Document document;
        document.Parse<0>(httpResponse.GetBody().AsString().c_str());
        auto& tokenJosn = document["access_token"];
        auto& timeJson = document["expires_in"];
        if (!tokenJosn.IsNull())
            return m_token;
            
        m_token = std::make_shared<OidcToken>(tokenJosn.GetString());
        
        m_tokenValidUntil = BeTimePoint::FromNow(BeDuration::FromSeconds(timeJson.GetInt() - 10));//Reduce the expiration by 10 s.
        return m_token;
    });
    }

/*--------------------------------------------------------------------------------------+
* @bsiclass                                     Algirdas.Mikoliunas             08/2018
+---------------+---------------+---------------+---------------+---------------+------*/
WebServices::ISecurityTokenPtr OidcTokenProvider::GetToken()
    {
    if (m_tokenValidUntil.IsInPast())
        m_token = nullptr;

    return m_token;
    }
