/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/Connect/Connect.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

// TODO: refactor to non-static class ImsClient
//__PUBLISH_SECTION_START__

#include <MobileDgn/MobileDgnApplication.h>
#include <WebServices/Connect/SamlToken.h>
#include <WebServices/Client/ClientInfo.h>
#include <MobileDgn/Utils/Http/Credentials.h>
#include <MobileDgn/Utils/Http/HttpClient.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

USING_NAMESPACE_BENTLEY_MOBILEDGN_UTILS

typedef AsyncResult<SamlTokenPtr, HttpError> SamlTokenResult;

/*--------------------------------------------------------------------------------------+
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct Connect
    {
    private:
        static AsyncTaskPtr<SamlTokenResult> GetStsToken(Utf8StringCR authorization, JsonValueCR issueExParams, Utf8CP appliesToUrl, Utf8CP stsUrl, uint64_t tokenLifetime);
        static Utf8String GetClientRelyingPartyUri();
        static Utf8String GetClientRelyingPartyUriForWtrealm();

    public:
        //! Initialize once in app lifetime
        WSCLIENT_EXPORT static void Initialize(ClientInfoPtr clientInfo, IHttpHandlerPtr customHttpHandler = nullptr);
        WSCLIENT_EXPORT static void Uninintialize();

        WSCLIENT_EXPORT static AsyncTaskPtr<SamlTokenResult> Login(CredentialsCR creds, Utf8CP appliesToUrl = nullptr, Utf8CP stsUrl = nullptr, uint64_t tokenLifetime = 0);
        WSCLIENT_EXPORT static AsyncTaskPtr<SamlTokenResult> RenewToken(SamlTokenCR parentToken, Utf8CP appliesToUrl = nullptr, Utf8CP stsUrl = nullptr, uint64_t tokenLifetime = 0);

        WSCLIENT_EXPORT static AsyncTaskPtr<SamlTokenResult> GetStsToken(CredentialsCR creds, Utf8CP appliesToUrl, Utf8CP stsUrl, uint64_t tokenLifetime = 0);
        WSCLIENT_EXPORT static AsyncTaskPtr<SamlTokenResult> GetStsToken(SamlTokenCR parentToken, Utf8CP appliesToUrl, Utf8CP stsUrl, uint64_t tokenLifetime = 0);

        // Checks if given response is IMS Login redirect that should be treated as invalid credentials.
        // This is workaround because IMS does not give any other indication.
        WSCLIENT_EXPORT static bool IsImsLoginRedirect(HttpResponseCR response);

        WSCLIENT_EXPORT static Utf8String GetFederatedSignInUrl(Utf8String windowsDomainName = nullptr);
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
