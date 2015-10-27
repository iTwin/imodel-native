/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/Connect/Connect.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <DgnClientFx/DgnClientApp.h>
#include <WebServices/Connect/SamlToken.h>
#include <WebServices/Client/ClientInfo.h>
#include <DgnClientFx/Utils/Http/Credentials.h>
#include <DgnClientFx/Utils/Http/HttpClient.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

USING_NAMESPACE_BENTLEY_DGNCLIENTFX_UTILS

/*--------------------------------------------------------------------------------------+
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct Connect
    {
    public:
        enum ConnectStatus
            {
            BC_SUCCESS = 0,
            BC_ERROR = 0x8000,
            BC_LOGIN_ERROR = 0x8001,
            };

    private:
        static StatusInt GetStsToken(Utf8StringCR authorization, JsonValueCR issueExParams, SamlTokenR tokenOut, Utf8CP appliesToUrl, Utf8CP stsUrl);

    public:
        //! Initialize once in app lifetime
        WSCLIENT_EXPORT static void Initialize(ClientInfoPtr clientInfo, IHttpHandlerPtr customHttpHandler = nullptr);
        WSCLIENT_EXPORT static void Uninintialize();

        WSCLIENT_EXPORT static StatusInt Login(CredentialsCR creds, SamlTokenR tokenOut, Utf8CP appliesToUrl = nullptr, Utf8CP stsUrl = nullptr);
        WSCLIENT_EXPORT static StatusInt GetStsToken(CredentialsCR creds, SamlTokenR tokenOut, Utf8CP appliesToUrl, Utf8CP stsUrl);
        WSCLIENT_EXPORT static StatusInt GetStsToken(SamlTokenCR parentToken, SamlTokenR tokenOut, Utf8CP appliesToUrl, Utf8CP stsUrl);

        // Checks if given response is IMS Login redirect that should be treated as invalid credentials.
        // This is workaround because IMS does not give any other indication.
        WSCLIENT_EXPORT static bool IsImsLoginRedirect(HttpResponseCR response);
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
