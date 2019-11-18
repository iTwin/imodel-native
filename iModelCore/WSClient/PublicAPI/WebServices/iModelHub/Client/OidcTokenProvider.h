/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <WebServices/Connect/IConnectTokenProvider.h>
#include <BeHttp/HttpClient.h>
#include <WebServices/iModelHub/Client/OidcToken.h>

BEGIN_BENTLEY_IMODELHUB_NAMESPACE
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
struct OidcTokenProvider : WebServices::IConnectTokenProvider
    {
    private:
        OidcTokenPtr        m_token;
        BeTimePoint         m_tokenValidUntil;
        Utf8String          m_callBackUrl;
        Http::HttpClient    m_client;

    public:
		IMODELHUBCLIENT_EXPORT OidcTokenProvider(Utf8StringCR callBackUrl);
		IMODELHUBCLIENT_EXPORT Tasks::AsyncTaskPtr<WebServices::ISecurityTokenPtr> UpdateToken() override;
		IMODELHUBCLIENT_EXPORT WebServices::ISecurityTokenPtr GetToken() override;
    };
END_BENTLEY_IMODELHUB_NAMESPACE