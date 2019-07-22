/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <Bentley/Bentley.h>
#include <Bentley/WString.h>
#include <Bentley/RefCounted.h>
#include <Bentley/Tasks/AsyncError.h>
#include <WebServices/iModelHub/Client/Client.h>
#include <WebServices/iModelHub/Client/Error.h>
#include <BeHttp/Credentials.h>
#include <BeHttp/HttpRequest.h>
#include <WebServices/Configuration/UrlProvider.h>
#include <WebServices/Connect/ConnectSignInManager.h>

BEGIN_BENTLEY_IMODELHUB_NAMESPACE

/*--------------------------------------------------------------------------------------+
* Helps the caller with tasks such as signing into the iModel Hub Services via Connect
* and looking up a project ID from a project number.
* @bsiclass                                                     Sam.Wilson      03/17
+---------------+---------------+---------------+---------------+---------------+------*/
struct ClientHelper
{
private:
    IJsonLocalState* m_localState;
    ClientInfoPtr m_clientInfo;
    IConnectSignInManagerPtr m_signinMgr;
    static ClientHelper* s_instance;
    static BeMutex s_mutex;
    IHttpHandlerPtr m_customHandler;
    Utf8String m_url;

    static Utf8CP str_BentleyConnectMain() {return "BentleyCONNECT--Main";}

    //! Construct the helper
    ClientHelper(ClientInfoPtr clientInfo, IJsonLocalState* ls = nullptr, IHttpHandlerPtr customHandler = nullptr)
        : m_clientInfo(clientInfo), m_localState(ls), m_customHandler(customHandler) {}

    //! Get the Url of iModelHub
    Utf8StringCR GetUrl();
public:
    //! Create or update the singleton instance.
    //! @param clientInfo       Client information.
    //! @param ls               Local storage.
    //! @param customHandler    Custom HTTP handler.
    //! @return the singleton instance.
    IMODELHUBCLIENT_EXPORT static ClientHelper* Initialize(ClientInfoPtr clientInfo, IJsonLocalState* ls = nullptr, IHttpHandlerPtr customHandler = nullptr);

    //! Get the singleton instance
    IMODELHUBCLIENT_EXPORT static ClientHelper* GetInstance();

    //! Sign in with credentials
    //! @param errorOut     Optional. If not null, an explanation of signin failure is returned here.
    //! @param credentials  User credentials.
    //! @return a connected client if signin succeeds
    IMODELHUBCLIENT_EXPORT iModel::Hub::ClientPtr SignInWithCredentials(AsyncError* errorOut, Credentials credentials);

    //! Sign in with Saml token
    //! @param errorOut     Optional. If not null, an explanation of signin failure is returned here.
    //! @param token        User token.
    //! @return a connected client if signin succeeds
    IMODELHUBCLIENT_EXPORT iModel::Hub::ClientPtr SignInWithToken(AsyncError* errorOut, SamlTokenPtr token);

    //! Sign in with IConnectSignInManager
    //! @param signInManagerPtr Sign-in manager
    //! @return a connected client if signin succeeds
    IMODELHUBCLIENT_EXPORT iModel::Hub::ClientPtr SignInWithManager(IConnectSignInManagerPtr signInManagerPtr);

    //! Sign in with IConnectSignInManager
    //! @param signInManagerPtr Sign-in manager
    //! @param environment      Environment
    //! @return a connected client if signin succeeds
    IMODELHUBCLIENT_EXPORT iModel::Hub::ClientPtr SignInWithManager(IConnectSignInManagerPtr signInManagerPtr, WebServices::UrlProvider::Environment environment);

    //! Look up a BCS project ID from a BCS project number
    //! @param wserrorOut     Optional. If not null, then details about a query failure are returned here if the failure is due to a communications error or some 
    //! server failure. No details would be returned if the lookup failed simply because the project number was not found.
    //! @param bcsProjectNumber  The project number to look up
    //! @param wsgBentleyConnectRepository The WSG Bentley Connect repository to query.
    //! @return the project ID if the lookup succeeded or the empty string if not
    IMODELHUBCLIENT_EXPORT Utf8String QueryProjectId(WSError* wserrorOut, Utf8StringCR bcsProjectNumber, Utf8CP wsgBentleyConnectRepository = str_BentleyConnectMain());

    //! Get user information stored in identity token
    IMODELHUBCLIENT_EXPORT ConnectSignInManager::UserInfo GetUserInfo() { return m_signinMgr->GetUserInfo(); }

    //! Create a client that always uses the same authorizationHeader
    IMODELHUBCLIENT_EXPORT iModel::Hub::ClientPtr SignInWithStaticHeader(Utf8StringCR authorizationHeader);

    //! Set a custom iModelHub Url
    void SetUrl(Utf8StringCR url) { m_url = url; }

    //! Create client from IConnectTokenProvider
    //! @param tokenProviderPtr Token provider
    //! @param prefix Token prefix that should be used for authentication
    //! @return a connected client
    IMODELHUBCLIENT_EXPORT static iModel::Hub::ClientPtr CreateClient(IConnectTokenProviderPtr tokenProviderPtr, IConnectAuthenticationProvider::HeaderPrefix prefix = IConnectAuthenticationProvider::HeaderPrefix::Bearer);
};

END_BENTLEY_IMODELHUB_NAMESPACE
