/*--------------------------------------------------------------------------------------+
 |
 |     $Source: PublicAPI/WebServices/Connect/Connect.h $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <MobileDgn/MobileDgnApplication.h>
#include <WebServices/Connect/SamlToken.h>
#include <MobileDgn/Utils/Http/Credentials.h>
#include <MobileDgn/Utils/Http/HttpClient.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

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
        static StatusInt GetStsToken (Utf8StringCR authorization, JsonValueCR issueExParams, SamlTokenR tokenOut, Utf8CP appliesToUrl, Utf8CP stsUrl);

    public:
        WS_EXPORT static StatusInt Login (MobileDgn::Utils::CredentialsCR creds, SamlTokenR tokenOut, Utf8CP appliesToUrl = nullptr, Utf8CP stsUrl = nullptr);
        WS_EXPORT static StatusInt GetStsToken (MobileDgn::Utils::CredentialsCR creds, SamlTokenR tokenOut, Utf8CP appliesToUrl, Utf8CP stsUrl);
        WS_EXPORT static StatusInt GetStsToken (SamlTokenCR parentToken, SamlTokenR tokenOut, Utf8CP appliesToUrl, Utf8CP stsUrl);

        // Checks if given response is IMS Login redirect that should be treated as invalid credentials.
        // This is workaround because IMS does not give any other indication.
        WS_EXPORT static bool IsImsLoginRedirect (MobileDgn::Utils::HttpResponseCR response);

        WS_EXPORT static Utf8StringCR GetWsgUrl ();
        WS_EXPORT static void SetWsgUrl (Utf8StringCR url);
        WS_EXPORT static Utf8StringCR GetEulaUrl ();
        WS_EXPORT static void SetEulaUrl (Utf8StringCR url);
        WS_EXPORT static Utf8StringCR GetUserAgent ();
        WS_EXPORT static void SetUserAgent (Utf8StringCR url);
        WS_EXPORT static Utf8StringCR GetAppId ();
        WS_EXPORT static void SetAppId (Utf8StringCR url);
        WS_EXPORT static Utf8StringCR GetDeviceId ();
        WS_EXPORT static void SetDeviceId (Utf8StringCR url);
        WS_EXPORT static void Initialize (std::shared_ptr<MobileDgn::Utils::IHttpHandler> customHttpHandler = nullptr);
        WS_EXPORT static void Uninintialize ();
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
