/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/iModelHub/Client/ClientHelper.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
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
* and looking up a project ID from a project name.
* @bsiclass                                                     Sam.Wilson      03/17
+---------------+---------------+---------------+---------------+---------------+------*/
struct ClientHelper
{
private:
    IJsonLocalState* m_localState;
    ClientInfoPtr m_clientInfo;
    ConnectSignInManagerPtr m_signinMgr;
    static ClientHelper* s_instance;
    static BeMutex s_mutex;

    static Utf8CP str_BentleyConnectGlobal() {return "BentleyCONNECT.Global--CONNECT.GLOBAL";}

    //! Construct the helper
    ClientHelper(ClientInfoPtr clientInfo, IJsonLocalState* ls = nullptr) : m_clientInfo(clientInfo), m_localState(ls) {}
public:
    //! Create or update the singleton instance.
    //! @param clientInfo   Client information.
    //! @param ls           Local storage.
    //! @return the singleton instance.
    IMODELHUBCLIENT_EXPORT static ClientHelper* Initialize(ClientInfoPtr clientInfo, IJsonLocalState* ls = nullptr);

    //! Get the singleton instance
    IMODELHUBCLIENT_EXPORT static ClientHelper* GetInstance();

    //! Sign in with credentials
    //! @param errorOut     Optional. If not null, an explanation of signin failure is returned here.
    //! @param credentials  User credentials.
    //! @return a connected client if signin succeeds
    IMODELHUBCLIENT_EXPORT iModel::Hub::ClientPtr SignInWithCredentials(AsyncError* errorOut, Credentials credentials);

    //! Sign in with token
    //! @param errorOut     Optional. If not null, an explanation of signin failure is returned here.
    //! @param token        User token.
    //! @return a connected client if signin succeeds
    IMODELHUBCLIENT_EXPORT iModel::Hub::ClientPtr SignInWithToken(AsyncError* errorOut, SamlTokenPtr token);

    //! Sign in with ConnectSignInManager
    //! @param signInManagerPtr Connect sign-in manager
    //! @return a connected client if signin succeeds
    IMODELHUBCLIENT_EXPORT iModel::Hub::ClientPtr SignInWithManager(ConnectSignInManagerPtr signInManagerPtr);

    //! Sign in with ConnectSignInManager
    //! @param signInManagerPtr Connect sign-in manager
    //! @param environment      Environment
    //! @return a connected client if signin succeeds
    IMODELHUBCLIENT_EXPORT iModel::Hub::ClientPtr SignInWithManager(ConnectSignInManagerPtr signInManagerPtr, WebServices::UrlProvider::Environment environment);

    //! Look up a BCS project ID from a BCS project name
    //! @param wserrorOut     Optional. If not null, then details about a query failure are returned here if the failure is due to a communications error or some 
    //! server failure. No details would be returned if the lookup failed simply because the project name was not found.
    //! @param bcsProjectName  The project name to look up
    //! @param wsgBentleyConnectRepository The WSG Bentley Connect repository to query.
    //! @return the project ID if the lookup succeeded or the empty string if not
    IMODELHUBCLIENT_EXPORT Utf8String QueryProjectId(WSError* wserrorOut, Utf8StringCR bcsProjectName, Utf8CP wsgBentleyConnectRepository = str_BentleyConnectGlobal());

    //! Get user information stored in identity token
    IMODELHUBCLIENT_EXPORT ConnectSignInManager::UserInfo GetUserInfo() { return m_signinMgr->GetUserInfo(); }
};

END_BENTLEY_IMODELHUB_NAMESPACE
