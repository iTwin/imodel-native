/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/Connect/ImsClient.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

// TODO: refactor to non-static class ImsClient
//__PUBLISH_SECTION_START__

#include <DgnClientFx/DgnClientApp.h>
#include <WebServices/Connect/IImsClient.h>
#include <WebServices/Connect/SamlToken.h>
#include <WebServices/Client/ClientInfo.h>
#include <BeHttp/Credentials.h>
#include <BeHttp/HttpClient.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

typedef AsyncResult<SamlTokenPtr, HttpError> SamlTokenResult;

/*--------------------------------------------------------------------------------------+
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
typedef std::shared_ptr<struct ImsClient> ImsClientPtr;
struct ImsClient : IImsClient
    {
    private:
        ClientInfoPtr m_clientInfo;
        IHttpHandlerPtr m_httpHandler;

    private:
        ImsClient(ClientInfoPtr clientInfo, IHttpHandlerPtr httpHandler);

        AsyncTaskPtr<SamlTokenResult> RequestToken
            (
            Utf8StringCR authorization,
            JsonValueR params,
            Utf8StringCR rpUri,
            Utf8StringCR stsUrl,
            uint64_t lifetime
            );

        static Utf8String GetClientRelyingPartyUriForWtrealm(ClientInfoCR clientInfo);

    public:
        // TODO: static API for compatibility with old static code (ConnectSpaces, ConnectSetup, etc.)
        //! Will be DEPRECATED in future.
        WSCLIENT_EXPORT static void InitializeShared(ClientInfoPtr info, IHttpHandlerPtr httpHandler = nullptr);
        WSCLIENT_EXPORT static void UnitializeShared();
        //! Will be DEPRECATED in future.
        WSCLIENT_EXPORT static ImsClientPtr GetShared();

        //! Create new client
        //! @param info - client applicaiton info, see ClientInfo documentation for more info
        //! @param httpHandler - custom httpHandler to route requests trough
        WSCLIENT_EXPORT static ImsClientPtr Create(ClientInfoPtr info, IHttpHandlerPtr httpHandler = nullptr);

        //! Get security token using credentials
        //! @param creds - credentials to use for token
        //! @param rpUri - relying party URI the token will be used for. Defaults to application RP.
        //! @param lifetime - request specific token lifetime in minutes. Zero defaults to lifetime defined by service
        WSCLIENT_EXPORT AsyncTaskPtr<SamlTokenResult> RequestToken(CredentialsCR creds, Utf8String rpUri = nullptr, uint64_t lifetime = 0) override;

        //! Get security token using other token. Can be used to renew same token or get delegation token for different RP
        //! @param parent - parent.
        //! @param rpUri - relying party URI the token will be used for. Defaults to application RP.
        //! @param lifetime - request specific token lifetime in minutes. Zero defaults to lifetime defined by service
        WSCLIENT_EXPORT AsyncTaskPtr<SamlTokenResult> RequestToken(SamlTokenCR parent, Utf8String rpUri = nullptr, uint64_t lifetime = 0) override;

        //! Checks if given response is IMS Login redirect that should be treated as invalid credentials.
        //! This is workaround because IMS does not give any other indication.
        WSCLIENT_EXPORT static bool IsLoginRedirect(Http::ResponseCR response);

        //! Get Relying Party for given client info
        WSCLIENT_EXPORT static Utf8String GetRelyingPartyUri(ClientInfoCR info);

        //! Get legacy Relying Party URI to be used for connecting to all services.
        //! Will be DEPRECATED in future
        WSCLIENT_EXPORT static Utf8String GetLegacyRelyingPartyUri();

        //! Get URL for WebView sing-in
        WSCLIENT_EXPORT static Utf8String GetFederatedSignInUrl(ClientInfoCR info, Utf8StringCR windowsDomainName = nullptr);
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
