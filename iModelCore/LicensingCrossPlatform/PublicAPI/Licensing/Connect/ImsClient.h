/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Licensing/Connect/ImsClient.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

// TODO: refactor to non-static class ImsClient
//__PUBLISH_SECTION_START__

#include "IImsClient.h"
#include "SamlToken.h"
#include "../../PublicAPI/Licensing/Utils/ApplicationInfo.h"
#include <BeHttp/Credentials.h>
#include <BeHttp/HttpClient.h>
#include <json\json.h>

BEGIN_BENTLEY_LICENSING_NAMESPACE

typedef AsyncResult<SamlTokenPtr, HttpError> SamlTokenResult;

/*--------------------------------------------------------------------------------------+
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
typedef std::shared_ptr<struct ImsClient> ImsClientPtr;
struct ImsClient : IImsClient
    {
    private:
        ApplicationInfoPtr m_clientInfo;
        Http::IHttpHandlerPtr m_httpHandler;
        Utf8String m_ImsActiveSTSHelperServiceAuthKey;

    private:
        ImsClient(ApplicationInfoPtr clientInfo, Http::IHttpHandlerPtr httpHandler);

        AsyncTaskPtr<SamlTokenResult> RequestToken
            (
            Utf8StringCR authorization,
            JsonValueR params,
            Utf8StringCR rpUri,
            Utf8StringCR stsUrl,
            uint64_t lifetime,
            Utf8StringCR pastUsername
            );

        static Utf8String GetClientRelyingPartyUriForWtrealm(ApplicationInfoCR clientInfo);
        
    public:
        // TODO: static API for compatibility with old static code (ConnectSpaces, ConnectSetup, etc.)
        //! Will be DEPRECATED in future.
        LICENSING_EXPORT static void InitializeShared(ApplicationInfoPtr info, Http::IHttpHandlerPtr httpHandler = nullptr);
        LICENSING_EXPORT static void UnitializeShared();
        //! Will be DEPRECATED in future.
        LICENSING_EXPORT static ImsClientPtr GetShared();

        //! Create new client
        //! @param info - client applicaiton info, see ClientInfo documentation for more info
        //! @param httpHandler - custom httpHandler to route requests trough
        LICENSING_EXPORT static ImsClientPtr Create(ApplicationInfoPtr info, Http::IHttpHandlerPtr httpHandler = nullptr);

        //! Get security token using credentials
        //! @param creds - credentials to use for token
        //! @param rpUri - relying party URI the token will be used for. Defaults to application RP.
        //! @param lifetime - request specific token lifetime in minutes. Zero defaults to lifetime defined by service
        LICENSING_EXPORT AsyncTaskPtr<SamlTokenResult> RequestToken(CredentialsCR creds, Utf8String rpUri = nullptr, uint64_t lifetime = 0) override;

        //! Get security token using other token. Can be used to renew same token or get delegation token for different RP
        //! @param parent - parent.
        //! @param rpUri - relying party URI the token will be used for. Defaults to application RP.
        //! @param lifetime - request specific token lifetime in minutes. Zero defaults to lifetime defined by service
        LICENSING_EXPORT AsyncTaskPtr<SamlTokenResult> RequestToken(SamlTokenCR parent, Utf8String rpUri = nullptr, uint64_t lifetime = 0) override;

        //! Checks if given response is IMS Login redirect that should be treated as invalid credentials.
        //! This is workaround because IMS does not give any other indication.
        LICENSING_EXPORT static bool IsLoginRedirect(Http::ResponseCR response);

        //! Get Relying Party for given client info
        LICENSING_EXPORT static Utf8String GetRelyingPartyUri(ApplicationInfoCR info);

        //! Get legacy Relying Party URI to be used for connecting to all services.
        //! Will be DEPRECATED in future
        LICENSING_EXPORT static Utf8String GetLegacyRelyingPartyUri();

        //! Get URL for WebView sing-in
        LICENSING_EXPORT static Utf8String GetFederatedSignInUrl(ApplicationInfoCR info, Utf8StringCR windowsDomainName = nullptr);

        //! Get Active To Passive Url - craft an url that does passive ims authentication and redirects to the specified url afterwards
        //! @param url - a url to which to redirect after authentication
        //! @param token - a valid saml token that will be registered for authentication
        //! @param tokenGuid - random guid that will be used for token registration and url composition
        LICENSING_EXPORT Utf8String GetA2PUrl(Utf8String url, SamlTokenPtr token, Utf8String tokenGuid);

        //! Set Active To Passive token registration service API key
        //! @param user - user for the API key
        //! @param apiKey - api key
        LICENSING_EXPORT Utf8StringCR SetImsActiveSTSHelperServiceAuthKey(Utf8StringCR user, Utf8StringCR apiKey);
    };

END_BENTLEY_LICENSING_NAMESPACE
